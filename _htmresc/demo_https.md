# demo_https

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_https.c

## Overview

This file provides an interactive, menu-driven HTTPS client demonstration for the SIMCom A7672 module. It implements a complete HTTP/HTTPS session lifecycle through UART input, supporting parameter configuration, POST data submission, request execution, response header/content reading, and file upload. Results are delivered asynchronously through a message queue.

## Main Features

- Interactive UART-based menu for HTTPS operations
- HTTP session lifecycle: Initialize, Set Parameters, Set Data, Send Request, Read Header, Read Content, Post File, Terminate
- Asynchronous response handling via message queue (`httpsUIResp_msgq`)
- Structured parameter input system using `param_element` descriptors
- Support for GET (0), POST (1), HEAD (2), and DELETE (3) HTTP methods
- File upload capability via `sAPI_HttpPostfile()`
- Response display functions for action results, headers, and read data
- Configurable parameter types: URL, CONNECTTO, CONTENT, USERDATA, ACCEPT, SSLCFG, RECVTO, READMODE

## Key Header Dependencies

- `simcom_https.h` -- Provides all HTTPS API functions (`sAPI_HttpInit`, `sAPI_HttpPara`, `sAPI_HttpAction`, etc.), return code enum (`SC_HTTP_RETURNCODE`), and the `SChttpApiTrans` response structure
- `simcom_os.h` -- Provides OS primitives: message queue creation/receive
- `simcom_common.h` -- Provides common type definitions
- `simcom_debug.h` -- Provides `sAPI_Debug()` for debug logging
- `simcom_uart.h` -- Provides UART write functions
- `uart_api.h` -- Provides `UartReadValue()` and `UartReadLine()` for input

## SDK APIs Observed

- `sAPI_HttpInit(channel, msgQ)` -- Initialize the HTTPS service; activates PDP context and sets up the message queue for async responses
- `sAPI_HttpTerm()` -- Terminate the HTTPS session and clean up resources
- `sAPI_HttpPara(commandType, parameters)` -- Set HTTP parameters such as URL, content type, SSL config, etc.
- `sAPI_HttpData(buffer, len)` -- Set POST data payload to be sent with the next action
- `sAPI_HttpAction(method)` -- Execute an HTTP request (0=GET, 1=POST, 2=HEAD, 3=DELETE)
- `sAPI_HttpHead()` -- Read the response headers after an action completes
- `sAPI_HttpRead(cmdType, offset, size)` -- Read response content; type 0 gets total content length, type 1 reads a data chunk
- `sAPI_HttpPostfile(filename, dir)` -- Upload a file from the module filesystem (dir: 1=C:/, 2=D:/)
- `sAPI_MsgQCreate(ref, name, msgSize, queueSize, mode)` -- Create message queue for HTTP responses
- `sAPI_MsgQRecv(ref, msg, timeout)` -- Receive HTTP response messages

## Usage

1. Call `HttpsDemo()` to enter the menu.
2. Follow the menu options in sequence:
   - **Option 1 (Init):** Creates the message queue and calls `sAPI_HttpInit(1, httpsUIResp_msgq)` to start the HTTPS service via USB channel.
   - **Option 2 (Set Parameters):** Enter parameter type (e.g., "URL") and value (e.g., "https://example.com") via UART. Calls `sAPI_HttpPara()`.
   - **Option 3 (Set Data):** Enter POST data string via UART. Calls `sAPI_HttpData()`.
   - **Option 4 (Send Request):** Enter HTTP method number (0=GET, 1=POST, 2=HEAD, 3=DELETE). Calls `sAPI_HttpAction()` and waits for the response via message queue.
   - **Option 5 (Read Header):** Calls `sAPI_HttpHead()` and displays the response header data.
   - **Option 6 (Read Content):** Enter read type, offset, and size. Calls `sAPI_HttpRead()`. Type 0 returns content length; type 1 returns data.
   - **Option 7 (Post File):** Enter filename and directory code. Calls `sAPI_HttpPostfile()`.
   - **Option 8 (Terminate):** Calls `sAPI_HttpTerm()` to end the session.
   - **Option 99:** Returns to the main menu after calling `sAPI_HttpTerm()`.

### Typical HTTPS GET Flow

1. Init -> Set URL parameter -> Send GET request -> Read header -> Read content -> Terminate

### Typical HTTPS POST Flow

1. Init -> Set URL parameter -> Set POST data -> Send POST request -> Read response -> Terminate

## Implementation Notes

- The `param_element` struct provides a generic parameter input framework with fields for prompt text, parameter name, type (numeric or string), pointer to storage, and size.
- The `SChttpApiTrans` response structure contains `status_code`, `method`, `action_content_len`, `data`, and `dataLen` fields.
- HTTP parameter types accepted: URL, CONNECTTO, CONTENT, USERDATA, ACCEPT, SSLCFG, RECVTO, READMODE.
- The `CHUNK` constant (`-0x100`) is defined in the header, likely for chunked transfer encoding handling.
- The message queue size is 8 messages. Response handlers use `SC_SUSPEND` (infinite wait) to block until a response arrives.
- The `show_httpaction_urc()` function displays both `status_code` and `content_length` from the response.
- The `show_httpread_urc()` function handles both type 0 (shows content-length) and type 1 (shows actual data).
- All response data pointers in `SChttpApiTrans` must be freed after use.

## Best Practices

- Always set the URL parameter before calling `sAPI_HttpAction()`.
- For HTTPS connections, set the URL with `https://` prefix and configure SSL parameters via `SSLCFG` parameter type if certificates are required.
- Check the `status_code` in the action response to determine HTTP response status (200, 404, 500, etc.).
- Use `READMODE` parameter to control how data is returned before executing actions.
- Call `sAPI_HttpTerm()` before returning from the demo to properly release PDP context resources.
- The message queue must be created before `sAPI_HttpInit()` and should remain valid throughout the HTTPS session.
- For large responses, use `sAPI_HttpRead()` with offset and size parameters to read data in chunks rather than all at once.
