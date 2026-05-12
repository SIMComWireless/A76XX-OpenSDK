# demo_ntp

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_ntp.c

## Overview

This file provides an interactive NTP (Network Time Protocol) client demonstration for the SIMCom A7672 module. It synchronizes the module's system clock with a remote NTP server (using `ntp3.aliyun.com` as the example). The demo shows the complete NTP workflow: configure server, query configuration, execute time update, and display the before/after system time.

## Main Features

- Interactive UART-based menu for NTP operations
- NTP time synchronization with configurable server address and timezone
- System time display before and after NTP update
- Three-step NTP operation: SET server address, GET current configuration, EXEcute update
- Asynchronous result delivery via message queue
- Support for timezone configuration (range: -47 to 48)

## Key Header Dependencies

- `simcom_ntp_client.h` -- Provides `sAPI_NtpUpdate()`, `sAPI_GetSysLocalTime()`, `sAPI_SetSysLocalTime()`, `sAPI_GetTxnNo()`, the `SCsysTime_t` time structure, and NTP result codes (`SCntpResultType`)
- `simcom_os.h` -- Provides OS primitives: message queue creation/receive
- `simcom_debug.h` -- Provides `sAPI_Debug()` for debug logging
- `simcom_uart.h` -- Provides UART write functions
- `uart_api.h` -- Provides `UartReadValue()` for UART input

## SDK APIs Observed

- `sAPI_NtpUpdate(cmdType, hostAddr, timeZone, msgQ)` -- Three-in-one NTP operation:
  - `SC_NTP_OP_SET`: Set the NTP server address and timezone
  - `SC_NTP_OP_GET`: Get the current configured NTP server address into a buffer
  - `SC_NTP_OP_EXC`: Execute the time synchronization; result delivered via message queue
- `sAPI_GetSysLocalTime(&time)` -- Get the current system local time into `SCsysTime_t` structure
- `sAPI_SetSysLocalTime(timeStr)` -- Set system time from a string ("yy/MM/dd,hh:mm:ss")
- `sAPI_GetTxnNo()` -- Get current timestamp as UINT64
- `sAPI_MsgQCreate(ref, name, msgSize, queueSize, mode)` -- Create message queue for NTP results
- `sAPI_MsgQRecv(ref, msg, timeout)` -- Receive NTP result messages

## Usage

1. Call `NtpDemo()` to enter the menu.
2. Select option 1 (Update):
   - The current system time is displayed via `sAPI_GetSysLocalTime()`.
   - `sAPI_NtpUpdate(SC_NTP_OP_SET, "ntp3.aliyun.com", 32, NULL)` configures the NTP server and timezone (32 corresponds to UTC+8/CST).
   - `sAPI_NtpUpdate(SC_NTP_OP_GET, buff, 0, NULL)` reads back the configured server into `buff`.
   - `sAPI_NtpUpdate(SC_NTP_OP_EXC, NULL, 0, ntpUIResp_msgq)` triggers the time update and delivers results to the message queue.
   - The code waits on `sAPI_MsgQRecv()` for the result, checking `msg_id == SC_SRV_NTP` and `arg1 == SC_NTP_OK`.
   - After completion, the updated system time is displayed.
3. Option 99 returns to the previous menu.

### NTP Result Codes

- `SC_NTP_OK` (0) -- Time update succeeded
- `SC_NTP_ERROR` -- General error
- `SC_NTP_ERROR_INVALID_PARAM` -- Invalid parameters
- `SC_NTP_ERROR_TIME_CALCULATED` -- Time calculation error
- `SC_NTP_ERROR_NETWORK_FAIL` -- Network failure
- `SC_NTP_ERROR_TIME_ZONE` -- Timezone error
- `SC_NTP_ERROR_TIME_OUT` -- Timeout

## Implementation Notes

- The `SCsysTime_t` structure contains: `tm_sec` (0-59), `tm_min` (0-59), `tm_hour` (0-23), `tm_mday` (1-31), `tm_mon` (1-12), `tm_year` (since 1970), `tm_wday` (Sunday=0).
- The timezone value of 32 in the demo corresponds to UTC+8 (China Standard Time). The range is -47 to 48, where each unit represents 15 minutes offset from UTC.
- The message queue result loop filters for `SC_SRV_NTP` message ID to ignore unrelated messages.
- The message queue is named "htpUIResp_msgq" (appears to be a copy-paste from HTP demo).
- The NTP operation combines three steps (SET/GET/EXC) in one API function with different operation types.
- The demo displays time in the format: year - month - day - hour : minute : second weekday.

## Best Practices

- Always call NTP time synchronization after network registration is complete and PDP context is active.
- Use reliable NTP servers; the demo uses `ntp3.aliyun.com` which is specific to China. Adjust for your region (e.g., `pool.ntp.org`).
- Set the timezone correctly for your deployment region to get accurate local time.
- After NTP sync, the system clock is updated globally and affects all time-dependent operations (logging, certificates, scheduled tasks).
- Consider calling NTP synchronization at startup and periodically (e.g., daily) to maintain clock accuracy.
- The NTP operation may take several seconds; ensure the message queue timeout is adequate.
- If the NTP server address is unreachable, the operation may hang; use a reasonable timeout in `sAPI_MsgQRecv()` instead of `SC_SUSPEND` for production.
