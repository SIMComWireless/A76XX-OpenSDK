# demo_pppd

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_pppd.c

## Overview
This demo provides an interactive menu-driven interface for PPP (Point-to-Point Protocol) dial-up connection management on the SIMCom A7672 module. It establishes a GPRS data connection using ATD*99# dialing and provides status monitoring and traffic statistics. The demo uses two message queues: one for receiving PPP response events and one for forwarding received data to the UART.

## Main Features
- PPP dial-up connection start (ATD*99#)
- Dial connection status query (CONNECTED, DISCONNECTED, CONNECTING)
- Data transfer statistics (upload/download byte counters)
- PPP dial-up connection stop
- Bidirectional data forwarding between PPP interface and UART3
- Asynchronous connection event handling via message queues

## Key Header Dependencies
- `simcom_pppd.h` — Defines PPP dial API functions and dial state enums (header file not found on disk; inferred from demo usage: `sAPI_DialStart()`, `sAPI_GetDialStatus()`, `sAPI_GetDialCount()`, `sAPI_DialStop()`, `DialStateE`, `DIAL_STATE_CONNECTED`, `DIAL_STATE_NONE`, `DIAL_TYPE_PPP`, `SC_DIAL_START`, `SC_DIAL_STATUS`, `SC_DIAL_STATICS`, `SC_DIAL_STOP`)
- `scfw_socket.h` — LWIP socket API wrappers (socket, bind, connect, send, recv, etc.)
- `scfw_netdb.h` — Network database functions (DNS resolution)
- `scfw_inet.h` — Internet address conversion functions
- `simcom_os.h` — OS abstraction for message queues, tasks
- `simcom_debug.h` — Debug logging via `sAPI_Debug()`
- `simcom_uart.h` — UART communication types and SC_UART3 definitions
- `uart_api.h` — UART read helpers

## SDK APIs Observed
- `sAPI_DialStart(respMsgQ, sendMsgQ, dialType)` — Start PPP dial-up connection; respMsgQ receives connection events, sendMsgQ is used for outbound data
- `sAPI_GetDialStatus()` — Get current dial state; returns `DialStateE` (DIAL_STATE_CONNECTED, DIAL_STATE_NONE, or CONNECTING)
- `sAPI_GetDialCount(&downcnt, &upcnt)` — Get download and upload byte counters (unsigned long long)
- `sAPI_DialStop()` — Stop the PPP dial-up connection
- `sAPI_MsgQCreate(&msgQ, name, msgSize, maxMsgs, policy)` — Create message queue
- `sAPI_MsgQRecv(msgQ, &msg, timeout)` — Receive message from queue (blocking)
- `sAPI_TaskCreate(&taskRef, stack, stackSize, priority, name, entryFunc, arg)` — Create PPP send data forwarding task
- `sAPI_UartWrite(uartId, data, len)` — Write data to UART3
- `sAPI_Free(ptr)` — Free allocated memory
- `sAPI_Debug(fmt, ...)` — Debug log output

## Usage
1. Call `PppdDemo()` to enter the interactive menu.
2. Two message queues are created lazily: `SC_PppResp_msgq` (for dial events) and `SC_PppSend_msgq` (for data forwarding).
3. A background task `ppp_sendtask` is created to listen on `SC_PppSend_msgq` and forward data to UART3.
4. Select option 1 ("Dial start"): sends "ATD*99#" to UART3, then calls `sAPI_DialStart()` with the two message queues and `DIAL_TYPE_PPP`.
5. Select option 2 ("Get dial status"): polls `sAPI_GetDialStatus()` and prints the current connection state.
6. Select option 3 ("Get dial statics"): retrieves upload/download byte counters via `sAPI_GetDialCount()`.
7. Select option 4 ("Dial stop"): calls `sAPI_DialStop()` to terminate the connection.
8. Option 99 returns to the previous menu.

## Implementation Notes
- The demo uses two external message queue references (`SC_PppResp_msgq`, `SC_PppSend_msgq`) shared with other modules.
- The `ppp_sendtask` background task runs in an infinite loop, receiving messages from `SC_PppSend_msgq`. If the message contains "NO CARRIER", it prints a disconnection notice; otherwise, it forwards the data to UART3 via `sAPI_UartWrite()`. Memory for `uartMsg.arg3` is freed after processing.
- The task stack is `SC_DEFAULT_THREAD_STACKSIZE` bytes with `SC_DEFAULT_TASK_PRIORITY`.
- `sAPI_DialStart()` initiates the PPP connection. The AT command "ATD*99#" is sent to UART3 before calling the API, which establishes the GPRS PDP context.
- Dial status values: `DIAL_STATE_CONNECTED` means data connection is active; `DIAL_STATE_NONE` means disconnected; other values indicate connecting.
- The demo includes socket/network headers (`scfw_socket.h`, `scfw_netdb.h`, `scfw_inet.h`) but does not directly use socket APIs in this file. These are likely included for the broader PPP networking context.
- Note: The header file `simcom_pppd.h` was not found in the SDK include directory, suggesting it may be provided at a different path or dynamically resolved during build.

## Best Practices
- Always check dial status before starting a new dial session to avoid conflicting connections.
- Use `sAPI_DialStop()` to cleanly disconnect before re-dialing or shutting down.
- Monitor the download/upload counters to detect abnormal data usage.
- The message queue-based data forwarding model allows asynchronous handling of incoming PPP data.
- In production, implement reconnection logic with exponential backoff if the dial fails.
- Ensure the PPP connection is stopped before the module enters sleep mode or powers off.
- The `SC_PppResp_msgq` and `SC_PppSend_msgq` are external globals; ensure they are properly initialized before use and cleaned up on exit.
