# demo_ftps_test

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_ftps_test.c

## Overview

This file provides an automated FTP/FTPS stress test implementation for the SIMCom A7672 module. It runs a complete sequence of FTP operations (init, login, list, get directory, download file, download to buffer, upload file, get file size, logout, deinit) in a single function, and supports repeated execution through a semaphore-driven task loop. It is conditionally compiled with `FEATURE_SIMCOM_FTPS` and is designed for automated regression and stress testing.

## Main Features

- Fully automated FTP test sequence: Init -> Login -> List -> GetDir -> Download -> DownloadToBuffer -> Upload -> GetSize -> Logout -> DeInit
- Supports repeated test execution via configurable test count
- Semaphore-based task scheduling for test iteration control
- Chunked data reception for list and download-to-buffer operations
- Separated test logic (`FtpsDemoTest`) from scheduling logic (`sTask_ftpsDemoTestProcesser`)
- One-time initialization function `FtpsTestDemoInit()` callable from `demo_ftps.c`

## Key Header Dependencies

- `simcom_ftps.h` -- Provides all FTPS API functions, structures (`SCftpsLoginMsg`, `SCapiFtpsData`), error codes, and enums
- `simcom_os.h` -- Provides OS primitives: task creation, semaphore, message queue
- `simcom_common.h` -- Provides common type definitions
- `simcom_debug.h` -- Provides `sAPI_Debug()` for debug logging
- `simcom_uart.h` -- Provides UART write functions

## SDK APIs Observed

- `sAPI_FtpsInit(type, msgQRef)` -- Initialize FTPS client and PDP context
- `sAPI_FtpsDeInit(type)` -- Stop FTPS client and PDP deactivation
- `sAPI_FtpsLogin(msg)` -- Login to FTP server
- `sAPI_FtpsLogout()` -- Logout from FTP server
- `sAPI_FtpsList(dir)` -- List directory contents
- `sAPI_FtpsGetCurrentDirectory()` -- Get current working directory
- `sAPI_FtpsDownloadFile(file, loc)` -- Download file to flash
- `sAPI_FtpsDownloadFileToBuffer(file, startPos)` -- Download file to memory buffer
- `sAPI_FtpsUploadFile(file, loc, startPos)` -- Upload file from flash
- `sAPI_FtpsGetFileSize(file)` -- Get file size
- `sAPI_MsgQCreate(ref, name, msgSize, queueSize, mode)` -- Create message queue
- `sAPI_MsgQRecv(ref, msg, timeout)` -- Receive response messages
- `sAPI_SemaphoreCreate(ref, count, mode)` -- Create semaphore
- `sAPI_SemaphoreAcquire(ref, timeout)` -- Acquire semaphore
- `sAPI_SemaphoreRelease(ref)` -- Release semaphore
- `sAPI_TaskCreate(ref, stack, stackSize, priority, name, func, arg)` -- Create task
- `sAPI_TaskSleep(ticks)` -- Sleep task
- `sAPI_UartWrite(port, data, len)` -- Write data to UART

## Usage

1. This file is called from `demo_ftps.c` via `FtpsTestDemoInit()`.
2. `FtpsTestDemoInit()` creates a semaphore (`ftpsDemotestSema`) and a task (`ftpsDemoTestTask`) on first call.
3. On subsequent calls, it simply releases the semaphore to trigger another test run.
4. The test count (`ftpsTestTime`) is set by the user in `demo_ftps.c` before calling `FtpsTestDemoInit()`.
5. `sTask_ftpsDemoTestProcesser()` loops, acquiring the semaphore, running `FtpsDemoTest()`, and releasing the semaphore until `ftpsTestTime` reaches 0.
6. Each `FtpsDemoTest()` iteration creates a message queue, initializes FTPS, logs in, performs all file operations, logs out, and deinitializes.

## Implementation Notes

- The demo test uses hardcoded server credentials (host "000.000.000.000", port 6047, user "test", pass "test") that must be updated for actual testing.
- Network registration check code is present but commented out.
- The semaphore-based scheduling ensures tests run sequentially.
- The task stack is 4096 bytes with priority 100.
- For list and download-to-buffer operations, the code loops on `sAPI_MsgQRecv()` until `SC_DATA_COMPLETE` is received.
- `isCreated` is a static variable ensuring one-time initialization.

## Best Practices

- Update the hardcoded server credentials before running tests.
- Use a test FTP server with known files (e.g., "test.txt") to validate the test sequence.
- Monitor the `ftpsCurrentTime` output to track test progress during stress runs.
- Ensure adequate flash space exists for downloaded files during testing.
- Consider adding assertion checks on return codes for automated pass/fail determination.
