# demo_ble

Location: `SIMCOM_SDK_SET/sc_demo/V1/src/demo_ble.c`

## Overview

This demo provides an interactive UART-driven menu for BLE (Bluetooth Low Energy) operations using the standalone BLE API (prefixed `sAPI_Ble*` with capitalized "Ble"). It is conditionally compiled under `BT_SUPPORT`. The demo focuses on the BLE peripheral role: opening/closing the BLE device, configuring advertising data and parameters, registering a custom GATT service with characteristics, sending indicate/notify, and performing BLE scanning as an observer. It also supports BLE pairing and device configuration.

## Main Features

- BLE device open/close lifecycle
- BLE address set/get
- Custom GATT service registration with read/write callbacks
- Advertising data creation (flags, name, specific data)
- Advertising parameter configuration (legacy and extended via `SC_EXT_ADV`)
- Enable/disable advertising
- BLE scan (observer role) with results via callback
- Indicate and notify to connected central devices
- BLE SMP pairing: initiate, enable/disable, get/delete/clear pair info
- RSSI reading
- Device name set/get
- Custom BT device configuration (UART, pins) under `CUS_DRIVER_BT`

## Key Header Dependencies

- `simcom_ble.h` -- Standalone BLE API header (not found in the SDK inc directory; likely provided through the BLE subsystem build chain). Provides `sAPI_Ble*` function declarations, `SC_BLE_*` types, and the `SC_BLE_SERVICE_T` service declaration macros.
- `simcom_os.h` -- RTOS primitives
- `simcom_common.h` -- Common types
- `simcom_debug.h` -- Debug logging
- `simcom_uart.h` -- UART I/O
- `uart_api.c` -- UI helpers

## SDK APIs Observed

- `sAPI_BleRegisterEventHandleEx(cb)` -- Register BLE event callback (void return variant)
- `sAPI_BleOpen(NULL, 0)` -- Open BLE device
- `sAPI_BleClose()` -- Close BLE device
- `sAPI_BleSetAddress(&address)` -- Set BLE MAC address
- `sAPI_BleGetAddress(&address)` -- Get BLE MAC address
- `sAPI_BleSetGapName("name")` -- Set GAP device name
- `sAPI_BleSetDeviceName("name")` / `sAPI_BleGetDeviceName()` -- Set/get device name (before open)
- `sAPI_BleCreateAdvData(type, buf, bufSize, data, len)` -- Build advertising PDU segments. Types: `BLE_ADV_DATA_TYPE_FLAG` (0x01), `BLE_ADV_DATA_TYPE_COMPLETE_NAME` (0x09), `BLE_ADV_DATA_TYPE_SPECIFIC_DATA` (0xFF).
- `sAPI_BleSetAdvData(data, len)` / `sAPI_BleSetExtAdvData(data, len)` -- Set advertising data
- `sAPI_BleSetScanResponseData(data, len)` / `sAPI_BleSetExtScanResponseData(data, len)` -- Set scan response
- `sAPI_BleSetAdvParam(&param)` -- Set legacy advertising parameters (interval, type, address type)
- `sAPI_BleSetExtAdvParam(&param)` -- Set extended advertising parameters (properties, channel, PHY, tx_power)
- `sAPI_BleRegisterService(attrs, count)` -- Register a GATT service defined by `SC_BLE_SERVICE_T` array with macros `SC_BLE_DECLARE_PRIMARY_SERVICE`, `SC_BLE_DECLARE_CHARACTERISTIC`, `SC_BLE_DECLARE_CLINET_CHRAC_CONFIG`
- `sAPI_BleUnregisterService()` -- Unregister the current GATT service
- `sAPI_BleEnableAdv()` / `sAPI_BleDisableAdv()` -- Legacy advertising control
- `sAPI_BleEnableExtAdv()` / `sAPI_BleDisableExtAdv()` -- Extended advertising control
- `sAPI_BleDisconnect()` -- Disconnect from the connected central
- `sAPI_BleIndicate(handle, data, len)` -- Send indication (GATT handle only, no connection handle)
- `sAPI_BleNotify(handle, data, len)` -- Send notification (GATT handle only, no connection handle)
- `sAPI_BleScan(type, interval, window, addr_type)` -- Start BLE scan. Example: `LE_ACTIVE_SCAN, 0x1F40, 0x1F40, LE_ADDRESS_TYPE_RANDOM`
- `sAPI_BleScanStop()` -- Stop scanning
- `sAPI_BleSmpIniate(addr)` -- Initiate SMP pairing to a device
- `sAPI_BleSetPairEnable(enable)` -- Enable/disable pairing acceptance
- `sAPI_BleGetPairInfo(index, &info)` -- Get pairing record by index (max 10)
- `sAPI_BleDeleteSinglePairInfo(index)` -- Delete a specific pairing record
- `sAPI_BleClearPairInfo()` -- Clear all pairing records
- `sAPI_BleReadRssi()` -- Read RSSI
- `sAPI_BleDevConfigInit(&device)` -- Configure custom BT hardware (under `CUS_DRIVER_BT`)

## Usage

1. Build with `BT_SUPPORT` defined.
2. Enter the BLEDemo menu.
3. **Peripheral advertising flow**: Option 9 (register event handle) -> 1 (open, which also sets name) -> 4 (create adv data with flags + name) -> 6 (set adv params) -> 7 (register custom service) -> 10 (enable adv). The code comment notes the sequence: 9->1->3->6->4->5->7->10.
4. **Data exchange**: After a central connects, use option 13 (indicate) or 14 (notify) to push data to the central. Incoming writes arrive via `custom_characteristic_write_cb`.
5. **Observer scan flow**: Option 9 -> 1 -> 15 (scan start) -> 16 (scan stop).
6. **Pairing**: Option 18 (pair to device address) -> 19 (enable pairing) -> 20 (get pair info).

## Implementation Notes

- **Custom GATT service definition**: Uses `SC_BLE_SERVICE_T` array with macros:
  - `SC_BLE_DECLARE_PRIMARY_SERVICE` for service UUID 0xFFF0
  - `SC_BLE_DECLARE_CHARACTERISTIC` for a characteristic with UUID 0xFFF1 supporting READ, WRITE_WITHOUT_RESPONSE, NOTIFY, and INDICATE
  - `SC_BLE_DECLARE_CLINET_CHRAC_CONFIG` for CCCD (Client Characteristic Configuration Descriptor)
- **Characteristic value buffer**: Fixed at `CHARACTERISTIC_VALUE_LENGTH` (512 bytes). Read callback dynamically allocates memory via `sAPI_Malloc()` and returns data through `rw->data`; the caller must free it.
- **Write callback validation**: Returns `SC_BLE_ERR_INVALID_OFFSET` if offset/length exceeds buffer size. CCCD write callback validates value <= 0x03.
- **Event callback**: `custom_ble_handle_event` handles `EVENT_TYPE_COMMON` (powerup, bond complete, RSSI) and `EVENT_TYPE_LE` (scan, connect, disconnect, MTU, indicate, client responses).
- **Extended advertising**: Controlled by `#define SC_EXT_ADV` (commented out by default). Extended mode supports up to 255-byte advertising data versus 31-byte legacy.
- **Device config under CUS_DRIVER_BT**: Allows configuring UART ID (1 or 4), baudrate (115200/921600/3000000), and GPIO pins for host_wakeup_bt, bt_wakeup_host, bt_reset, bt_pwdn.
- **Differences from demo_bt_stack.c**: This demo uses a different API layer (`sAPI_Ble*` with capitalized "Ble" vs `sAPI_Ble*` with lowercase "t"). The service registration uses `sAPI_BleRegisterService` with macro-declared service tables, whereas bt_stack uses `sAPI_BleRegisterAttServer` with raw binary profile data. The indicate/notify APIs here take only a handle (no connection handle), implying single-connection operation.

## Best Practices

- Register the event callback before opening BLE.
- Create advertising data before setting it; check the return value of `sAPI_BleCreateAdvData` for -1 (failure).
- Register the GATT service before enabling advertising so that connected devices can discover characteristics.
- Validate characteristic write data offsets in production code to prevent buffer overflows.
- Use the CCCD write callback to track whether the remote central has enabled notifications or indications.
- For applications requiring multiple simultaneous connections, use the bt_stack API instead.
- When using extended advertising, ensure the remote scanner supports BLE 5.0 extended scanning.
