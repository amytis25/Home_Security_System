# Debug Output Control

## Debug Levels

There are two levels of debug output:
1. **Normal Debug** (`DEBUG_PRINT`) - Important events and errors
2. **Verbose Debug** (`DEBUG_VERBOSE`) - Per-operation details (can be very noisy)

## How to Control Debug Messages

### 1. GPIO Debug Messages
**File:** `hal/src/GPIO.c`
```c
/* Debug output control */
#define GPIO_DEBUG 1          // Set to 0 to disable all GPIO debug
#define GPIO_DEBUG_VERBOSE 0  // Set to 1 to enable per-read debug messages
```

### 2. HC-SR04 Debug Messages
**File:** `hal/src/HC-SR04.c`
```c
/* Debug output control */
#define HC_SR04_DEBUG 1          // Set to 0 to disable all HC-SR04 debug
#define HC_SR04_DEBUG_VERBOSE 0  // Set to 1 to enable per-read debug messages
```

## Recommended Settings

### Normal Operation (Minimal Output)
```c
#define GPIO_DEBUG 0
#define GPIO_DEBUG_VERBOSE 0
#define HC_SR04_DEBUG 0
#define HC_SR04_DEBUG_VERBOSE 0
```

### Troubleshooting (Current Setting)
```c
#define GPIO_DEBUG 1          // Shows initialization and errors
#define GPIO_DEBUG_VERBOSE 0  // Hides noisy per-read messages
#define HC_SR04_DEBUG 1
#define HC_SR04_DEBUG_VERBOSE 0
```

### Deep Debugging (Very Verbose)
```c
#define GPIO_DEBUG 1
#define GPIO_DEBUG_VERBOSE 1  // Shows every GPIO read operation
#define HC_SR04_DEBUG 1
#define HC_SR04_DEBUG_VERBOSE 1  // Shows every echo pin value
```

## Rebuild After Changes

After changing the debug flags, rebuild the project:
```bash
./build-aarch64.sh
```

The updated binary will be automatically copied to `~/ensc351Proj/public/doorMod/doorMod`

## Current Status
- **GPIO_DEBUG**: Enabled (1) - Shows initialization and errors
- **GPIO_DEBUG_VERBOSE**: Disabled (0) - Hides per-read messages  
- **HC_SR04_DEBUG**: Enabled (1) - Shows sensor operation
- **HC_SR04_DEBUG_VERBOSE**: Disabled (0) - Hides per-read echo values

When debug is disabled (0), the macros compile to nothing, so there's zero runtime overhead.
