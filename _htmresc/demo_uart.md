# demo_uart

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_uart.c

## Overview

This demo provides a comprehensive interactive menu for configuring and controlling UART ports on SIMCom modules. It demonstrates setting baud rate, data bits, parity, stop bits, reading configuration, checking RX status, controlling sleep/wake modes, sending internal AT commands, and configuring RS485 DE pins. It targets the full-function UART port (SC_UART or SC_UART4 depending on module).

## Main Features

- Set UART baud rate (4800, 9600, 19200, 38400, 57600, 115200)
- Set data bits (5, 6, 7, or 8 bits)
- Set parity bit (none, odd, even)
- Set stop bits (1, or 1.5/2)
- Get and display current UART configuration
- Check UART RX status (idle or busy)
- Enable/disable system sleep mode
- Get current sleep status
- Open/close UART3 port
- Set alarm clock wakeup from sleep
- Send internal AT command (`AT+CPIN?`) and get response
- Configure RS485 DE pin with active level selection
- Extended sleep mode with timeout parameter

## Key Header Dependencies

- `simcom_uart.h` -- Defines `SC_Uart_Port_Number` (SC_UART 1-4), `SCuartConfiguration` struct, `SC_UART_BaudRates`, `SC_UART_WordLen`, `SC_UART_StopBits`, `SC_UART_ParityTBits` enums, `SC_Uart_Return_Code`, `SC_Uart_Control`, `SC_UartRs485DeActiveLevel`, and all `sAPI_Uart*()` and `sAPI_SendATCMDWaitResp()` prototypes
- `uart_api.h` -- Provides `UartReadValue()` and `UartReadLine()` for UART-based user input
- `simcom_system.h` -- Provides `sAPI_SystemSleepSet()`, `sAPI_SystemSleepGet()`, `sAPI_SystemSleepExSet()`, `sAPI_SystemAlarmClock2Wakeup()`

## SDK APIs Observed

- `sAPI_UartSetConfig(port, &config)` -- Set UART configuration (baud rate, data bits, stop bits, parity)
- `sAPI_UartGetConfig(port, &config)` -- Get current UART configuration
- `sAPI_UartRxStatus(port, timeout)` -- Check if UART RX is idle or busy, with a timeout threshold in ms
- `sAPI_UartControl(port, control)` -- Open or close a UART port (SC_UART_OPEN / SC_UART_CLOSE)
- `sAPI_UartWrite(port, data, length)` -- Write data to UART (not directly used in demo but referenced)
- `sAPI_SendATCMDWaitResp(channel, cmd, timeout, okFmt, okFlag, errFmt, outStr, respLen)` -- Send an AT command internally and wait for response; channel 10 is recommended
- `sAPI_UartRs485DePinAssignEx(port, gpioNum, activeLevel)` -- Assign a GPIO pin as RS485 DE (Driver Enable) with configurable active level
- `sAPI_SystemSleepSet(enable)` -- Enable or disable system sleep mode
- `sAPI_SystemSleepGet()` -- Get current system sleep status
- `sAPI_SystemSleepExSet(enable, timeout)` -- Enable extended sleep with a timeout parameter
- `sAPI_SystemAlarmClock2Wakeup(ms)` -- Set an alarm to wakeup the module after specified milliseconds

## Usage

1. Call `UartDemo()` from the application task.
2. The demo defaults to using `SC_UART` (or `SC_UART4` on A7680C/A7670C variants).
3. A default configuration of 115200 baud, 8 data bits, 1 stop bit, no parity is pre-set.
4. Select menu options 1-14 (99 to exit).
5. For baud/data/parity/stop bit changes, select the sub-option and the new config is applied immediately via `sAPI_UartSetConfig()`.
6. For internal AT commands (option 11), the demo sends `AT+CPIN?` on channel 10 with a 150ms timeout, expecting `+CPIN` in the response.
7. For RS485 (option 12), enter the UART port number, GPIO pin number, and DE active level.
8. For sleep (options 7, 13), the module enters sleep; pull DTR pin low or wait for alarm to wakeup.

## Implementation Notes

- The `SC_UART` macro is conditionally defined: on A7680C and certain A7670C modules it maps to `SC_UART4`; otherwise it maps to `SC_UART` (port 1).
- The `SCuartConfiguration` struct is initialized globally with 115200/8N1 defaults.
- Menu navigation uses `goto reSelect` labels for re-prompting after sub-operations, rather than nested loops.
- The RX status check (option 6) specifically tests `SC_UART3` with a 50ms timeout, not the main UART port.
- The alarm wakeup demo sets a 120-second (120000ms) timer before entering sleep.
- The `sAPI_SendATCMDWaitResp` call uses channel 10 (`TEL_AT_CMD_ATP_10`), which is the recommended internal AT channel.
- SleepEx status retrieval is compiled out (`#if 0`) in the demo, suggesting the API may not be finalized.
- UART RX wakeup (`sAPI_SystemSleepRxWakeSet`) is conditionally compiled only when `UART_RX_WAKE_DEMO` is defined.

## Best Practices

- Always verify `sAPI_UartSetConfig()` return value -- it returns `SC_UART_RETURN_CODE_ERROR` (-1) on failure.
- Use `SC_UART4` instead of `SC_UART` on A7680C modules since SC_UART is the debug port on those modules.
- For internal AT commands, use channel 10 and set an adequate timeout (150ms minimum recommended).
- When configuring RS485, ensure the GPIO pin is not already used by another peripheral.
- Before entering sleep mode, ensure all pending UART operations are complete.
- Use `sAPI_UartRxStatus()` to check if data is still being received before putting the module to sleep.
- For production code, register a UART callback via `sAPI_UartRegisterCallback()` or `sAPI_UartRegisterCallbackEX()` for asynchronous data handling instead of polling.
