"use strict";
/*
 * WebSocket server for door control system
 * Communicates with door-system API to get status and send commands
 */

const fs = require('fs');
const socketio = require('socket.io');
const path = require('path');
const mime = require('mime');
const http = require('http');

const PORT = 8080;

// HTTP server for serving static files
const httpServer = http.createServer(function(req, res) {
    let filePath = (req.url === '/') ? 'index.html' : req.url;
    const absPath = path.join(__dirname, filePath);
    
    fs.readFile(absPath, function(err, data) {
        if (err) {
            res.writeHead(404, {'Content-Type': 'text/plain'});
            res.end('Error 404: resource not found.');
            return;
        }
        res.writeHead(200, {'Content-Type': mime.getType(path.basename(absPath)) || 'application/octet-stream'});
        res.end(data);
    });
});

// WebSocket server
const io = socketio(httpServer, {
    cors: { origin: "*" }
});

httpServer.listen(PORT, function() {
    console.log('Server listening on port ' + PORT);
});

const DoorState = {
    CLOSED: 'CLOSED',
    LOCKED: 'LOCKED',
    UNLOCKED: 'UNLOCKED',
    OPEN: 'OPEN',
    UNKNOWN: 'UNKNOWN',
    DISCONNECTED: 'DISCONNECTED'
};

function normalizeStatusJson(json) {
    if (!json) return DoorState.DISCONNECTED;
    if (typeof json.state !== 'undefined') {
        switch (json.state) {
            case 0: return DoorState.LOCKED;
            case 1: return DoorState.UNLOCKED;
            case 2: return DoorState.OPEN;
            default: return DoorState.UNKNOWN;
        }
    }
    if (typeof json.d0_open !== 'undefined' && typeof json.d0_locked !== 'undefined') {
        if (json.d0_open) return DoorState.OPEN;
        if (json.d0_locked) return DoorState.LOCKED;
        return DoorState.UNLOCKED;
    }
    return DoorState.DISCONNECTED;
}

function formatModuleId(moduleId) {
    if (moduleId === null || moduleId === undefined) return '';
    const s = String(moduleId);
    if (/^[Dd]\d+$/.test(s)) return s.toUpperCase();
    if (/^\d+$/.test(s)) return `D${s}`;
    return s;
}

async function getDoorInfo(moduleId) {
    const mod = formatModuleId(moduleId);
    try {
        const res = await fetch(`http://localhost:9000/api/status?module=${encodeURIComponent(mod)}`);
        if (!res.ok) return null;
        const j = await res.json();
        return {
            state: normalizeStatusJson(j),
            raw: j,
            frontDoorOpen: !!j.front_door_open || !!j.d0_open,
            frontLockLocked: !!j.front_lock_locked || !!j.d1_locked
        };
    } catch (e) {
        console.error('Error fetching door info:', e);
        return null;
    }
}

async function sendCommand(moduleId, target, action) {
    const mod = formatModuleId(moduleId);
    const body = `module=${encodeURIComponent(mod)}&target=${encodeURIComponent(target||'')}&action=${encodeURIComponent(action)}`;
    try {
        const res = await fetch(`http://localhost:9000/api/command`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body
        });
        if (!res.ok) throw new Error(await res.text());
        return res.json();
    } catch (e) {
        console.error('Error sending command:', e);
        throw e;
    }
}

io.on('connection', function(socket) {
    console.log('WebSocket client connected:', socket.id);

    socket.on('get-door-info', async (moduleId, callback) => {
        console.log('get-door-info for module:', moduleId);
        const info = await getDoorInfo(moduleId);
        if (callback) callback(info);
    });

    socket.on('lock-door', async (moduleId, callback) => {
        console.log('lock-door for module:', moduleId);
        try {
            const result = await sendCommand(moduleId, 'D0', 'LOCK');
            if (callback) callback({ success: true, data: result });
        } catch (e) {
            if (callback) callback({ success: false, error: e.message });
        }
    });

    socket.on('unlock-door', async (moduleId, callback) => {
        console.log('unlock-door for module:', moduleId);
        try {
            const result = await sendCommand(moduleId, 'D0', 'UNLOCK');
            if (callback) callback({ success: true, data: result });
        } catch (e) {
            if (callback) callback({ success: false, error: e.message });
        }
    });

    socket.on('disconnect', function() {
        console.log('WebSocket client disconnected:', socket.id);
    });
});
