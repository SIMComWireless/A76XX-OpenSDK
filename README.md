
<img src="_htmresc/simcom_logo.png" style="background-color:rgb(251, 252, 252); padding: 5px;" width="100">

<h1 style="text-align:center">A76XX OpenSDK</h1>

A76XX OpenSDK solution is based on A76XX CAT-1 module which allows customer to run application code inside module for smart IoT applications. All A76XX CAT-1 modules could support OpenSDK,you can get module list from [SIMCom Website](https://en.simcom.com/module/4g.html).For more technical support you can contact with [SIMCom support team](https://en.simcom.com/service_cat-20.html).

[OpenSDK Supported Features](#opensdk-supported-features)

[OpenSDK Hardware Design Manual](#opensdk-hardware-design-manual)

[OpenSDK Architecture](#opensdk-architecture)

[OpenSDK Download](#opensdk-download)

[OpenSDK Compile and Image Update](#)

## OpenSDK Supported Features

|Feature|Description|Note|
|---|---|---|
|[GPIO](_htmresc/AN_GPIO.md)|For gpio map please find header file from sc_lib\inc\GPIO|Please refer to corresponding HD for OpenSDK application,see `demo_gpio.c`
|[ADC](_htmresc/AN_ADC.md)|There is general ADC and VBAT ADC|For VBAT ADC customer must follow the referrence circuit from HD,see `demo_gps.c`
|[UART](_htmresc/AN_UART.md)|UART1--Main UART<br>UART2--Dedicate for debug purpose<br>UART3--For GNSS |For modules without GNSS,UART3 can be used by customer application,see `uart_api.c\demo_uart.c`
|[I2C](_htmresc/AN_I2C.md)|Support up to 400KHz|Only for master role,see `demo_i2c.c`
|[SPI](_htmresc/AN_SPI.md)|There are 2 SPI interfaces,one for LCD and one for general purpose|For LCD application need special MMI SDK,please contact with [SIMCom Support](https://en.simcom.com/service_cat-20.html),see `demo_spi.c`
|LCD|Support SPI 3\4 line LCD|Driver supported on ST7735\S7789 series,see `demo_lcd.c`
|Audio|Support Analog and Digital Audio|see `demo_audio.c`
|BT\BLE|Support classic BT and BLE 4.0\5.0(server\client)|For BT only SPP profile supported,see `demo_ble.c\demo_bt_stack.c\demo_bt.c`
|Network|Network management|see `demo_network.c`
|FTP|Support FTP and FTPS(TLS1.2)|see `demo_ftps.c\demo_ftps_test.c`
|HTTP|Support HTTP and HTTPS(TLS1.2)|see `demo_https.c`
|GNSS|Support UC6228 and ASR5311 with different module PN|see `demo_gps.c`
|MQTT|Support up to 2 connection,support MQTTS|see `demo_mqtt.c`
|NTP|Update time with NTP|see `demo_ntp.c`
|RF TX\RX|TX\RX power measurement|see `demo_txrx_power.c`
|Flash|Direct flash read\write\erase|see `demo_flash.c`
|PWM|Support up to 4 channels|see `demo_pwm.c`
|OS|Support task\semaphore\queue\mutex\flag\timer\memory management|see `demo_system.c`
|SIM Card|Could support DSDS\DSSS|see `demo_simcard.c`
|SMS|Short Message|see `demo_sms.c`
|SSL|Support up to TLS1.3|see `demo_ssl.c\demo_ssl_test.c\demo_mbedtls.c`
|TCP\IP|Support TCP client\server and UDP client|see `demo_tcpip.c`
|TTS|Text to speech with local and remote|Only support Chinese\English,see `demo_tts.c`
|Voice Call|Support CS\VOLTE|see `demo_call.c`
|WiFi Scan|Module scan wifi AP info with cellular B140 and get SSID\RSSI which can be used for WiFi Location|Only Wifi RX supported,no TX.Only specific hardware PN could support it,see `demo_wifi.c`
|Power management|Module power control|see `demo_pm.c`
|JD|Jamming Detection|see `demo_shdr.c`
|LBS|Location based on cellular cell scaning|see `demo_loc.c\demo_loc_test.c`
|Data Call|Support IP\IPV6\IPV4V6 with different PDP|PPP\RNDIS\ECM,driver support on Windows\Linux
|File System|Support standard fs operation|see `demo_file_system.c`
|FOTA|Support both SDK core and app update|see `demo_fota.c\demo_app_download.c\demo_app_updater.c`
|HTP|Update time with HTTP|see `demo_ftp.c`
|PING|Support IPV4\V6|see `demo_ping.c`
|RTC|Support RTC alarm to wake up module from sleep mode|see `demo_rtc.c`
|USB|Write\Read from USB_AT interface|For OpenSDK application,as default USB_AT interface will not support AT cmd,while you can use USB_Modem port instead,see `cus_usb_vcom.c`
|Watchdog|Hardware Watchdog,support up to 60 seconds timer configuration|see `demo_wtd.c`
|Camera|Camera Driver|Only for MMI version of SDK and dedicate hardware,see `demo_cam_driver.c\demo_cam.c`
|POC|POC application with PCM play\record|see `demo_poc.c`
|Public lib|cjson\zlib\sm2\miracl|see `demo_zlib.c\demo_cjson.c\demo_crypto.c`
|OneWire|Onewire protocol for sensor like DS18B20|see `demo_onewire.c`

## OpenSDK Hardware Design Manual

A hardware design manual is usually needed when a developer starts to evaluate the peripheral interfaces used for application.With SIMCom A76XX CAT-1 module there are 2 typical applications,Standard and OpenSDK.For standard application there is a host processor(CPU\MCU\MPU) which will comunicate with module over AT cmd via UART\USB interface, while for OpenSDK application usually there is no external host to control module and it runs application code to manage avaliable peripherals inside module. The hardware design manual(HD) for both application is different due to some IO\feature differences.

You can request OpenSDK HD for specific modules from [SIMCom support team](https://en.simcom.com/service_cat-20.html).Here are some general OpenSDK HD files for A7672x\A7602x\A7683x\A7676x series(continue updating).

[OpenSDK HD](https://1drv.ms/f/c/1964fa2b798f638e/EjjekV7eIWFIsTU_6Qdv4NQByvkphD0XEcp4LE92QgxZiQ?e=5uMjff)

## OpenSDK Architecture

## OpenSDK Download

## OpenSDK Compile and Image Update
### *SDK Compile under Windows OS*
1. Please copy GNUmake.exe file from SDK root path to `C:\Windows\System32` and `C:\Windows\SysWOW64` folders.
2. Run CMD line and check `gnumake -v` command to see if make tool has been installed well.You will see following messages if gnumake is correctly installed.
```
GNU Make 3.81
Copyright (C) 2006  Free Software Foundation, Inc.
This is free software; see the source for copying conditions.
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.

This program built for i386-pc-mingw32
```
3. Run gnumake.exe 