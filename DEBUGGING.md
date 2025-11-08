# DSP Firmware Debugging Guide

This guide explains how to debug the OpenCentauri FreeRTOS DSP firmware and understand the initialization sequence.

## Overview

The DSP firmware initializes in the following sequence:
1. Print banner with system information
2. Initialize logging system (lprintf to shared memory)
3. Initialize sharespace communication with ARM
4. Start FreeRTOS scheduler
5. Run vTaskMain in a loop

## Debug Output

The firmware uses two output methods:

### 1. UART Output (printf)
- Goes directly to serial console at 115200 baud
- Available immediately on boot
- Use for critical early-boot messages

### 2. Shared Memory Logging (lprintf)
- Writes to DSP log buffer in shared memory
- Can be read from Linux side
- Available after log_init() completes

## Understanding the Initialization Sequence

### Step 1: Banner and Initial Setup
```
OpenCentauri FreeRTOS for HIFI4 DSP v0.0.0
DSP Clock Frequency: XXXXXX Hz
OEM Header Address: 0xXXXXXXXX
FreeRTOS Tick Rate: 1000 Hz (1 tick = 1 ms)
Platform: Allwinner R528 SoC
```

### Step 2: Sharespace Discovery
The DSP reads configuration from the DTS (Device Tree):
```
DTS Sharespace Configuration:
  DSP Write Address: 0xXXXXXXXX  (DSP->ARM buffer)
  DSP Write Size:    0xXXXXXXXX
  ARM Write Address: 0xXXXXXXXX  (ARM->DSP buffer)
  ARM Write Size:    0xXXXXXXXX
  DSP Log Address:   0xXXXXXXXX  (logging buffer)
  DSP Log Size:      0xXXXXXXXX
```

### Step 3: Logging System Initialization
```
log_init: Checking DTS sharespace status...
log_init: DTS sharespace is OPEN
log_init:   log_buffer = 0xXXXXXXXX
log_init:   log_buffer_size = XXXX bytes
log_clear: Clearing log buffer at 0xXXXXXXXX (size XXXX bytes)
```

### Step 4: Sharespace Handshake
The DSP and ARM perform a handshake to establish communication:

```
sharespace_init: Starting initialization.
sharespace_init: arm_head_ptr initially set to 0xXXXXXXXX
sharespace_clear: Clearing ARM head at 0xXXXXXXXX
sharespace_clear: Initializing ARM head: init_state=0
```

The DSP waits for ARM to provide kbuf addresses:
```
sharespace_reinit: Waiting for ARM initialization...
sharespace_reinit: Iteration 1 - Reading ARM head from 0xXXXXXXXX
sharespace_reinit: arm_head.init_state = 2  (ARM is ready!)
sharespace_reinit: arm_head.write_addr = 0xXXXXXXXX (kbuf ARM->DSP)
sharespace_reinit: arm_head.read_addr = 0xXXXXXXXX (kbuf DSP->ARM)
```

Final handshake completion:
```
sharespace_init: Wrote DSP head with init_state=1
sharespace_init: Waiting for ARM to acknowledge...
sharespace_init: Poll #1 - ARM head: init_state=1 (ARM acknowledged!)
sharespace_init: Initialization complete.
```

### Step 5: FreeRTOS Scheduler
```
main: Creating vTaskMain...
main: Calling vTaskStartScheduler...
vTaskStartScheduler
vTaskMain: Task started, entering main loop.
vTaskMain loop: 0
vTaskMain loop: 1
...
```

## Common Issues and Solutions

### Issue: Stuck waiting for ARM initialization
**Symptoms**: Logs show repeated "sharespace_reinit: Iteration N" messages

**Possible Causes**:
1. ARM-side driver not loaded or not running
2. Wrong shared memory addresses in device tree
3. Cache coherency issues (should be fixed now)

**Debug Steps**:
1. Check ARM dmesg for DSP driver messages
2. Verify shared memory regions in device tree match DSP firmware expectations
3. Check that arm_head.init_state never becomes 2

### Issue: Addresses show 0xa5a5a5a5
**Symptoms**: Log shows 0xa5a5a5a5 for addresses or init_state values

**Meaning**: Reading uninitialized or invalid memory

**Debug Steps**:
1. Check that DTS sharespace status is OPEN (not CLOSED)
2. Verify memory addresses in device tree are correct
3. Check that platform_head pointer is valid

### Issue: vTaskStartScheduler never returns
**This is normal!** The scheduler should never return. If it does return, you'll see:
```
main: ERROR - vTaskStartScheduler returned!
vTaskStartScheduler FAILED!
```

This indicates a critical error in FreeRTOS configuration or memory.

## Cache Coherency

The R528 DSP has data cache that must be manually managed for shared memory:

### Reading from Shared Memory
```c
// ALWAYS invalidate cache before reading
xthal_dcache_region_invalidate((void*)address, size);
// Now read the data
memcpy(&local_data, (void*)address, size);
```

### Writing to Shared Memory
```c
// Write the data
memcpy((void*)address, &local_data, size);
// ALWAYS writeback cache after writing
xthal_dcache_region_writeback((void*)address, size);
```

**Why?** Without these operations:
- Reads may get stale cached data instead of ARM's updates
- Writes may stay in cache and never reach ARM

## Memory Layout

```
Shared Memory Region Structure:
┌─────────────────────────────────┐ 0x00000000
│     MsgHead (12 bytes)          │ Header at start
├─────────────────────────────────┤ MIN_ADDR (sizeof(MsgHead))
│                                 │
│     Circular Buffer Data        │ Actual message data
│     (BUFFER_SIZE - 2*sizeof)    │
│                                 │
├─────────────────────────────────┤ MAX_ADDR (BUFFER_SIZE - sizeof(MsgHead))
│     MsgHead (12 bytes)          │ Header at end
└─────────────────────────────────┘ BUFFER_SIZE (4096)
```

The MsgHead structure:
```c
typedef struct {
    uint32_t read_addr;   // Where consumer reads next
    uint32_t write_addr;  // Where producer writes next
    uint32_t init_state;  // Handshake state
} MsgHead;
```

## State Machine

### Init States
- `0`: DSP is ready, waiting for ARM to start handshake
- `1`: Both sides initialized and ready for communication
- `2`: ARM has provided kbuf addresses

### Handshake Flow
1. DSP sets init_state=0 in initial DTS location
2. ARM sets init_state=2 with kbuf addresses
3. DSP switches to kbuf locations
4. DSP sets init_state=1 in kbuf location
5. ARM responds with init_state=1
6. Communication ready!

## Reading DSP Logs from Linux

The DSP log buffer can be read from Linux using the shared memory region. The format is:

```
[0-3]:  Write pointer (4 bytes)
[4-N]:  Log messages (null-terminated strings)
```

To read logs:
```bash
# Example: Read from the log buffer
dd if=/dev/mem bs=1 skip=$((0xLOG_ADDR)) count=$((0xLOG_SIZE)) | strings
```

## Tips for Adding More Debug

### Add More lprintf() Statements
```c
lprintf("function_name: Doing something important at address 0x%08x\n", addr);
```

### Use printf() for Early Boot
If lprintf() isn't working yet, use printf() which goes to UART.

### Dump Memory Regions
```c
lprintf("Memory dump at 0x%08x:\n", addr);
for (int i = 0; i < size; i += 4) {
    lprintf("  [%04x] = 0x%08x\n", i, *(uint32_t*)(addr + i));
}
```

### Time-Critical Sections
```c
uint32_t start = xthal_get_ccount();
// ... do something ...
uint32_t cycles = xthal_get_ccount() - start;
lprintf("Operation took %u cycles\n", cycles);
```

## Useful Commands

### Read CPU Clock
```c
uint32_t freq_hz = xtbsp_clock_freq_hz();
```

### Get Cycle Count
```c
uint32_t cycles = xthal_get_ccount();
```

### Sleep Microseconds
```c
hw_usleep(1000000);  // Sleep 1 second
```

### FreeRTOS Delay
```c
vTaskDelay(1000);  // Delay 1000 ticks = 1 second (at 1000 Hz tick rate)
```

## Summary

The key to debugging DSP firmware is understanding:
1. **The initialization sequence**: Each step must complete before the next
2. **Cache coherency**: Always invalidate before reading, writeback after writing
3. **The handshake protocol**: DSP and ARM must coordinate through init_state
4. **Logging output**: Use both UART (printf) and shared memory (lprintf)

With the debug logging now in place, you should see detailed messages at each step, making it easy to identify where initialization fails or hangs.
