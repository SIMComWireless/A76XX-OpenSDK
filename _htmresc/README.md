# SIMCom OpenSDK Demo Documentation Index

This directory contains detailed English documentation for all demo modules in the SIMCom OpenSDK (A7672 series). Each markdown file corresponds to a `.c` source file in `sc_demo/V1/src/` and describes the main features, SDK APIs, usage flow, and best practices.

## Framework / Utility

| File | Description |
|------|-------------|
| [simcom_demo.md](simcom_demo.md) | Central demo task manager and interactive CLI UI framework |
| [uart_api.md](uart_api.md) | UART initialization, circular data cache, and input reading helpers |
| [cus_urc.md](cus_urc.md) | URC (Unsolicited Result Code) processing framework |
| [cus_usb_vcom.md](cus_usb_vcom.md) | USB Virtual COM Port communication |
| [demo_helloworld.md](demo_helloworld.md) | Minimal entry-point demo for getting started |

## Bluetooth & BLE

| File | Description |
|------|-------------|
| [demo_bt.md](demo_bt.md) | Classic Bluetooth: adapter control, inquiry, pairing, SPP |
| [demo_bt_stack.md](demo_bt_stack.md) | BTstack dual-mode: BT + BLE peripheral/central GATT |
| [demo_ble.md](demo_ble.md) | Standalone BLE: advertising, custom GATT service, notify/indicate |

## Audio & Voice

| File | Description |
|------|-------------|
| [demo_audio.md](demo_audio.md) | Audio playback, recording, volume, mic gain, echo suppression |
| [demo_tts.md](demo_tts.md) | Text-to-Speech playback and parameter control |
| [demo_call.md](demo_call.md) | Voice call: dial, answer, hang up, auto-answer, DTMF |
| [demo_poc.md](demo_poc.md) | Push-to-Talk (POC): low-level PCM playback and recording |

## Messaging

| File | Description |
|------|-------------|
| [demo_sms.md](demo_sms.md) | SMS full lifecycle: init, write, read, send, delete, PDU encoding |

## Networking & TCP/IP

| File | Description |
|------|-------------|
| [demo_network.md](demo_network.md) | Cellular network management: CSQ, CREG, APN, PDP, PSM |
| [demo_tcpip.md](demo_tcpip.md) | TCP/UDP sockets, DNS resolution, dual-stack IPv4/IPv6 |
| [demo_ping.md](demo_ping.md) | ICMP ping with callback-based result reporting |
| [demo_pppd.md](demo_pppd.md) | PPP dial-up for GPRS data connection |

## MQTT

| File | Description |
|------|-------------|
| [demo_mqtt.md](demo_mqtt.md) | MQTT/MQTTS: connect, publish, subscribe, Aliyun/Tencent/OneNET demos |
| [demo_auto_mqtt.md](demo_auto_mqtt.md) | Automated MQTT stress test with offline notification |
| [mqtt_OneNET.md](mqtt_OneNET.md) | OneNET cloud platform integration with HMAC-SHA1 token |
| [mqtt_tencent.md](mqtt_tencent.md) | Tencent Cloud IoT integration with HMAC-SHA1 auth |

## HTTP / HTTPS / FTPS

| File | Description |
|------|-------------|
| [demo_https.md](demo_https.md) | HTTP/HTTPS client: GET, POST, file upload, header management |
| [demo_ftps.md](demo_ftps.md) | FTP/FTPS: login, list, download, upload, directory operations |
| [demo_ftps_test.md](demo_ftps_test.md) | Automated FTPS stress test with semaphore-driven task loop |
| [demo_htp.md](demo_htp.md) | HTTP Time Protocol client for time synchronization |

## SSL / TLS

| File | Description |
|------|-------------|
| [demo_ssl.md](demo_ssl.md) | SSL/TLS: one-way and two-way authentication, handshake, read/send |
| [demo_ssl_test.md](demo_ssl_test.md) | Automated SSL stress test |

## Time & Location

| File | Description |
|------|-------------|
| [demo_ntp.md](demo_ntp.md) | NTP time synchronization |
| [demo_rtc.md](demo_rtc.md) | Real-time clock: set/get time, alarm, UTC conversion |
| [demo_gps.md](demo_gps.md) | GNSS control: power, modes, AGPS, ephemeris, constellation config |
| [demo_loc.md](demo_loc.md) | LBS location via cell tower triangulation |
| [demo_loc_test.md](demo_loc_test.md) | LBS automated test with background task |

## GPIO & Peripherals

| File | Description |
|------|-------------|
| [demo_gpio.md](demo_gpio.md) | GPIO: direction, level, interrupt, wakeup, bulk pin test |
| [demo_i2c.md](demo_i2c.md) | I2C bus read/write with NAU8810 codec target |
| [demo_spi.md](demo_spi.md) | SPI: flash ID, NOR flash, and NAND flash operations |
| [demo_uart.md](demo_uart.md) | UART: baud/data/parity/stop config, sleep control, RS485 |
| [demo_pwm.md](demo_pwm.md) | PWM output: frequency and duty cycle control |
| [demo_onewire.md](demo_onewire.md) | 1-Wire protocol for CT1820B temperature sensor |

## Display & Camera

| File | Description |
|------|-------------|
| [demo_lcd.md](demo_lcd.md) | LCD display: ST7735S, ST7789V, ST7567A, ST7796u controllers |
| [demo_cam.md](demo_cam.md) | Camera: capture, preview, barcode scanning |
| [demo_cam_dirver.md](demo_cam_dirver.md) | Custom GC032A camera sensor driver with register tables |

## SIM & Telecom

| File | Description |
|------|-------------|
| [demo_simcard.md](demo_simcard.md) | SIM card: PIN, ICCID, IMSI, hot-swap, CRSM, dual-SIM |
| [demo_sjdr.md](demo_sjdr.md) | RF jamming detection with callback notification |

## Wi-Fi

| File | Description |
|------|-------------|
| [demo_wifi_ap.md](demo_wifi_ap.md) | Wi-Fi AP configuration: SSID, WPA2, client management |
| [demo_wifiscan.md](demo_wifiscan.md) | Wi-Fi scanning for location services (RSSI, MAC, channel) |

## Security & Crypto

| File | Description |
|------|-------------|
| [demo_crypto.md](demo_crypto.md) | Hardware crypto engine: AES, random number generation |
| [demo_mbedtls.md](demo_mbedtls.md) | mbedTLS: RSA encrypt/decrypt, AES-ECB, MD hash |
| [demo_sm2.md](demo_sm2.md) | SM2 elliptic curve: key generation, encrypt, decrypt |

## File System & Storage

| File | Description |
|------|-------------|
| [demo_file_system.md](demo_file_system.md) | File operations: open, read, write, stat, directory listing |
| [demo_flash.md](demo_flash.md) | Flash memory: erase, read, write raw sectors |
| [demo_exfs.md](demo_exfs.md) | External flash file system mounting (NOR LFS, NAND YAFFS) |

## Firmware Update

| File | Description |
|------|-------------|
| [demo_fota.md](demo_fota.md) | Firmware OTA: FTP/HTTP/local update, image write, verify |
| [demo_app_download.md](demo_app_download.md) | App firmware download and package management |
| [demo_app_updater.md](demo_app_updater.md) | App firmware update from downloaded package |

## Power & System

| File | Description |
|------|-------------|
| [demo_pm.md](demo_pm.md) | Power management: power key, ADC, VBAT, VDD_AUX, reset |
| [demo_wtd.md](demo_wtd.md) | Watchdog timer: timeout, enable, feed, disable |
| [demo_txrx_power.md](demo_txrx_power.md) | RF TX/RX power level measurement (FALCON_1803 only) |
| [demo_system.md](demo_system.md) | System info: task stack monitoring |

## Data Processing

| File | Description |
|------|-------------|
| [demo_cjson.md](demo_cjson.md) | cJSON: parse, create, modify JSON documents |
| [demo_zlib.md](demo_zlib.md) | zlib: compress/decompress, zip file creation and extraction |
