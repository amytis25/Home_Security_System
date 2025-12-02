// gui/web_control.js
// Web-based control panel for Central Door System
// Communicates with server.js via WebSocket to send commands and receive status updates via UDP
// Pattern derived from as3-server/lib/beatbox_server.js

const socket = io();

// Module cache to track door states
const doorStates = {
    'D1': { doorOpen: false, lockLocked: false, moduleId: 'D1' },
    'D2': { doorOpen: false, lockLocked: false, moduleId: 'D2' },
    'D3': { doorOpen: false, lockLocked: false, moduleId: 'D3' }
};

// Setup server message handlers (similar to beatbox_ui.js pattern)
function setupServerMessageHandlers() {
    // Listen for hub events (STATUS, DOOR OPEN, DOOR CLOSE, LOCK, UNLOCK, etc.)
    socket.on('hub-event', (data) => {
        console.log('Received hub-event:', data);
        const moduleId = data.module; // e.g., 'D1'
        const eventStr = data.event || ''; // e.g., 'D0 DOOR OPEN' or 'CLOSED,UNLOCKED'
        
        if (doorStates[moduleId]) {
            // Parse status events like "OPEN,LOCKED" or "CLOSED,UNLOCKED"
            if (eventStr.includes('CLOSED') || eventStr.includes('OPEN')) {
                const parts = eventStr.split(/[,\s]+/).map(s => s?.trim().toUpperCase());
                if (parts.includes('OPEN')) {
                    doorStates[moduleId].doorOpen = true;
                } else if (parts.includes('CLOSED')) {
                    doorStates[moduleId].doorOpen = false;
                }
                if (parts.includes('LOCKED')) {
                    doorStates[moduleId].lockLocked = true;
                } else if (parts.includes('UNLOCKED')) {
                    doorStates[moduleId].lockLocked = false;
                }
            }
            updateUIForModule(moduleId);
        }
    });

    // Listen for command feedback responses
    socket.on('command-feedback', (data) => {
        console.log('Received command-feedback:', data);
        const moduleId = data.module;
        const action = (data.action || '').toUpperCase();
        
        if (moduleId && doorStates[moduleId]) {
            // Parse the action field for status (e.g., "LOCKED", "CLOSED,UNLOCKED")
            const parts = action.split(/[,\s]+/).map(s => s?.trim().toUpperCase());
            if (parts.includes('OPEN')) {
                doorStates[moduleId].doorOpen = true;
            } else if (parts.includes('CLOSED')) {
                doorStates[moduleId].doorOpen = false;
            }
            if (parts.includes('LOCKED')) {
                doorStates[moduleId].lockLocked = true;
            } else if (parts.includes('UNLOCKED')) {
                doorStates[moduleId].lockLocked = false;
            }
            updateUIForModule(moduleId);
        }
    });

    // Listen for heartbeats to confirm hub is alive
    socket.on('hub-heartbeat', (data) => {
        console.log('Heartbeat from hub:', data.module);
    });

    // Listen for errors
    socket.on('command-error', (data) => {
        console.error('Command error:', data);
        const moduleNum = data.module ? data.module.replace('D', '') : 'unknown';
        const msg = document.getElementById(`door${moduleNum}_message`);
        if (msg) {
            msg.textContent = `Error: ${data.error || 'Unknown error'}`;
            setTimeout(() => { msg.textContent = ''; }, 3000);
        }
    });

    socket.on('hub-raw', (data) => {
        console.log('Raw hub message:', data.raw);
    });
}

// Update UI for a specific module based on cached state
function updateUIForModule(moduleId) {
    const num = moduleId.replace('D', '');
    const span = document.getElementById(`door${num}_status`);
    const btn = document.getElementById(`toggle_door${num}`);
    const state = doorStates[moduleId];

    if (!span || !btn || !state) return;

    // Display combined status: door state / lock state
    const doorStr = state.doorOpen ? 'open' : 'closed';
    const lockStr = state.lockLocked ? 'locked' : 'unlocked';
    span.textContent = `${doorStr} / ${lockStr}`;

    // Update button state and label
    if (state.doorOpen) {
        btn.textContent = 'Cannot lock open door';
        btn.disabled = true;
    } else if (state.lockLocked) {
        btn.textContent = 'Unlock Door';
        btn.disabled = false;
    } else {
        btn.textContent = 'Lock Door';
        btn.disabled = false;
    }
}

// Send command to door module via WebSocket -> UDP
function sendCommandToModule(moduleId, target, action) {
    console.log(`Sending command: ${moduleId} -> ${action}`);
    socket.emit('send-command', {
        module: moduleId,
        target: target || 'D0',
        action: action
    });
}

// Request current status for a module
function requestModuleStatus(moduleId) {
    console.log(`Requesting status for ${moduleId}`);
    socket.emit('send-command', {
        module: moduleId,
        target: 'D0',
        action: 'STATUS'
    });
}

// Toggle lock for a door
async function toggleDoor(id) {
    const moduleId = `D${id}`;
    const btn = document.getElementById(`toggle_door${id}`);
    const msg = document.getElementById(`door${id}_message`);
    const state = doorStates[moduleId];

    if (!btn || !state) return;

    try {
        if (state.doorOpen) {
            msg && (msg.textContent = 'Door is open; cannot change lock');
            setTimeout(() => { msg && (msg.textContent = ''); }, 2000);
            return;
        }

        msg && (msg.textContent = 'Sending command...');
        const action = state.lockLocked ? 'UNLOCK' : 'LOCK';
        sendCommandToModule(moduleId, 'D0', action);
        
        // Clear message after a delay
        setTimeout(() => { msg && (msg.textContent = ''); }, 2000);
    } catch (e) {
        msg && (msg.textContent = `Error: ${e.message}`);
        setTimeout(() => { msg && (msg.textContent = ''); }, 3000);
    }
}

document.addEventListener('DOMContentLoaded', () => {
    // Setup WebSocket handlers
    setupServerMessageHandlers();

    // Wire up buttons for doors 1..3
    for (let id = 1; id <= 3; id++) {
        const btn = document.getElementById(`toggle_door${id}`);
        if (!btn) continue;
        btn.addEventListener('click', () => {
            toggleDoor(id);
        });
    }

    // Request initial status for all doors
    requestModuleStatus('D1');
    requestModuleStatus('D2');
    requestModuleStatus('D3');

    // Periodically request status updates (every 2 seconds)
    // This pattern matches beatbox_ui.js periodic polling
    setInterval(() => {
        requestModuleStatus('D1');
        requestModuleStatus('D2');
        requestModuleStatus('D3');
    }, 2000);
});