# demo_pm

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_pm.c

## Overview
This demo provides an interactive menu-driven interface for power management operations on the SIMCom A7672 module. It covers power key detection, power up/down event querying, ADC and battery voltage reading, auxiliary voltage setting, reset key status, and system power-off/reset functions. The APIs come primarily from the `sc_power.h` header.

## Main Features
- Detect power key press/release status
- Enable/disable power key shutdown with configurable long-press duration
- Query power-up reason and power-down event codes
- Read ADC channel voltage
- Read VBAT battery voltage
- Set VDD_AUX auxiliary voltage output (2500-3000mV)
- Get reset key status
- System power-off
- System reset

## Key Header Dependencies
- `sc_power.h` — Defines power key and reset status enums, power up/down reason enums, callback typedefs, and all power management API declarations
- `simcom_system.h` — Defines system functions including `sAPI_SysReset()`, system sleep, and version info (used indirectly)
- `simcom_os.h` — OS abstraction layer
- `simcom_common.h` — Common type definitions
- `uart_api.h` — UART read helpers for interactive input

## SDK APIs Observed
- `sAPI_GetPowerKeyStatus()` — Returns `POWER_KEY_STATUS` enum (SC_ONKEY_PRESSED or SC_ONKEY_RELEASED)
- `sAPI_SetPowerKeyOffFunc(onoff, powerKeyOffTime)` — Enable/disable power key shutdown; powerKeyOffTime is in 100ms units (e.g., 50 = ~5 seconds)
- `sAPI_GetPowerUpEvent()` — Returns `POWER_UP_REASON` enum (1=reset key, 2=power key, 3=software reset, 4=RTC alarm, 5=error)
- `sAPI_GetPowerDownEvent()` — Returns `POWER_DOWN_REASON` enum (1=long-press power key, 2=VRTC low, 3=overtemperature, 4=VIN LDO UV, 5=PMIC watchdog, 6=long-press reset key, 7=VIN overvoltage, 8=error)
- `sAPI_ReadAdc(channel)` — Read ADC voltage for given channel, returns value in millivolts
- `sAPI_ReadVbat()` — Read VBAT battery voltage, returns value in millivolts
- `sAPI_SetVddAux(voltage)` — Set VDD_AUX voltage (valid range 2500-3000mV); returns 0 on success, -2 if not supported
- `sAPI_GetResetKeyStatus()` — Returns `RESET_STATUS` enum (SC_RESET_PRESSED or SC_RESET_RELEASED)
- `sAPI_SysPowerOff()` — Power off the module immediately
- `sAPI_SysReset()` — Software reset the module

## Usage
1. Call `PMUDemo()` to enter the interactive menu.
2. Select option 1 to read the current power key state.
3. Select option 2 to enable or disable power key shutdown (input 0=disable, 1=enable; long-press duration is hardcoded to 50 x 100ms = ~5 seconds).
4. Select option 3 to print the last power-up reason and power-down event codes.
5. Select option 4 to read an ADC channel voltage (input channel number).
6. Select option 5 to read the VBAT battery voltage (displayed as volts with 3 decimal places).
7. Select option 6 to set VDD_AUX output voltage (input 2500-3000mV).
8. Select option 7 to read the reset key state.
9. Select option 8 for immediate system power-off.
10. Select option 9 for immediate system reset.
11. Option 99 returns to the previous menu.

## Implementation Notes
- `sAPI_SetPowerKeyOffFunc()` takes a `uint8_t` onoff parameter and a `uint32_t` powerKeyOffTime parameter. The time unit is approximately 100ms, so a value of 50 corresponds to about 5 seconds.
- `sAPI_ReadVbat()` returns voltage in millivolts. The demo formats it as "X.XXXV" by dividing by 1000.
- `sAPI_SetVddAux()` validates voltage range in the demo (2500-3000mV). A return of -2 indicates the platform does not support VDD_AUX.
- The demo includes a potential bug in the VDD_AUX setting: `sAPI_SetVddAux(voltage)` is called twice -- once for the success check and once for the -2 check. The second call would re-send the command unnecessarily.
- Power-up and power-down event codes help diagnose why the module started or shut down, useful for crash analysis and power fault diagnosis.
- `sAPI_SysPowerOff()` and `sAPI_SysReset()` are terminal operations that do not return; the demo will exit after these calls.

## Best Practices
- Always check power-up and power-down events at boot to detect abnormal shutdowns (e.g., watchdog expiry, overtemperature).
- Configure power key shutdown duration appropriately for the application (too short may cause accidental shutdowns, too long may frustrate users).
- Validate ADC channel numbers before reading to avoid invalid channel errors.
- Use VDD_AUX to power external peripherals only within the supported voltage range (2500-3000mV). Check the return value for -2 (unsupported) before relying on it.
- Register power key callbacks via `sAPI_PowerKeyRegisterCallback()` and `sAPI_LongPressPowerKeyRegisterCallback()` for interrupt-driven power key handling (not demonstrated in this demo but available in the header).
- Use `sAPI_SysReset()` instead of power cycling for cleaner module restarts.
- Monitor battery voltage periodically in battery-powered applications to trigger graceful shutdown before voltage drops below the minimum threshold.
- Use `sAPI_SetLongPressPowerKeyOffDisable()` to completely disable long-press shutdown in safety-critical applications.
