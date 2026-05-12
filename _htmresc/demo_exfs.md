# demo_exfs

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_exfs.c

## Overview

This demo shows how to mount external flash storage as a file system on the ASR1603 module. It supports mounting external NOR flash using the LittleFS (LFS) file system and external NAND flash using the YAFFS file system. The mounting operations are performed in dedicated tasks to avoid blocking the main execution.

## Main Features

- Mount external NOR flash with LittleFS file system
- Mount external NAND flash with YAFFS file system
- Query the current external flash mount status

## Key Header Dependencies

- `simcom_exfsmount.h` -- Provides external flash mount APIs: sAPI_NorFlashMountLfs, sAPI_NandFlashMountYaffs, sAPI_ExFlashMountState, and types SC_ExFsMount_ReturnCode, SC_ExFsMount_State
- `sc_spi.h` -- Provides SPI interface definitions (used for external flash communication)
- `simcom_os.h` -- Provides OS primitives: sAPI_TaskCreate, sAPI_Malloc, sAPI_Debug
- `simcom_debug.h` -- Provides sAPI_Debug for debug logging
- `uart_api.h` -- Provides UartReadValue for menu input

## SDK APIs Observed

- `sAPI_NorFlashMountLfs()` -- Mounts the external NOR flash using the LittleFS file system. Returns SC_ExFsMount_ReturnCode indicating success or failure.
- `sAPI_NandFlashMountYaffs()` -- Mounts the external NAND flash using the YAFFS file system. Returns SC_ExFsMount_ReturnCode.
- `sAPI_ExFlashMountState()` -- Returns the current mount state as SC_ExFsMount_State.
- `sAPI_TaskCreate(ref, stack, stackSize, priority, name, func, arg)` -- Creates a task to run the mount operation asynchronously.
- `sAPI_Malloc(size)` -- Allocates stack memory for the mount task.

## Usage

The demo presents an interactive menu:

1. **Extern NorFlash mount LFS** (Option 1): Allocates a 60KB stack, creates a task (`norflashmountlfsProcesser`) that calls `sAPI_NorFlashMountLfs()`. The task runs at priority 101.
2. **Extern NandFlash mount YAFFS** (Option 2): Allocates a 60KB stack, creates a task (`nandflashmountlfsProcesser`) that calls `sAPI_NandFlashMountYaffs()`.
3. **Obtain current mount status** (Option 3): Calls `sAPI_ExFlashMountState()` and displays the result.
4. **Back** (Option 99): Return to parent menu.

## Implementation Notes

- Mount operations are performed in separate tasks (not inline) because they may take significant time and should not block the main UI task.
- Each mount task is allocated a 60KB stack via `sAPI_Malloc(60 * 1024)`. This is a substantial allocation to accommodate the file system driver's memory requirements.
- The task priority is set to 101, which is relatively low, meaning these tasks will run with lower scheduling priority.
- The task references (`norflashmountlfsProcesser`, `nandflashmountlfsProcesser`) are declared as global static variables.
- The menu option string for option 3 has a missing comma between the string and the next item (line 126), which would cause a compile-time string concatenation. This is a minor bug in the source.
- The YAFFS mount option has a commented-out message suggesting it was previously considered not fully developed.

## Best Practices

- Mount external flash only once during initialization or when explicitly requested. Repeated mounting without unmounting can cause issues.
- Allocate sufficient stack space for mount tasks (60KB as shown). Under-allocation can cause stack overflow and system crashes.
- Check the return code of `sAPI_NorFlashMountLfs()` and `sAPI_NandFlashMountYaffs()` before proceeding to use the mounted file system.
- After successful mounting, use the standard file system APIs (`sAPI_fopen`, `sAPI_fread`, `sAPI_fwrite`, etc.) to access files on the external flash, using the appropriate drive path.
- Query mount status with `sAPI_ExFlashMountState()` before performing file operations on external storage to ensure it is properly mounted.
- External flash operations are slower than internal flash; account for this in timing-critical applications.
