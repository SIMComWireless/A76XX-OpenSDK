# demo_lcd

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_lcd.c

## Overview

This demo provides an interactive interface for operating SPI-connected LCD panels on SIMCom modules. It supports opening/closing various LCD types, rendering 8x16 ASCII and 16x16 Chinese font bitmaps, clearing the screen with a solid color, and adjusting backlight brightness. The companion header `demo_lcd.h` contains all font data and LCD controller initialization command sequences.

## Main Features

- Open LCD with type selection (ST7735S 128x128, ST7735S 128x160, ST7789V 240x240, ST7789V 240x320, ST7567A 128x64, ST7796u 320x480, ST7789V SZGMD 240x240)
- Close LCD with backlight off
- Render mixed ASCII and Chinese character strings with foreground/background color
- Clear screen to a solid color (handles both segment-type and pixel-type LCDs)
- Set backlight brightness level (0-5)
- Font rendering supports both ASCII (8x16) and GB2312 Chinese characters (16x16)

## Key Header Dependencies

- `simcom_lcd.h` -- Provides LCD SPI types (`sc_spi_lcd_write_t`, `sc_lcd_data_t`), format enums (`SC_SPI_FORMAT_RGB565`), clock enums (`SC_SPI_LCD_CLK_26M`), and all `sAPI_Lcd*()` prototypes
- `demo_lcd.h` -- Contains `ASCII_8X16[]` bitmap data, LCD init command tables for 7 display controllers, `lcd_list[]` array of `sc_lcd_data_t` pointers, `sc_lcd_info_t` struct, `typFNT_GB16_t` Chinese font structs (`GB_16[]` for "welcome to use", `TN_GB_16[]` for "card"), and color constant macros (BLACK, WHITE, RED, etc.)

## SDK APIs Observed

- `sAPI_LcdOpen(lcd_data)` -- Initialize and open an LCD panel using a `sc_lcd_data_t` configuration structure
- `sAPI_LcdClose()` -- Close the LCD panel and release resources
- `sAPI_LcdWrite(data, x1, y1, x2, y2)` -- Write a pixel buffer to a rectangular region of the LCD
- `sAPI_LcdClearScreen(buf)` -- Fill the entire LCD screen with data from a buffer
- `sAPI_LcdSetBLPWM(level)` -- Set backlight brightness (0 = off, 1-5 = brightness levels)
- `sAPI_LcdWriteCmd(cmd)` -- Write a raw command byte to the LCD controller (used for segment-type LCDs)
- `sAPI_LcdWriteData(data, len)` -- Write raw data bytes to the LCD controller
- `sAPI_LcdPinConfig(...)` -- Configure LCD-related GPIO pins (used on A7670C_V6_02 only)
- `sAPI_Malloc(size)` / `sAPI_Free(ptr)` -- Dynamic memory allocation for pixel buffers

## Usage

1. Call `LcdDemo()` from the application task.
2. Select option 1 to open an LCD: enter the LCD type number (1-7).
3. Select option 2 to close the LCD.
4. Select option 3 to display fonts: Chinese "welcome to use" and "SIMCom" strings are rendered centered on screen.
5. Select option 4 to clear the screen to white.
6. Select option 5 to set brightness (0-5).
7. Select 99 to return.

## Implementation Notes

- The `show_font_16x16()` function handles both ASCII (< 128) and multi-byte Chinese characters. For ASCII, it uses the `ASCII_8X16[]` table (8 pixels wide). For Chinese, it searches `GB_16[]` for matching byte pairs and renders 16x16 pixel glyphs.
- The font rendering allocates a pixel buffer of `8 * 16 * 2 * len` bytes (RGB565 format, 2 bytes per pixel).
- Chinese characters consume 2 bytes per character and render 16 pixels wide; ASCII characters render 8 pixels wide.
- The `lcd_clean_screen()` function has two code paths: for segment-type LCDs (type 5, ST7567A), it writes directly via command/data sequences; for pixel-type LCDs, it allocates a full-screen buffer and calls `sAPI_LcdClearScreen()`.
- `get_lcd_info()` maps LCD type numbers to resolution: type 1 = 128x128, type 2 = 128x160, type 3 = 240x240, type 4 = 240x320, default = 240x320.
- The `lcd_list[]` array in `demo_lcd.h` provides indexed access to all 7 LCD configurations.
- Each LCD init sequence in `demo_lcd.h` contains the full register programming for that controller, including gamma correction, power settings, and display orientation.
- The entire demo is wrapped in `#ifdef SIMCOM_LCD_SUPPORT` and will not compile without that define.
- `sAPI_LcdPinConfig(1, 13, 1, 4, 1)` is conditionally called only for `SIMCOM_A7670C_V6_02` to configure the TE (Tearing Effect) pin.

## Best Practices

- Always call `sAPI_LcdClose()` when done with the LCD to release the SPI bus and GPIO pins.
- Set backlight to 0 before closing to avoid leaving the backlight on.
- Use `sAPI_Malloc()`/`sAPI_Free()` for frame buffers rather than large stack allocations, since the module has limited stack space.
- Ensure the LCD type matches the actual connected hardware -- incorrect initialization sequences can damage the display.
- For segment-type LCDs (ST7567A), use `sAPI_LcdWriteCmd()` and `sAPI_LcdWriteData()` directly instead of `sAPI_LcdClearScreen()`.
- Center text by calculating x/y positions based on the LCD width and string length.
- The `lcd_clean_screen()` function calls `sAPI_TaskSleep(30)` after clearing to allow the display controller to process.
