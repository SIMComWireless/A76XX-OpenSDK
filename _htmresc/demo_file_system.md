# demo_file_system

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_file_system.c

## Overview

This demo demonstrates how to perform file system operations on the ASR1603-based SIMCom module. It provides a comprehensive interactive menu for writing, reading, deleting, renaming files, managing directories, querying disk size, and transferring files between the host (via UART) and the module's internal file system.

## Main Features

- Write a file to the file system with user-provided filename and data
- Read a file from the file system, including seek and re-read
- Delete a file and verify its removal via stat
- Rename a file and verify both old and new names
- Query disk size (total, free, used) on C:/ or D:/
- Create and remove directories
- List directory contents (open, read, close)
- Format a drive (D:/)
- Transfer a file from the host to the module (RX file) using a ring buffer with mutex and flag synchronization
- Transfer a file from the module to the host (TX file) via sequential reads

## Key Header Dependencies

- `simcom_file.h` -- Provides file system API types and function declarations (SCFILE, SCDIR, dirent_t) and all sAPI_fopen/fread/fwrite/fclose/stat/mkdir/opendir/readdir/closedir/remove/rename/fseek/ftell/fsize/GetSize/GetFreeSize/GetUsedSize/fformat functions
- `simcom_os.h` -- Provides OS primitives: sAPI_Malloc, sAPI_Free, sAPI_MutexCreate, sAPI_MutexLock, sAPI_MutexUnLock, sAPI_MutexDelete, sAPI_FlagCreate, sAPI_FlagSet, sAPI_FlagWait, sAPI_FlagDelete, sAPI_TaskCreate, sAPI_TaskSleep
- `simcom_debug.h` -- Provides sAPI_Debug for debug logging
- `uart_api.h` -- Provides UartReadValue, UartReadLine, UartRead for UART terminal input

## SDK APIs Observed

- `sAPI_fopen(fileName, mode)` -- Open/create a file (modes: "rb", "wb", "wb+", etc.)
- `sAPI_fclose(handle)` -- Close a file handle; returns 0 on success
- `sAPI_fwrite(buffer, size, num, handle)` -- Write data to a file; returns bytes written
- `sAPI_fread(buffer, size, num, handle)` -- Read data from a file; returns bytes read
- `sAPI_fseek(handle, offset, whence)` -- Seek to position in file
- `sAPI_ftell(handle)` -- Get current file position
- `sAPI_fsize(handle)` -- Get total file size
- `sAPI_stat(path, info)` -- Get file/directory metadata (name, size, type)
- `sAPI_remove(name)` -- Delete a file or directory
- `sAPI_rename(old, new)` -- Rename a file or directory
- `sAPI_mkdir(path, mode)` -- Create a directory
- `sAPI_opendir(path)` -- Open a directory for listing
- `sAPI_readdir(handle)` -- Read next directory entry (returns dirent_t pointer)
- `sAPI_closedir(handle)` -- Close directory handle
- `sAPI_fformat(drive)` -- Format a drive (e.g., "D:/")
- `sAPI_GetSize(disc)` -- Get total file system size
- `sAPI_GetFreeSize(disc)` -- Get free space on file system
- `sAPI_GetUsedSize(disc)` -- Get used space on file system
- `sAPI_Malloc(size)` / `sAPI_Free(ptr)` -- Module heap memory management
- `sAPI_MutexCreate(ref, mode)` -- Create a mutex
- `sAPI_MutexLock(ref, suspend)` / `sAPI_MutexUnLock(ref)` -- Lock/unlock mutex
- `sAPI_FlagCreate(ref)` -- Create an event flag group
- `sAPI_FlagSet(ref, flags, op)` -- Set flags on a flag group
- `sAPI_FlagWait(ref, flags, op, ret_flags, suspend)` -- Wait for flags
- `sAPI_TaskCreate(ref, stack, stackSize, priority, name, func, arg)` -- Create a task/thread
- `sAPI_enableDUMP()` -- Enable UART data dump mode (for file transfer)
- `sAPI_Debug(fmt, ...)` -- Print debug log (captured by CATSTUDIO)

## Usage

1. **Write File**: Select option 1, enter a filename (e.g., "D:/test_file.txt"), then enter the data string to write.
2. **Read File**: Select option 2, enter the filename. The demo reads the file, seeks to position 1, then re-reads to demonstrate seek behavior.
3. **Delete File**: Select option 3, enter the filename. The demo first calls sAPI_stat to show file info, then removes it and verifies removal.
4. **Rename File**: Select option 4, enter the old filename and new filename. The demo verifies old file exists, checks new name does not exist, renames, then confirms.
5. **Get Disk Size**: Select option 5, input 0 for C:/ or 1 for D:/. Displays total, free, and used sizes.
6. **Make Directory**: Select option 6, enter the directory path. Creates and verifies via stat.
7. **Remove Directory**: Select option 7, enter the directory path.
8. **List Files**: Select option 8, enter the directory path. Opens, iterates with readdir, prints name/size/type for each entry, then closes.
9. **Format Drive**: Select option 9, enter the drive (e.g., "D:/"). Formats then lists contents.
10. **Transfer File In (RX)**: Select option 10, enter destination filename and data length. A background task writes incoming UART data to the file via a 20KB ring buffer protected by a mutex and event flags.
11. **Transfer File Out (TX)**: Select option 11, enter the filename. Reads the file in 512-byte chunks and sends data to UART.
12. **Back**: Select 99 to return to the parent menu.

## Implementation Notes

- The file system uses paths prefixed with drive letters: `C:/` for internal flash, `D:/` for SD card.
- The RX file transfer feature uses a producer-consumer pattern: `sRecvProcesser()` reads from UART into a ring buffer, while `sTask_FsHandeProcesser()` (a separate task) reads from the ring buffer and writes to disk. Synchronization is done via `sAPI_MutexCreate` and `sAPI_FlagCreate/FlagSet/FlagWait`.
- The ring buffer (`pRXDataBuffer`) is 20KB and uses circular read/write pointers (`pDataR`, `pDataW`) with `rxDataFreeSize` tracking available space.
- Event flags (`RECEIVE_FLAG_MASK_SYNC`, `RECEIVE_FLAG_MASK_SUCCESS`, `RECEIVE_FLAG_MASK_FAIL`) coordinate between the reader and writer tasks.
- File write uses 128KB blocks (`READ_FILE_PACKET_SIZE`) to batch disk writes.
- The TX file reads in 512-byte (`SIM_FILE_OUTPUT_SIZE`) chunks and outputs to the serial terminal.
- The `cur_path_in` default path is `"C:/"`.

## Best Practices

- Always check return values of file operations (`sAPI_fopen`, `sAPI_fclose`, `sAPI_fwrite`, `sAPI_fread`) and handle errors gracefully.
- Use `sAPI_fopen` with "wb+" mode for write operations to create files that do not yet exist.
- Before writing a file, check available disk space with `sAPI_GetFreeSize` to avoid write failures.
- Use `sAPI_stat` before delete/rename to verify the file exists and obtain its metadata.
- Always close file handles (`sAPI_fclose`) and directory handles (`sAPI_closedir`) after use to prevent resource leaks.
- For large file transfers, use the ring buffer pattern with mutex and flag synchronization as demonstrated in the RX file transfer.
- Free all dynamically allocated memory (via `sAPI_Free`) after use, especially in error paths.
- Use `sAPI_enableDUMP()` before large UART data reception to ensure reliable data capture.
