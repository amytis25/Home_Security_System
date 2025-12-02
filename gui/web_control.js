// gui/web_control.js
// Web-based control panel for Central Door System
// Communicates with server.js via WebSocket

const socket = io();

async function getDoorInfo(moduleId) {
    return new Promise((resolve) => {
        socket.emit('get-door-info', moduleId, (info) => {
            resolve(info);
        });
    });
}

async function lockDoor(moduleId) {
    return new Promise((resolve) => {
        socket.emit('lock-door', moduleId, (result) => {
            resolve(result);
        });
    });
}

async function unlockDoor(moduleId) {
    return new Promise((resolve) => {
        socket.emit('unlock-door', moduleId, (result) => {
            resolve(result);
        });
    });
}

async function refreshStatus() {
    for (let id = 1; id <= 3; id++) {
        const info = await getDoorInfo(id);
        const span = document.getElementById(`door${id}_status`);
        const btn = document.getElementById(`toggle_door${id}`);
        const div = document.getElementById(`door${id}`);
        const msg = document.getElementById(`door${id}_message`);

        if (!span || !btn || !div) continue;

        msg && (msg.textContent = '');

        if (!info) {
            span.textContent = 'disconnected';
            btn.textContent = 'Unavailable';
            btn.disabled = true;
            continue;
        }

        // Show combined human-friendly status and lock state
        const open = info.frontDoorOpen;
        const locked = info.frontLockLocked;
        span.textContent = `${open ? 'open' : 'closed'} / ${locked ? 'locked' : 'unlocked'}`;

        if (open) {
            btn.textContent = 'Cannot lock open door';
            btn.disabled = true;
        } else if (locked) {
            btn.textContent = 'Unlock Door';
            btn.disabled = false;
        } else {
            btn.textContent = 'Lock Door';
            btn.disabled = false;
        }
    }
}

async function toggleDoor(id) {
    const btn = document.getElementById(`toggle_door${id}`);
    const msg = document.getElementById(`door${id}_message`);
    if (!btn) return;

    const info = await getDoorInfo(id);
    if (!info) {
        msg && (msg.textContent = 'No connection to door');
        return;
    }

    try {
        if (info.frontDoorOpen) {
            msg && (msg.textContent = 'Door is open; cannot change lock');
            return;
        }
        if (info.frontLockLocked) {
            await unlockDoor(id);
        } else {
            await lockDoor(id);
        }
    } catch (e) {
        msg && (msg.textContent = `Error: ${e.message}`);
    }
    await refreshStatus();
}

document.addEventListener('DOMContentLoaded', async () => {
    // Wire up buttons for doors 1..3
    for (let id = 1; id <= 3; id++) {
        const btn = document.getElementById(`toggle_door${id}`);
        if (!btn) continue;
        btn.addEventListener('click', async () => {
            await toggleDoor(id);
        });
    }

    // Initial refresh and periodic updates
    await refreshStatus();
    setInterval(refreshStatus, 1000);
});