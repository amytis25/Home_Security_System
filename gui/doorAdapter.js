// doorAdapter.js
// link to Central Door System

const DoorState = {
    CLOSED: 'CLOSED',
    LOCKED: 'LOCKED',
    UNLOCKED: 'UNLOCKED',
    OPEN: 'OPEN',
    UNKNOWN: 'UNKNOWN'
};

// Internal simulated doors (id starts at 1)
const doors = [
    { id: 1, state: DoorState.UNLOCKED, distance: 0, position: 0 },
    { id: 2, state: DoorState.UNLOCKED, distance: 0, position: 0 }
];

export function initializeDoorSystem() {
// In the real system this would initialize sensors and motors.
    
}


// Get status: may randomly flip to OPEN to simulate someone opening the door
export async function getDoorStatus(id) {
   
}

export async function lockDoor(id) {

}

export async function unlockDoor(id) {

}

export { DoorState };
