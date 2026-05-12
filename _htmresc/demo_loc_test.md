# demo_loc_test

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_loc_test.c

## Overview
This demo provides an automated LBS test framework that runs in a dedicated background task. It is designed to be triggered from `demo_loc.c` and executes sequential LBS queries (lon/lat, detail address, error number, lon/lat+datetime) in a loop controlled by a configurable test iteration count. It uses semaphores for synchronization between the test trigger and test execution.

## Main Features
- Automated sequential LBS query for all four location types
- Network registration polling before query execution
- Background task-based test execution with semaphore synchronization
- Configurable test iteration count with automatic termination
- Sign-extended longitude/latitude handling (negative coordinate support)

## Key Header Dependencies
- `simcom_loc.h` — Defines `sAPI_LocGet()`, `SC_lbs_info_t` response struct, LBS error codes
- `simcom_os.h` — OS abstraction for tasks (`sAPI_TaskCreate`), semaphores, message queues, task sleep
- `simcom_network.h` — Provides `sAPI_NetworkGetCgreg()` for network registration status checking
- `simcom_common.h` — Common types (`SIM_MSG_T`, `SC_STATUS`)
- `simcom_debug.h` — Debug logging via `sAPI_Debug()`
- `simcom_uart.h` — UART communication

## SDK APIs Observed
- `sAPI_NetworkGetCgreg(&pGreg)` — Get GPRS registration status; returns 1 when registered
- `sAPI_MsgQCreate(&msgQ, name, msgSize, maxMsgs, policy)` — Create message queue for LBS responses
- `sAPI_MsgQRecv(msgQ, &msg, timeout)` — Block-receive LBS response
- `sAPI_LocGet(channel, msgQ, type)` — Request LBS data (type 1-4)
- `sAPI_Free(ptr)` — Free allocated response data
- `sAPI_TaskCreate(&taskRef, stack, stackSize, priority, name, entryFunc, arg)` — Create background test task
- `sAPI_SemaphoreCreate(&sema, initialCount, policy)` — Create semaphore for test synchronization
- `sAPI_SemaphoreAcquire(sema, timeout)` — Acquire semaphore (blocking)
- `sAPI_SemaphoreRelease(sema)` — Release semaphore to trigger next test iteration
- `sAPI_Debug(fmt, ...)` — Debug log output
- `sAPI_TaskSleep(ticks)` — Sleep task during network polling

## Usage
1. This module is not called directly by the user; it is triggered by `demo_loc.c` via `LbsTestDemoInit()`.
2. In `demo_loc.c`, the user selects menu option 5 ("test the API for LBS") and inputs the desired test iteration count.
3. `LbsTestDemoInit()` creates a background task (`sTask_lbsDemoTestProcesser`) and a semaphore (only once, guarded by `isCreated` flag).
4. The test task waits on the semaphore, then calls `LbsDemoTest()` which performs all four LBS queries sequentially.
5. After each full test cycle, `lbsTestTime` is decremented and `lbsCurrentTime` is incremented. When `lbsTestTime` reaches 0, the test completes.
6. Between iterations, the semaphore is re-released to allow the next cycle to proceed.

## Implementation Notes
- The test uses a single semaphore with initial count 1. `LbsTestDemoInit()` releases it to start, and `sTask_lbsDemoTestProcesser` re-releases it after each iteration (if more iterations remain).
- `LbsDemoTest()` performs all four LBS query types sequentially in a single call: type 1 (lon/lat), type 2 (detail address), type 3 (error number), type 4 (lon/lat+datetime).
- Network registration is checked via a while-loop in `LbsDemoTest()` that polls `sAPI_NetworkGetCgreg()` every 3000 ticks until CGREG reports 1 (registered).
- The test task stack is allocated as `1024 * 8` bytes with priority 100.
- Global variables `lbsCurrentTime` and `lbsTestTime` are shared between `demo_loc.c` and this file (declared as `extern`).
- Each LBS query type reuses the same message queue (`SC_LbstestResp_msgq`) created once at the start of `LbsDemoTest()`.
- Coordinate sign handling: `u8LngMinusFlag` and `u8LatMinusFlag` determine whether to prefix a minus sign to the longitude/latitude values (divided by 1,000,000).

## Best Practices
- Do not call `LbsDemoTest()` directly; use `LbsTestDemoInit()` to ensure the task and semaphore are properly initialized.
- The network registration polling loop has no timeout; in production, add a maximum retry count or timeout to avoid infinite looping if the network never registers.
- Monitor memory usage: each LBS response allocates an `SC_lbs_info_t` struct that must be freed after processing.
- Use appropriate test iteration counts; very high counts will generate significant network traffic and power consumption.
- The semaphore-based synchronization ensures sequential test execution, preventing concurrent LBS requests that could corrupt shared resources.
