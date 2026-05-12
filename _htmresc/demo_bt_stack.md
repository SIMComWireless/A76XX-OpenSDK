# demo_bt_stack

Location: `SIMCOM_SDK_SET/sc_demo/V1/src/demo_bt_stack.c`

## Overview

This demo provides an interactive UART-driven menu for controlling both classic Bluetooth (BR/EDR) and BLE (Bluetooth Low Energy) using the newer BTstack-based API (prefixed `sAPI_Bt*` and `sAPI_Ble*`). It is conditionally compiled under `HAS_DEMO_BT_STACK`. The demo combines classic BT operations (scan, pair, SPP) with full BLE peripheral and central role operations (advertising, GATT server, GATT client scanning, indicate/notify, read/write, pairing, and white list management).

## Main Features

- Classic Bluetooth: open/close, address/name management, inquiry scan, pairing (with PIN, passkey, SSP), SPP connect/disconnect/send, RSSI reading
- BLE Peripheral: set random address, configure advertising parameters (legacy and extended), create/set advertising data, register GATT ATT server, enable/disable advertising, send indicate/notify
- BLE Central: BLE scan start/stop, connect/disconnect, MTU request, GATT scan (service/characteristic/descriptor discovery), read request, write request, write command
- BLE Security: SMP pairing request, enable/disable pairing acceptance, get/remove/clear pairing records, set/clear white list
- Comprehensive event handling: 30+ event types across common, SPP, and LE event categories

## Key Header Dependencies

- `simcom_bt_stack.h` -- BTstack API declarations, all `SC_BT_*` and `SC_BLE_*` type definitions, event structures, and `sAPI_Bt*`/`sAPI_Ble*` function prototypes
- `ble_gatt_demo.h` -- Auto-generated GATT profile data (`profile_data[]`) from a `.gatt` file, defines ATT service/characteristic handle ranges
- `userspaceConfig.h` -- Feature configuration flags
- `simcom_os.h` -- RTOS primitives
- `simcom_debug.h` -- Debug logging
- `simcom_uart.h` -- UART interactive input
- `uart_api.c` -- UI helpers

## SDK APIs Observed

### Classic BT (sAPI_Bt*)
- `sAPI_BtRegisterEventHandle(cb)` -- Register event callback (must be before open)
- `sAPI_BtSetIoCap(io_cap)` -- Set IO capability (0-3)
- `sAPI_BtOpen(mode)` -- Open BT with specified mode (SC_BT_BR, SC_BT_BR_LE, SC_BT_LE)
- `sAPI_BtClose()` -- Close BT
- `sAPI_BtSetAddress(addr)` / `sAPI_BtGetAddress(&addr)` -- Address management
- `sAPI_BtSetName(name, len)` / `sAPI_BtGetName(name, &len)` -- Name management
- `sAPI_BtScanStart(duration, num)` -- Inquiry scan (duration: 1-48, unit 1.28s)
- `sAPI_BtScanStop()` -- Stop inquiry
- `sAPI_BtPairRequest(addr)` -- Initiate pairing
- `sAPI_BtPairAccept(mode, addr, verification_code, pin)` -- Accept pairing (supports JUST_WORK, NUMERIC_COMPARISON, PASSKEY_ENTRY, PIN_CODE)
- `sAPI_BtPairingMode(mode)` -- Set pairing mode (0=PIN, 1=SSP)
- `sAPI_BtUnpair(addr)` -- Remove pairing
- `sAPI_BtPairedQueryByIndex(i, &record)` -- Query paired device by index (max 9)
- `sAPI_BtSppConnect(addr)` / `sAPI_BtSppDisconnect(port)` -- SPP lifecycle
- `sAPI_BtSppSend(data, len, port)` -- SPP data send (max 200 bytes)
- `sAPI_BtReadRssi(handle)` -- Read RSSI by connection handle

### BLE (sAPI_Ble*)
- `sAPI_BleSetRandomAddress(&addr)` -- Set BLE random address
- `sAPI_BleSetAdvParam(&param)` -- Set legacy advertising parameters
- `sAPI_BleSetExtAdvParam(&param)` -- Set extended advertising parameters
- `sAPI_BleCreateAdvData(type, buf, bufSize, data, len)` -- Build advertising PDU
- `sAPI_BleSetAdvData(data, len)` / `sAPI_BleSetExtAdvData(data, len)` -- Set adv data
- `sAPI_BleSetScanResponseData(data, len)` / `sAPI_BleSetExtScanResponseData(data, len)` -- Set scan response
- `sAPI_BleRegisterAttServer(profile, len, read_cb, write_cb)` -- Register GATT server with profile data
- `sAPI_BleEnableAdv()` / `sAPI_BleDisableAdv()` -- Legacy adv control
- `sAPI_BleEnableExtAdv()` / `sAPI_BleDisableExtAdv()` -- Extended adv control
- `sAPI_BleScan(type, interval, window, addr_type)` -- Start BLE scan
- `sAPI_BleScanStop()` -- Stop BLE scan
- `sAPI_BleConnect(&addr, type)` -- Connect to BLE device
- `sAPI_BleDisconnect(handle)` -- Disconnect BLE
- `sAPI_BleMtuRequest(handle, mtu)` -- Request MTU exchange
- `sAPI_BleGattScanStart(handle)` -- Discover remote GATT services/characteristics
- `sAPI_BleReadRequest(handle, att_handle)` -- Read remote characteristic
- `sAPI_BleWriteRequest(handle, att_handle, data, len)` -- Write with response
- `sAPI_BleWriteCommand(handle, att_handle, data, len)` -- Write without response
- `sAPI_BleIndicate(conn, att, data, len)` -- Send indication
- `sAPI_BleNotify(conn, att, data, len)` -- Send notification
- `sAPI_BlePairRequest(handle)` -- Initiate LE SMP pairing
- `sAPI_BleSetPairEnable(flag)` -- Enable/disable pairing acceptance
- `sAPI_BleGetPairInfo(index)` -- Get pairing record
- `sAPI_BleRemovePairInfo(index)` -- Remove single pairing record
- `sAPI_BleSetWhiteList(addr, type)` -- Add to white list
- `sAPI_BleClearWhiteList()` -- Clear white list

## Usage

1. Build with `HAS_DEMO_BT_STACK` defined.
2. Enter the BTDemo menu.
3. **Classic BT flow**: Option 1 (open with mode) -> 7 (scan) -> 9 (pair) -> 12 (SPP connect) -> 14 (SPP send).
4. **BLE Peripheral flow**: Option 1 (open, mode=2 for LE) -> 30 (set random addr) -> 31 (set adv params) -> 32 (set adv data) -> 33 (register ATT server) -> 34 (enable adv) -> wait for connection -> 37/38 (indicate/notify).
5. **BLE Central flow**: Option 1 (open, mode=1 for BR+LE) -> 42 (scan start) -> 39 (connect to discovered device) -> 41 (MTU request) -> 36 (GATT scan) -> 44 (read) / 45 (write).
6. **Recommended peripheral sequence** (from code comment): 9->1->3->6->4->5->7->10->(link)->(13/14)->12->8->11->2.

## Implementation Notes

- **Dual-mode event callback**: The single `bt_event_handle_cb` dispatches by `msg->event_type`: `SC_BT_EVENT_TYPE_COMMON`, `SC_BT_EVENT_TYPE_SPP`, and `SC_BT_EVENT_TYPE_LE`. This handles all classic and BLE events.
- **Extended advertising**: Controlled via `#define SC_EXT_ADV`. When defined, uses `sAPI_BleSetExtAdvParam`, `sAPI_BleSetExtAdvData`, `sAPI_BleEnableExtAdv` APIs; otherwise uses legacy APIs. Standard adv data is limited to 31 bytes; extended supports up to 255 bytes.
- **ATT server registration**: Uses `sAPI_BleRegisterAttServer` with a pre-generated binary `profile_data[]` from `ble_gatt_demo.h`, plus custom read/write callbacks. The profile contains GATT service, GAP service, and a custom 128-bit UUID service with read/write/notify/indicate characteristics.
- **GATT client discovery**: On `SC_BLE_GATT_CONNECTED`, the demo automatically calls `sAPI_BleGattScanStart()` to discover remote services. Results arrive as `SC_BLE_GATT_SCAN_DUMP_SERVICE`, `SC_BLE_GATT_SCAN_DUMP_CHARACTERISTIC`, and `SC_BLE_GATT_SCAN_DUMP_DESCRIPTOR` events.
- **Scan auto-stop**: BLE scan results are counted; when `scan_num >= 60`, the scan is automatically stopped.
- **Helper functions**: `ble_demo_uuid128_to_str()`, `ble_demo_print_properties()`, `ble_demo_att_read_callback_handle_blob()` provide utility for UUID formatting, property string conversion, and blob reading.
- **SPP frame size**: Same 200-byte limit as demo_bt.c.

## Best Practices

- Always call `sAPI_BtRegisterEventHandle()` before `sAPI_BtOpen()`.
- For BLE peripheral, set random address after open but before advertising.
- Register the ATT server before enabling advertising.
- Use `sAPI_BleIndicate()` for confirmed delivery and `sAPI_BleNotify()` for unconfirmed delivery; select based on application reliability requirements.
- After GATT connection, the demo auto-triggers service discovery. Production code should handle the full discovery flow (services -> characteristics -> descriptors) and cache handles.
- For extended advertising, ensure `SC_EXT_ADV` is defined and set appropriate PHY (primary/secondary) and channel parameters.
- Manage pairing records carefully; the demo supports up to 10 records (index 0-9).
- The `sAPI_BleWriteCommand` (write without response) is faster but unreliable; use `sAPI_BleWriteRequest` when confirmation is needed.
