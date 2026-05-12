# demo_sjdr

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_sjdr.c

## Overview
This demo provides a simple interactive menu-driven interface for Spurious Jamming Detection and Reporting (SJDR) functionality on the SIMCom A7672 module. It demonstrates configuring the jamming detection parameters, enabling the jamming detector, and receiving jamming status notifications via a registered callback function.

## Main Features
- Configure jamming detection parameters (period, MNL threshold, minimum channel count, detection status)
- Enable/disable jamming detection
- Receive jamming/no-jamming notifications via callback
- Status reporting via both debug output and UART

## Key Header Dependencies
- `simcom_sjdr.h` — Defines three jamming detection API functions: `sAPI_JDConfig()`, `sAPI_JDSet()`, and `sAPI_JDTopFlyConfig()`
- `simcom_common.h` — Common type definitions (`UINT8`, `UINT16`, `UINT32`)
- `simcom_debug.h` — Debug logging via `sAPI_Debug()`
- `simcom_uart.h` — UART communication
- `uart_api.h` — UART read helpers for interactive input

## SDK APIs Observed
- `sAPI_JDConfig(period, mnl, minch, detecstat, callBackFunc)` — Configure jamming detection parameters:
  - `period` (UINT16): Detection period
  - `mnl` (UINT16): MNL (Minimum Noise Level) threshold
  - `minch` (UINT16): Minimum channel count
  - `detecstat` (UINT8): Detection status flag
  - `callBackFunc`: Callback function pointer with signature `char (*callBackFunc)(UINT32 id)`
- `sAPI_JDSet(enable)` — Enable (1) or disable (0) jamming detection; returns `SC_SUCCESS` or failure
- `sAPI_JDTopFlyConfig(period, mnl, minsinr, minch, detecstat, callBackFunc)` — Extended TopFly configuration with additional `minsinr` (minimum SINR) parameter

## Usage
1. Call `JamDectDemo()` to enter the interactive menu.
2. Select option 1 ("Config JamDect"): configures the jamming detector with period=5, mnl=17, minch=5, detecstat=1, and registers `getJamDectStatusDemo` as the callback.
3. Select option 2 ("Enable JamDect"): enables the jamming detector via `sAPI_JDSet(1)`. If successful, the detector runs in the background and calls the registered callback when jamming status changes.
4. When jamming is detected or cleared, the callback fires with `Id=0` (no jamming) or `Id=1` (jamming detected).
5. Option 99 returns to the previous menu.

## Implementation Notes
- The callback function `getJamDectStatusDemo(UINT32 Id)` receives a jamming status ID: `JAMMING_NO_DETECT_ID (0)` means no jamming, `JAMMING_DETECT_ID (1)` means jamming is detected.
- The callback outputs status via both `sAPI_Debug()` and `PrintfResp()` (UART output).
- `sAPI_JDConfig()` must be called before `sAPI_JDSet()` to set the detection parameters first.
- The `sAPI_JDTopFlyConfig()` API is declared in the header but not used in this demo. It adds a `minsinr` (minimum Signal-to-Interference-plus-Noise Ratio) parameter compared to `sAPI_JDConfig()`.
- The demo configures with: period=5, mnl=17, minch=5, detecstat=1. These values represent the specific tuning for jamming detection sensitivity.
- The return type of `sAPI_JDConfig()` is `void` (no return value), while `sAPI_JDSet()` returns `unsigned int`.
- The callback return type is `char` and the demo returns 0 from the callback.

## Best Practices
- Call `sAPI_JDConfig()` first to set parameters, then `sAPI_JDSet(1)` to enable detection. The order matters.
- Tune the jamming detection parameters (period, mnl, minch) based on the deployment environment. Higher MNL thresholds reduce false positives but may miss actual jamming.
- The callback function should return quickly to avoid blocking the jamming detection processing.
- In production, log jamming events with timestamps for security auditing and diagnostics.
- Consider using `sAPI_JDTopFlyConfig()` if SINR-based detection is needed for more advanced interference detection.
- Disable the jamming detector when not needed to save power and reduce processing overhead.
- Jamming detection is particularly relevant for IoT deployments in environments with potential RF interference.
