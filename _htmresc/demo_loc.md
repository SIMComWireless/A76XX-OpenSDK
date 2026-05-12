# demo_loc

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_loc.c

## Overview
This demo provides an interactive menu-driven interface for LBS (Location-Based Service) functionality, obtaining the module's approximate location by using cellular network cell tower information rather than GPS. It supports querying longitude/latitude, detail address, error codes, and combined location with date/time, plus an automated test mode.

## Main Features
- Get longitude and latitude via LBS cell tower triangulation
- Get detailed address information
- Get LBS error number/status
- Get combined longitude, latitude, and date/time
- Automated LBS test with configurable iteration count
- Asynchronous response handling via message queue

## Key Header Dependencies
- `simcom_loc.h` — Defines LBS error codes (`SC_lbs_err_e`), query type enums (`SC_LBS_DEMO_TYPE`), response struct (`SC_lbs_info_t`), and `sAPI_LocGet()` API
- `simcom_os.h` — OS abstraction for message queues (`sAPI_MsgQCreate`, `sAPI_MsgQRecv`), tasks, semaphores
- `simcom_common.h` — Common type definitions and status codes (`SC_STATUS`, `SIM_MSG_T`)
- `simcom_debug.h` — Debug logging via `sAPI_Debug()`
- `simcom_uart.h` — UART communication types

## SDK APIs Observed
- `sAPI_MsgQCreate(&msgQ, name, msgSize, maxMsgs, policy)` — Create a message queue for LBS response reception
- `sAPI_MsgQRecv(msgQ, &msg, timeout)` — Block-receive LBS response message from the queue
- `sAPI_LocGet(channel, msgQ, type)` — Request LBS location data; type 1=lon/lat, 2=detail address, 3=error number, 4=lon/lat+datetime
- `sAPI_Free(ptr)` — Free dynamically allocated response data
- `sAPI_Debug(fmt, ...)` — Debug log output
- `sAPI_TaskSleep(ticks)` — Sleep current task for network polling
- `sAPI_MsgQCreate` for test message queue creation
- `sAPI_TaskCreate` — Create the LBS test processing task
- `sAPI_SemaphoreCreate` / `sAPI_SemaphoreAcquire` / `sAPI_SemaphoreRelease` — Semaphore-based test synchronization

## Usage
1. Ensure the IMEI is set before using LBS (via AT+SIMEI=IMEI).
2. Insert a SIM card and wait for network registration.
3. Call `LbsDemo()` to enter the interactive menu.
4. Select option 1-4 for one-shot LBS queries. The demo creates a message queue, calls `sAPI_LocGet()`, then blocks on `sAPI_MsgQRecv()` to wait for the asynchronous response.
5. Select option 5 for automated testing: enter the number of test iterations, which spawns a background task that repeatedly queries all four LBS types.
6. The response data is cast from `msgQ_data_recv.arg3` to `SC_lbs_info_t*` and must be freed with `sAPI_Free()` after processing.

## Implementation Notes
- `sAPI_LocGet()` is asynchronous: the request is submitted and the result arrives later via the message queue with `msg_id == SC_SRV_LBS`.
- The response struct `SC_lbs_info_t` includes `u8LngMinusFlag` and `u8LatMinusFlag` fields to indicate negative (west/south) coordinates. The raw `u32Lng`/`u32Lat` values are divided by 1,000,000 to obtain decimal degrees.
- The detail address type (type=2) returns raw hex bytes in `u8LocAddress` up to `u32AddrLen` bytes.
- The automated test mode (`LbsTestDemoInit` / `sTask_lbsDemoTestProcesser`) uses a semaphore-based loop with `lbsCurrentTime` and `lbsTestTime` counters. The test task runs in a separate thread and calls `LbsDemoTest()` from `demo_loc_test.c`.
- The global `SC_LbsResp_msgq` message queue is created lazily (only once) and reused across menu selections.
- Error handling checks `sub_data->u8ErrorCode` before processing; a non-zero error code indicates a failure.

## Best Practices
- Always verify IMEI is configured before calling LBS APIs; the demo prints a warning if not set.
- Free the `SC_lbs_info_t` pointer returned in `msgQ_data_recv.arg3` after each successful reception to avoid memory leaks.
- Use the message queue with `SC_SUSPEND` timeout for reliable blocking reception.
- For production, implement a timeout on `sAPI_MsgQRecv()` to avoid indefinite blocking if the LBS server is unreachable.
- The automated test mode is useful for stress testing but should not be run in production as it makes repeated network calls.
- Ensure network registration (CGREG=1) before issuing LBS requests; the test code in `demo_loc_test.c` handles this with a polling loop.
