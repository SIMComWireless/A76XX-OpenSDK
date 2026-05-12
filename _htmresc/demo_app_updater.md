# demo_app_updater

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_app_updater.c

## Overview

This demo demonstrates how to update the customer application firmware by reading a `customer_app.bin` file from the module's file system, writing it to the app package partition, and verifying it with CRC. This is the local (file-system-based) counterpart to the remote download approach in demo_app_download.

## Main Features

- Read a customer_app.bin file from the file system (C:/)
- Write the file contents to the app package partition in chunks
- Verify the written package with CRC
- Handle path switching between C:/ and C:/simdir (platform-dependent)

## Key Header Dependencies

- `sc_app_update.h` -- Provides app package management APIs: sAPI_AppPackageOpen, sAPI_AppPackageWrite, sAPI_AppPackageClose, sAPI_AppPackageCrc, SCAppPackageInfo
- `simcom_file.h` -- Provides sAPI_fopen, sAPI_fread, sAPI_fclose, sAPI_FsSwitchDir for file system access
- `simcom_os.h` -- Provides sAPI_Malloc, sAPI_Free
- `userspaceConfig.h` -- Provides platform configuration macros (HAS_DEMO_FS, FALCON_1803_PLATFORM)

## SDK APIs Observed

- `sAPI_FsSwitchDir(file_name, direction)` -- Switch the file path between C:/ and C:/simdir. Direction: DIR_C_TO_SIMDIR (0) or DIR_SIMDIR_TO_C (1). This is needed because the app binary must be in the simdir for the file system APIs to access it on some platforms.
- `sAPI_fopen(path, mode)` -- Open the customer_app.bin file for reading.
- `sAPI_fread(buffer, size, count, fp)` -- Read data from the file in 1024-byte chunks.
- `sAPI_fclose(fp)` -- Close the file after reading.
- `sAPI_AppPackageOpen("w")` -- Open the app package partition for writing.
- `sAPI_AppPackageWrite(data, size)` -- Write a chunk of data to the partition.
- `sAPI_AppPackageClose()` -- Close the partition after all data is written.
- `sAPI_AppPackageCrc(&pInfo)` -- Verify the CRC of the written package.

## Usage

The `AppUpdateDemo()` function runs the update process:

1. Opens `c:/customer_app.bin` for reading.
2. Opens the app package partition with `sAPI_AppPackageOpen("w")`.
3. Reads the file in 1024-byte chunks and writes each chunk with `sAPI_AppPackageWrite`.
4. When EOF is reached, closes the package with `sAPI_AppPackageClose()`.
5. Verifies the package CRC with `sAPI_AppPackageCrc(&pInfo)`.
6. On success, prints "app write suc --> reset --> update!".

The demo requires `HAS_DEMO_FS` to be defined. If not, it prints "NO support HAS_DEMO_FS" and returns.

## Implementation Notes

- On non-1803 platforms, `sAPI_FsSwitchDir(APP_FILE_NAME, DIR_C_TO_SIMDIR)` is called to move the binary path from C:/ to C:/simdir. The 1803 platform does not have a simdir directory, so this step is skipped with `#ifndef FALCON_1803_PLATFORM`.
- The file is read in 1024-byte chunks into a dynamically allocated buffer (`malloc(1024)`).
- The demo uses standard `malloc`/`free` rather than `sAPI_Malloc`/`sAPI_Free` for the read buffer.
- Error handling uses `goto error` for cleanup, ensuring the file is closed and memory is freed even on failure.
- The `SCAppPackageInfo` structure is initialized to zero and populated by `sAPI_AppPackageCrc` with `hasPackage`, `binSize`, and `crcValue`.
- The demo does NOT reset the module after a successful update. The caller (or a separate function) must call `sAPI_SysReset()` to complete the update.
- The `APP_FILE_NAME` is defined as `"customer_app.bin"`.

## Best Practices

- Always verify the file exists and can be opened before attempting to write to the partition.
- Use the correct file path: on non-1803 platforms, the file must be in the simdir, so call `sAPI_FsSwitchDir` appropriately.
- Verify the CRC after writing to ensure data integrity before resetting.
- Handle all error cases: file open failure, partition open failure, write failure, CRC failure.
- Free allocated memory and close file handles in all code paths (including error paths).
- The `customer_app.bin` must be a valid firmware binary. Writing an invalid binary may brick the device.
- For production, consider adding file size validation before starting the update process.
- The chunk size (1024 bytes) balances memory usage and I/O efficiency. Larger chunks reduce system call overhead but increase memory usage.
