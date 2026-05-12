# demo_ssl_test

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_ssl_test.c

## Overview

This file provides an automated SSL/TLS stress test implementation for the SIMCom A7672 module. It establishes a one-way authenticated SSL connection to `www.baidu.com:443`, sends a raw HTTP GET request, reads the response, and cleanly closes. The `SslTestDemoInit()` function runs this test sequence repeatedly for a configurable number of iterations, suitable for regression and stability testing.

## Main Features

- Automated SSL connection test: PDP activation, socket creation, DNS resolution, TCP connect, SSL handshake, send, receive, cleanup
- Configurable test iteration count
- Single-function test logic (`SslDemoTest`) with complete error handling
- PDP context activation and deactivation integrated into each test cycle
- Clean resource cleanup on all error paths (DNS failure, connect failure, handshake failure)

## Key Header Dependencies

- `simcom_ssl.h` -- Provides `sAPI_SslHandShake()`, `sAPI_SslSend()`, `sAPI_SslRead()`, `sAPI_SslClose()`, `SCSslCtx_t` structure
- `simcom_tcpip.h` -- Provides `sAPI_TcpipSocket()`, `sAPI_TcpipConnect()`, `sAPI_TcpipGethostbyname()`, `sAPI_TcpipPdpActive()`, `sAPI_TcpipPdpDeactive()`, `sAPI_TcpipClose()`, `sAPI_TcpipHtons()`
- `simcom_tcpip_old.h` -- Provides legacy TCP/IP definitions
- `simcom_debug.h` -- Provides `sAPI_Debug()` for debug logging
- `simcom_os.h` -- Provides OS primitives
- `simcom_common.h` -- Provides common type definitions

## SDK APIs Observed

- `sAPI_TcpipPdpActive(cid, ch)` -- Activate PDP context
- `sAPI_TcpipPdpDeactive(cid, ch)` -- Deactivate PDP context
- `sAPI_TcpipSocket(domain, type, protocol)` -- Create a TCP socket
- `sAPI_TcpipGethostbyname(hostname)` -- DNS resolution
- `sAPI_TcpipHtons(port)` -- Convert port to network byte order
- `sAPI_TcpipConnect(sockfd, addr, len)` -- Connect to remote server
- `sAPI_TcpipClose(sockfd)` -- Close socket
- `sAPI_SslHandShake(ctx)` -- Perform SSL handshake
- `sAPI_SslSend(clientId, buf, len)` -- Send data over SSL
- `sAPI_SslRead(clientId, buf, len)` -- Read data from SSL
- `sAPI_SslClose(clientId)` -- Close SSL connection

## Usage

1. This file is called from `demo_ssl.c` via `SslTestDemoInit(testTime)`.
2. `SslTestDemoInit()` runs a loop calling `SslDemoTest()` for the specified number of iterations.
3. Each `SslDemoTest()` iteration:
   - Activates PDP context (`sAPI_TcpipPdpActive(1, 1)`)
   - Creates a TCP socket
   - Resolves "www.baidu.com" via DNS
   - Connects to port 443
   - Configures SSL context with `ssl_version=SC_SSL_CFG_VERSION_ALL`
   - Performs SSL handshake
   - Sends a raw HTTP GET request
   - Reads response into a 1024-byte buffer
   - Closes SSL, socket, and deactivates PDP
4. On any error (socket, DNS, connect, handshake), the test cleans up resources and continues to the next iteration.
5. Progress is reported via UART with current iteration number.

## Implementation Notes

- The HTTP request is a pre-formed byte array containing: `GET / HTTP/1.1\r\nHost: www.baidu.com\r\nCache-Control: no-cache\r\nContent-Type: text/plain\r\nAccept: */*\r\n\r\n`
- The SSL context uses `SC_SSL_CFG_VERSION_ALL` which negotiates the highest available protocol (SSL 3.0 through TLS 1.2).
- Error messages are formatted and sent to UART via `PrintfResp()` on each failure path.
- The receive buffer is 1024 bytes; larger responses will be truncated.
- Unlike the main `SslDemo()`, this test does not include the two-way authentication mode.
- PDP deactivation is called on every error path to prevent resource leaks.

## Best Practices

- Run this test with a stable network connection for meaningful results.
- Monitor for SSL handshake failures which may indicate certificate or network issues.
- Use a reasonable test count; very high counts may accumulate connection state issues.
- For production testing, consider testing against your actual target servers rather than baidu.com.
- Check the return value of `sAPI_SslRead()` to determine how many bytes were actually received.
