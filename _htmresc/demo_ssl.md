# demo_ssl

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_ssl.c

## Overview

This file provides an interactive SSL/TLS client demonstration for the SIMCom A7672 module. It supports three modes of operation: basic SSL connection (one-way authentication), automated SSL stress testing, and two-way mutual TLS authentication with client certificates. The demo establishes TCP sockets, performs DNS resolution, and uses the SIMCom SSL library for encrypted communication with remote servers.

## Main Features

- Three operational modes: SSL connect demo, SSL stress test, and two-way mutual authentication
- Basic SSL connection with one-way server authentication to `www.baidu.com:443`
- Two-way mutual TLS authentication with CA certificate, client certificate, and client key verification
- PDP context activation/deactivation management
- DNS resolution using both `sAPI_TcpipGethostbyname()` and POSIX `getaddrinfo()`
- Configurable SSL version (SSL 3.0, TLS 1.0, 1.1, 1.2, or ALL)
- SNI (Server Name Indication) support
- Configurable cipher suites
- Certificate-based authentication with embedded PEM certificates
- Stress test integration delegating to `demo_ssl_test.c`

## Key Header Dependencies

- `simcom_ssl.h` -- Provides SSL API functions (`sAPI_SslHandShake`, `sAPI_SslSend`, `sAPI_SslRead`, `sAPI_SslClose`), `SCSslCtx_t` context structure, SSL version and verify mode enums, and error code constants
- `simcom_tcpip.h` -- Provides TCP/IP socket functions (`sAPI_TcpipSocket`, `sAPI_TcpipConnect`, `sAPI_TcpipGethostbyname`, `sAPI_TcpipPdpActive`, `sAPI_TcpipPdpDeactive`, `sAPI_TcpipClose`, `sAPI_TcpipHtons`)
- `simcom_tcpip_old.h` -- Provides legacy TCP/IP definitions
- `simcom_os.h` -- Provides OS primitives: task creation, sleep
- `simcom_debug.h` -- Provides `sAPI_Debug()` for debug logging
- `uart_api.h` -- Provides `UartReadValue()` for UART input

## SDK APIs Observed

- `sAPI_TcpipPdpActive(cid, ch)` -- Activate PDP context for data communication
- `sAPI_TcpipPdpDeactive(cid, ch)` -- Deactivate PDP context
- `sAPI_TcpipSocket(domain, type, protocol)` -- Create a TCP socket
- `sAPI_TcpipGethostbyname(hostname)` -- DNS resolution for a hostname
- `sAPI_TcpipHtons(port)` -- Convert port number to network byte order
- `sAPI_TcpipConnect(sockfd, addr, len)` -- Connect socket to remote server
- `sAPI_TcpipClose(sockfd)` -- Close a socket
- `sAPI_SslHandShake(ctx)` -- Perform SSL/TLS handshake with configured context
- `sAPI_SslSend(clientId, buf, len)` -- Send data over SSL connection
- `sAPI_SslRead(clientId, buf, len)` -- Read data from SSL connection
- `sAPI_SslClose(clientId)` -- Close SSL connection
- `sAPI_TaskCreate(ref, stack, stackSize, priority, name, func, arg)` -- Create task (for two-way auth demo)

## Usage

1. Call `SslDemo()` to enter the menu.
2. Select an operational mode:

### Mode 1: SSL Connect Demo (One-Way Authentication)

1. Activate PDP context via `sAPI_TcpipPdpActive(1, 1)`.
2. Create a TCP socket via `sAPI_TcpipSocket(SC_AF_INET, SC_SOCK_STREAM, 0)`.
3. Resolve "www.baidu.com" via `sAPI_TcpipGethostbyname()`.
4. Connect to port 443 via `sAPI_TcpipConnect()`.
5. Configure `SCSslCtx_t` with `fd=sockfd`, `ssl_version=SC_SSL_CFG_VERSION_ALL`.
6. Perform handshake via `sAPI_SslHandShake(&ctx)`.
7. If handshake succeeds: send HTTP GET request via `sAPI_SslSend()`, read response via `sAPI_SslRead()`.
8. Close SSL and socket, deactivate PDP.

### Mode 2: SSL Stress Test

Enter the number of test iterations. Delegates to `SslTestDemoInit(testTime)` in `demo_ssl_test.c`.

### Mode 3: Two-Way Mutual Authentication

1. A separate task (`ssl_verify`) is created via `sAPI_TaskCreate()`.
2. Activates PDP context with retry (up to 10 attempts, 3s between).
3. Resolves "mouwy.top:6066" using POSIX `getaddrinfo()`.
4. Connects via standard `socket()` and `connect()` calls.
5. Configures `SCSslCtx_t` with:
   - `ssl_version = SC_SSL_CFG_VERSION_TLS12`
   - `auth_mode = SC_SSL_CFG_VERIFY_MODE_SERVER_CLIENT`
   - `enable_SNI = 1`, `ipstr = "mouwy.top"`
   - Embedded CA certificate, client certificate, and RSA private key
   - `ignore_local_time = 1` (skip certificate time validation)
6. Performs handshake, sends HTTP GET request, reads response in a loop until connection closes.
7. Closes SSL, socket, and deactivates PDP.

## Implementation Notes

- The basic demo (Mode 1) uses `sAPI_TcpipGethostbyname()` for DNS, while the two-way auth demo uses the POSIX `getaddrinfo()` API.
- The two-way auth demo includes full PEM-encoded CA certificate, client certificate, and RSA private key embedded in the source code for testing against `mouwy.top:6066`.
- The HTTP request sent is a raw GET request: `GET / HTTP/1.1\r\nHost: www.baidu.com\r\n...`
- `SCSslCtx_t` fields include: `ClientId`, `ciphersuitesetflg`, `ciphersuite[8]`, `ssl_version`, `enable_SNI`, `auth_mode`, `ignore_local_time`, `ipstr`, `root_ca`, `client_cert`, `client_key` (plus PSK fields), `fd`, `timeout_ms`, `negotiate_time`.
- Verify modes: `SC_SSL_CFG_VERIFY_MODE_NONE` (0), `SC_SSL_CFG_VERIFY_MODE_REQUIRED` (1), `SC_SSL_CFG_VERIFY_MODE_SERVER_CLIENT` (2), `SC_SSL_CFG_VERIFY_MODE_CLIENT` (3).
- SSL versions: `SC_SSL_CFG_VERSION_SSL30` (0) through `SC_SSL_CFG_VERSION_ALL` (4).
- A large block of legacy code at the bottom (`#if 0`) shows an alternative SSL test implementation using `sAPI_SslSetContextIdMsg()` and `sAPI_SslGetContextIdMsg()`.
- Error codes defined: `SC_SSL_WANT_READ` (-0x6900), `SC_SSL_WANT_WRITE` (-0x6880), `SC_SSL_TIMEOUT` (-0x6800), `SC_SSL_NET_CON_RESET` (-0x0050), `SC_SSL_NET_RECV_FAILED` (-0x004C), `SC_SSL_NET_SEND_FAILED` (-0x004E).

## Best Practices

- Always activate PDP context before creating sockets and performing SSL operations.
- Deactivate PDP context after closing all sockets and SSL connections to release network resources.
- Set `ssl_version` to `SC_SSL_CFG_VERSION_TLS12` or higher for production security; avoid `SC_SSL_CFG_VERSION_SSL30`.
- Enable SNI (`enable_SSI=1`) when connecting to servers that host multiple domains.
- Set `ignore_local_time=0` in production to validate certificate validity periods.
- For mutual authentication, provide complete certificate chains (CA cert, client cert, client key) in PEM format.
- Handle SSL error codes (negative return values from `sAPI_SslHandShake`) to diagnose connection issues.
- Use `sAPI_SslSend()`/`sAPI_SslRead()` return values to detect partial sends/receives and connection closures.
