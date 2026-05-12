# demo_simcard

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_simcard.c

## Overview
This demo provides a comprehensive interactive menu-driven interface for SIM card operations on the SIMCom A7672 module. It covers PIN management, ICCID/IMSI/HPLMN/phone number retrieval, SIM card hot-swap detection, SIM lock operations, restricted access (CRSM), generic access (CSIM), and dual-SIM management features. The demo uses a message queue for asynchronous hot-swap event handling.

## Main Features
- Get and set SIM card PIN (PIN verification, PIN change)
- SIM card hot-swap detection configuration (query state/level, set switch/level, plugout control)
- Retrieve ICCID, IMSI, HPLMN, and subscriber phone number
- SIM restricted access (CRSM commands: READ BINARY, READ RECORD, GET RESPONSE, UPDATE BINARY, UPDATE RECORD)
- SIM generic access (CSIM raw APDU commands)
- SIM card lock/unlock (facility lock)
- Dual-SIM switching, bind SIM, dual-SIM type configuration (conditionally compiled)
- Extended API for dual-SIM PIN query and hot-plug per SIM slot

## Key Header Dependencies
- `simcom_simcard.h` — Defines SIM card error codes, hot-swap command types, response structs (`CrsmResponse_st`, `CsimResponse_st`, `Hplmn_st`, `StkResult`), STK enums, and all SIM card API declarations
- `simcom_os.h` — OS abstraction for message queues, task creation, memory management
- `simcom_debug.h` — Debug logging via `sAPI_Debug()`
- `simcom_uart.h` — UART types and SC_UART definitions
- `simcom_common.h` — Common type definitions (`SIM_MSG_T`, `SC_STATUS`)

## SDK APIs Observed
- `sAPI_SimcardPinGet(&cpin)` — Get SIM card PIN state (0=READY, 1=PIN, 2=PUK, 3=BLOCKED, 4=REMOVED, 5=CRASH, 6=NO_INSERT, 7=UNKNOWN)
- `sAPI_SimcardPinSet(oldPin, newPin, option)` — Set/change PIN; option 0=no new PIN needed, 1=new PIN required
- `sAPI_SimcardHotSwapMsg(opt, param, msgQ)` — Configure hot-swap detection (query state/level, set switch/level, plugout control)
- `sAPI_SysGetIccid(&iccid)` — Retrieve SIM card ICCID string
- `sAPI_SysGetImsi(&imsi)` — Retrieve SIM card IMSI string
- `sAPI_SysGetHplmn(&hplmn)` — Get Home PLMN info (SPN, MCC, MNC)
- `sAPI_SysGetSubscriber(&num)` — Get subscriber phone number
- `sAPI_SimRestrictedAccess(cmd, fileId, p1, p2, p3, data, pathid, &response)` — Execute SIM restricted access (CRSM)
- `sAPI_SimGenericAccess(cmdLen, cmd, &response)` — Execute SIM generic access (CSIM)
- `sAPI_SimLockSet(mode, pin)` — Set facility lock (mode 1=lock, 0=unlock)
- `sAPI_SimcardSwitchMsg(simId, msgQ)` — Switch active SIM card (dual-SIM)
- `sAPI_SysGetBindSim(&bindsim)` — Get bound SIM ID
- `sAPI_SysSetBindSim(bindsim)` — Set bound SIM ID
- `sAPI_SysGetDualSimType(&type)` — Get dual-SIM mode type
- `sAPI_SysSetDualSimType(type)` — Set dual-SIM mode type
- `sAPI_MsgQCreate(&msgQ, name, size, maxMsgs, policy)` — Create message queue
- `sAPI_MsgQRecv(msgQ, &msg, timeout)` — Receive message with timeout
- `sAPI_MsgQDelete(msgQ)` — Delete message queue on exit
- `sAPI_SimcardPinGet_Ex(simId, &cpin)` — Get PIN state for specific SIM slot (dual-SIM)
- `sAPI_SimcardHotSwap_Ex(simId, opt, &param)` — Configure hot-swap for specific SIM slot (dual-SIM)
- `sAPI_StkService(opt, data, &result)` — STK service operations (conditionally compiled)

## Usage
1. Call `SimcardDemo()` to enter the interactive menu.
2. A message queue (`g_simcard_demo_msgQ`) is created at startup for hot-swap event handling.
3. Select options 1-16 to perform specific SIM card operations.
4. For hot-swap (option 2): sub-menu allows querying state/level, setting detection switch/level, and configuring plugout behavior. Responses arrive via the message queue with a 3-second timeout.
5. For restricted access (option 7): select a CRSM command, provide fileId, p1, p2, p3 parameters, and optionally data/pathid for update commands.
6. For generic access (option 9): enter the APDU command length and hex string, then receive the response.
7. For SIM lock (option 8): select lock or unlock, then enter the PIN password.
8. Dual-SIM features (options 12-16, 92) are only available when `FEATURE_SIMCOM_DUALSIM` is defined.
9. Option 99 returns to the previous menu and deletes the message queue.

## Implementation Notes
- The `SimcardHotSwapDemo()` function uses asynchronous message-based communication: it sends a hot-swap command via `sAPI_SimcardHotSwapMsg()` and waits for a response on the message queue with a 3-second timeout.
- Hot-swap configuration includes both "plug in" and "plug out" detection, configurable as high or low level trigger.
- SIM restricted access commands (176=READ BINARY, 178=READ RECORD, 192=GET RESPONSE, 214=UPDATE BINARY, 220=UPDATE RECORD) map directly to standard GSM 11.11 SIM file operations.
- The `CrsmResponse_st` struct returns `sw1`, `sw2` (status words) and `data` (response data). The `CsimResponse_st` struct returns `length` and `data`.
- For dual-SIM builds, `SC_UART` is remapped to `SC_UART4` on specific board configurations (A7680C_V5_01, A7670C_V701, A7670C_V702, A7680C_V506, A7680C_V603).
- The `SimcardGenericAccessDemo()` has a potential bug: the switch-case for `SC_SIMCARD_DEMO_SIMGENERICACCESS` (case 9) is missing a `break` statement, causing fall-through to `SC_SIMCARD_DEMO_GETNUM` (case 10).
- STK, slot selection, and some dual-SIM features are conditionally compiled out (`#if 0`) in the demo but the API declarations exist in the header.
- The extended API (`Extend_API_Demo`, option 92) provides dual-SIM-aware versions of PIN query and hot-plug configuration.

## Best Practices
- Always check the return code of `sAPI_SimcardPinGet()` before performing SIM-dependent operations.
- Use the message queue timeout (3000ms) to avoid indefinite blocking during hot-swap operations.
- Free `msg.arg3` after processing hot-swap responses to prevent memory leaks.
- For CRSM operations, ensure the file ID and path are valid for the installed SIM card; invalid IDs will return error status words.
- In dual-SIM applications, use the `_Ex` API variants (`sAPI_SimcardPinGet_Ex`, `sAPI_SimcardHotSwap_Ex`) to target specific SIM slots.
- Delete the message queue on application exit via `sAPI_MsgQDelete()`.
- PIN passwords should be at least 4 characters; the demo validates this.
