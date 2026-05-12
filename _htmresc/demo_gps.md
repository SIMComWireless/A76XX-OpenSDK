# demo_gps

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_gps.c

## Overview
This demo provides a comprehensive interactive menu-driven interface for controlling GNSS (GPS/BDS/GLONASS/GALILEO) functionality on the SIMCom A7672 module. It covers power management, NMEA data output, constellation mode selection, AGPS operations, baud rate configuration, and advanced features like predicting ephemeris and PVT partition cleanup. The demo is conditionally compiled under `FEATURE_SIMCOM_GPS` or `JACANA_GPS_SUPPORT`.

## Main Features
- GNSS AP flash hot-start flag control (on/off/get)
- GNSS power on/off and status query
- NMEA data output control (via NMEA port or URC)
- GNSS start mode selection (hot/warm/cold start)
- Baud rate configuration (9600, 115200, 230400 depending on chip)
- Constellation mode set/get (GPS, BDS, GLONASS, GALILEO combinations)
- NMEA update rate configuration (1Hz, 2Hz, 5Hz)
- NMEA sentence type bitmask configuration
- GPS and GNSS information periodic reporting
- Direct NMEA command pass-through to GNSS chip
- AGPS data retrieval, local storage, injection, and validity checking
- Predicting ephemeris download and update
- PVT partition cleanup
- AGPS server parameter configuration
- GNSS dynamic load file type selection (conditional on `GPS_DY_LOAD`)
- GNSS navigation data information

## Key Header Dependencies
- `simcom_gps.h` — Defines all GNSS enums, structs, and API function declarations for GNSS control
- `simcom_simcard.h` — Provides `sAPI_SimcardPinGet()` for SIM card state checking before AGPS
- `simcom_network.h` — Provides `sAPI_NetworkInit()` and `sAPI_NetworkGetCgreg()` for network registration checks
- `simcom_os.h` — OS abstraction layer (tasks, sleep, message queues)
- `simcom_common.h` — Common type definitions
- `simcom_debug.h` — Debug logging via `sAPI_Debug()`

## SDK APIs Observed
- `sAPI_GnssPowerStatusSet(power)` — Set GNSS power on/off
- `sAPI_GnssPowerStatusGet()` — Get current GNSS power status
- `sAPI_GnssApFlashSet(ctl)` — Set AP flash hot-start flag on/off
- `sAPI_GnssApFlashGet()` — Get AP flash hot-start flag status
- `sAPI_GnssApFlashDataGet()` — Retrieve data from AP flash and store in module NVM
- `sAPI_GnssNmeaDataGet(ctl, mode)` — Start/stop NMEA data output (by port or URC)
- `sAPI_GnssStartMode(mode)` — Set GNSS start mode (hot/warm/cold)
- `sAPI_GnssBaudRateSet(baudrate)` — Set GNSS baud rate
- `sAPI_GnssBaudRateGet()` — Get current GNSS baud rate
- `sAPI_GnssModeSet(mode)` — Set constellation mode
- `sAPI_GnssModeGet()` — Get current constellation mode
- `sAPI_GnssNmeaRateSet(rate)` — Set NMEA update rate
- `sAPI_GnssNmeaRateGet()` — Get current NMEA update rate
- `sAPI_GnssNmeaSentenceSet(mask)` — Set NMEA sentence bitmask
- `sAPI_GnssNmeaSentenceGet()` — Get NMEA sentence bitmask array
- `sAPI_GpsInfoGet(period)` — Set GPS information reporting cycle (0 to stop)
- `sAPI_GnssInfoGet(period)` — Set GNSS information reporting cycle (0 to stop)
- `sAPI_SendCmd2Gnss(string)` — Send raw NMEA command to GNSS chip
- `sAPI_GnssAgpsSeviceOpen()` — Download AGPS data from server to GPS chip
- `sAPI_GnssAgpsDataSavetoLocal(method)` — Save AGPS data locally (RAM/Flash/delete)
- `sAPI_GnssAgpsDatatoGpsChip(opt)` — Inject locally stored AGPS data to GPS chip with optional time/position aid
- `sAPI_GnssAgpsDataCheck()` — Verify AGPS data injection validity
- `sAPI_GnssNavdataGet()` — Get pointer to GNSS navigation data structure
- `sAPI_GnssDynamicLoadFileTypeSet(type)` — Set GNSS dynamic load file type
- `sAPI_GnssDynamicLoadFileTypeGet()` — Get GNSS dynamic load file type
- `sAPI_GnssPreEphDataLoadAndUpdate(opt)` — Download/update predicted ephemeris
- `sAPI_GnssPvtClean()` — Clean jacana_pvt partition
- `sAPI_GnssAgpsParSet(config)` — Set AGPS server parameters
- `sAPI_GnssAgpsParGet()` — Get AGPS server parameters
- `sAPI_SimcardPinGet(&cpin)` — Get SIM card PIN state
- `sAPI_NetworkInit()` — Initialize network module
- `sAPI_NetworkGetCgreg(&pGreg)` — Get network GPRS registration status
- `sAPI_Debug(fmt, ...)` — Debug log output
- `sAPI_TaskSleep(ticks)` — Sleep current task

## Usage
1. Ensure the module has `FEATURE_SIMCOM_GPS` or `JACANA_GPS_SUPPORT` defined at build time.
2. Call `GNSSDemo()` to enter the interactive menu.
3. Power on GNSS first (option 2 -> power on) before performing any GNSS operations.
4. Select menu options to configure constellation mode, baud rate, NMEA rate, and sentence types.
5. Use the NMEA output option (option 3) to start/stop receiving NMEA data via port or URC.
6. For AGPS operations, ensure the SIM card is inserted, PIN is ready, and the network is registered (CGREG=1). The demo automatically checks these conditions.
7. GPS and GNSS information reporting uses a 0-255 second period; set to 0 to stop.
8. The "send command" option (option 11) allows sending raw NMEA strings directly to the GNSS chip.

## Implementation Notes
- The demo uses `#ifdef` guards extensively to support multiple GNSS chip families: JACANA (Jacana), UNICORECOMM (UC6228), CC1161W, CC1167Q, CC1177W, AG3352, and ZKW. Each chip has different supported modes, baud rates, and NMEA rate options.
- The `networkIsNormal()` helper function verifies SIM card readiness and network registration before AGPS operations, with a retry loop (10 attempts with 600ms intervals).
- The `powerOn()` helper returns 1 if GNSS is powered on, 0 otherwise. Most operations require GNSS to be powered on and will redirect back if it is not.
- Menu navigation uses `goto` labels for sub-menu loops, returning to the main menu on option 99.
- NMEA sentence bitmask bits vary by chip: JACANA uses 9 bits (RMC, VTG, GGA, GSA, GSV, GLL, ZDA, GST, TXT); UNICORECOMM uses 8 bits (GGA, GLL, GSA, GSV, RMC, VTG, ZDA, GST).
- The `SC_GNSSINFO` struct provides comprehensive satellite info including per-constellation satellite counts and SV arrays.
- AGPS data injection supports optional auxiliary time (format: "year,month,day,hour,minute,second,milliseconds") and auxiliary position (format: "latitude_ddmm.mmmmm,longitude_dddmm.mmmmmm").

## Best Practices
- Always power on GNSS before configuring modes, rates, or retrieving data.
- Verify SIM card state and network registration before initiating AGPS downloads.
- Use appropriate constellation modes for the target region (e.g., GPS+GLONASS for Russia, GPS+BDS+QZSS for China/Japan).
- When using AGPS, select the correct server version (2 or 3) based on the AGNSS server URL.
- For assisted time/position injection accuracy: time error must be within 1ms, position error within 5KM. Invalid values slow down positioning.
- Set NMEA reporting period to 0 to stop periodic reporting and reduce power consumption.
- Use AP flash hot-start for faster TTFF after power cycle if GNSS data is stored in flash.
- Clean the PVT partition only when troubleshooting positioning issues; it clears historical data.
- Monitor the `SC_Gnss_User_CallBack` state events (via `sAPI_GnssStateBindCB`) in production to handle initialization/download failures gracefully.
