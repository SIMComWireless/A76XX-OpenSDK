# demo_wifiscan

Location: `SIMCOM_SDK_SET/sc_demo/V1/src/demo_wifiscan.c`

## Overview

This demo provides a simple interactive UART menu for Wi-Fi scanning on the A7672 module. It demonstrates the Wi-Fi receiver-only scanning functionality, which listens for nearby access points and reports their MAC addresses, channel numbers, and RSSI values. This is not full Wi-Fi connectivity; it is designed primarily for Wi-Fi-based location services (e.g., feeding scan data to a location server).

## Main Features

- Start Wi-Fi scanning with asynchronous results via callback
- Stop Wi-Fi scanning
- Event-driven reporting of scanned access points (MAC, channel, RSSI)

## Key Header Dependencies

- `simcom_wifi.h` -- Wi-Fi scan API header. Provides `sAPI_WifiSetHandler()`, `sAPI_WifiScanStart()`, `sAPI_WifiScanStop()`, and the `SC_WIFI_EVENT_T` / `SC_WIFI_INFO_T` / `SC_WIFI_SCANPARAM_T` type definitions.
- `simcom_os.h` -- RTOS primitives
- `simcom_common.h` -- Common types
- `simcom_debug.h` -- Debug logging
- `simcom_uart.h` -- UART I/O
- `uart_api.c` -- UI helpers

## SDK APIs Observed

- `sAPI_WifiSetHandler(wifi_handle_event)` -- Registers a callback function that receives Wi-Fi events. The callback signature is `void (*handler)(const SC_WIFI_EVENT_T *param)`.
- `sAPI_WifiScanStart()` -- Starts Wi-Fi scanning. Scan results are delivered asynchronously through the registered handler.
- `sAPI_WifiScanStop()` -- Stops an ongoing Wi-Fi scan.
- (Not used in this demo but available in the header) `sAPI_WifiSetScanParam(&param)` -- Configures scan parameters: `round_number` (1-3, default 3), `max_bssid_number` (4-10, default 5), `timeout` (seconds, default 25), `priority` (0=PS priority, 1=Wifi priority, default 0).

## Usage

1. Enter the WIFISCANDemo menu (via the main demo dispatcher).
2. Select option 1 to start scanning. The registered callback will be invoked for each discovered AP and when scanning stops.
3. Wait for scan results to appear via UART output.
4. Select option 2 to stop scanning early if needed.
5. Select option 99 to return to the previous menu.

## Implementation Notes

- **Event types**: The callback receives `SC_WIFI_EVENT_T` with a `type` field:
  - `SC_WIFI_EVENT_SCAN_INFO` (1) -- Contains `SC_WIFI_INFO_T` with `mac_addr[6]`, `rssi`, and `channel_number`. One event per discovered AP.
  - `SC_WIFI_EVENT_SCAN_STOP` (0) -- Indicates scanning has completed or was stopped.
- **MAC address format**: Displayed as `%02x:%02x:%02x:%02x:%02x:%02x` in byte order [5..0] (note: this is the raw byte order, which may differ from the typical display order).
- **Asynchronous design**: The scan is non-blocking. Results arrive at the callback as they are discovered, not in a batch.
- **No scan parameter configuration**: The demo uses default scan parameters. For custom scan behavior, call `sAPI_WifiSetScanParam()` before starting the scan.
- **RX-only mode**: The module's Wi-Fi is receiver-only for scanning; it cannot connect to access points. This is a hardware/firmware limitation for cellular modules.

## Best Practices

- Always register the handler with `sAPI_WifiSetHandler()` before calling `sAPI_WifiScanStart()`.
- For location-based services, configure scan parameters with `sAPI_WifiSetScanParam()` to balance scan speed and accuracy. Increase `round_number` and `max_bssid_number` for more thorough scans; reduce `timeout` for faster results.
- Stop scanning with `sAPI_WifiScanStop()` when results are no longer needed to save power.
- The Wi-Fi scanner shares hardware resources with other subsystems; avoid running scans during active data transfers if timing-sensitive.
- For production use, process scan results in the callback rather than printing to UART -- parse MAC/RSSI/channel into structured data for submission to a location API.
