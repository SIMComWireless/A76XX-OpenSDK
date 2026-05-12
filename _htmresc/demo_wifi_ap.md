# demo_wifi_ap

Location: `SIMCOM_SDK_SET/sc_demo/V1/src/demo_wifi_ap.c`

## Overview

This demo provides a simple interactive UART menu for configuring the A7672 module as a Wi-Fi Access Point (AP). It demonstrates the Wi-Fi AP configuration API for setting SSID, authentication/encryption, password, network mode, and channel. The module is not a full-featured Wi-Fi router; this is intended for basic AP functionality, primarily to allow nearby devices to connect for local data exchange.

## Main Features

- Open/close the Wi-Fi AP interface
- Set and get the AP SSID
- Configure authentication type (WPA2) and encryption (TKIP) with password
- Set and get Wi-Fi network mode (802.11bg) and channel number
- Retrieve connected client MAC addresses and client count
- Power-save mode reset

## Key Header Dependencies

- `simcom_wifi_ap.h` -- Wi-Fi AP API header (not found as a standalone file in the SDK inc directory; provided through the Wi-Fi subsystem build chain). Declares `sAPI_WiFi*` functions and `SC_WIFI_*` types/enums.
- `simcom_os.h` -- RTOS primitives
- `simcom_common.h` -- Common types
- `simcom_debug.h` -- Debug logging
- `simcom_uart.h` -- UART I/O
- `uart_api.c` -- UI helpers

## SDK APIs Observed

- `sAPI_WiFiStatusSet(API_WIFI_OPEN)` -- Opens the Wi-Fi AP interface.
- `sAPI_WiFiSSIDSet(SSID)` -- Sets the AP SSID string.
- `sAPI_WiFiSSIDGet(SSID)` -- Gets the current AP SSID.
- `sAPI_WiFiAuthGet(&auth, &encrypt, password)` -- Gets the current authentication type, encryption type, and password.
- `sAPI_WiFiAuthSet(SC_WIFI_WAPI_WPA2, SC_WIFI_TKIP, password)` -- Sets authentication (e.g., `SC_WIFI_WAPI_WPA2`), encryption (e.g., `SC_WIFI_TKIP`), and password.
- `sAPI_WiFiMochGet(&net_mode, &wifichannel)` -- Gets the current network mode and channel.
- `sAPI_WiFiMochSet(SC_WIFI_802_11bg, 9)` -- Sets network mode (e.g., `SC_WIFI_802_11bg`) and channel number.
- `sAPI_WiFiCLICNT()` -- Returns the number of connected Wi-Fi clients (unsigned char).
- `sAPI_WiFiMACAddrGet(mac)` -- Gets MAC addresses of connected clients. `mac` is a `char**` array of pre-allocated 18-byte strings.
- `sAPI_WiFiPSMreset()` -- Resets the power-save mode.

## Usage

1. Enter the WIFIAPDemo menu (via the main demo dispatcher).
2. Select option 1 (wifiap test).
3. The demo automatically:
   - Opens Wi-Fi (`sAPI_WiFiStatusSet(API_WIFI_OPEN)`)
   - Sets SSID to "ssid_demo"
   - Reads back the SSID
   - Gets current auth/encrypt/password, then sets WPA2/TKIP with password "1234567890"
   - Reads back the new auth configuration
   - Gets current network mode/channel, sets 802.11bg on channel 9, reads back
   - Gets connected client count, allocates memory for MAC addresses, retrieves first MAC
   - Resets power-save mode

## Implementation Notes

- **Single test flow**: The demo does not expose individual operations; all configuration steps run sequentially in a single menu selection.
- **Memory management**: Client MAC addresses are dynamically allocated as a `char**` array with `(cnt+1)` elements of 18 bytes each. The demo frees `mac[0]` after use but does not free the remaining allocations (potential memory leak in the demo code).
- **Default configuration**: SSID is "ssid_demo", auth is WPA2, encryption is TKIP, password is "1234567890", mode is 802.11bg, channel is 9.
- **Auth types**: The demo references `SC_WIFI_WAPI_WPA2` and `SC_WIFI_TKIP` from the `SC_WIFI_AUTH` and `SC_WIFI_ENCRYPT` enums.
- **Network modes**: References `SC_WIFI_80211_MODE` with value `SC_WIFI_802_11bg`.

## Best Practices

- Always open Wi-Fi before setting SSID, auth, or channel parameters.
- Use a strong password (the demo's "1234567890" is for testing only).
- Check the return value of `sAPI_WiFiCLICNT()` before allocating MAC address arrays to avoid buffer issues.
- Free all dynamically allocated MAC address strings to prevent memory leaks.
- Call `sAPI_WiFiPSMreset()` after configuration changes to ensure the AP operates correctly.
- For production use, set the SSID and auth before opening Wi-Fi if the API supports it.
- The Wi-Fi AP on this module is limited in throughput and client capacity; use it primarily for configuration or local communication rather than high-bandwidth applications.
