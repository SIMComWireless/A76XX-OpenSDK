# demo_gpio

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_gpio.c

## Overview

This demo provides a comprehensive interactive menu-driven interface for testing GPIO functionality on SIMCom A76xx-series cellular modules. It demonstrates setting direction, reading/writing levels, configuring interrupts, wakeup sources, and bulk GPIO testing across all available pins for a given module variant.

## Main Features

- Set GPIO pin direction (input/output)
- Get current GPIO pin direction
- Set GPIO output level (high/low)
- Read GPIO input level
- Configure GPIO interrupt with edge detection (rise/fall/both)
- Enable GPIO wakeup from sleep mode
- Full parameter configuration via comma-delimited input (direction, init level, pull type, edge type)
- Bulk high-level and low-level test across all module GPIOs
- SIM DET (SIM card detect) function switch (currently disabled via `#if 0`)
- Platform-specific GPIO pin mapping via extensive compile-time `#ifdef` blocks

## Key Header Dependencies

- `simcom_gpio.h` -- Defines `SC_GPIOReturnCode` enums, `SC_GPIOConfiguration` struct, `SC_GPIOPinDirection`, `SC_GPIOPullUpDown`, `SC_GPIOTransitionType` enums, and all `sAPI_Gpio*()` function prototypes
- `uart_api.h` -- Provides `UartReadValue()` and `UartReadLine()` for UART-based user input
- Platform-specific GPIO headers (e.g., `A7678_V102_GPIO.h`) -- Define `SC_MODULE_GPIO_xx` pin number macros for each module variant

## SDK APIs Observed

- `sAPI_GpioSetDirection(gpio, direction)` -- Set a GPIO pin as input (0) or output (1)
- `sAPI_GpioGetDirection(gpio)` -- Get the current direction of a GPIO pin
- `sAPI_GpioSetValue(gpio, value)` -- Set a GPIO output to low (0) or high (1)
- `sAPI_GpioGetValue(gpio)` -- Read the current level of a GPIO pin
- `sAPI_GpioConfig(gpio, SC_GPIOConfiguration)` -- Full configuration of a GPIO pin including direction, initial level, pull-up/down, edge type, ISR callback, and wakeup callback
- `sAPI_GpioConfigInterrupt(gpio, edge_type, handler)` -- Configure a GPIO interrupt with a specific edge type and callback handler
- `sAPI_GpioWakeupEnable(gpio, edge_type)` -- Enable a GPIO pin as a wakeup source from sleep mode
- `sAPI_SimDetectPinConfig(enable/disable)` -- Enable or disable SIM card detection pin function (commented out in demo)
- `sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_DISABLE)` -- Used in the wakeup handler to disable system sleep after wakeup

## Usage

1. Call `GpioDemo()` from the application task.
2. The demo presents a UART menu with numbered options (1-12, 99 to exit).
3. Select an option number via `UartReadValue()`.
4. For direction/level operations, enter the GPIO number and parameter when prompted.
5. For interrupt configuration, the GPIO is first set as input, then configured with a two-edge interrupt handler.
6. For wakeup, a GPIO is enabled as a wakeup source with a falling-edge trigger.
7. For full config (option 7), enter comma-separated values: `gpio_num,direction,init_level,pull_type,edge_type` (e.g., `0,0,1,1,3`).
8. For bulk test (options 10-11), all GPIOs in the platform-specific `all_gpio[]` array are configured as output and set to high or low, then verified by reading back.

## Implementation Notes

- The `all_gpio[]` static array is defined per-module variant using extensive `#ifdef` conditional compilation covering over 30 different SIMCom module models. The array is terminated with `-1`.
- GPIO numbers like `SC_MODULE_GPIO_00` are module-specific macros defined in the included platform GPIO header.
- The interrupt handler `GPIO_IntHandler` and wakeup handler `GPIO_WakeupHandler` are only active when `GPIO_INT_WAKEUP_TEST` is defined.
- `GPIO_WakeupHandler` calls `sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_DISABLE)` to prevent the module from re-entering sleep immediately after wakeup.
- The `SC_GPIO_MAX` check validates that user-entered GPIO numbers are within the valid range for the module.
- The SIM DET function switch is compiled out (`#if 0`) but shows the intended pattern: disable SIM detect, then reconfigure the pin as a regular GPIO with interrupt.
- The bulk test iterates through `all_gpio[]` until the `-1` sentinel, configuring each as output and verifying the level matches.

## Best Practices

- Always check `SC_GPIOReturnCode` return values after every GPIO API call.
- Set GPIO direction to input before configuring interrupts.
- Use `sAPI_GpioConfig()` for comprehensive one-shot configuration rather than calling individual set functions sequentially.
- Validate GPIO numbers against `SC_MODULE_GPIO_MAX` before any operation.
- When using GPIO wakeup, disable system sleep in the wakeup callback to allow the application to process the wakeup event.
- Avoid using pins reserved for DTR, RI, or SIM detect unless those functions are explicitly disabled via feature flags.
- For production code, define the GPIO pin list in a centralized header rather than using the demo's platform-conditional arrays.
