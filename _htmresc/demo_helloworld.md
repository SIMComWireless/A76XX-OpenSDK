# demo_helloworld

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_helloworld.c

## Overview

This demo serves as a basic "Hello World" entry point for the SIMCom OpenSDK on the ASR1603 module. It demonstrates how to create a task and initialize I2C communication, continuously reading from an I2C codec device. Despite the "helloworld" name, this demo is primarily an I2C read example.

## Main Features

- Create and start a background task
- Initialize I2C bus configuration
- Continuously read a 16-bit register from an I2C codec device (address 0x34)
- Demonstrate the basic task lifecycle (create, run loop, sleep)

## Key Header Dependencies

- `sc_i2c.h` -- Provides I2C API types and functions: sAPI_I2CConfigInit, sAPI_I2CReadEx, SC_I2C_DEV, SC_I2C_CHANNEL0, SC_I2C_FAST_MODE, SC_I2C_RC_OK
- `simcom_os.h` -- Provides OS primitives: sAPI_TaskCreate, sAPI_TaskSleep, sTaskRef
- `simcom_debug.h` -- Provides sAPI_Debug for debug logging
- `simcom_common.h` -- Provides common types (UINT8, UINT16, SC_STATUS, SC_SUCCESS)

## SDK APIs Observed

- `sAPI_TaskCreate(&taskRef, stack, stackSize, priority, name, func, arg)` -- Create a new task/thread. Parameters: task reference, stack buffer, stack size (bytes), scheduling priority, task name string, entry function, and argument.
- `sAPI_TaskSleep(ms)` -- Sleep the current task for the specified number of milliseconds. The demo uses `sAPI_TaskSleep(1)` (1ms) in the main loop.
- `sAPI_I2CConfigInit(&i2cDev)` -- Initialize the I2C bus with the specified configuration. Returns SC_I2C_RC_OK on success.
- `sAPI_I2CReadEx(channel, devAddr, regAddr, data, len)` -- Read data from an I2C device register. Parameters: I2C channel, 7-bit device address, register address (shifted left by 1), data buffer, number of bytes to read.

## Usage

The demo is initialized by calling `sAPP_HelloWorldDemo()`:

1. Creates a task named `"helloWorldProcesser"` with a 5KB static stack, priority 150.
2. The task entry function `sTask_HelloWorldProcesser` runs in an infinite loop:
   a. Configures I2C channel 0 in fast mode.
   b. Repeatedly reads 2 bytes from I2C device 0x34, register 0x40 (a codec register).
   c. Combines the two bytes into a 16-bit register value and logs it via `sAPI_Debug`.
   d. Sleeps for 1ms between reads.

## Implementation Notes

- The task stack is statically allocated as `static UINT8 helloWorldProcesserStack[1024*5]` (5KB).
- The task priority is 150, which is relatively high in the SIMCom RTOS priority scheme.
- I2C channel 0 (`SC_I2C_CHANNEL0`) is used, with fast mode clock (`SC_I2C_FAST_MODE`).
- The I2C device address is `0x34` (7-bit), and the register address `0x40` is shifted left by 1 (`regAddr << 1`) as required by the I2C protocol for the register address byte.
- The `I2C_read_CODEC()` function reads 2 bytes and combines them as big-endian: `(((UINT16)data[0]) << 8) | data[1]`.
- The demo includes a commented-out `simcom_printf` function that wraps `sAPI_Vsnprintf` and `sAPI_Debug` for formatted output.
- The 1ms sleep in the loop prevents I2C bus saturation and allows other tasks to run.
- Error handling is minimal: if I2C init fails, an error is logged but execution continues (the read loop will likely fail on each iteration).

## Best Practices

- Allocate sufficient stack space for tasks. The 5KB shown here is appropriate for simple I2C operations. More complex tasks may need larger stacks.
- Choose task priorities carefully. Higher priority (lower number in some RTOS, higher in others) tasks preempt lower priority ones. Priority 150 should be verified against the system's priority scheme.
- Always check the return value of `sAPI_I2CConfigInit` and handle initialization failures.
- Use appropriate I2C clock speeds. `SC_I2C_FAST_MODE` (400kHz) is suitable for most peripherals, but some devices may require standard mode (100kHz).
- Add appropriate delays between I2C operations to respect device timing requirements.
- In production, add timeout and retry logic for I2C reads to handle bus errors or device unresponsiveness.
- Use `sAPI_Debug` for logging rather than `printf` to ensure output is captured by the CATSTUDIO debug tool.
