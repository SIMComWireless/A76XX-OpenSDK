# demo_ftps

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_ftps.c

## Overview

This file provides an interactive, menu-driven FTP/FTPS client demonstration for the SIMCom A7672 module. It implements a comprehensive FTP(S) operation set including session management, directory operations, file transfer (download/upload to flash and buffer), file deletion, and automated stress testing. All API results are delivered asynchronously through a message queue.

## Main Features

- Interactive UART-based menu for FTP/FTPS operations
- Full FTP lifecycle: Init, Login, Logout, DeInit
- Directory operations: Get current directory, List, Change directory, Create directory, Delete directory
- File operations: Delete file, Get file size, Download file (to flash), Download file to buffer, Upload file
- Support for multiple transfer protocols: FTP, FTPS (SSL/TLS), Implicit FTPS
- Asynchronous response handling via message queue (`ftpsUIResp_msgq`)
- Chunked data reception for list and buffer-download operations
- Automated stress test integration (delegates to `demo_ftps_test.c`)

## Key Header Dependencies

- `simcom_ftps.h` -- Provides all FTPS API functions, `SCftpsLoginMsg` struct, `SCapiFtpsData` struct, error code enum (`SC_FTPS_ERROR_CODE`), file location enum, and data flag enum
- `simcom_os.h` -- Provides OS primitives: message queues
- `simcom_common.h` -- Provides common type definitions
- `simcom_debug.h` -- Provides `sAPI_Debug()` for debug logging
- `simcom_uart.h` -- Provides UART write functions
- `uart_api.h` -- Provides `UartReadValue()` and `UartReadLine()` for input

## SDK APIs Observed

- `sAPI_FtpsInit(type, msgQRef)` -- Initialize FTPS client and activate PDP context; `type` is `SC_FTPS_USB` or `SC_FTPS_UART`
- `sAPI_FtpsDeInit(type)` -- Stop FTPS client and deactivate PDP context
- `sAPI_FtpsLogin(msg)` -- Login to FTP server using `SCftpsLoginMsg` (host, port, username, password, serverType)
- `sAPI_FtpsLogout()` -- Logout from the FTP server
- `sAPI_FtpsGetCurrentDirectory()` -- Get current working directory on the server
- `sAPI_FtpsList(dir)` -- List directory contents (returns data in chunks via msgQ)
- `sAPI_FtpsChangeDirectory(dir)` -- Change current working directory
- `sAPI_FtpsCreateDirectory(dir)` -- Create a new directory
- `sAPI_FtpsDeleteDirectory(dir)` -- Delete a directory
- `sAPI_FtpsDeleteFile(file)` -- Delete a file on the server
- `sAPI_FtpsGetFileSize(file)` -- Get the size of a file on the server
- `sAPI_FtpsDownloadFile(file, loc)` -- Download a file to flash or SD card (`SC_FTPS_FILE_FLASH`, `SC_FTPS_FILE_SDCARD`)
- `sAPI_FtpsDownloadFileToBuffer(file, startPos)` -- Download a file to memory buffer with resume support
- `sAPI_FtpsUploadFile(file, loc, startPos)` -- Upload a file from flash or SD card with resume support
- `sAPI_FtpsTransferType(type)` -- Set transfer type (ASCII 'A' or Binary 'I')
- `sAPI_FtpsSetMode(mode)` -- Set transfer mode (0=PORT, 1=PASV)
- `sAPI_FtpsSslConfig(session, sslId)` -- Set SSL context for FTPS session
- `sAPI_MsgQCreate(ref, name, msgSize, queueSize, mode)` -- Create message queue
- `sAPI_MsgQRecv(ref, msg, timeout)` -- Receive response messages

## Usage

1. Call `FtpsDemo()` to enter the menu.
2. Follow the menu options:
   - **Option 1 (Init):** Create message queue and call `sAPI_FtpsInit(SC_FTPS_USB, ftpsUIResp_msgq)`. Wait for the init response.
   - **Option 2 (Login):** Enter server IP, port, username, and password. Populates `SCftpsLoginMsg` with `serverType=0` (plain FTP). Call `sAPI_FtpsLogin()`.
   - **Option 3 (Logout):** Call `sAPI_FtpsLogout()`.
   - **Option 4 (Get Directory):** Call `sAPI_FtpsGetCurrentDirectory()`. The directory string is returned via msgQ.
   - **Option 5 (List):** Enter directory path. Call `sAPI_FtpsList()`. Loop through msgQ responses handling `SC_DATA_RESUME` (data chunk) and `SC_DATA_COMPLETE` (end of list).
   - **Option 6 (Change Directory):** Enter new path. Call `sAPI_FtpsChangeDirectory()`.
   - **Option 7 (Mkdir):** Enter directory name. Call `sAPI_FtpsCreateDirectory()`.
   - **Option 8 (Delete File):** Enter filename. Call `sAPI_FtpsDeleteFile()`.
   - **Option 9 (Delete Folder):** Enter directory name. Call `sAPI_FtpsDeleteDirectory()`.
   - **Option 10 (Get Size):** Enter filename. Call `sAPI_FtpsGetFileSize()`. Size is returned via msgQ.
   - **Option 11 (Download):** Enter filename. Call `sAPI_FtpsDownloadFile(file, SC_FTPS_FILE_FLASH)`.
   - **Option 12 (Download to Buffer):** Enter filename. Call `sAPI_FtpsDownloadFileToBuffer()`. Loop through chunks.
   - **Option 13 (Upload):** Enter filename. Call `sAPI_FtpsUploadFile(file, SC_FTPS_FILE_FLASH, 0)`.
   - **Option 14 (DeInit):** Call `sAPI_FtpsDeInit(SC_FTPS_USB)`.
   - **Option 15 (Test):** Enter test count. Delegates to `FtpsTestDemoInit()`.
   - **Option 99:** Logout and DeInit before returning.

## Implementation Notes

- The `SCftpsLoginMsg` struct contains host (256 chars), username (256 chars), password (256 chars), port, and serverType (0=FTP, 1=FTPS SSL, 2=FTPS TLS, 3=Implicit FTPS).
- The `SCapiFtpsData` struct contains a `flag` field (`SC_DATA_RESUME` for data chunks, `SC_DATA_COMPLETE` for end of stream), `data` pointer, and `len`.
- For list and download-to-buffer operations, the code loops on `sAPI_MsgQRecv()` until `SC_DATA_COMPLETE` is received, processing each `SC_DATA_RESUME` chunk.
- Each chunk's `data` pointer must be freed separately after processing; the `SCapiFtpsData` struct must also be freed.
- The `GET_SRV_FROM_MSG_ID` macro extracts the service type from the message ID.
- File location enum: `SC_FTPS_FILE_FLASH=1`, `SC_FTPS_FILE_SDCARD=2`, `SC_FTPS_FILE_EXT_FLASH_FS=3`.
- The test option (15) integrates with `demo_ftps_test.c` for automated stress testing.

## Best Practices

- Always call `sAPI_FtpsInit()` before any other FTP operation and `sAPI_FtpsDeInit()` when done.
- Login before performing any file/directory operations; logout before deinitializing.
- For large file downloads, use `sAPI_FtpsDownloadFile()` to flash rather than `DownloadFileToBuffer` to avoid memory exhaustion.
- Use `startPos` parameter in download/upload for resume support on interrupted transfers.
- Set appropriate `serverType` for your server: 0 for plain FTP, 1-2 for explicit FTPS, 3 for implicit FTPS.
- Process chunked data quickly to avoid message queue overflow; the queue size is 8 by default.
- Always free data pointers returned in `SCapiFtpsData` after processing each chunk.
