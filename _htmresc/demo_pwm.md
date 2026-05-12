# demo_pwm

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_pwm.c

## Overview

This demo provides a simple interactive menu for enabling and disabling PWM (Pulse Width Modulation) output on any of the four available PWM channels. It demonstrates the two configuration methods available in the SDK: a simplified API with default 32K clock source, and an extended API allowing explicit clock source selection.

## Main Features

- Open (enable) PWM output on channels 1-4
- Close (disable) PWM output on channels 1-4
- Interactive channel selection per operation
- Two PWM configuration methods demonstrated in comments

## Key Header Dependencies

- `simcom_pwm.h` -- Defines `PWM_DEVICE_NUM` enum (DEVICE_1-4, MAX), `PWM_SWITCH` (ON/OFF), `PWM_CLK_SRC` (13M/32K), `PWM_ReturnCode` enum, `SC_PWM_CONFIG` struct, and `sAPI_PWMConfig()` / `sAPI_PWMConfigEx()` prototypes

## SDK APIs Observed

- `sAPI_PWMConfig(pwm_num, on_off, frq_div, high_level_cnt, total_cnt)` -- Configure and enable/disable a PWM channel with the default 32K clock source
- `sAPI_PWMConfigEx(&pwmDev)` -- Extended configuration allowing explicit clock source selection (13M or 32K) via the `SC_PWM_CONFIG` struct

## Usage

1. Call `PwmDemo()` from the application task.
2. Select option 1 to open a PWM channel, then choose channel 1-4.
3. Select option 2 to close a PWM channel, then choose channel 1-4.
4. The demo uses fixed parameters: `frq_div=64`, `high_level_cnt=7`, `total_cnt=14`.
5. Select 99 to return to the previous menu.

## Implementation Notes

- The frequency and duty cycle calculation formula is documented in the code comments:
  - `period = frq_div * total_cnt / 32000` (seconds)
  - `frequency = 32000 / (frq_div * total_cnt)` (Hz)
  - `duty = high_level_cnt / total_cnt`
- With the demo's default parameters (frq_div=64, high_level_cnt=7, total_cnt=14):
  - Theory frequency: 32000 / (64 * 14) = 35.71 Hz
  - Actual measured: ~34.15 Hz
  - Duty cycle: 7/14 = 50%
- The commented-out extended configuration method (`sAPI_PWMConfigEx`) shows how to use `PWM_CLK_SRC_13M` for higher-frequency PWM output. With the 13M clock, the same parameters yield approximately 14.5 KHz.
- The `PWM_DEVICE_NUM` enum starts at 0 (`SC_PWM_DEVICE_1 = 0`), but the demo's sub-menu options use 1-based numbering and map them to the enum.
- The demo uses `goto` labels (`reSelect`, `reSelectPwmOpenDevice`, `reSelectPwmCloseDevice`) for menu re-prompting.
- The `#if 1` / `#else` block shows both the interactive menu version and a direct-configuration version (compiled out).

## Best Practices

- Verify `PWM_RC_OK` return value from `sAPI_PWMConfig()` or `sAPI_PWMConfigEx()`.
- Use `sAPI_PWMConfigEx()` when you need the 13 MHz clock source for higher-frequency PWM output.
- Ensure `frq_div` is in the range (0, 64] as documented.
- Ensure `high_level_cnt <= total_cnt` for valid duty cycle.
- Disable PWM channels when not in use to save power.
- Be aware that PWM pins are shared with other functions on some modules -- consult the module datasheet for pin mux configuration.
- For precise frequency control, prefer the 13M clock source with appropriate divider values.
