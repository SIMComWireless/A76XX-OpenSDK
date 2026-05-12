# demo_network

Location: `SIMCOM_SDK_SET/sc_demo/V1/src/demo_network.c`

## Overview

This demo provides a comprehensive interactive UART menu for cellular network management on the A7672 module. It wraps AT command equivalents (CSQ, CREG, CGREG, CPSI, CNMP, COPS, CGDCONT, CGACT, CGATT, CFUN, CGPADDR, CNETCI) into programmatic API calls. It also includes sub-demos for GPRS readiness testing, network mode detection, multiple PDP dialing, real-time clock (CCLK/CTZU), Power Save Mode (PSM), DNS address retrieval, and dual-APN configuration (under `CUS_EMH`). The demo must call `sAPI_NetworkInit()` before using any network API.

## Main Features

- Signal quality query (CSQ)
- CS domain registration status (CREG)
- PS domain registration status (CGREG)
- System information query (CPSI): network mode, MCC/MNC, LAC, Cell ID, band, TAC, RSRP, RSRQ, RXLEV, TA, SINR
- Network mode selection (CNMP): set to GSM-only (2), Automatic (13), or LTE-only (38)
- Operator selection (COPS): get/set/search
- PDP context management (CGDCONT): get/set CID, PDP type (IP/IPV6/IPV4V6), APN
- PDP authentication (CGAUTH): get/set auth type, username, password
- PDP activation/deactivation (CGACT)
- Packet domain attach/detach (CGATT)
- Phone functionality (CFUN): set to 0 (minimum), 1 (full), 4 (reset)
- IP address query (CGPADDR)
- Adjacent cell information (CNETCI): LTE intra/inter cells, GSM neighboring cells
- GPRS readiness polling (SIM card status -> network mode detection)
- Network mode polling (CPSI + CREG)
- Multiple PDP dialing (up to 6 APNs)
- Real-time clock (CCLK via CTZU + CFUN cycle)
- Power Save Mode (PSM): T3324 timer, T3412 timer, hardware PSM, 1Bis enable
- DNS address retrieval
- Dual-APN configuration (under `CUS_EMH`)

## Key Header Dependencies

- `simcom_network.h` -- Network management API declarations (`sAPI_Network*`), data types (`SCcpsiParm`, `SCApnParm`, `SCAPNact`, `SCcgpaddrParm`, `SCcnetciParm`, `SCCGAUTHParm`, `scRealTime`, `SCDNSAddr`), and status enums (`SC_NET_SUCCESS`, `SC_NET_FAIL`).
- `simcom_simcard.h` -- SIM card status types (`SC_SIM_READY`, `SC_SIM_PIN`, `SC_SIM_PUK`)
- `simcom_tcpip.h` -- PDP activation API (`sAPI_TcpipPdpActive`)
- `simcom_os.h` -- RTOS primitives
- `simcom_common.h` -- Common types
- `simcom_debug.h` -- Debug logging
- `simcom_uart.h` -- UART I/O
- `uart_api.c` -- UI helpers

## SDK APIs Observed

- `sAPI_NetworkInit()` -- Initializes the network subsystem. **Must be called before any other network API.**
- `sAPI_NetworkGetCsq(&csq)` -- Gets signal quality (0-31, 99=unknown).
- `sAPI_NetworkGetCreg(&creg)` -- Gets CS domain registration status (0=not searching, 1=home, 2=searching, 3=denied, 5=roaming).
- `sAPI_NetworkGetCgreg(&cgreg)` -- Gets PS domain registration status.
- `sAPI_NetworkGetCpsi(&Scpsi)` -- Gets system information into `SCcpsiParm` struct (networkmode, Mnc_Mcc, LAC, CellID, band strings, TAC, RSRP, RSRQ, RXLEV, TA, SINR).
- `sAPI_NetworkGetCnmp(&cnmp)` / `sAPI_NetworkSetCnmp(val)` -- Get/set preferred network mode.
- `sAPI_NetworkGetCops(cops)` / `sAPI_NetworkSetCops(mode, format, oper, act)` -- Get/set operator selection.
- `sAPI_NetworkGetCgdcont(array)` / `sAPI_NetworkSetCgdcont(cid, type, apn)` -- Get/set PDP contexts (up to 8).
- `sAPI_NetworkGetCgact(array)` / `sAPI_NetworkSetCgact(state, cid)` -- Get/set PDP activation state.
- `sAPI_NetworkGetCgatt(&val)` / `sAPI_NetworkSetCgatt(val)` -- Get/set packet domain attach state.
- `sAPI_NetworkGetCfun(&cfun)` / `sAPI_NetworkSetCfun(val)` -- Get/set phone functionality level.
- `sAPI_NetworkGetCgpaddr(cid, &parm)` -- Get IP address for a CID.
- `sAPI_NetworkGetCnetci(&parm)` -- Get adjacent cell information. Platform-dependent: 160X uses struct with intra/inter/GSM cell arrays; other platforms use array of 12 entries.
- `sAPI_NetworkGetCgauth(&auth, cid)` / `sAPI_NetworkSetCgauth(&auth, delflag)` -- Get/set PDP authentication credentials.
- `sAPI_NetworkGetCtzu(&val)` / `sAPI_NetworkSetCtzu(val)` -- Get/set time zone update mode.
- `sAPI_NetworkGetRealTime(&rt)` -- Get real-time clock (year, month, day, hour, min, sec, timezone, sign).
- `sAPI_NetworkGetDNSAddr(cid, &dns)` -- Get DNS addresses (IPv4/IPv6 primary/secondary) for a CID.
- `sAPI_NetworkGetCnetcon(array)` / `sAPI_NetworkSetCnetcon(ch, &parm)` -- Get/set dial APN configuration (up to 6).
- `sAPI_EnableOneBis()` -- Enable 1Bis reception for PSM.
- `sAPI_EnableHardWarePSM()` -- Enable hardware PSM mode.
- `sAPI_PSMSetT3324Timer(mode, t3324)` -- Set T3324 timer (active time). Mode 1=set, 0=close PSM.
- `sAPI_PSMSetT3412Timer(t3412)` -- Set T3412 timer (periodic TAU) in minutes.
- `sAPI_SimcardPinGet(&cpin)` -- Get SIM card PIN status.
- `sAPI_TcpipPdpActive(cid, channel)` -- Activate PDP context.

## Usage

1. Enter the NetWorkDemo menu.
2. Call `sAPI_NetworkInit()` on first entry (done automatically).
3. Use options 1-18 to access specific sub-functions:
   - Option 1: Query CSQ signal quality
   - Option 2-4: Query registration and system info
   - Option 5-6: Set network mode and operator
   - Option 7-9: Manage PDP contexts, activation, and attach
   - Option 10: Set CFUN level
   - Option 11: Query IP address by CID
   - Option 12: Query neighboring cells
   - Option 13: GPRS readiness test (polling loop)
   - Option 14: Network mode test (CPSI + CREG polling)
   - Option 15: Multiple PDP dialing test
   - Option 16: Real-time clock (platform 160X only)
   - Option 17: PSM configuration (platform 160X only)
   - Option 18: DNS address retrieval (platform 160X only)
   - Option 20: Dual-APN demo (CUS_EMH only)

## Implementation Notes

- **NetworkInit requirement**: `sAPI_NetworkInit()` is called both at the top of `NetWorkDemo()` and inside sub-demos like `TestDualApnDemo()`. It must precede all network API calls.
- **Polling patterns**: The GPRS readiness test (option 13) and network mode test (option 14) use `while` loops with `sAPI_TaskSleep(200)` (~1 second) and retry counters to poll until the network is ready. The CPSI loop checks for "LTE,Online" or "GSM" network mode strings.
- **Platform branching**: Several features are conditionally compiled: `PLATFORM_160X` for real-time clock and PSM; `FALCON_1803_PLATFORM` for different CNETCI and CNETCON handling; `CUS_EMH` for dual-APN.
- **Dual-APN demo**: Configures two PDP contexts with separate APNs, auth credentials, cycles CFUN, waits for "LTE,Online" CPSI mode, then activates both PDP contexts in a polling loop.
- **CNETCI platform differences**: On 160X, `sAPI_NetworkGetCnetci` returns a single struct with arrays for LTE intra/inter and GSM cells. On other platforms, it returns an array of 12 `SCcnetciParm` entries.
- **Data types**: `SCcpsiParm` contains networkmode (40 chars), Mnc_Mcc (20 chars), band strings, and integer fields for LAC, CellID, TAC, RSRP, RSRQ, RXLEV, TA, SINR.

## Best Practices

- Always call `sAPI_NetworkInit()` before using network APIs.
- Use polling loops with reasonable intervals (1-2 seconds) and maximum retry counts to avoid infinite loops.
- Check `SC_NET_SUCCESS` return values and handle failures gracefully.
- Before activating PDP contexts, verify SIM card readiness (`sAPI_SimcardPinGet`), CS/PS registration (`sAPI_NetworkGetCreg`/`sAPI_NetworkGetCgreg`), and network mode (`sAPI_NetworkGetCpsi`).
- Set CFUN to 0 then 1 after changing CNMP/COPS to force a re-registration cycle.
- For PSM, set T3324 and T3412 timers before enabling hardware PSM, and cycle CFUN to apply changes.
- Use `sAPI_NetworkGetCpsi()` to determine whether the module is on LTE or GSM, as this affects which TCP/IP capabilities are available.
