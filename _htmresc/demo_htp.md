# demo_htp

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_htp.c

## Overview

This file provides an interactive HTTP Time Protocol (HTP) demonstration for the SIMCom A7672 module. HTP synchronizes the module's system clock by fetching time from an HTTP server, as an alternative to NTP. The demo allows configuring one or more HTTP time servers and then executing a time update operation with asynchronous result delivery.

## Main Features

- Interactive UART-based menu for HTP operations
- HTTP server configuration: add, remove, and query time servers
- Asynchronous time update via message queue
- Support for multiple HTTP time servers in a list
- Configurable HTTP version (1.0 or 1.1), port, and proxy settings
- Result notification via message queue with success/failure indication

## Key Header Dependencies

- `simcom_htp_client.h` -- Provides `sAPI_HtpSrvConfig()`, `sAPI_HtpUpdate()`, `SChtpResultType` enum, and `SChtpOperationType` enum
- `simcom_os.h` -- Provides OS primitives: message queue creation/receive
- `simcom_debug.h` -- Provides `sAPI_Debug()` for debug logging
- `simcom_uart.h` -- Provides UART write functions
- `uart_api.h` -- Provides `UartReadValue()` for UART input

## SDK APIs Observed

- `sAPI_HtpSrvConfig(cmdType, returnString, cmd, hostOrIdx, port, httpVersion, proxy, proxyPort)` -- Configure the HTTP time server list:
  - `cmdType`: `SC_HTP_OP_SET` to add/remove, `SC_HTP_OP_GET` to query
  - `cmd`: "ADD" to add a server, "DEL" to remove by index
  - `hostOrIdx`: hostname for ADD, index for DEL
  - `port`: HTTP server port (1-65535)
  - `httpVersion`: 0=HTTP 1.0, 1=HTTP 1.1
  - `proxy`: proxy address (optional)
  - `proxyPort`: proxy port (optional)
- `sAPI_HtpUpdate(msgQ)` -- Trigger the time update thread; result is delivered via message queue
- `sAPI_MsgQCreate(ref, name, msgSize, queueSize, mode)` -- Create message queue for HTP results
- `sAPI_MsgQRecv(ref, msg, timeout)` -- Receive HTP result messages

### HTP Result Codes

- `SC_HTP_OK` (0) -- Time update succeeded
- `SC_HTP_ERROR` -- General error
- `SC_HTP_UNKNOWN_ERROR` -- Unknown error
- `SC_HTP_INVALID_PARAM` -- Invalid parameters
- `SC_HTP_BAD_DATETIME_GOT` -- Invalid datetime received
- `SC_HTP_NETWORK_ERROR` -- Network error

## Usage

1. Call `HtpDemo()` to enter the menu.
2. Follow the menu options:
   - **Option 1 (Config Server):**
     - Creates the message queue if not already created.
     - Adds two HTTP time servers: "www.baidu.com:80" and "www.52im.net:80", both HTTP 1.1.
     - Queries the configured server list via `sAPI_HtpSrvConfig(SC_HTP_OP_GET, ...)`.
     - Sets `true=1` flag indicating servers are configured.
   - **Option 2 (Update):**
     - Checks if servers were configured (`true` flag).
     - Calls `sAPI_HtpUpdate(htpUIResp_msgq)` to trigger the time update.
     - Loops on `sAPI_MsgQRecv()` waiting for the result.
     - Filters for `SC_SRV_HTP` message ID.
     - Reports success (`SC_HTP_OK`) or failure with error code.
   - **Option 99:** Returns to the previous menu.

### Typical HTP Flow

1. Config Server (add one or more HTTP time servers)
2. Update (trigger time synchronization)
3. Check result via message queue

## Implementation Notes

- HTP is an alternative to NTP for modules without direct NTP access or behind firewalls that block UDP/123.
- The demo adds two servers for redundancy: "www.baidu.com" and "www.52im.net".
- The `true` flag (UINT32) tracks whether server configuration was successful, preventing update attempts before configuration.
- The message queue is named "htpUIResp_msgq" with a size of 4 messages.
- The demo comment notes that "unavailable addresses may cause long time suspension" (e.g., google.com in China).
- The `sAPI_HtpSrvConfig` with `SC_HTP_OP_GET` returns the server list in `buff` (220 bytes).
- HTTP version 1.1 is used in the demo (the `http_version` parameter value is 1).

## Best Practices

- Configure reliable HTTP time servers accessible from the module's network location.
- Use multiple servers for redundancy; if the primary server fails, the SDK may try the next one.
- HTP depends on the HTTP server providing accurate Date headers; ensure your chosen servers maintain accurate time.
- For production, use well-known time servers rather than arbitrary websites like baidu.com.
- Consider NTP as the primary time sync mechanism and HTP as a fallback, since NTP is more accurate.
- The time update may take several seconds depending on network latency and server response time.
- If operating behind a proxy, configure the proxy address and port in the `sAPI_HtpSrvConfig` call.
- HTP operates over HTTP (not HTTPS); if security is a concern, use HTTPS-based time services or NTP with authentication.
