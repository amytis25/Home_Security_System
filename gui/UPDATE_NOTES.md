# Web Control UI Updates - UDP Hub Integration

## Overview
Updated `web_control.js` and `UI.html` to follow the **as3-server pattern** for receiving status updates and sending commands to the hub UDP interface.

## Architecture Changes

### Previous Pattern (Request-Response)
The old code used async request-response where each status check was a separate awaited operation, blocking UI updates.

### New Pattern (Event-Based)
Following the as3-server/beatbox_server.js model:
1. **Emit commands** via WebSocket: `socket.emit('send-command', {...})`
2. **Listen for events** from hub:
   - `hub-event`: Status updates, door changes, lock changes
   - `command-feedback`: Response to specific commands
   - `command-error`: Error notifications

## Key Updates

### web_control.js

#### 1. **State Management**
```javascript
const doorStates = {
    'D1': { doorOpen: false, lockLocked: false, moduleId: 'D1' },
    'D2': { doorOpen: false, lockLocked: false, moduleId: 'D2' },
    'D3': { doorOpen: false, lockLocked: false, moduleId: 'D3' }
};
```
Maintains cached door states that are updated by incoming UDP events from the hub.

#### 2. **WebSocket Event Handlers** (setupServerMessageHandlers)
- **hub-event**: Parses status strings like "CLOSED,UNLOCKED" and updates door state
- **command-feedback**: Processes feedback from executed commands
- **command-error**: Handles error messages
- **hub-heartbeat**: Confirms hub connectivity

#### 3. **Command Sending**
```javascript
socket.emit('send-command', {
    module: moduleId,      // 'D1', 'D2', 'D3'
    target: 'D0',         // Target within module
    action: 'LOCK|UNLOCK|STATUS'
});
```
Sends structured commands matching the door_server.js protocol format:
```
<MODULE> COMMAND <CMDID> <TARGET> <ACTION>
```

#### 4. **Periodic Status Polling**
```javascript
setInterval(() => {
    requestModuleStatus('D1');
    requestModuleStatus('D2');
    requestModuleStatus('D3');
}, 2000);
```
Requests status every 2 seconds (matching beatbox_ui.js pattern).

### UI.html

#### Updates
- Added connection status indicator placeholder (`#connection-status`)
- Added debug info div (`#debug-info`)
- Changed button labels from "Toggle" to "--" (updated dynamically)
- Improved semantics with connection-info and debug CSS classes

## Command Protocol

### Status Request
```
send-command: { module: 'D1', target: 'D0', action: 'STATUS' }
→ Hub: D1 COMMAND <id> D0 STATUS
← Hub: D1 FEEDBACK <id> D0 CLOSED,UNLOCKED
```

### Lock Command
```
send-command: { module: 'D1', target: 'D0', action: 'LOCK' }
→ Hub: D1 COMMAND <id> D0 LOCK
← Hub: D1 FEEDBACK <id> D0 LOCKED
```

### Unlock Command
```
send-command: { module: 'D1', target: 'D0', action: 'UNLOCK' }
→ Hub: D1 COMMAND <id> D0 UNLOCK
← Hub: D1 FEEDBACK <id> D0 UNLOCKED
```

## Status Display Format
Doors display status as: **door_state / lock_state**
- Examples: "open / locked", "closed / unlocked"

## Error Handling
- Command timeouts show error messages in the UI (3-second duration)
- Open door prevents locking attempts (UI shows "Cannot lock open door")
- Hub connectivity issues trigger command-error events

## Integration Points

### With server.js (Main HTTP/WebSocket Server)
- Receives WebSocket connections
- Relays send-command events to door_server.js

### With door_server.js (UDP Hub Bridge)
- Receives 'send-command' events with { module, target, action }
- Sends UDP messages to HUB_HOST:HUB_PORT
- Parses incoming UDP messages and broadcasts to all clients as:
  - `hub-event`: EVENT type messages
  - `command-feedback`: FEEDBACK type messages
  - `command-error`: Connection/timeout errors

## Testing Checklist
- [ ] Status requests return door state
- [ ] Lock command succeeds on closed/unlocked door
- [ ] Unlock command succeeds on closed/locked door
- [ ] Cannot lock open door (UI prevents action)
- [ ] Periodic polling updates UI every 2 seconds
- [ ] Error messages appear and disappear
- [ ] Multiple door controls work independently
