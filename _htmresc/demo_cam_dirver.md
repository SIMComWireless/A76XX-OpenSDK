# demo_cam_dirver

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_cam_dirver.c

## Overview

This file provides a complete custom camera sensor driver implementation for the GC032A CMOS image sensor connected via SPI 2-lane interface. It demonstrates how to register a new camera sensor driver with the SIMCom camera framework by defining all required initialization registers, resolution tables, I2C attributes, and sensor control callbacks (power, effect, contrast, exposure, framesize, mirror).

## Main Features

- Full GC032A sensor initialization register table for YUV422 output via SPI 2-lane SDR
- Sensor ID verification registers
- Stream on/off control
- I2C slave address and attribute configuration (8-bit address, 8-bit data, slave 0x21)
- 640x480 VGA resolution configuration
- Power control via GPIO (A7670 series) or LDO (A7630 series)
- Image effect control (normal, color invert, blackboard, whiteboard, antique, red, green, blue, black-white, negative)
- Contrast control (high, medium, low)
- Exposure compensation (9 levels from -4/3 to +4/3)
- Frame size control from 96x96 up to VGA (640x480)
- Mirror/flip control (horizontal, vertical, both, normal)
- Complete `sc_cam_data_t` driver registration struct

## Key Header Dependencies

- `simcom_cam.h` -- Provides `sc_cam_data_t`, `sc_cam_ctrl_t`, `sc_cam_global_config_t`, `cam_sensor_spec_ops`, `cam_regval_tab`, `cam_i2c_attr`, `cam_resolution` structs, and `sAPI_CamWriteReg()`, `sAPI_CamGetGlobalConfig()`, `sAPI_CamIspsizeSet()`, `sAPI_CamMclkOnOff()`, `sAPI_CamLdoOnOff()` prototypes
- `simcom_cam_common.h` -- Defines `FRAMESIZE_*` enums, `framesize_info_t`, `HAL_EFFECT_*`, `CAM_CONTRAST_*`, `CAM_EV_*`, `CAM_HAL_MIRROR_*` enums
- `simcom_gpio.h` -- Provides GPIO functions for camera power control pins

## SDK APIs Observed

- `sAPI_CamWriteReg(i2c_attr, reg, val)` -- Write a register value to the camera sensor via I2C
- `sAPI_CamGetGlobalConfig()` -- Get pointer to global camera configuration structure
- `sAPI_CamIspsizeSet(w, h, out_w, out_h)` -- Set ISP input/output image dimensions
- `sAPI_CamMclkOnOff(enable)` -- Enable or disable the camera master clock output
- `sAPI_CamLdoOnOff(enable, ldo_type)` -- Enable or disable camera LDO power (SC_CAM_AVDD, SC_CAM_IOVDD)
- `sAPI_GpioSetDirection(gpio, direction)` -- Set GPIO direction for camera power pins
- `sAPI_GpioSetValue(gpio, value)` -- Set GPIO level for camera power sequencing

## Usage

1. The file is conditionally compiled only when `SIMCOM_CAMERA_SUPPORT` is defined.
2. The driver is registered as a global struct `CAM_GC032a_spi` of type `sc_cam_data_t`.
3. To use the custom driver, call `sAPI_CamOpenEx(&CAM_GC032a_spi)` instead of `sAPI_CamOpen()` (as shown in `demo_cam.c` when `USER_CAM_DRIVER` is defined).
4. The camera framework automatically calls the registered callbacks: `GC032a_s_power()` for power sequencing, `GC032a_YUV_set_effect()`, `GC032a_YUV_set_contrast()`, etc. when parameters are set via `sAPI_CamParaSet()`.

## Implementation Notes

- **Power control** has two modes selected by `#define`:
  - `CAM_POW_CTRL_GPIO` (A7670 series): Controls PWDN, RST, AVDD, IOVDD via separate GPIOs. Power-on sequence: IOVDD up, AVDD up, MCLK on, PWDN high then low.
  - `CAM_POW_CTRL_LDO` (A7630 series): Uses internal LDO for AVDD/IOVDD via `sAPI_CamLdoOnOff()`. Only PWDN and RST are GPIO-controlled.
- **Register table** `gc032a_init_yuv422_spi2lan_nocrc_sdr[]` contains 200+ register writes covering system config, analog/CIS control, SPI interface setup, black level, ISP, AEC, shading, AWB, color correction, gamma, denoise, edge enhancement, and YCP processing.
- **I2C attributes**: Slave address is `0x21` (7-bit), both register address and data are 8-bit width. Note the address `0x42` is commented out (the 8-bit address variant).
- **Frame size**: Only up to VGA (640x480) is supported; larger sizes are capped. The window is centered within the VGA sensor array using row/column offset calculations.
- **`cam_regval_tab`** struct contains `{register_address, value}` pairs. The `cam_i2c_attr` struct specifies I2C address/data width and slave address.
- **`cam_sensor_spec_ops`** struct has 15 function pointer slots; only 5 are populated (power, effect, contrast, exposure, framesize, mirror). The rest are NULL.
- The `SC_CAM_INF_SPI` and `SC_SPI_2_LAN` constants in the driver struct specify the SPI 2-lane interface. The `0` values for `spi_sdr` and `spi_crc` indicate SDR mode without CRC.

## Best Practices

- Follow the exact power-on/power-off sequencing as shown in `GC032a_s_power()` -- incorrect sequencing can damage the sensor or cause initialization failures.
- Always set the global config (`sAPI_CamGetGlobalConfig()`) after successfully changing a parameter, so the framework tracks the current state.
- Use `sAPI_CamIspsizeSet()` after changing frame size to synchronize the ISP pipeline.
- When adding a new camera sensor, copy this file as a template and modify: register tables, I2C address, resolution table, and sensor ID registers.
- Verify the sensor ID after power-on by reading the ID registers and comparing against `GC032a_id[]`.
- The SPI 2-lane interface requires specific register configuration in the init table (registers 0x51-0x64 in page 3) -- do not modify these without understanding the SPI protocol.
- Register bank switching (via register 0xFE/page select) must be done before accessing registers in different pages.
