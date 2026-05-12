# demo_ping

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_ping.c

## Overview

This file provides a simple ping demonstration for the SIMCom A7672 module. It sends ICMP echo requests to a target host (domain name or IP address) and receives echo replies, reporting results through a callback function. The demo is one of the simplest network diagnostic tools in the SDK.

## Main Features

- ICMP ping to a domain name or IP address
- Configurable ping count and interval
- Asynchronous result reporting via callback function
- Statistics output including packet size and summary information

## Key Header Dependencies

- `simcom_ping.h` -- Provides `sAPI_Ping()`, `SCping` context structure, `SCPingResultCode` enum, and `pingResultCallback` function pointer type
- `simcom_debug.h` -- Provides `sAPI_Debug()` for debug logging

## SDK APIs Observed

- `sAPI_Ping(ctx)` -- Execute a ping operation with the given context; returns `SCPingResultCode`

### SCping Context Structure

- `destination` (INT8*) -- Target hostname or IP address
- `count` (UINT32) -- Number of ping packets to send
- `interval` (UINT32) -- Interval between packets (in seconds)
- `resultcb` (pingResultCallback) -- Callback function invoked for each result

### SCPingResultCode Enum

- `SC_PING_SUCESSED` (0) -- Ping succeeded
- `SC_PING_DOMAIN_UNKNOW` -- DNS resolution failed
- `SC_PING_PRAM_INVALID` -- Invalid parameters
- `SC_PING_OPERATION_BUSY` -- Ping operation already in progress
- `SC_PING_SEND_ERR` -- Send error
- `SC_PING_RECV_ERR` -- Receive error
- `SC_PING_FAIL` -- General failure

## Usage

1. Call `PingDemo()` to execute the ping test.
2. The function configures a `SCping` structure:
   - `destination` = "www.baidu.com"
   - `count` = 20 (send 20 ping packets)
   - `interval` = 10 (10-second interval between packets)
   - `resultcb` = `pingResult` callback
3. Calls `sAPI_Ping(&ctx)` which performs the ping operation.
4. The `pingResult` callback is invoked with each response, printing the packet size and statistics string via `sAPI_Debug()`.
5. The return code from `sAPI_Ping()` is logged.

## Implementation Notes

- The `pingResult` callback receives `size` (response packet size) and `statistics` (summary string from the SDK).
- The ping operation is blocking -- `sAPI_Ping()` does not return until all packets are sent/received or a timeout occurs.
- The destination can be either a domain name (requiring DNS resolution) or a dotted-decimal IP address.
- The interval parameter is in seconds (10 in the demo = 10-second gap between pings).
- With `count=20` and `interval=10`, the full ping session can take up to ~200 seconds.

## Best Practices

- Verify network connectivity (PDP context active, registered to network) before running ping.
- Use ping as a basic network diagnostic tool to verify connectivity to a server before attempting application-level connections.
- Keep the count reasonable for production use; 20 pings with 10-second intervals takes over 3 minutes.
- The callback function should process results quickly to avoid blocking the ping thread.
- Consider using shorter intervals (e.g., 1-3 seconds) for faster diagnostics.
- The `statistics` string typically contains loss percentage, round-trip min/avg/max times -- format depends on the SDK implementation.
