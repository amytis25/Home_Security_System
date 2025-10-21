# Smart Home Security System# DoorMod - HC-SR04 Ultrasonic Sensor Interface



## OverviewDoor module for ENSC 351 project using HC-SR04 ultrasonic distance sensor on BeagleY-ai.

A modular home security system providing a simple and reliable solution for residential safety. Each door module combines sensors, motors, and BeagleY-AI boards to create an intelligent monitoring network.

## Hardware Setup

## System Architecture

### Pin Connections

### Door Modules- **Trigger Pin**: GPIO27 (gpiochip1, line 33)

- **Hardware per Module:**- **Echo Pin**: GPIO17 (gpiochip2, line 8)

  - BeagleY-AI board- **Power**: 5V

  - HC-SR04 Ultrasonic sensor (door state detection)- **Ground**: GND

  - Stepper motor with driver board (lock control)

  - Miniature door setup (demonstration unit)### HC-SR04 Sensor Specifications

- Operating Voltage: 5V DC

### Central Control Unit- Working Current: 15mA

- BeagleY-AI board as central hub- Working Frequency: 40Hz

- Network connectivity for all modules- Measuring Range: 2cm - 400cm

- User notification system- Resolution: 0.3cm

- Measuring Angle: 15 degrees

## Connectivity

- **Module Communication:** BeagleY-AI boards networked to central unit## Building the Project

- **User Interface Options:**

  1. Discord webhook integration for alerts### On BeagleY-ai

  2. Mobile UI control hub *(planned feature)*```bash

     - Door status monitoring# Navigate to project directory

     - Remote lock controlcd /mnt/proj/doorMod

     - Alert management

# Create and enter build directory

## Hardware Requirementsmkdir -p build

cd build

### Core Components

- 4× BeagleY-AI Boards:# Configure and build

  - 3× Door module controllerscmake ..

  - 1× Central control unitmake -j4

- 3× HC-SR04 Ultrasonic Sensors```

- 3× Stepper Motors with Driver Boards

- 3× Demo Door Units (3D printed or wooden)### Cross-Compiling (if needed)

Contact the project maintainer for cross-compilation toolchain setup.

### Optional Components

- Front door camera module## Running the Application



## GPIO ConfigurationThe application requires root privileges to access GPIO:

- **Ultrasonic Sensor:**

  - Trigger: GPIO27 (gpiochip1, line 33)```bash

  - Echo: GPIO17 (gpiochip2, line 8)sudo ./doorMod

```

## Building and Running

### Expected Output

### Door Module Setup```

```bashDoorMod application started.

cd /mnt/proj/doorModInitializing HC-SR04 sensor:

mkdir -p build && cd buildTrigger pin: chip 1, line 33

cmake ..Echo pin: chip 2, line 8

make -j4Setting up trigger pin...

sudo ./doorModSetting up echo pin...

```Setting trigger initial state...

HC-SR04 initialization successful

### Expected Output```

```

DoorMod application started.## Troubleshooting

Initializing HC-SR04 sensor...

[Additional status messages]### Common Issues

```1. **Permission Denied**: Make sure to run with `sudo`

2. **GPIO Access Error**: Verify GPIO chip and line numbers match your board

## Project Structure3. **Build Errors**: Ensure cmake and build tools are installed

```

DoorMod/### GPIO Debug Commands

├── app/          # Application code```bash

├── hal/          # Hardware abstraction layer# Check GPIO availability

│   ├── include/  # Public headersls -l /dev/gpiochip*

│   └── src/      # Implementation

└── README.md     # This file# View GPIO line info

```gpioinfo "GPIO17"

gpioinfo "GPIO27"

## Development Status```

- [x] Door module base implementation

- [x] Sensor integration## Development

- [ ] Network communication

- [ ] Mobile interface### Project Structure

- [ ] Multi-module coordination```

DoorMod/

## Contributors├── app/

- Project Team Members│   └── doorMod.c          # Main application
├── hal/
│   ├── include/hal/
│   │   ├── HC-SR04.h      # Sensor interface header
│   │   ├── GPIO.h         # GPIO interface header
│   │   └── timing.h       # Timing utilities header
│   └── src/
│       ├── HC-SR04.c      # Sensor implementation
│       ├── GPIO.c         # GPIO implementation
│       └── timing.c       # Timing implementation
├── CMakeLists.txt
└── README.md
```

### Adding New Features
1. Implement in appropriate source files
2. Update headers if needed
3. Add to CMakeLists.txt if new files are added
4. Build and test on target hardware

## License
TBD

## Contributors
- Your Name