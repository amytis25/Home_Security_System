# Embedded Home Security System
## Team 

Amytis Saghafi (asa414@sfu.ca), Melissa Mosavi (mmosavi@sfu.ca), Richard Kim (rmk7@sfu.ca), Simon Schaufele (ses16@sfu.ca)

A distributed embedded Linux security system for multi-door monitoring and control, built on BeagleY-AI boards with real-time sensor integration and centralized command handling.

## Overview

This project implements a complete home security solution consisting of:

- **Three door modules** equipped with ultrasonic sensors and stepper motor locks
- **One central hub** for aggregating status, processing commands, and routing alerts
- **Network communication** via UDP for low-latency inter-node messaging
- **User interface** with web dashboard for real-time monitoring and lock control
- **Alert system** for immediate notification of security events

Each door module runs on a dedicated BeagleY-AI board and communicates with a central control unit, which coordinates operations and forwards updates to end users. The system implements a command-response protocol over UDP, ensuring reliable distributed operation.

## System Architecture

### Distributed Node Architecture

```
┌─────────────────────────┐
│   Central Hub (D0)      │
│  - BeagleY-AI Board     │
│  - Command orchestration│
│  - Alert management     │
│  - Web server (port 8080)
└──────────┬──────────────┘
           │ UDP (port 12345)
    ┌──────┼──────┬──────────┐
    │      │      │          │
    ▼      ▼      ▼          ▼
   D1     D2     D3      (Extensible)
 (Door)  (Door) (Door)
```

### Door Module Hardware

Each door module includes:
- **HC-SR04 Ultrasonic Sensor** – detects door open/closed state
- **Stepper Motor** – controls electromagnetic lock
- **Status LEDs** – visual feedback (red: locked, green: unlocked)
- **BeagleY-AI Board** – onboard processor and networking

### Central Hub Hardware

- **BeagleY-AI Board** – command dispatcher
- **GPIO Controllers** – status monitoring and alerting
- **Ethernet + WiFi** – multi-path connectivity
- **Web Server** – real-time dashboard

### Communication Protocol

**UDP-based command format:**
```
<MODULE> COMMAND <CMDID> <TARGET> <ACTION>
```

Examples:
- `D1 COMMAND 42 D0 LOCK` – lock door D1
- `D2 COMMAND 43 D0 STATUS` – query D2 status
- `D1 FEEDBACK 42 D0 LOCKED` – confirmation response

Each module maintains state for two channels:
- **D0 (Sensor)** – OPEN/CLOSED
- **D1 (Lock)** – LOCKED/UNLOCKED

## Team Contributions

This was a team project developed by **Amytis Saghafi (AS)**, **Melissa Mosavi (MM)**, **Richard Kim (RK)**, and **Simon Schaufele (SS)**.

### Core Modules

| File | Component | Host/Target | Completeness | Language | Authors | Description |
|------|-----------|------------|--------------|----------|---------|-------------|
| `GPIO.c` / `GPIO.h` | GPIO Access | T(D) | ★★★★★ | C | AS, MM | Accesses and configures GPIO pins for sensor/actuator control |
| `HC-SR04.c` / `HC-SR04.h` | Ultrasonic Sensor | T(D) | ★★★★★ | C | AS, MM | Reads and processes HC-SR04 ultrasonic sensor data |
| `timing.c` / `timing.h` | Timing Functions | T(H/D) | ★★★★☆ | C | AS, MM | Provides delay functions and time management utilities |
| `UI.html` | Web Dashboard | H | ★★★★★ | HTML | AS, MM | Interactive web UI for controlling and monitoring door modules |
| `web_control.js` | UI Logic | H | ★★★★☆ | JS | AS, MM | Client-side UI functions and real-time status updates |
| `doorMod.c` / `doorMod.h` | Door Module | T(D) | ★★★★★ | C | AS, MM | Main door module controller (sensor polling, lock control, command handling) |
| `door_system.c` | Hub System | T(H) | ★★★★☆ | C | MM, SS | Central hub hosting door control system and command orchestration |
| `discord_alert.c` / `discord_alert.h` | Discord Alerts | O | ★★★★☆ | C | SS, RK | Sends security alerts to Discord webhook |
| `door_udp.c` / `hub_udp.c` | UDP Infrastructure | T(H/D)/H | ★★★★☆ | C | SS, MM | UDP communication layer for hub-door and web-hub interaction |
| `LED.c` / `LED.h` | LED Control | T(H/D) | ★★★☆☆ | C | MM, AS | PWM-based LED control for status indication (incomplete for hub) |
| `StepperMotor.c` / `StepperMotor.h` | Stepper Motor | T(D) | ★★★★★ | C | MM, AS | Stepper motor initialization and control (rotation by degrees) |
| `server.js` | Web Server | H | ★★★★★ | JS | AS, SS, MM, RK | Main HTTP/WebSocket server (adapted from AS3 reference code) |

### Helper Functions & Integration

| File | Helper For | Host/Target | Completeness | Language | Authors | Description |
|------|------------|-------------|--------------|----------|---------|-------------|
| `doorMod_cli.c` | doorMod | T(D) | ★★★★★ | C | MM | Executable wrapper for door module testing and CLI control |
| `door_udp_handler.c` | doorMod | T(D) | ★★★★☆ | C | MM | UDP handler ensuring UI never directly interacts with HAL |
| `http_api.c` | HTTP API | H | ★★★★★ | C | MM | HTTP bridge linking door_system to web UI interface |
| `led_worker.c` | LED Control | T(H/D) | ★★★☆☆ | C | MM, AS | Thread-safe LED execution across system stack |
| `system_webhook.c` | Discord Alerts | H | ★★★★★ | C | MM | System-level webhook integration for alert dispatch |
| `door_server.js` | UDP/Web Bridge | H | ★★★★★ | JS | MM | Bridges UDP hub communication to web client WebSockets |

### Individual Contributions

**Amytis Saghafi (AS):**
- Designed and 3D-printed door module enclosures
- Integrated sensors and actuators while maintaining signal integrity
- Worked on GPIO access layer and stepper motor control
- Contributed to embedded firmware and web UI
- Physical assembly and connector management

**Melissa Mosavi (MM):**
- Designed door module firmware architecture and command protocol
- Implemented UDP communication infrastructure (hub and door modules)
- Provided miniature demo doors for testing and validation
- Developed HTTP API and system webhook integration
- Coordinated overall system integration and testing

**Richard Kim (RK):**
- Implemented Discord alert system with webhook integration
- Contributed to server architecture and web integration
- Worked on cross-module communication

**Simon Schaufele (SS):**
- Designed UDP protocol and hub system architecture
- Implemented door_system hub controller
- Contributed to Discord alert integration
- Worked on server infrastructure

## Tools & Technologies

- **Languages:** C (embedded), JavaScript (web UI), Python (testing)
- **Operating System:** Embedded Linux (BeagleOS on BeagleY-AI)
- **Platforms:** BeagleY-AI (ARM Cortex-A53, 2 GB RAM)
- **Hardware Interfaces:** GPIO, PWM, UDP networking
- **Sensors & Actuators:** HC-SR04 ultrasonic, NEMA stepper motors, RGB LEDs
- **Build System:** CMake, cross-GCC (aarch64-linux-gnu)
- **Web Stack:** Node.js, Socket.io, JavaScript
- **Protocols:** UDP (custom command protocol), HTTP/WebSocket

## Repository Structure

```
ENSC351-project/
├── app/                    # Door module firmware
│   ├── include/
│   ├── src/               # Core door logic, UDP handling, CLI
│   └── CMakeLists.txt
├── hal/                   # Hardware abstraction layer
│   ├── include/           # GPIO, PWM, sensor, motor interfaces
│   └── src/               # Platform-specific implementations
├── gui/                   # Web dashboard and Node.js server
│   ├── server.js          # Main HTTP/WebSocket server
│   ├── web_control.js     # Client-side UI logic
│   ├── UI.html
│   └── lib/
│       ├── door_server.js # UDP bridge (hub ↔ web clients)
│       └── beatbox_server.js
├── scripts/               # Testing and deployment utilities
├── systemd/               # Service files for auto-start
├── CMakeLists.txt         # Top-level build configuration
├── buildHost.sh           # Build script for host compilation
├── buildTarget.sh         # Build script for cross-compilation
├── REFERENCE.md           # Detailed technical notes & setup procedures
└── readme.MD              # This file
```

For detailed technical setup, GPIO pin mappings, network configuration, and testing procedures, see [REFERENCE.md](./REFERENCE.md).

## Coursework Context

This project was developed as part of **ENSC 351: Real-Time Operating Systems**, an upper-division embedded systems course at Simon Fraser University. The assignment focused on distributed embedded systems design, hardware-software integration, and network communication protocols.

- Assignment specifications and starter code are intentionally excluded from this repository
- The project demonstrates practical application of RTOS concepts, real-time I/O, and network protocol design
- All code is original work by the team members

## Building the Project

### Build for Host (Development)
```bash
./buildHost.sh
```

### Build for Target (BeagleY-AI, Cross-Compiled)
```bash
./buildTarget.sh
```

For detailed CMake options and manual build procedures, see [REFERENCE.md](./REFERENCE.md#cmake-build-system).

## Running the System

### Deploy Hub Executable
```bash
scp build/app/door_system root@192.168.8.108:~/
```

### Deploy Door Module Executables
```bash
scp build/app/doorMod_cli root@<DOOR_IP>:~/
```

### Launch Web Dashboard
The web server runs on the hub at `http://<hub-ip>:8080`. The dashboard provides real-time status display and lock control.

For detailed deployment, network setup, and operational commands, see [REFERENCE.md](./REFERENCE.md#deployment-and-operation).

## Key Features

- **Real-Time Monitoring** – sub-second door state updates via UDP
- **Centralized Command Dispatch** – single hub receives all commands and coordinates responses
- **Multi-Module Scalability** – extensible to additional door modules without protocol changes
- **Robust Error Handling** – timeout detection, command retries, and state validation
- **Web Dashboard** – live status and lock control with multi-tab synchronization
- **Alert Integration** – Discord webhook for immediate security notifications

## Troubleshooting & Testing

Common issues and diagnostic procedures are documented in [REFERENCE.md](./REFERENCE.md#troubleshooting). This includes:

- Network connectivity verification (UDP port testing with `netcat`)
- Sensor calibration and timing validation
- Cross-compilation environment setup
- Door module bring-up procedures

## License

This is academic coursework. Use, modification, and distribution are subject to institutional policies.


---

**Questions or technical inquiries?** See [REFERENCE.md](./REFERENCE.md) for detailed implementation notes.



