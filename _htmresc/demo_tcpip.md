# demo_tcpip

Location: `SIMCOM_SDK_SET/sc_demo/V1/src/demo_tcpip.c`

## Overview

This demo provides an interactive UART menu for TCP/IP networking on the A7672 module. It demonstrates PDP context activation, TCP client connect/send/receive, UDP client open/send/receive, and TCP server operations using standard BSD socket APIs (via `scfw_socket.h`) combined with SIMCom-specific PDP management APIs (from `simcom_tcpip.h`). The demo uses a multi-threaded architecture with dedicated message-queue-driven receive tasks for client and server sockets.

## Main Features

- PDP context activation/deactivation with IP address retrieval (IPv4 and IPv6)
- TCP client: DNS resolution, connect, send, receive (via select-based polling)
- UDP client: open socket, send to remote address, receive (via select-based polling)
- TCP server: socket creation, bind, listen, accept
- Multi-threaded receive handling with message queues
- Mutex-protected socket descriptor management
- Standard BSD socket API usage (socket, connect, send, sendto, recvfrom, close, select)
- DNS resolution via `getaddrinfo()`
- Dual-stack IPv4/IPv6 support

## Key Header Dependencies

- `simcom_tcpip.h` -- PDP management API: `sAPI_TcpipPdpActive()`, `sAPI_TcpipPdpDeactive()`, `sAPI_TcpipGetSocketPdpAddr()`, and `SCipInfo`/`SCnetType` types.
- `simcom_tcpip_old.h` -- Legacy TCP/IP API (referenced but not read; likely backward compatibility).
- `scfw_socket.h` -- BSD socket API declarations (socket, connect, send, sendto, recvfrom, close, select, bind, listen, accept, FD_SET/FD_ISSET/FD_ZERO macros).
- `scfw_netdb.h` -- DNS resolution (getaddrinfo, freeaddrinfo).
- `scfw_inet.h` -- IP address conversion (inet_ntoa, inet_ntop, inet_pton, ntohs, htons).
- `simcom_os.h` -- RTOS primitives (sAPI_MsgQCreate, sAPI_MsgQSend, sAPI_MsgQRecv, sAPI_TaskCreate, sAPI_MutexCreate, sAPI_MutexLock, sAPI_MutexUnLock).
- `simcom_common.h` -- Common types
- `simcom_debug.h` -- Debug logging
- `uart_api.c` -- UI helpers

## SDK APIs Observed

### SIMCom TCP/IP (simcom_tcpip.h)
- `sAPI_TcpipPdpActive(cid, channel)` -- Activates PDP context. Returns `SC_TCPIP_SUCCESS` (0) on success. Must be called before socket operations.
- `sAPI_TcpipPdpDeactive(cid, channel)` -- Deactivates PDP context.
- `sAPI_TcpipGetSocketPdpAddr(cid, channel, &ipinfo)` -- Gets the local IP address for a CID. Returns `SCipInfo` with type (TCPIP_PDP_IPV4, TCPIP_PDP_IPV6, TCPIP_PDP_IPV4V6), ip4 (UINT32), and ip6 (UINT32[4]).

### BSD Sockets (scfw_socket.h)
- `socket(family, type, protocol)` -- Creates a socket. Family: AF_INET or AF_INET6. Type: SOCK_STREAM (TCP) or SOCK_DGRAM (UDP).
- `connect(fd, addr, len)` -- Connects a TCP socket.
- `send(fd, data, len, flags)` -- Sends data on a connected TCP socket.
- `sendto(fd, data, len, flags, dest, destlen)` -- Sends data to a specific address (UDP).
- `recvfrom(fd, buf, len, flags, addr, &addrlen)` -- Receives data and source address.
- `select(fdmax, &readfds, NULL, NULL, &tv)` -- I/O multiplexing with timeout.
- `close(fd)` -- Closes a socket.
- `bind(fd, addr, len)` -- Binds a socket to a local address (server).
- `listen(fd, backlog)` -- Listens for incoming connections.
- `accept(fd, addr, &len)` -- Accepts an incoming TCP connection.
- `lwip_getsockerrno(fd)` -- Gets the last socket error code.
- `FD_ZERO`, `FD_SET`, `FD_ISSET` -- File descriptor set operations.

### DNS (scfw_netdb.h)
- `getaddrinfo(host, port, &hints, &addr_list)` -- Resolves hostname/IP to address list.
- `freeaddrinfo(addr_list)` -- Frees address list.

### IP Conversion (scfw_inet.h)
- `inet_ntoa(addr)` -- Converts IPv4 address to string.
- `inet_ntop(family, src, dst, size)` -- Converts binary address to string (IPv4/IPv6).
- `ntohs(val)` -- Network to host byte order (16-bit).

### RTOS (simcom_os.h)
- `sAPI_MsgQCreate(&q, name, msgSize, count, mode)` -- Creates a message queue.
- `sAPI_MsgQSend(q, &msg)` / `sAPI_MsgQRecv(q, &msg, SC_SUSPEND)` -- Send/receive messages.
- `sAPI_TaskCreate(&task, stack, stackSize, priority, name, func, arg)` -- Creates a task/thread.
- `sAPI_MutexCreate(&mutex, mode)` -- Creates a mutex.
- `sAPI_MutexLock(mutex, SC_SUSPEND)` / `sAPI_MutexUnLock(mutex)` -- Lock/unlock mutex.

## Usage

1. Enter the TcpipDemo menu.
2. **Init**: Select option 1 to activate PDP context (cid=1) and retrieve the local IP address.
3. **TCP Client**:
   - Option 2: Enter `host:port` (e.g., `123.45.67.89:8888` or `www.example.com:8888`). The demo resolves the hostname and connects.
   - Option 3: Sends "hello" over the established TCP connection.
   - Option 4: Closes the client socket.
4. **UDP Client**:
   - Option 5: Opens a UDP socket.
   - Option 6: Sends "hello" to a hardcoded remote address (47.108.134.223:8888). Modify the code for custom destinations.
   - Option 7: Closes the UDP socket.
5. **TCP Server**:
   - Option 8: Creates a TCP server socket on port 6666, binds, listens, and blocks on accept.
   - Option 9: (Server close is not implemented in the demo.)
6. **DeInit**: Option 10 closes any open socket and deactivates the PDP context.

## Implementation Notes

- **Multi-threaded architecture**: The demo creates two background tasks at initialization:
  - `clientProcesser`: Waits on `gClientMsgQueue`, receives the socket fd, and runs `tcp_udp_client_recv()`.
  - `serverProcesser`: Waits on `gServerMsgQueue`, receives the socket fd, and runs `server_recv()` (currently an empty infinite loop).
  - Task stack: 4096 bytes, priority: 150.
- **Receive loop**: `tcp_udp_client_recv()` uses `select()` with a 5-second timeout in an infinite loop. On data, it calls `recvfrom()` and prints the source IP, port, and data. On error (not EAGAIN), it closes the socket and exits.
- **Socket descriptor protection**: `g_tcp_udp_client_sockfd` is protected by `sockMutexRef` mutex. The getter/setter functions `get_tcp_udp_client_sockfd()` and `set_tcp_udp_client_sockfd()` ensure thread-safe access.
- **Single client limitation**: The demo only supports one TCP or UDP client at a time. The code comments note "the module itself can support 64-way tcp."
- **IPv4/IPv6 dual-stack**: `get_ipaddr()` supports IPv4, IPv6, and IPv4V6 PDP types. `Connect()` uses `AF_UNSPEC` for `getaddrinfo()` to support both address families.
- **Helper functions**:
  - `Inet_ntop()`: Converts a `struct sockaddr` to a string representation with port.
  - `Send()`: Wrapper that loops until all data is sent, supporting both TCP (send) and UDP (sendto).
  - `udp_send()`: Resolves the remote address and sends data via `sendto()`.
  - `ui_print()`: Formatted output helper wrapping `PrintfResp`.
- **Server implementation**: The TCP server (option 8) does a blocking `accept()` on the UI thread, which will freeze the menu until a client connects. The server receive function (`server_recv`) is an empty loop.

## Best Practices

- Always activate PDP (`sAPI_TcpipPdpActive`) before creating sockets, and deactivate it (`sAPI_TcpipPdpDeactive`) during shutdown.
- Use `getaddrinfo()` for DNS resolution instead of hardcoding IP addresses. It supports both IPv4 and IPv6.
- Use `select()` with a reasonable timeout to avoid blocking indefinitely on receive.
- Check `lwip_getsockerrno()` after send/receive failures to distinguish transient errors (EAGAIN) from fatal errors.
- Protect shared socket descriptors with mutexes in multi-threaded applications.
- For production applications, implement proper reconnection logic on connection loss.
- The demo's UDP send hardcodes the remote address; modify it to accept user input or configuration.
- The module supports up to 64 simultaneous TCP connections; the demo limits to 1 for simplicity but production code should manage multiple sockets.
- Close sockets and deactivate PDP on application exit to release resources.
- For server applications, move the `accept()` call to a background task to avoid blocking the UI thread.
