# demo_fota

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_fota.c

## Overview
This demo provides an interactive menu-driven interface for Firmware Over-The-Air (FOTA) update operations on the SIMCom A7672 module. It supports three FOTA methods: FTP download, HTTP download, and local file-based update. The demo handles the complete FOTA workflow including server authentication, image download, image writing to flash, and image verification. It also supports a mini-system status check and a mini local FOTA variant for NOR flash platforms.

## Main Features
- FTP-based FOTA with optional username/password authentication
- HTTP-based FOTA with optional username/password authentication
- Local file-based FOTA (reading firmware from local filesystem)
- Mini local FOTA for NOR flash platforms (conditionally compiled)
- FOTA download progress callback with success/failure indication
- Mini system status monitoring
- Pre-verification of local FOTA images

## Key Header Dependencies
- `simcom_fota_download.h` — Defines FOTA API functions, `SC_FotaApiParam` struct, `SC_MiniSysStatus` struct, `sc_fota_callback` typedef, and local FOTA API (`sAPI_LFotaServiceBegin`, `sAPI_LFotaPreVerify`)
- `simcom_file.h` — File system API (`sAPI_fopen`, `sAPI_fread`, `sAPI_fclose`, `sAPI_fsize`); conditionally excluded for `LTEONLY_THIN_SINGLE_SIM_2MFLASH`
- `simcom_uart.h` — UART communication types
- `simcom_common.h` — Common type definitions
- `uart_api.h` — UART read helpers for interactive input
- `local_fota.h` — Local FOTA types (conditionally included under `FEATURE_SIMCOM_NORFLASH_MINI_LFOTA`)

## SDK APIs Observed
- `sAPI_FotaServiceBegin(&param)` — Start FOTA download and update process; takes a pointer to `SC_FotaApiParam` struct
- `sAPI_FotaImageWrite(data, len, filesize)` — Write a chunk of firmware image data to flash; called repeatedly with 1024-byte chunks
- `sAPI_FotaImageVerify(filesize)` — Verify the written firmware image integrity; returns 0 on success (or 100 on Falcon 1803 platform)
- `sAPI_fopen(path, mode)` — Open a file from the module's filesystem
- `sAPI_fread(buffer, size, count, fp)` — Read data from file
- `sAPI_fclose(fp)` — Close file handle
- `sAPI_fsize(fp)` — Get file size in bytes
- `sAPI_FsSwitchDir(filename, dirType)` — Switch file directory context (used to change from C: to SIMDIR for syspatch.bin)
- `sAPI_GetMiniSysStatus(&status)` — Get mini system enable/stage status
- `sAPI_LFotaPreVerify(&param)` — Pre-verify local FOTA image (NOR flash variant)
- `sAPI_LFotaServiceBegin(&param)` — Start local FOTA update (NOR flash variant)

## Usage
1. Call `FotaDemo()` to enter the interactive menu.
2. **FTP FOTA (option 1)**: Enter the FTP server host address. Choose whether the server requires authentication (1=yes, 0=no). If yes, enter username and password. The demo calls `sAPI_FotaServiceBegin()` with `mode=0` (FTP).
3. **HTTP FOTA (option 2)**: Same flow as FTP but with `mode=1` (HTTP).
4. **Local FOTA (option 3)**: Available unless `SIMCOM_MINI_OTA` or `CRANEL_OTA` is defined. Opens `c:/syspatch.bin`, reads it in 1024-byte chunks, writes each chunk via `sAPI_FotaImageWrite()`, then verifies via `sAPI_FotaImageVerify()`.
5. **Mini Local FOTA (option 3 variant)**: Available when `FEATURE_SIMCOM_NORFLASH_MINI_LFOTA` is defined. Sets filepath to `D:/system_patch.bin`, mode to 1 (NOR flash), calls `sAPI_LFotaPreVerify()` then `sAPI_LFotaServiceBegin()`.
6. **Mini system status (option 8)**: Displays the mini system enable flag and stage via `sAPI_GetMiniSysStatus()`.
7. Option 99 returns to the previous menu.

## Implementation Notes
- The `SC_FotaApiParam` struct contains: `host[257]` (server URL with optional port), `username[257]`, `password[257]`, `mode` (0=FTP, 1=HTTP), and `sc_fota_cb` (callback function pointer).
- The `fotacb()` callback receives `isok=100` on success or other values on failure, and prints the appropriate message.
- The local FOTA workflow is a file-to-flash pipeline: open file -> read 1024-byte chunks -> write to flash via `sAPI_FotaImageWrite(data, read_len, filesize)` -> verify via `sAPI_FotaImageVerify(filesize)`.
- `sAPI_FsSwitchDir("sys_patch.bin", DIR_C_TO_SIMDIR)` changes the directory context from C: to SIMDIR before reading the local file. This is not needed on Falcon 1803 platform.
- The verification return code differs by platform: returns 0 for success on standard platforms, returns 100 for success on Falcon 1803.
- The demo conditionally excludes `simcom_file.h` and local FOTA for `LTEONLY_THIN_SINGLE_SIM_2MFLASH` to reduce flash usage on constrained builds.
- Mini FOTA (`SIMCOM_MINI_OTA` and `CRANEL_OTA`) builds exclude the local FOTA option to save memory.
- The `goto ftpstart` / `goto httpstart` patterns retry the authentication selection on invalid input.
- The demo includes a `goto error` label for local FOTA cleanup (close file, free buffer).

## Best Practices
- Always use the FOTA callback to monitor download progress and handle failures; the module may be unusable after a failed update.
- For FTP/HTTP FOTA, provide correct server credentials. Wrong credentials will cause download failure without detailed error reporting.
- The local FOTA method is useful for updating modules without network connectivity or for testing firmware images before deploying via FTP/HTTP.
- Allocate sufficient heap for the 1024-byte read buffer; the demo uses `malloc(1024)` and frees it in the error/cleanup path.
- Call `sAPI_GetMiniSysStatus()` before initiating FOTA to check if the module is already in a mini-system update state.
- After successful FOTA, the module must be reset for the new firmware to take effect. The callback prints "Please reset the module."
- For NOR flash platforms, use `sAPI_LFotaPreVerify()` to validate the image before applying it, preventing corrupted updates.
- The `SC_FotaApiParam.host` field can contain a URL or IP:port combination (e.g., "ftp.example.com" or "192.168.1.100:21").
- Ensure the firmware file (`syspatch.bin` or `system_patch.bin`) is a valid SIMCom firmware image; corrupt files will fail verification.
- In production, implement retry logic for network-based FOTA and validate network connectivity before starting the download.
