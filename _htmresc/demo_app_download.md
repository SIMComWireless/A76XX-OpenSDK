# demo_app_download

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_app_download.c

## Overview

This demo demonstrates how to download a new customer application binary (`customer_app.bin`) from a remote server using HTTP, HTTPS, FTP, or FTPS protocols, and write it to the module's reserved flash partition for OTA (Over-The-Air) firmware updates. After a successful download and CRC verification, the module resets to apply the update.

## Main Features

- Download application binary via HTTP (port 80)
- Download application binary via HTTPS (port 443) with SSL/TLS
- Download application binary via FTP with passive mode (PASV/EPSV)
- Download application binary via FTPS with SSL/TLS
- Parse server addresses from URLs for both HTTP and FTP formats
- Write downloaded data directly to the app package partition
- CRC verification of downloaded package
- Automatic system reset after successful update
- Interactive UART menu for URL input

## Key Header Dependencies

- `sc_app_download.h` -- Provides download API types and functions: SCAppDownloadPram, SCAppDwonLoadReturnCode, SCAppDownloadMod, sAPI_AppDownload, SC_FOTA_URL_MAX_SIZE
- `sc_app_update.h` -- Provides app package management: sAPI_AppPackageOpen, sAPI_AppPackageWrite, sAPI_AppPackageClose, sAPI_AppPackageCrc, SCAppPackageInfo
- `sc_flash.h` -- Provides sAPI_FBF_Disable (disable fast boot flash protection before download)
- `simcom_tcpip.h` -- Provides sAPI_TcpipPdpActive, sAPI_TcpipPdpDeactive for network connectivity
- `scfw_socket.h`, `scfw_netdb.h`, `scfw_inet.h` -- Provides BSD socket API: socket, connect, send, recv, close, getaddrinfo, freeaddrinfo, setsockopt, getsockname, lwip_getsockerrno
- `simcom_ssl.h` -- Provides sAPI_SslHandShake, sAPI_SslSend, sAPI_SslRead, sAPI_SslClose for SSL/TLS connections
- `sc_power.h` -- Provides sAPI_SysReset for system reboot

## SDK APIs Observed

- `sAPI_TcpipPdpActive(id, state)` -- Activate/deactivate PDP context for network connectivity. Must be called before socket operations.
- `sAPI_FBF_Disable()` -- Disable Fast Boot Flash protection. Must be called before writing to the app partition.
- `sAPI_AppPackageOpen("w")` -- Open the app package partition for writing. Returns SC_SUCCESS on success.
- `sAPI_AppPackageWrite(data, size)` -- Write a chunk of data to the app package partition.
- `sAPI_AppPackageClose()` -- Close the app package partition after writing is complete.
- `sAPI_AppPackageCrc(&pInfo)` -- Verify the CRC of the downloaded app package. Fills SCAppPackageInfo with hasPackage, binSize, and crcValue.
- `sAPI_SysReset()` -- Perform a system reset to boot into the newly downloaded firmware.
- `sAPI_SslHandShake(&ctx)` -- Perform SSL/TLS handshake on a connected socket.
- `sAPI_SslSend(client_id, buffer, size)` -- Send data over an SSL connection.
- `sAPI_SslRead(client_id, buffer, size)` -- Read data from an SSL connection.
- `sAPI_SslClose(client_id)` -- Close an SSL connection.
- `sAPI_TcpipPdpDeactive(id, state)` -- Deactivate PDP context after download.

## Usage

The demo presents an interactive menu:

1. **HTTP APP DOWNLOAD** (Option 1): Enter the server host (e.g., "47.108.134.22/lugc_fota/customer_app.bin"). Constructs an HTTP URL, connects, sends GET request, receives data, writes to app partition.
2. **HTTPS APP DOWNLOAD** (Option 2): Same as HTTP but with SSL/TLS on port 443.
3. **FTP APP DOWNLOAD** (Option 3): Enter host, username, and password. Connects via FTP passive mode, retrieves the file.
4. **FTPS APP DOWNLOAD** (Option 4): Same as FTP but with SSL/TLS, including PBSZ and PROT commands.
5. **APP DOWNLOAD TEST** (Option 5): Reserved for testing.
6. **Back** (Option 99): Return to parent menu.

After successful download, the demo calls `sAPI_AppPackageCrc` to verify the package, then `sAPI_SysReset` to reboot into the new firmware.

## Implementation Notes

- The `demo_AppDownload` function handles the entire download workflow: PDP activation, URL parsing, TCP connection, optional SSL handshake, HTTP GET or FTP RETR command, data reception, and partition writing.
- For HTTP, the response header is parsed to extract Content-Length. Data after the header boundary is written to the partition in 2048-byte chunks.
- For FTP, passive mode is used (PASV for IPv4, EPSV for IPv6). The file size is obtained via the SIZE command. Data is read from the data connection and written to the partition.
- The URL parser (`sc_parse_server_addr`) handles IPv6 addresses in brackets, custom ports, and extracts hostname, port, and path.
- The FTP URL parser (`sc_parse_ftp_server_addr`) extracts username, password, host, port, and filename from URLs like `ftp://user:pass@host:port/path`.
- `sAPI_FBF_Disable()` is called before download to unlock the flash for writing. This is platform-specific (not needed on 1803).
- The partition is opened with `sAPI_AppPackageOpen("w")`, written chunk by chunk, and closed with `sAPI_AppPackageClose()`.
- Error codes are defined in the `SCAppDwonLoadReturnCode` enum covering all failure modes.
- The `sAPI_app_download_test` function provides a hardcoded FTP test URL for automated testing.

## Best Practices

- Always activate the PDP context (`sAPI_TcpipPdpActive`) before attempting any network operations.
- Call `sAPI_FBF_Disable()` before downloading to the app partition. On the 1803 platform, this API is not available.
- Verify network registration status before starting a download (check CGREG).
- Set appropriate receive timeouts (`recvtimeout` in SCAppDownloadPram) to avoid indefinite blocking.
- Always verify the downloaded package with `sAPI_AppPackageCrc` before resetting.
- Free all allocated memory (URL, buffers) after download, especially in error paths.
- For HTTPS/FTPS, ensure SSL certificates are properly configured before attempting secure connections.
- Use a stable network connection for OTA downloads. Consider implementing retry logic for failed downloads.
- The `customer_app.bin` file must be a valid, properly compiled application binary for the target module.
