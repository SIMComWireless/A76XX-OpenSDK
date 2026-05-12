# demo_txrx_power

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_txrx_power.c

## Overview
This demo provides a simple interactive menu-driven interface for reading RF transmit (TX) and receive (RX) power levels on the SIMCom A7672 module. It is conditionally compiled only for the `FALCON_1803_PLATFORM` and uses the `sapi_txrx_power.h` header for API definitions. The demo allows querying the current TX and RX power in dBm.

## Main Features
- Get current RX (receive) power level in dBm
- Get current TX (transmit) power level in dBm
- Interactive menu with error reporting

## Key Header Dependencies
- `sapi_txrx_power.h` — Defines TX/RX power API functions (`sAPI_getRxPower()`, `sAPI_getTxPower()`) and return code enum (`SC_TXRX_POWER_RETURN_ERRCODE`). Note: This header was not found in the SDK include directory, suggesting it is platform-specific for FALCON_1803.
- `simcom_os.h` — OS abstraction layer
- `simcom_common.h` — Common type definitions
- `simcom_debug.h` — Debug logging via `sAPI_Debug()`
- `simcom_uart.h` — UART communication
- `uart_api.h` — UART read helpers

## SDK APIs Observed
- `sAPI_getRxPower(&rxPwrVal)` — Get current RX power level; stores result as `INT16` in dBm. Returns `SC_TXRX_POWER_RETURN_SUCCESS` on success.
- `sAPI_getTxPower(&txPwrVal)` — Get current TX power level; stores result as `INT16` in dBm. Returns `SC_TXRX_POWER_RETURN_SUCCESS` on success.

## Usage
1. The demo is only compiled when `FALCON_1803_PLATFORM` is defined.
2. Call `txRxPowerDemo()` to enter the interactive menu.
3. Select option 1 ("GetRxPower"): reads the current RX power and displays it in dBm.
4. Select option 2 ("GetTxPower"): reads the current TX power and displays it in dBm.
5. Option 99 returns to the previous menu.

## Implementation Notes
- The entire file is wrapped in `#ifdef FALCON_1803_PLATFORM`, making it exclusive to the Falcon 1803 platform variant.
- `sAPI_getRxPower()` and `sAPI_getTxPower()` both take a pointer to `INT16` and return an error code enum. On success (`SC_TXRX_POWER_RETURN_SUCCESS`), the power value is written to the pointed variable.
- The return code type `SC_TXRX_POWER_RETURN_ERRCODE` includes at minimum `SC_TXRX_POWER_RETURN_SUCCESS` and `SC_TXRX_POWER_RTEURN_UNKNOW` (note the typo in the enum name).
- Power values are displayed in dBm format (e.g., "Get rx power: -85 dbm").
- The response buffer is 120 bytes, sufficient for the formatted output strings.
- The demo is compact at 107 lines and serves as a simple RF power diagnostic tool.

## Best Practices
- Use TX/RX power readings to diagnose RF signal quality issues and optimize antenna placement.
- RX power indicates received signal strength; lower (more negative) values indicate weaker signal.
- TX power indicates the module's transmission power level; higher values indicate the module is compensating for poor signal by transmitting at higher power.
- Monitor TX power in battery-powered applications as higher TX power directly increases current consumption.
- Use these readings alongside network signal quality metrics (CSQ, RSRP, RSRQ, SINR) for comprehensive RF diagnostics.
- This feature is platform-specific (FALCON_1803); do not depend on it for cross-platform applications.
