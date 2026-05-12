# simcom_demo (Demo Task Manager and UI Framework)

Location: SIMCOM_SDK_SET/sc_demo/V1/src/simcom_demo.c

## Overview
`simcom_demo` is the central demo management framework for the SIMCom OpenSDK. It creates a UI task that presents an interactive menu on UART or USB VCOM, allowing users to select and run individual demo modules. Each demo is registered as a callback in a global demo list, conditionally compiled based on feature flags.

## Main Features
- Registers all available demo modules into a global demo list (`gDemoList`) with description strings and callback function pointers.
- Provides a UART/USB interactive CLI that displays numbered menu options and dispatches to the selected demo.
- Supports output to either UART1 (default debug port) or USB AT port based on `SIMCOM_UI_DEMO_TO_UART1_PORT` / `SIMCOM_UI_DEMO_TO_USB_AT_PORT` definitions.
- Provides utility functions for formatted output: `PrintfResp`, `PrintfRespData`, `PrintfRespHexData`, `PrintfOptionMenu`, `PrintfMainMenu`.
- Creates a 60KB stack task at priority 100 for the UI processer.

### Registered Demo Modules (conditional compilation)
| Macro | Demo | Callback |
|-------|------|----------|
| `HAS_DEMO_NETWORK` | NETWORK | `NetWorkDemo` |
| `HAS_DEMO_SIMCARD` | SIMCARD | `SimcardDemo` |
| `HAS_DEMO_SMS` | SMS | `SMSDemo` |
| `HAS_DEMO_UART` | UART | `UartDemo` |
| `HAS_DEMO_USB` | USB | NULL |
| `HAS_DEMO_GPIO` | GPIO | `GpioDemo` |
| `HAS_DEMO_PMU` | PMU | `PMUDemo` |
| `HAS_DEMO_I2C` | I2C | `I2cDemo` |
| `HAS_DEMO_AUDIO` | AUDIO | `AudioDemo` |
| `HAS_DEMO_FS` | FILE SYSTEM | `FsDemo` |
| `HAS_DEMO_TCPIP` | TCPIP | `TcpipDemo` |
| `HAS_DEMO_HTTPS` | HTTPS | `HttpsDemo` |
| `HAS_DEMO_FTPS` | FTPS | `FtpsDemo` |
| `HAS_DEMO_MQTTS` | MQTTS | `MqttDemo` |
| `HAS_DEMO_SSL` | SSL | `SslDemo` |
| `HAS_DEMO_OTA` | FOTA | `FotaDemo` |
| `HAS_DEMO_LBS` | LBS | `LbsDemo` |
| `HAS_DEMO_SJDR` | JAMMING DETECT | `JamDectDemo` |
| `HAS_DEMO_PPPD` | PPPD | `PppdDemo` |
| `HAS_DEMO_NTP` | NTP | `NtpDemo` |
| `HAS_DEMO_HTP` | HTP | `HtpDemo` |
| `HAS_DEMO_TTS` | TTS | `TTSDemo` |
| `HAS_DEMO_CALL` | CALL | `CALLDemo` |
| `HAS_DEMO_WIFISCAN` | WIFISCAN | `WIFISCANDemo` |
| `HAS_DEMO_GNSS` | GNSS | `GNSSDemo` |
| `HAS_DEMO_LCD` | LCD | `LcdDemo` |
| `HAS_DEMO_RTC` | RTC | `RTCDemo` |
| `HAS_DEMO_FLASH` | FLASH | `FlashRWdemo` |
| `HAS_DEMO_SPI` | SPI | `SpiDemo` / `SpiNorDemo` / `SpiNandDemo` |
| `HAS_DEMO_CAM` | CAM | `CamDemo` |
| `HAS_DEMO_SYS` | SYS | `SysDemo` |
| `HAS_DEMO_BLE` | BLE | `BLEDemo` |
| `HAS_DEMO_BT` | BT | `BTDemo` |
| `HAS_DEMO_BT_STACK` | BT/BLE | `BTDemo` |
| `HAS_DEMO_APP_DOWNLOAD` | APP_DOWNLOAD | `AppDownloadDemo` |
| `HAS_DEMO_APP_UPDATE` | APP_UPDATE_FOR_NVM | `AppUpdateDemo` |
| `HAS_DEMO_PWM` | PWM | `PwmDemo` |
| `HAS_DEMO_POC` | POC | `POCDemo` |
| `HAS_DEMO_WTD` | WTD | `WTDDemo` |
| `HAS_DEMO_PING` | PING | `PingDemo` |
| `HAS_SM2` | SM2 | `SM2Demo` |
| `HAS_ZLIB` | ZLIB | `ZLIBDemo` |
| `HAS_CJSON` | CJSON | `CjsonDemo` |
| `HAS_MBEDTLS` | MBEDTLS | `MbedTLSDemo` |
| `HAS_CRYPTO` | CRYPTO | `CryptoDemo` |
| `HAS_DEMO_ONEWIRE` | ONE_WIRE | `OneWireDemo` |

## Key Header Dependencies
- `simcom_demo.h` — Demo list structure definitions, `SC_DEMO_T`, demo callback prototypes.
- `simcom_os.h` — `sAPI_TaskCreate` for UI task creation.
- `simcom_uart.h` — `sAPI_UartWrite`, `sAPI_UartSetConfig` for UART-based menu output.
- `simcom_usb_vcom.h` — `sAPI_UsbVcomWrite` for USB-based menu output.
- `simcom_debug.h` — `sAPI_Debug` for logging.
- `uart_api.h` — `UartReadValue` for reading numeric user input from UART.

## SDK APIs Observed
- `sAPI_TaskCreate(&simcomUIProcesser, stack, 60KB, priority, "simcomUIProcesser", sTask_SimcomUIProcesser, arg)` — creates the UI task.
- `sAPI_UartWrite(SC_UART, data, len)` — writes output to UART port (varies by platform: `SC_UART` vs `SC_UART4`).
- `sAPI_UsbVcomWrite(data, len)` — writes output to USB AT port.
- `UartReadValue()` — reads a numeric selection from the user via UART input.

## Usage
1. Call `sAPP_SimcomUIDemo()` during application initialization.
2. The function allocates a 60KB stack and creates the `simcomUIProcesser` task at priority 100.
3. `scDemoListInit()` populates the `gDemoList` array with all enabled demo modules.
4. The task loop prints the main menu with numbered options and waits for user input via `UartReadValue`.
5. When the user enters a valid number, the corresponding demo callback is invoked.
6. After the demo completes, control returns to the menu loop.

## Implementation Notes
- Platform-specific UART port selection: some modules (A7680C_V5_01, A7670C_V701, etc.) use `SC_UART4` instead of `SC_UART` for debug output.
- `PrintfRespHexData` converts binary data to hexadecimal string before output.
- `PrintfOptionMenu` formats option lists in two columns for compact display.
- The demo list array is indexed by `SC_DEMO_*` enum values from `simcom_demo.h`.
- Each demo callback is expected to manage its own interactive sub-menu and return to the main menu when done.

## Best Practices
- Use `userspaceConfig.h` to enable only the demos needed for your application, reducing flash and RAM usage.
- Each demo callback should have its own menu loop and handle input validation to prevent crashes from invalid input.
- Ensure sufficient stack size (60KB default) to accommodate the deepest demo call chains.
- Use `PrintfResp` for text output and `PrintfRespData` for binary data to maintain consistent output routing.
- In production, disable the UI demo task entirely or replace it with a lightweight command parser.
