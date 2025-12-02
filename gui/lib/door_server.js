"use strict";
/*
 * UDP-based door server bridge for web UI.
 *
 * - Sends COMMAND datagrams to hub at 192.168.8.108:12345
 * - Listens for incoming datagrams from hub and forwards to web clients
 * - Emits FEEDBACK messages for matching command ids back to requestor
 */

const dgram = require('dgram');
const socketio = require('socket.io');

const HUB_HOST = process.env.HUB_HOST || '192.168.8.108';
const HUB_PORT = parseInt(process.env.HUB_PORT || '12345', 10);
const UDP_FAMILY = 'udp4';

let io = null;
let udpSocket = null; // single socket for sending/receiving
let cmdCounter = 1; // simple increasing command id

// pending map: cmdid -> { socket, timer }
const pending = new Map();
const PENDING_TIMEOUT_MS = 1500; // how long to wait for FEEDBACK before giving up

function startUdpSocket() {
    if (udpSocket) return;
    udpSocket = dgram.createSocket(UDP_FAMILY);

    udpSocket.on('error', (err) => {
        console.error('UDP socket error:', err);
    });

    udpSocket.on('listening', () => {
        const address = udpSocket.address();
        console.log(`UDP socket listening ${address.address}:${address.port}`);
    });

    // Handle any incoming datagram from the hub
    udpSocket.on('message', (msgBuf, rinfo) => {
        const msg = msgBuf.toString('utf8').trim();
        console.log(`UDP rx from ${rinfo.address}:${rinfo.port} -> ${msg}`);

        // Parse basic messages and route FEEDBACK to pending requester
        // Tokenize by whitespace
        const parts = msg.split(/\s+/);
        if (parts.length >= 2) {
            const moduleId = parts[0];
            const type = parts[1];

            if (type === 'FEEDBACK' && parts.length >= 5) {
                // Format: <MODULE> FEEDBACK <CMDID> <TARGET> <ACTION>\n

                const cmdid = parseInt(parts[2], 10);
                const target = parts[3];
                const action = parts.slice(4).join(' ');

                // If a pending request exists, send the FEEDBACK only to that socket
                const p = pending.get(cmdid);
                if (p) {
                    try {
                        p.socket.emit('command-feedback', { module: moduleId, cmdid, target, action, raw: msg });
                    } catch (e) {
                        console.error('Error emitting feedback to socket:', e);
                    }
                    clearTimeout(p.timer);
                    pending.delete(cmdid);
                    return; // consumed
                }
            }

            // Broadcast other messages to all web clients
            if (io) {
                // Provide structured event where possible
                if (type === 'EVENT' && parts.length >= 4) {
                    // e.g. D1 EVENT D0 DOOR OPEN
                    const target = parts[2];
                    const event = parts.slice(3).join(' ');
                    io.sockets.emit('hub-event', { module: moduleId, type: 'EVENT', target, event, raw: msg });
                } else if (type === 'HEARTBEAT') {
                    io.sockets.emit('hub-heartbeat', { module: moduleId, raw: msg });
                } else if (type === 'HELLO') {
                    io.sockets.emit('hub-hello', { module: moduleId, raw: msg });
                } else {
                    io.sockets.emit('hub-raw', { raw: msg });
                }
            }
        } else {
            // Unstructured message; broadcast raw
            if (io) io.sockets.emit('hub-raw', { raw: msg });
        }
    });

    // Bind to an ephemeral port so hub can send FEEDBACK back to us
    udpSocket.bind(0, '0.0.0.0');
}

// Send a COMMAND to the hub for a specific module/target/action.
// `socket` is optional: if provided, the FEEDBACK will be emitted back to that socket.
function sendCommand(moduleId, target, action, socket) {
    if (!udpSocket) startUdpSocket();
    if (!moduleId) moduleId = 'D1';
    if (!target) target = 'D0';
    if (!action) action = '';

    const cmdid = cmdCounter++;
    // Format per project: <MODULE> COMMAND <CMDID> <TARGET> <ACTION>\n

    const payload = `${moduleId} COMMAND ${cmdid} ${target} ${action}\n`;
    const buf = Buffer.from(payload, 'utf8');

    // Send to hub, which will forward to the module
    udpSocket.send(buf, 0, buf.length, HUB_PORT, HUB_HOST, (err, bytes) => {
        if (err) {
            console.error('UDP send error:', err);
            if (socket) socket.emit('command-error', { module: moduleId, cmdid, error: String(err) });
            return;
        }
        console.log(`Sent COMMAND to ${HUB_HOST}:${HUB_PORT} -> ${payload.trim()}`);

        // Register pending feedback if caller provided a socket
        if (socket) {
            const timer = setTimeout(() => {
                // timed out waiting for FEEDBACK
                pending.delete(cmdid);
                try { socket.emit('command-error', { module: moduleId, cmdid, error: 'No FEEDBACK from hub' }); } catch(e){}
            }, PENDING_TIMEOUT_MS);

            pending.set(cmdid, { socket, timer });
        }
    });
}

// Websocket handlers
function handleSocket(ws) {
    console.log('Client connected');

    ws.on('send-command', (payload) => {
        // payload: { module: 'D1', target: 'D0', action: 'LOCK' }
        const moduleId = payload.module || 'D1';
        const target = payload.target || 'D0';
        const action = payload.action || 'STATUS';
        console.log('Web->server send-command', payload);
        sendCommand(moduleId, target, action, ws);
    });

    // convenience: allow web client to request raw message send (not recommended)
    ws.on('raw-send', (raw) => {
        if (!raw) return;
        if (!udpSocket) startUdpSocket();
        const buf = Buffer.from(String(raw), 'utf8');
        udpSocket.send(buf, 0, buf.length, HUB_PORT, HUB_HOST, (err) => {
            if (err) ws.emit('command-error', { error: String(err) });
            else ws.emit('raw-sent', { raw });
        });
    });

    // Legacy handlers for web_control.js compatibility
    ws.on('get-door-info', (moduleId, callback) => {
        // Query door status: sends STATUS command and waits for FEEDBACK
        console.log(`Client requested door info for module ${moduleId}`);
        if (!udpSocket) startUdpSocket();
        
        // Ensure moduleId has 'D' prefix
        const mod = (String(moduleId).startsWith('D')) ? String(moduleId) : `D${moduleId}`;
        const cmdid = cmdCounter++;
        const payload = `${mod} COMMAND ${cmdid} D0 STATUS\n`;
        const buf = Buffer.from(payload, 'utf8');

        // Set up timeout for response
        const timer = setTimeout(() => {
            pending.delete(cmdid);
            callback(null); // timeout = no info
        }, PENDING_TIMEOUT_MS);

        // Register handler for FEEDBACK
        pending.set(cmdid, {
            socket: {
                emit: (eventType, data) => {
                    if (eventType === 'command-feedback') {
                        clearTimeout(timer);
                        pending.delete(cmdid);
                        // Parse FEEDBACK for STATUS response
                        // Format examples: 
                        //   "CLOSED,UNLOCKED" (door closed and unlocked)
                        //   "OPEN,LOCKED" (door open and locked)
                        // The hub sends back: D1 FEEDBACK cmdid D0 CLOSED,UNLOCKED
                        const statusStr = data.action || '';
                        const [doorState, lockState] = statusStr.split(',').map(s => s?.trim().toUpperCase());
                        
                        callback({
                            moduleId: data.module,
                            target: data.target,
                            status: statusStr,
                            frontDoorOpen: doorState === 'OPEN',
                            frontLockLocked: lockState === 'LOCKED',
                            success: true
                        });
                    }
                }
            },
            timer
        });

        udpSocket.send(buf, 0, buf.length, HUB_PORT, HUB_HOST, (err) => {
            if (err) {
                clearTimeout(timer);
                pending.delete(cmdid);
                callback(null);
            }
        });
    });

    ws.on('lock-door', (moduleId, callback) => {
        console.log(`Client requested lock for module ${moduleId}`);
        sendCommand(moduleId, 'D0', 'LOCK', {
            emit: (eventType, data) => {
                if (eventType === 'command-feedback') {
                    callback({ success: true, action: 'LOCK', module: moduleId });
                } else if (eventType === 'command-error') {
                    callback({ success: false, error: data.error });
                }
            }
        });
    });

    ws.on('unlock-door', (moduleId, callback) => {
        console.log(`Client requested unlock for module ${moduleId}`);
        sendCommand(moduleId, 'D0', 'UNLOCK', {
            emit: (eventType, data) => {
                if (eventType === 'command-feedback') {
                    callback({ success: true, action: 'UNLOCK', module: moduleId });
                } else if (eventType === 'command-error') {
                    callback({ success: false, error: data.error });
                }
            }
        });
    });

    ws.on('disconnect', () => {
        console.log('Client disconnected');
    });
}

// Public: start server
exports.listen = function(server) {
    io = socketio(server, {
        cors: { origin: "*" }
    });

    // Start UDP socket immediately so we can receive any hub messages
    startUdpSocket();

    io.on('connection', function(socket) {
        handleSocket(socket);
    });

    console.log(`Door server: forwarding commands to ${HUB_HOST}:${HUB_PORT}`);
};

// Export sendCommand for other server-side code to call directly
exports.sendCommand = sendCommand;