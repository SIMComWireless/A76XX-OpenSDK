# demo_i2c

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_i2c.c

## Overview

This demo provides an interactive menu for testing I2C bus operations on SIMCom modules. It demonstrates opening, reading, and writing to I2C devices across three I2C channels (I2C0, I2C2, I2C3), using a NAU8810 audio codec (slave address 0x34) as the target device. It also shows how to initialize I2C configuration with different clock modes.

## Main Features

- I2C write operations (standard and extended)
- I2C read operations (standard and extended)
- I2C channel configuration with clock speed selection
- Support for I2C channels 0, 2, and 3
- NAU8810 codec register read/write as a concrete example

## Key Header Dependencies

- `sc_i2c.h` -- Defines `SC_I2C_ReturnCode` enums, `SC_I2C_CHANNEL` (CHANNEL0-3), `SC_I2C_CLOCK` (STANDARD/FAST/HS modes), `SC_I2C_DEV` struct, and all `sAPI_I2C*()` function prototypes
- `uart_api.h` -- Provides `UartReadValue()` for UART-based user input

## SDK APIs Observed

- `sAPI_I2CWrite(i2cChannel, slaveAddr, regAddr, data, len)` -- Write data to a specific register of an I2C slave device
- `sAPI_I2CRead(i2cChannel, slaveAddr, regAddr, data, len)` -- Read data from a specific register of an I2C slave device
- `sAPI_I2CWriteEx(i2cChannel, slaveAddr, regAddr, data, len)` -- Extended write (alternative timing/protocol variant)
- `sAPI_I2CReadEx(i2cChannel, slaveAddr, regAddr, data, len)` -- Extended read (alternative timing/protocol variant)
- `sAPI_I2CConfigInit(&i2cDev)` -- Initialize I2C bus with specific channel and clock configuration

## Usage

1. Call `I2cDemo()` from the application task.
2. The demo presents a UART menu with options 1-14 and 99 to exit.
3. Select an I2C channel operation (channel 0 options 1-5, channel 2 options 6-10, channel 3 options 12-14).
4. For read/write operations, the demo uses the NAU8810 codec at slave address `0x34` as the target.
5. For config init (options 10, 11, 14), the I2C bus is initialized with `SC_I2C_FAST_MODE` (400 Kbps) on the selected channel.
6. All read results are printed via `sAPI_Debug()` showing register address and data.

## Implementation Notes

- The NAU8810 slave address is defined as `0x34` (7-bit address).
- Register address and data are packed into a 2-byte buffer: byte 0 contains `(regAddr << 1) | MSB_of_data`, byte 1 contains the lower 8 bits. This follows the NAU8810 protocol where the register address and data bit are interleaved.
- `sAPI_I2CWrite` and `sAPI_I2CWriteEx` both send 1 data byte after the register address byte.
- `sAPI_I2CRead` and `sAPI_I2CReadEx` read 2 bytes (high byte, low byte) and combine them into a 16-bit register value.
- I2C channel 0 maps to specific module pins (e.g., PIN68/69 on A7680C, PIN37/38 on A7670C). Channel 2 is on PIN35/36 of A7670C. These pin mappings are documented in the `sc_i2c.h` header comments.
- The `SC_I2C_CHANNEL3` case uses the raw value `3` instead of an enum constant, suggesting it may be a newer addition.
- The I2C2 config init is guarded by `#ifndef FALCON_1803_PLATFORM`, indicating it is not supported on 1803-class platforms.

## Best Practices

- Always call `sAPI_I2CConfigInit()` before performing read/write operations on a channel, to set the correct clock speed.
- Check the return value of every I2C call against `SC_I2C_RC_OK`.
- Use `sAPI_I2CReadEx`/`sAPI_I2CWriteEx` for devices that require a repeated-start condition between the register address and data phases.
- Ensure the slave address matches the target device's 7-bit address (not including the R/W bit).
- Be aware that I2C channels have different pin mappings on different module variants -- consult the module datasheet.
- For high-speed mode (`SC_I2C_HS_MODE`), verify the target device supports 3.4 Mbps signaling.
