# demo_spi

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_spi.c

## Overview

This demo provides three separate SPI test functions: `SpiDemo()` for basic SPI read/write operations using the new extended API, `SpiNorDemo()` for SPI NOR flash operations, and `SpiNandDemo()` for SPI NAND flash operations. The basic demo targets an XM25QU128B flash IC for device ID reading and status register manipulation.

## Main Features

- SPI bus initialization with configurable clock, mode, CS mode, and channel
- SPI flash device ID reading (both standard and DMA modes)
- SPI flash status register read/write
- SPI NOR flash init, read ID, sector erase, read, and write operations
- SPI NAND flash init, read ID, block erase, page read, page write, and YAFFS2 filesystem testing
- Support for both legacy (`sAPI_SPIConfigInit`) and new extended (`sAPI_SpiConfigInitEx`) initialization APIs

## Key Header Dependencies

- `sc_spi.h` -- Defines `SC_SPI_DEV` struct, `SC_SPI_CLOCK` enums (with `SPIDRV_SUPPORT` variant providing finer granularity), `SC_SPI_MODE` (4 SPI modes), `CS_MODE` (SSP/GPIO/Custom), `SC_SPI_ReturnCode`, and all `sAPI_Spi*()` and `sAPI_ExtNorFlash*()` prototypes
- `simcom_gpio.h` -- Used for custom CS pin control via `sAPI_GpioSetDirection()` and `sAPI_GpioSetValue()`

## SDK APIs Observed

- `sAPI_SpiConfigInitEx(&spiDev)` -- Initialize SPI with extended configuration (clock, mode, CS mode, channel index)
- `sAPI_SpiReadBytesEx(&spiDev, sendBuff, sendSize, revBuff, revSize)` -- Send command bytes then read response bytes in a single transaction
- `sAPI_SpiWriteBytesEx(&spiDev, sendBuff, sendSize)` -- Send command and data bytes
- `sAPI_SPIConfigInit(ssp_clk, ssp_mode)` -- Legacy SPI initialization (single call with clock and mode)
- `sAPI_ExtSpiReadBytes(sendBuff, sendSize, revBuff, revSize)` -- Legacy extended SPI read
- `sAPI_ExtNorFlashInit()` -- Initialize external NOR flash driver
- `sAPI_ExtNorFlashReadID(id)` -- Read NOR flash manufacturer/device ID
- `sAPI_ExtNorFlashSectorErase(offset, size)` -- Erase a sector of NOR flash
- `sAPI_ExtNorFlashRead(offset, size, buffer)` -- Read data from NOR flash
- `sAPI_ExtNorFlashWrite(offset, size, buffer)` -- Write data to NOR flash
- `extSpiNandInit()` -- Initialize external NAND flash
- `sAPI_SPINAND_Erase(offset, size, flashType)` -- Erase a NAND flash block
- `sAPI_SPINAND_Read(offset, buffer, size, flashType)` -- Read a page from NAND flash
- `sAPI_SPINAND_Write(offset, buffer, size, flashType)` -- Write a page to NAND flash
- `sAPI_yaffs_II_test()` -- Run YAFFS2 filesystem test on NAND flash (conditional on `YAFFS2` define)

## Usage

1. Call `SpiDemo()`, `SpiNorDemo()`, or `SpiNandDemo()` from the application task.
2. The SPI bus is automatically initialized at the start of `SpiDemo()` with channel 1, `SPI_MODE_PH0_PO0`, and `GPIO_MODE` CS.
3. For `SpiDemo()`: select option 1 to read device ID (sends 0x9F command), option 2 to read/write status register, option 3 for DMA-based ID read.
4. For `SpiNorDemo()`: first initialize (option 1), then read ID (option 2), erase a sector (option 3), write data (option 5), and read back (option 4).
5. For `SpiNandDemo()`: initialize (option 1), erase a block (option 3), write a page (option 5), read it back (option 4), or run YAFFS2 test (option 6).

## Implementation Notes

- The demo uses `#define NEW_SPI_API` to select between the new `sAPI_SpiConfigInitEx`/`sAPI_SpiReadBytesEx` API and the legacy `sAPI_SPIConfigInit`/`sAPI_ExtSpiReadBytes` API.
- When `SPIDRV_SUPPORT` is defined, the clock enum provides fine-grained options from 100 KHz to 52 MHz. Otherwise, a simpler 7-option enum is used (812 KHz to 52 MHz).
- `GPIO_MODE` CS keeps the chip-select low for the entire data stream, while `SSP_MODE` toggles CS per byte.
- The NOR flash demo uses a fixed `START_ADDR` of `4096*0` (sector 0). In production, choose a safe offset outside the firmware area.
- NOR flash read uses `sAPI_Malloc(256)` for the buffer, demonstrating dynamic allocation for flash operations.
- NAND flash operations use `BOOT_FLASH` as the flash type identifier.
- The NAND write demo performs erase-then-write-then-read-then-verify with `memcmp` comparison.
- SPI NOR flash functions are conditional on `SPINOR_SUPPORT` or `SC_DRIVER_NORFLASH` defines.
- NAND flash functions are conditional on `SIMCOM_SPINAND_FLASH_SUPPORT` define.

## Best Practices

- Always initialize the SPI bus before any read/write operations.
- Use `GPIO_MODE` CS for most applications; use `SSP_MODE` only if the slave device requires CS toggling between bytes.
- Verify flash ID after initialization to confirm the correct flash chip is detected.
- Erase before writing to NOR flash -- flash memory can only be written after erasing (bits go from 1 to 0 on write, from 0 to 1 on erase).
- Use page-aligned buffers and sizes for NAND flash operations.
- Free dynamically allocated buffers after use to avoid memory leaks.
- For NOR flash, avoid writing to the boot area (first few sectors) unless you know what you are doing.
- Check return values of all SPI and flash API calls.
