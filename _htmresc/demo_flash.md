# demo_flash

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_flash.c

## Overview

This demo provides interactive flash memory read, write, and erase operations on the ASR1603 module. It demonstrates direct raw flash access to the module's internal NOR flash, including sector-based erase, data write/read, app partition reading, and NVM (Non-Volatile Memory) partition operations.

## Main Features

- Erase a flash sector at a given offset
- Write raw data to flash at a specified offset
- Read raw data from flash at a specified offset
- Read from the application partition
- Query flash type and partition sizes (currently disabled via #if 0)
- NVM partition erase, write, and read (currently disabled via #if 0)

## Key Header Dependencies

- `sc_flash.h` -- Provides flash operation APIs: sAPI_EraseFlashSector, sAPI_WriteFlash, sAPI_ReadFlash, sAPI_ReadFlashAppPartition, sAPI_EraseFlashNvmPartition, sAPI_WriteFlashNvmPartition, sAPI_ReadFlashNvmPartition, sAPI_GetFlashSize, sAPI_FBF_Disable
- `simcom_os.h` -- Provides OS primitives (sAPI_Malloc, sAPI_Free, sAPI_Debug)
- `simcom_debug.h` -- Provides sAPI_Debug for debug output
- `uart_api.h` -- Provides UartReadValue for menu input

## SDK APIs Observed

- `sAPI_EraseFlashSector(offset, size)` -- Erase a flash sector. Offset must be 4K-aligned. Size should be a multiple of 4096 (4KB).
- `sAPI_WriteFlash(offset, buff, size)` -- Write raw data to flash at the given offset. The sector must be erased before writing.
- `sAPI_ReadFlash(offset, buff, size)` -- Read raw data from flash at the given offset.
- `sAPI_ReadFlashAppPartition(offset, buff, size)` -- Read from the application (firmware) partition.
- `sAPI_EraseFlashNvmPartition(offset, size)` -- Erase a sector in the NVM partition (disabled in demo).
- `sAPI_WriteFlashNvmPartition(offset, buff, size)` -- Write data to NVM partition (disabled in demo).
- `sAPI_ReadFlashNvmPartition(offset, buff, size)` -- Read data from NVM partition (disabled in demo).
- `sAPI_Debug(fmt, ...)` -- Debug logging.

## Usage

The demo presents an interactive menu via UART:

1. **Erase Sector** (Option 1): Erases flash at offset 0 (`SECTOR_1`), size 4096 bytes.
2. **Write Data** (Option 2): Writes a 16-byte test pattern (`0x01` through `0x16`) to offset 0. Writes 16 times in a loop to the same offset.
3. **Read Data** (Option 3): Reads 16 bytes from offset 0 and displays the hex values.
4. **Read App Partition** (Option 4): Reads 128 bytes from the application partition at offset 0 and displays the hex values.
5. **Flash Type / Sizes** (Option 5): Currently disabled. Would display flash type, cus_data partition size, customer_app partition size, and file system sizes.
6. **NVM Erase** (Option 6): Currently disabled. Would erase the NVM partition.
7. **NVM Write** (Option 7): Currently disabled. Would write test data to the NVM partition.
8. **NVM Read** (Option 8): Currently disabled. Would read and verify data from the NVM partition.
9. **Back** (Option 99): Return to parent menu.

## Implementation Notes

- Flash operations work on raw flash memory, not the file system. The file system (C:/ and D:/) is a separate abstraction layer.
- `SECTOR_1` is defined as offset 0, `SECTOR_2` as offset 4096. `ERASE_SIZE` is 4096 (4KB sector size).
- Before writing to flash, the sector must be erased first. Writing to un-erased flash will produce incorrect data.
- The `#if 0` disabled code blocks (lines 59-140) show an older version of the demo with error-condition testing (negative size, out-of-range, NULL pointer, unaligned offset) and a struct-based test pattern.
- NVM partition operations are conditionally compiled and currently disabled.
- The `flash_test` struct (6 ints = 24 bytes) is defined for potential use with NVM tests.
- Flash is a limited-write-endurance medium; excessive writes will degrade the flash over time.

## Best Practices

- Always erase the target sector before writing. Use `sAPI_EraseFlashSector(offset, size)` with a 4K-aligned offset.
- Verify data after writing by reading it back and comparing with `memcmp`.
- Minimize flash write cycles to preserve flash longevity. Use the NVM partition for frequently updated data if available.
- Use `sAPI_ReadFlashAppPartition` to read firmware image data, not `sAPI_ReadFlash` with arbitrary offsets, to avoid corrupting the app area.
- Check return values of all flash operations. Non-zero returns indicate errors.
- Ensure the data buffer pointer is not NULL and the size is positive before calling flash APIs.
- Be aware that flash offsets are absolute addresses within the flash chip; incorrect offsets can overwrite critical firmware data.
