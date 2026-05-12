# uart_api (UART Helper and Data Cache Framework)

Location: SIMCOM_SDK_SET/sc_demo/V1/src/uart_api.c

## Overview
`uart_api` provides the UART initialization, data caching, and input reading infrastructure used by all demo modules. It configures up to 4 UART ports with callbacks, implements a circular buffer for caching received data from UART and USB, and provides line-reading and value-reading functions for the interactive CLI demo menu.

## Main Features
- Configures UART1/UART2/UART3/UART4 with 115200 baud, 8N1 settings via `sAPI_UartSetConfig`.
- Registers receive callbacks for all UART ports (basic and extended with data length).
- Implements a 10KB circular data cache (`UIDemoDatacache`) with mutex protection and flag-based blocking.
- Provides `UartRead` for reading arbitrary-length data from the cache.
- Provides `UartReadLine` for reading a complete line (terminated by `\r` or `\n`) with suspend support.
- Provides `UartReadValue` for reading an integer value from UART input (validates digits only).
- Supports PPPD mode with dedicated message queues for routing UART3 data to the PPPD task.
- Supports callback-based UART write mode via `SC_DEMO_UART_CALLBACK_WRITE` (optional).
- Provides `CacheDataToUIDemo` for feeding data from UART/USB callbacks into the circular cache.

## Key Header Dependencies
- `simcom_uart.h` — UART configuration structures (`SCuartConfiguration`), port numbers (`SC_UART`, `SC_UART2`, `SC_UART3`, `SC_UART4`), read/write APIs, callback registration.
- `simcom_common.h` — `SIM_MSG_T`, OS types.
- `simcom_debug.h` — `sAPI_Debug` for debug logging.
- `simcom_os.h` — `sAPI_FlagCreate`, `sAPI_FlagWait`, `sAPI_FlagSet`, `sAPI_MutexCreate`, `sAPI_MutexLock`, `sAPI_MutexUnLock`.

## SDK APIs Observed
- `sAPI_UartSetConfig(port, &config)` — configures a UART port with baud rate, data bits, parity, stop bits.
- `sAPI_UartRegisterCallback(port, callback)` — registers a basic receive callback for a UART port.
- `sAPI_UartRegisterCallbackEX(port, callback, para)` — registers an extended receive callback that provides the exact data length.
- `sAPI_UartRead(port, buf, maxLen)` — reads data from a UART port's receive buffer.
- `sAPI_UartWrite(port, data, len)` — writes data to a UART port.
- `sAPI_FlagCreate(&flag)` / `sAPI_FlagWait(flag, ...)` / `sAPI_FlagSet(flag, ...)` — event flag signaling between cache writers and readers.
- `sAPI_MutexCreate(&mutex, SC_FIFO)` / `sAPI_MutexLock` / `sAPI_MutexUnLock` — mutex protection for the circular cache.
- `sAPI_MsgQCreate` / `sAPI_MsgQSend` / `sAPI_MsgQRecv` — message queues for PPPD data routing and callback write mode.

## Usage
1. Call `sDemo_UartInit()` at system startup (from application init).
2. This configures UART1-4 to 115200 8N1 and registers receive callbacks.
3. When data arrives on any UART port, the corresponding callback is invoked.
4. Callbacks read data via `sAPI_UartRead` and feed it into the circular cache via `CacheDataToUIDemo`.
5. Demo tasks call `UartReadLine(buf, maxLen)` to read a complete line from the cache (blocks until data is available).
6. Demo tasks call `UartReadValue()` to read an integer selection from the user.
7. For PPPD mode, UART3 data is routed via `SC_PppResp_msgq` to the PPPD processing task.

### Circular Cache Architecture
```
[cache]---->[head]...data...[tail]---->[cacheEnd]
              ^                          |
              |                          |
              +----- wraps around -------+
```
- Size: 10KB (`UIDEMO_CACHE_MAXLEN`)
- Protected by mutex for thread safety
- Uses event flags for blocking reads (writer sets flag, reader waits on flag)
- Head advances on read, tail advances on write, wraps around at `cacheEnd`

## Implementation Notes
- UART buffer sizes: 256 bytes for normal mode, 2048 bytes when PPPD is enabled.
- Platform-specific UART selection: some modules (A7680C_V5_01, A7670C_V701, A7670C_V702) use `SC_UART4` as the primary debug UART.
- `UartReadLine` strips trailing `\r\n` characters and leading `\r\n` from the next read position.
- `UartReadValue` loops until it receives a line containing only digits, preventing crashes from non-numeric input.
- The `SC_DEMO_UART_CALLBACK_WRITE` mode creates a separate task and message queue for non-blocking UART writes from within callbacks.
- PPPD support is conditionally compiled via `FEATURE_SIMCOM_PPPD`, with separate message queues for PPP response and send data.

## Best Practices
- Do not block, sleep, or call heavy APIs inside UART receive callbacks — they run in interrupt/callback context.
- Use the extended callback (`RegisterCallbackEX`) when possible for better memory efficiency (known length).
- Increase `UART_RX_BUFFER_SIZE` if your application receives large packets on UART.
- For production PPPD applications, ensure the PPPD message queues have sufficient depth to avoid data loss.
- The 10KB circular cache is shared across all demos; consider separate caches for high-throughput applications.
