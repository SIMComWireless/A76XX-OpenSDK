# cus_urc (URC Processing Framework)

Location: SIMCOM_SDK_SET/sc_demo/V1/src/cus_urc.c

## Overview
`cus_urc` provides the Unsolicited Result Code (URC) processing framework for the SIMCom OpenSDK. It creates a dedicated message queue and task that receives asynchronous notifications from all core modules (phonebook, SMS, SIM, network, GPS/GNSS, audio, TTS, calls, AT responses, and UART wake events) and dispatches them to the appropriate handler logic.

## Main Features
- Registers a single URC message queue for all modules via `sAPI_UrcRefRegister` with `SC_MODULE_ALL`.
- Processes SMS events: new message indicator (`+CMTI`), flash messages (`+CMT`), status reports (`+CDS`), SMS full, SMS done.
- Processes SIM card events: CPIN ready, CPIN removed, PIN/PUK lock states.
- Processes network status events: network activation/deactivation, PDP context activation/deactivation (with CID parsing).
- Processes GPS/GNSS events: GPS info, GNSS info, raw NMEA data, AP flash data get/install, AGNSS data check.
- Processes voice call events: incoming ring, DTMF detection, call list changes.
- Processes audio events: playback start/stop, recording start/stop, file full.
- Processes TTS playback status events.
- Processes AT command responses forwarded via internal AT response mask.
- Supports UART RX wake event notification for sleep/wake management.
- Supports dual-SIM URC routing (SIM1/SIM2) when `FEATURE_SIMCOM_DUALSIM` is defined.
- Forwards SMS URC events to the SMS demo processing task via `g_sms_demo_urc_process_msgQ`.

## Key Header Dependencies
- `simcom_os.h` — Task creation (`sAPI_TaskCreate`), message queue APIs (`sAPI_MsgQCreate`, `sAPI_MsgQRecv`, `sAPI_MsgQSend`).
- `simcom_common.h` — Common SDK types, `SIM_MSG_T` message structure, URC module masks (`SC_URC_*_MASK`), URC event codes (`SC_URC_*`).
- `simcom_debug.h` — `sAPI_Debug` for logging URC events.
- `simcom_gps.h` — GPS/GNSS URC event definitions (conditional on `FEATURE_SIMCOM_GPS`).
- `simcom_uart.h` — UART write for printing URC data to UART output.
- `simcom_usb_vcom.h` — USB VCOM write for printing AT responses.

## SDK APIs Observed
- `sAPI_MsgQCreate(&gUrcMsgQueue, "URC QUEUE", sizeof(SIM_MSG_T), 20, SC_FIFO)` — creates the URC message queue.
- `sAPI_UrcRefRegister(gUrcMsgQueue, SC_MODULE_ALL)` — registers the queue to receive URC events from all modules.
- `sAPI_MsgQRecv(gUrcMsgQueue, &msg, SC_SUSPEND)` — blocks until a URC message arrives.
- `sAPI_TaskCreate(&gUrcProcessTask, ...)` — creates the URC processing task with stack size 3KB and priority 150.
- `sAPI_UartWrite(SC_UART, ...)` — prints URC data to UART for debug/demo purposes.
- `sAPI_UsbVcomWrite(...)` — prints internal AT responses to USB AT port.

## Usage
1. Call `sAPP_UrcTask()` at system startup (from `sc_application.c` or application init).
2. The function creates a 3KB-stack task running `sTask_UrcProcesser` at priority 150.
3. Inside the task loop, `sAPI_MsgQRecv` blocks until a URC message arrives.
4. Messages are dispatched by `msg.arg1` (module mask) to the appropriate switch-case handler.
5. Each handler extracts data from `msg.arg3`, processes it, and frees the buffer.
6. SMS events can be forwarded to the SMS demo task via `g_sms_demo_urc_process_msgQ`.

## Implementation Notes
- Uses a fixed-size message queue of 20 entries. Overflow may drop URCs under heavy load.
- The `msg.arg3` pointer is dynamically allocated by the core SDK and must be freed after processing.
- PDP deactivation parsing uses `sscanf` to extract CID values from strings like `"+CGEV: ME/NW DEACT cid"` or `"+CGEV: ME/NW DEACT p_cid,cid"`.
- Module-specific features are conditionally compiled: `FEATURE_SIMCOM_GPS`, `FEATURE_SIMCOM_DUALSIM`, `SC_DRIVER_GNSS_UNIC`, `SC_DRIVER_GNSS_ZKW`.
- UART RX wake handling is enabled by defining `UART_RX_WAKE_DEMO` and uses `simcom_system.h` for `SC_SYSTEM_SLEEP_FLAG`.

## Best Practices
- Keep URC handler callbacks lightweight; avoid blocking operations inside the URC processing loop.
- Use separate message queues and tasks for application-specific heavy processing of URC events (as the SMS demo does with `g_sms_demo_urc_process_msgQ`).
- Always check `msg.arg3` for NULL before accessing URC payload data.
- In production, implement proper error recovery for PDP deactivation events (e.g., automatic re-activation).
- Monitor message queue depth to avoid URC drops under high-throughput scenarios.
