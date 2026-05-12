# demo_wtd

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_wtd.c

## Overview
This demo provides a simple interactive menu-driven interface for hardware watchdog timer (WDT) management on the SIMCom A7672 module. It enables configuring the watchdog timeout period, enabling/disabling fault wakeup, enabling/disabling the watchdog, and feeding the watchdog to prevent system reset. The implementation directly manipulates watchdog registers through the SDK API.

## Main Features
- Configure watchdog timeout period
- Enable fault wakeup functionality
- Enable software watchdog
- Feed the watchdog to prevent timeout reset
- Disable the watchdog and fault wakeup

## Key Header Dependencies
- `simcom_wtd.h` — Defines watchdog register addresses and four core watchdog API functions
- `simcom_system.h` — System-level functions (included for system context)
- `simcom_common.h` — Common type definitions
- `simcom_os.h` — OS abstraction layer
- `simcom_debug.h` — Debug logging via `sAPI_Debug()`
- `uart_api.h` — UART read helpers for interactive input

## SDK APIs Observed
- `sAPI_SetWtdTimeOutPeriod(value)` — Set the watchdog timeout period register (register 0x11). Bits [3:0] set active timeout, bits [5:4] set sleep mode timeout. Value 0x04 corresponds to a 16-second feed period.
- `sAPI_FalutWakeEnable(enable)` — Enable/disable fault wakeup. Write 1 to enable `fault_wu_en`, write 3 to enable `fault_wu`. The two bits must be enabled separately and in order.
- `sAPI_SoftWtdEnable(enable)` — Enable/disable the software watchdog (register 0x1D).
- `sAPI_FeedWtd()` — Feed (reset) the watchdog timer (register 0x0D). Must be called before the timeout period expires after WDT is enabled.

## Usage
1. Call `WTDDemo()` to enter the interactive menu.
2. Select option 0 ("wtd open") to enable the watchdog:
   - Sets timeout period to 0x04 (16-second feed cycle)
   - Enables fault wakeup (`sAPI_FalutWakeEnable(1)`)
   - Enables software watchdog (`sAPI_SoftWtdEnable(1)`)
   - Feeds the watchdog once immediately (`sAPI_FeedWtd()`) -- this is required to load the timeout register
3. Select option 1 ("wtd close") to disable the watchdog:
   - Disables fault wakeup (`sAPI_FalutWakeEnable(0)`)
   - Disables software watchdog (`sAPI_SoftWtdEnable(0)`)
4. Select option 2 ("feed dog") to manually feed the watchdog timer.
5. Option 99 returns to the previous menu.

## Implementation Notes
- The watchdog enable sequence is critical and must follow this exact order:
  1. Set timeout period
  2. Enable fault wakeup
  3. Enable software watchdog
  4. Feed the watchdog once (this triggers the timeout register to load)
- After the watchdog is enabled, the system will reset if `sAPI_FeedWtd()` is not called within the configured timeout period (16 seconds with value 0x04).
- `sAPI_FalutWakeEnable()` controls two bits: writing 1 enables `fault_wu_en`, and writing 3 enables both `fault_wu_en` and `fault_wu`. The demo uses value 1 for enable.
- The watchdog registers are hardware registers accessed directly: 0x11 (timeout), 0xE7 (fault wakeup), 0x1D (enable), 0x0D (feed).
- All four API functions return integer status codes (0 for success, non-zero for failure). The demo uses bitwise OR to accumulate the return values and reports overall success/failure.
- This is one of the simpler demos with only 105 lines of code and three operations.

## Best Practices
- Always feed the watchdog immediately after enabling it to start the timeout countdown.
- Implement watchdog feeding in a periodic timer or main loop task, not in a one-shot demo. Missing a feed cycle will cause an uncontrolled system reset.
- Choose the timeout period based on the application's worst-case loop time plus a safety margin. The 16-second period (0x04) is a reasonable default.
- Use the watchdog in production to recover from software hangs, but ensure the feeding mechanism is robust and fault-tolerant.
- Disable the watchdog during development/debugging to prevent unexpected resets when stepping through code or when the debugger halts execution.
- Consider the power mode when configuring the watchdog: bits [5:4] of register 0x11 control the sleep mode timeout, which may differ from the active mode timeout.
- The fault wakeup feature allows the watchdog reset to also wake the module from sleep mode; enable this if the module may enter sleep while the watchdog is active.
