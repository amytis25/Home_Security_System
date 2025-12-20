# Embedded Home Security System – Technical Reference

This document contains detailed implementation notes, setup procedures, network configuration, GPIO pin mappings, and testing commands for developers and integrators.

## Table of Contents

1. [BeagleY-AI Pin Configuration](#beagley-ai-pin-configuration)
2. [CMake Build System](#cmake-build-system)
3. [Network Configuration](#network-configuration)
4. [UDP Protocol Details](#udp-protocol-details)
5. [Deployment and Operation](#deployment-and-operation)
6. [Alert System (Discord Webhook)](#alert-system-discord-webhook)
7. [Cross-Compilation](#cross-compilation)
8. [Testing and Diagnostics](#testing-and-diagnostics)
9. [Troubleshooting](#troubleshooting)

---

## BeagleY-AI Pin Configuration

### GPIO Assignments

#### Stepper Motor (Door Lock Actuation)
```
Pin        | GPIO   | Function
-----------|--------|----------
P8.35      | GPIO04 | Motor coil IN1
P8.33      | GPIO22 | Motor coil IN2
P8.36      | GPIO05 | Motor coil IN3
P8.31      | GPIO06 | Motor coil IN4
```

Stepper motor operation: Apply 4-phase sequence to IN1–IN4 to drive NEMA motor. Sequence:
```
Phase 1: IN1=1, IN2=0, IN3=0, IN4=0
Phase 2: IN1=0, IN2=1, IN3=0, IN4=0
Phase 3: IN1=0, IN2=0, IN3=1, IN4=0
Phase 4: IN1=0, IN2=0, IN3=0, IN4=1
```

#### Ultrasonic Sensor (HC-SR04)
```
Pin        | GPIO/Chip | Line | Function
-----------|-----------|------|----------
P9.11      | gpiochip1 | 33   | Trigger (TRIG)
P9.13      | gpiochip2 | 8    | Echo (ECHO)
```

Sensor operation:
- Send 10 μs pulse on TRIG
- Measure duration of ECHO pulse
- Distance (cm) = (pulse duration in μs) / 58
- Debounce: require 3 consecutive readings within 2 cm before accepting state change

#### Status LEDs (PWM-based)
```
Pin        | GPIO   | PWM Module | Channel | Color
-----------|--------|------------|---------|--------
P9.14      | GPIO15 | EPWM0      | A       | Red
P8.16      | GPIO12 | EPWM0      | B       | Green
```

LED states:
- **Red ON, Green OFF:** Door locked
- **Red OFF, Green ON:** Door unlocked
- **Both dimmed:** Transitional state

### Device Tree Overlay

To enable PWM channels, add this overlay to `/boot/firmware/extlinux/extlinux.conf`:

```
k3-am67a-beagley-ai-pwm-epwm0-gpio15-gpio12.dtbo
```

Edit file:
```bash
sudo nano /boot/firmware/extlinux/extlinux.conf
```

Add to kernel command line (before reboot):
```
overlays=k3-am67a-beagley-ai-pwm-epwm0-gpio15-gpio12.dtbo
```

Reboot to apply:
```bash
sudo reboot
```

---

## CMake Build System

### Automatic Build Scripts

**For host compilation (Linux/macOS):**
```bash
./buildHost.sh
```

This runs:
```bash
rm -rf build/
cmake -S . -B build
cmake --build build -j4
```

**For target compilation (ARM64 cross-compilation):**
```bash
./buildTarget.sh
```

This runs:
```bash
rm -rf build/
cmake -S . -B build \
    -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++
cmake --build build -j4
```

### Manual CMake Invocation

#### Host Build (Development)

```bash
rm -rf build/
cmake -S . -B build
cmake --build build
```

#### Target Build (Cross-Compile for BeagleY-AI)

**Option A: Inline compiler specification**
```bash
rm -rf build/
cmake -S . -B build \
    -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++
cmake --build build
```

**Option B: Environment variables**
```bash
rm -rf build/
CC=aarch64-linux-gnu-gcc CXX=aarch64-linux-gnu-g++ cmake -S . -B build
cmake --build build
```

### Executable Output

After build, binaries are located at:
- **Hub:** `build/app/door_system`
- **Door Modules:** `build/app/doorMod_cli`

---

## Network Configuration

### WiFi Setup (iwctl)

Reference: https://docs.beagleboard.org/boards/beagley/ai/02-quick-start.html

#### List Wireless Devices
```bash
iwctl device list
```
Expected output: `wlan0` (or similar)

#### Scan for Available Networks
```bash
iwctl station wlan0 scan
```

#### List Detected Networks
```bash
iwctl station wlan0 get-networks
```

#### Connect to Network
```bash
iwctl --passphrase "PASSWORD" station wlan0 connect "SSID"
```

Example:
```bash
iwctl --passphrase "defender351" station wlan0 connect "351network"
```

#### Verify Connection
```bash
iwctl station wlan0 show
```

#### Test Internet Connection
```bash
ping 8.8.8.8
```

#### Disconnect from Network
```bash
iwctl station wlan0 disconnect
```

### Network Topology (Project Configuration)

```
Device         | Module   | IP Address    | MAC Address
---------------|----------|---------------|---------------
Central Hub    | D0       | 192.168.8.108 | (ethernet)
Door Module 1  | D1       | 192.168.8.213 | (WiFi)
Door Module 2  | D2       | 192.168.8.238 | (WiFi)
Door Module 3  | D3       | 192.168.8.233 | (WiFi)
```

Hub uses dual connectivity:
- **Ethernet:** Internal network with door modules
- **WiFi (wlan0):** Connected to SFUNET-SECURE for Discord alerts

---

## UDP Protocol Details

### Port Allocation

```
Port    | Direction     | Purpose
--------|---------------|----------------------------------------
12345   | Bidirectional | Command/Feedback stream (all modules)
12346   | Inbound (hub) | Heartbeat/polling (door modules → hub)
```

### Message Format

#### Command (Client → Hub/Module)

```
<MODULE> COMMAND <CMDID> <TARGET> <ACTION>
```

Fields:
- `MODULE`: Sender identifier (D0, D1, D2, D3, or web-generated prefix)
- `COMMAND`: Message type keyword
- `CMDID`: Unique command ID (auto-incremented by dispatcher)
- `TARGET`: Destination channel (D0 for sensor, D1 for lock)
- `ACTION`: Command verb (STATUS, LOCK, UNLOCK) or response data

Examples:
```
D1 COMMAND 42 D0 STATUS
D0 COMMAND 43 D1 LOCK
web COMMAND 44 D2 UNLOCK
```

#### Feedback (Hub/Module → Client)

```
<MODULE> FEEDBACK <CMDID> <TARGET> <ACTION>
```

Fields: Same as Command, but `FEEDBACK` indicates response.

Examples:
```
D1 FEEDBACK 42 D0 CLOSED,UNLOCKED
D0 FEEDBACK 43 D1 LOCKED
D2 FEEDBACK 44 D1 UNLOCKED
```

### State Format

Door state responses use comma-separated tuples:

```
CLOSED,LOCKED
CLOSED,UNLOCKED
OPEN,LOCKED        (anomalous but possible during transition)
OPEN,UNLOCKED      (expected if door physically opened)
```

### Command Timeout

Default timeout: **5000 ms** (5 seconds)

Defined in `door_server.js`:
```javascript
const PENDING_TIMEOUT_MS = 5000;
```

If no FEEDBACK received within this window, client receives `command-error` event with error message "No FEEDBACK from hub".

---

## Deployment and Operation

### Compilation Artifacts

After running `buildTarget.sh`:

```
build/
├── app/
│   ├── door_system      ← Hub executable
│   └── doorMod_cli      ← Door module executable
└── (library .a files)
```

### Deployment to Hub

```bash
scp build/app/door_system root@192.168.8.108:~/
```

Then on hub:
```bash
cd ~
sudo ./door_system
```

### Deployment to Door Modules

For each door (D1, D2, D3):

```bash
scp build/app/doorMod_cli root@<DOOR_IP>:~/
```

Then on each door:
```bash
cd ~
sudo ./doorMod_cli D1    # or D2, D3 respectively
```

### Auto-Start on Boot (systemd)

**For hub** (`door_system.service`):
```bash
sudo cp systemd/door_system.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable door_system
sudo systemctl start door_system
```

**For door modules** (`door_module.service`, if available):
```bash
sudo cp systemd/door_module.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable door_module
sudo systemctl start door_module
```

### Web Dashboard

Start web server on hub:
```bash
cd gui/
npm install
node server.js
```

Access dashboard:
```
http://192.168.8.108:8080
```

---

## Alert System (Discord Webhook)

### Dependencies

Install libcurl (HTTP client library):

**On build host (if cross-compiling):**
```bash
sudo apt update
sudo apt install libcurl4-openssl-dev
```

**For ARM64 target:**
```bash
sudo dpkg --add-architecture arm64
sudo apt update
sudo apt install libcurl4-openssl-dev:arm64
```

### Compilation with Webhook Support

```bash
gcc app/src/discordTest.c \
    app/src/discord_alert.c \
    hal/src/system_webhook.c \
    hal/src/timing.c \
    -Iapp/include \
    -Ihal/include \
    -lcurl -lpthread \
    -o test_hub_webhook
```

### Testing Webhook Locally

**Terminal 1: Start mock webhook server**
```bash
python3 mockwebhook.py
```

This listens on `localhost:8080` and logs incoming POST requests.

**Terminal 2: Run test executable**
```bash
./test_hub_webhook
```

Expected output in Terminal 1:
```
Received webhook POST:
{
  "content": "Door security alert...",
  "timestamp": "..."
}
```

---

## Cross-Compilation

### Prerequisites

Install ARM64 cross-compiler:

```bash
sudo apt update
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

Verify installation:
```bash
aarch64-linux-gnu-gcc --version
```

### Compiler Specifications

- **Target triple:** `aarch64-linux-gnu`
- **Architecture:** ARM Cortex-A53 (ARMv8)
- **Default ABI:** LP64

### Environment Variables

Set compilers for one-shot compilation:
```bash
export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export AR=aarch64-linux-gnu-ar
export RANLIB=aarch64-linux-gnu-ranlib
```

Then run CMake normally:
```bash
cmake -S . -B build
cmake --build build
```

---

## Testing and Diagnostics

### UDP Communication Testing (netcat)

#### Test Hub UDP Port (12345)

Send command to hub:
```bash
echo "D1 COMMAND 1 D0 STATUS" | nc -u 192.168.8.108 12345
```

Listen on hub for incoming commands:
```bash
nc -u -l -p 12345
```

#### Test Door Module UDP Port (12345)

Send command to specific door:
```bash
echo "D1 COMMAND 2 D1 LOCK" | nc -u 192.168.8.213 12345
```

#### Test Heartbeat Port (12346)

Listen on hub for heartbeats:
```bash
nc -u -l -p 12346
```

### Stress Testing

Run UDP burst test:
```bash
bash scripts/udp_burst_test.sh
```

This sends rapid command sequences to all modules and monitors response times.

### GPIO State Inspection

Export GPIO and read state:
```bash
# Export GPIO
echo 4 > /sys/class/gpio/export

# Read current state
cat /sys/class/gpio/gpio4/value

# Unexport when done
echo 4 > /sys/class/gpio/unexport
```

### Sensor Calibration

To debug HC-SR04 readings:

1. Run door module in verbose mode (if implemented):
   ```bash
   sudo ./doorMod_cli D1 --verbose
   ```

2. Monitor distance readings:
   ```bash
   dmesg | grep "distance"
   ```

3. Physically open/close door and verify sensor triggers state change within 2 seconds.

### Network Diagnostics

Check connectivity:
```bash
ping 192.168.8.108
```

List active network interfaces:
```bash
ip link show
```

Display IP addresses:
```bash
ip addr show
```

Monitor UDP traffic:
```bash
sudo tcpdump -u -i eth0 udp port 12345
```

---

## Troubleshooting

### Door Module Not Responding to Commands

1. **Check connectivity:**
   ```bash
   ping 192.168.8.213  # Replace with actual door IP
   ```

2. **Verify module is running:**
   ```bash
   ps aux | grep doorMod_cli
   ```

3. **Check for UDP listening:**
   ```bash
   netstat -uln | grep 12345
   ```

4. **Test UDP directly:**
   ```bash
   echo "D1 COMMAND 1 D0 STATUS" | nc -u 192.168.8.213 12345
   ```

### Sensor Not Detecting Door State Changes

1. **Verify GPIO exports:**
   ```bash
   ls /sys/class/gpio/ | grep gpio
   ```

2. **Check sensor wiring:**
   - Verify TRIG and ECHO pins are connected to correct GPIOs
   - Confirm 5V power supply to sensor

3. **Test raw sensor readings:**
   ```bash
   # Trigger measurement
   echo 1 > /sys/class/gpio/gpio27/value
   usleep 10
   echo 0 > /sys/class/gpio/gpio27/value
   
   # Monitor echo pulse
   cat /sys/class/gpio/gpio17/value
   ```

4. **Adjust debounce threshold** in `hal/src/HC-SR04.c` if experiencing noise:
   ```c
   #define DEBOUNCE_THRESHOLD_CM 2  // Increase if unstable
   ```

### Web Dashboard Not Displaying Status

1. **Check Node.js server:**
   ```bash
   ps aux | grep "node server"
   ```

2. **Verify web server is listening:**
   ```bash
   netstat -tlnp | grep 8080
   ```

3. **Test WebSocket connection:**
   Open browser console (F12) and check:
   ```javascript
   console.log(socket.connected);  // Should be true
   ```

4. **Check hub-to-web bridge logs:**
   View server console for errors or debug messages.

### Commands Timing Out (No FEEDBACK)

1. **Increase PENDING_TIMEOUT_MS** if hub is slow:
   ```javascript
   // In gui/lib/door_server.js
   const PENDING_TIMEOUT_MS = 10000;  // 10 seconds
   ```

2. **Verify hub is running and responsive:**
   ```bash
   ping 192.168.8.108
   nc -u -l -p 12345  # Listen for commands
   ```

3. **Check for network congestion:**
   ```bash
   sudo tcpdump -u -i eth0 -c 100
   ```

### Cross-Compilation Fails

1. **Verify cross-compiler is installed:**
   ```bash
   aarch64-linux-gnu-gcc --version
   ```

2. **Clear CMake cache:**
   ```bash
   rm -rf build/CMakeCache.txt
   ```

3. **Check for architecture mismatch:**
   ```bash
   file build/app/doorMod_cli
   # Should output: ELF 64-bit LSB executable, ARM aarch64
   ```

---

## Additional Resources

- BeagleY-AI Documentation: https://docs.beagleboard.org/boards/beagley/ai/
- GPIO Sysfs Interface: https://www.kernel.org/doc/html/latest/userspace-api/gpio/sysfs.html
- UDP Socket Programming: https://linux.die.net/man/7/udp
- CMake Manual: https://cmake.org/cmake/help/latest/

---

**Last Updated:** December 2025
**Project:** Embedded Home Security System (ENSC 351)
