# demo_rtc

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_rtc.c

## Overview
This demo provides an interactive menu-driven interface for Real-Time Clock (RTC) and alarm management on the SIMCom A7672 module. It supports setting/getting the RTC time, configuring alarms, enabling/disabling alarms, registering alarm callbacks, and includes an alarm test mode that triggers periodic system resets. The demo also supports GPIO toggling on alarm events and UTC time retrieval.

## Main Features
- Set RTC time (format: YY/MM/DD,HH:MM:SS)
- Get current RTC time
- Set alarm time
- Get current alarm time
- Enable/disable alarm
- Register alarm callback function
- RTC alarm test with periodic resets
- Get UTC time
- GPIO toggling on alarm callback (conditional on `RTC_GPIO_TEST`)

## Key Header Dependencies
- `simcom_rtc.h` — Defines `t_rtc` time struct, alarm callback typedef (`AlCallback`), and all RTC API declarations
- `simcom_gpio.h` — GPIO configuration and control APIs (`sAPI_GpioConfig()`, `sAPI_GpioSetValue()`)
- `sc_power.h` — Power management functions (`sAPI_SysPowerOff()`, `sAPI_SysReset()`)
- `simcom_os.h` — OS abstraction layer
- `simcom_common.h` — Common type definitions
- `simcom_debug.h` — Debug logging via `sAPI_Debug()`
- Board-specific GPIO headers (e.g., `A7670C_MANS_LANS_GPIO.h`) — GPIO pin definitions per hardware variant

## SDK APIs Observed
- `sAPI_SetRealTimeClock(&timeval)` — Set the RTC time; returns 0 on success, negative on failure. Year range: 1970-2069.
- `sAPI_GetRealTimeClock(&timeval)` — Get the current RTC time into `t_rtc` struct
- `sAPI_RtcSetAlarm(&alarmval)` — Set the alarm time
- `sAPI_RtcGetAlarm(&alarmval)` — Get the current alarm time
- `sAPI_RtcEnableAlarm(onoff)` — Enable (1) or disable (0) the alarm
- `sAPI_RtcRegisterCB(callback)` — Register a callback function to be invoked when the alarm fires
- `sAPI_GetUtcTime(&timeval)` — Get UTC time into `t_rtc` struct
- `sAPI_SysPowerOff()` — Power off the module (used in alarm test to enter deep sleep)
- `sAPI_SysReset()` — Software reset the module (used in alarm test)
- `sAPI_GpioConfig(pin, config)` — Configure GPIO pin (direction, pull, ISR)
- `sAPI_GpioSetValue(pin, value)` — Set GPIO output level (0 or 1)

## Usage
1. Call `RTCDemo()` to enter the interactive menu.
2. If `RTC_GPIO_TEST` is defined, GPIO_02 is configured as an output with pull-up during initialization.
3. Select option 1 ("Set time"): input time in format `YY/MM/DD,HH:MM:SS` (e.g., `21/03/08,09:36:00`). The year is converted: >=70 maps to 1900s, <70 maps to 2000s.
4. Select option 2 ("Get time"): displays current RTC time.
5. Select option 3 ("Set alarm"): input alarm time in the same format.
6. Select option 4 ("Get alarm"): displays current alarm time.
7. Select option 5 ("Enable alarm"): input 1 to enable or 0 to disable the alarm.
8. Select option 6 ("Register alarm cb"): registers the `cbfunc` callback which toggles GPIO_02 on each alarm event.
9. Select option 7 ("Alarm test"): input alarm interval in minutes. Sets the alarm to current time + 1 minute, registers `alarmTest` as the callback, enables the alarm, then calls `sAPI_SysPowerOff()` to enter deep sleep. The module wakes on alarm, the callback prints the RTC/alarm times, then calls `sAPI_SysReset()`.
10. Select option 8 ("Get UTC time"): retrieves and displays UTC time.
11. Option 99 returns to the previous menu.

## Implementation Notes
- The `t_rtc` struct contains: `tm_sec` (0-59), `tm_min` (0-59), `tm_hour` (0-23), `tm_mday` (1-31), `tm_mon` (1-12), `tm_year` (since 1970, displayed as full year), `tm_wday` (0=Sunday).
- The `setAlarmMin()` helper calculates alarm time by adding minutes to the current time with carry-over to hours, days, and months.
- The alarm test (option 7) is a terminal operation: it sets the alarm, enters deep sleep via `sAPI_SysPowerOff()`, wakes on alarm, resets via `sAPI_SysReset()`, and the `alarmTest` callback runs on each alarm interval.
- The `cbfunc` callback toggles GPIO_02 using a static counter (even=high, odd=low). This is useful for visual/alarm indicator testing.
- The `RTC_GPIO_TEST` conditional compiles GPIO initialization: configures GPIO_02 as output, pull-up, no edge detection. The GPIO header varies by board variant.
- The demo validates the input time carefully: checks leap year, month-day boundaries, hour/minute/second ranges, and timezone bounds (though timezone is always 0 in the input).
- UTC time retrieval uses a separate API (`sAPI_GetUtcTime`) that is distinct from the local RTC time functions.

## Best Practices
- Always validate user input for date/time before calling `sAPI_SetRealTimeClock()` to avoid setting invalid times.
- Use alarm callbacks for lightweight operations only; heavy processing in an interrupt context can cause system instability.
- The alarm test mode (option 7) is a destructive test that causes repeated resets. Use it only for validation, not in production.
- Register the alarm callback before enabling the alarm to ensure the callback is ready when the alarm fires.
- For production alarm-based wakeup, use `sAPI_RtcEnableAlarm(1)` combined with `sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_ENABLE)` to enter sleep mode and wake on alarm.
- GPIO toggling in the alarm callback provides a visual indicator; use it for debugging alarm timing accuracy.
- The RTC time persists across module resets but may lose accuracy over extended periods. Consider periodic NTP synchronization for time-critical applications.
