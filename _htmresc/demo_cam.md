# demo_cam

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_cam.c

## Overview

This demo provides an interactive menu for camera operations on SIMCom modules, including taking pictures, displaying live preview on LCD, showing saved images, barcode scanning (single and repeated), camera parameter adjustment (effect, contrast, exposure, mirror), and optional barcode decoding via the tiny_scanner library. It supports GC032A, BF30A2, and GC6153 camera sensors.

## Main Features

- Take a picture and save as NV12 file to the filesystem
- Live camera preview on LCD
- Display previously saved pictures (NV12 to RGB565 conversion)
- Set camera parameters: effect, contrast, exposure
- Set camera output image size
- Set camera mirror mode (horizontal, vertical, both, normal)
- Single barcode scan with result display on LCD
- Repeated barcode scan with live preview
- Optional decode-without-LCD mode (tiny_scanner integration)
- Optional decode-with-LCD mode
- Support for custom camera drivers via `sAPI_CamOpenEx()`

## Key Header Dependencies

- `simcom_cam.h` -- Provides camera sensor data structures, `sAPI_CamOpen()`, `sAPI_CamClose()`, `sAPI_StartShowLCD()`, `sAPI_StopShowLCD()`, `sAPI_GetCamBuf()`, `sAPI_CamParaSet()`, `sAPI_CamScanBarcode()`, `sAPI_YUVtoRGBZoomOut()`, `sAPI_CamOpenEx()` prototypes
- `simcom_cam_common.h` -- Defines frame size enums (96x96 to 1600x1200), `framesize_info_t` struct, effect enums (`HAL_EFFECT_*`), contrast enums (`CAM_CONTRAST_*`), exposure enums (`CAM_EV_*`), mirror enums (`HAL_MIRROR_*`)
- `demo_lcd.h` -- Provides LCD functions and font data used for displaying decode results
- `simcom_lcd.h` -- LCD API for preview display
- `simcom_file.h` -- File I/O (`sAPI_fopen`, `sAPI_fwrite`, `sAPI_fread`, `sAPI_fclose`, `sAPI_fsize`)

## SDK APIs Observed

- `sAPI_CamOpen()` -- Open the default camera sensor
- `sAPI_CamClose()` -- Close the camera
- `sAPI_CamOpenEx(&cam_data)` -- Open camera with a custom sensor driver (e.g., `CAM_GC032a_spi`)
- `sAPI_StartShowLCD()` -- Start live camera preview on the connected LCD
- `sAPI_StopShowLCD()` -- Stop live camera preview
- `sAPI_GetCamBuf(func, callback)` -- Capture a camera frame; `func=0` for single capture, `func=1` for decode mode; callback receives the image buffer pointer
- `sAPI_YUVtoRGBZoomOut(rgbBuf, rgbW, rgbH, yuvBuf, yuvW, yuvH)` -- Convert YUV422/NV12 camera data to RGB565 with scaling to fit the LCD
- `sAPI_CamParaSet(param_mode, pValue)` -- Set camera parameters: mode 1=effect, 2=contrast, 3=exposure, 4=image size, 5=mirror
- `sAPI_CamScanBarcode(mode, preview_cb, result_cb)` -- Scan barcode; `mode=0` for single scan, `mode=1` for repeated scan; preview callback for rendering, result callback for decoded string
- `sAPI_LcdOpen()`, `sAPI_LcdClose()`, `sAPI_LcdWrite()`, `sAPI_LcdSetBLPWM()` -- LCD operations for preview display
- `sAPI_fopen()`, `sAPI_fwrite()`, `sAPI_fread()`, `sAPI_fclose()`, `sAPI_fsize()` -- File operations for saving/loading NV12 images

## Usage

1. Call `CamDemo()` from the application task.
2. First select the LCD type (1-4) and camera sensor type (1=GC032A, 2=BF30A2, 3=GC6153).
3. Choose from the menu:
   - Option 1: Take a picture -- opens camera, captures one frame via callback `save_picture()`, saves to `c:/image.nv12`, closes camera.
   - Option 2: Live preview -- opens both camera and LCD, runs preview for 2 seconds, then stops.
   - Option 3: Show saved picture -- reads `c:/image.nv12`, converts YUV to RGB565, displays on LCD.
   - Option 6: Set parameters -- cycles through effect, contrast, exposure settings while previewing.
   - Option 7: Set image size -- opens camera at different resolutions.
   - Option 8: Set mirror -- cycles through H, V, HV, normal mirror modes.
   - Option 9: Single barcode scan -- scans once, displays result on LCD via `show_font_16x16()`.
   - Option 10: Repeated barcode scan -- continuous preview with barcode detection.

## Implementation Notes

- Image data is stored in NV12 format (Y plane followed by interleaved UV plane), with size `width * height * 3/2` bytes.
- Camera sensor parameters are pre-defined in `gc032a_set_para[9][2]`, `gc6153_set_para[9][2]`, and `bf30a2_set_para[9][2]` arrays, where each row is `{param_mode, value}`.
- The `zbar_decode_test()` callback converts YUV to RGB565 and writes directly to the LCD for live preview.
- The `recv_decode_result()` callback displays the decoded barcode string using `show_font_16x16()` with text wrapping.
- Barcode decode requires 120KB heap allocation for the `tiny_scanner` library.
- When `USER_CAM_DRIVER` is defined, `sAPI_CamOpenEx(&CAM_GC032a_spi)` is used instead of `sAPI_CamOpen()` to load a custom camera driver.
- The entire demo is wrapped in `#ifdef SIMCOM_CAMERA_SUPPORT`.
- The API reference table at the bottom of the file (lines 651-737) documents all camera parameter modes and values.

## Best Practices

- Always pair `sAPI_CamOpen()` with `sAPI_CamClose()` to release camera resources.
- Set camera parameters (size, effect) BEFORE calling `sAPI_CamOpen()` for image size changes, as noted in the API docs.
- Free all dynamically allocated buffers (`cam_buf`, `lcd_buf`) after use.
- For barcode scanning, allocate the decode heap once and reuse it.
- Use `sAPI_YUVtoRGBZoomOut()` for format conversion -- do not attempt manual YUV-to-RGB conversion.
- Ensure the LCD is opened before starting `sAPI_StartShowLCD()`.
- The NV12 file size is `cam_w * cam_h * 3/2` -- verify sufficient filesystem space before writing.
- For repeated barcode scanning, the preview callback should be lightweight to maintain frame rate.
