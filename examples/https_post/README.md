# HTTPS POST demo

## File overview
`demo_httppost.c` is a SIMCom OpenSDK example demonstrating an HTTPS POST workflow. It initializes the SSL/HTTPS stack, builds a compact JSON telemetry payload, and periodically posts that payload to a configured HTTPS endpoint using a hard-coded bearer token. The code runs as an RTOS task and ensures the modem is online and the PDP context is active before sending requests.

## High-level behavior
- Waits for network registration (`"LTE,Online"`) and ensures PDP activation.
- Initializes SSL and HTTPS (loads CA from an embedded PEM string, enables SNI, sets SSL version).
- Configures HTTP parameters (URL, `Content-Type`, `Authorization` header).
- Builds a timestamped JSON payload using `cJSON`.
- Sends the JSON via SIMCom HTTPS APIs, reads headers and body from a message queue, and logs responses.
- Repeats the POST periodically (approx. every 5 seconds) in a background task created by `sAPP_HTTPPOSTDemo()`.

## Key symbols and modules
- `ca[]` — embedded PEM certificate used as CA buffer (passed via `"cabuf"`).
- `HTTP_POST_URL` — hard-coded target HTTPS endpoint.
- `AUTH_BEARER` — hard-coded `Authorization: Bearer ...` header.
- `httpsUIResp_msgq` — message queue for receiving HTTP/SSL asynchronous responses.
- SIMCom SDK APIs (`sAPI_*`) for network, SSL, HTTP, tasks, queues, and logging.
- `cJSON` — used to build the JSON payload.

## Important functions (summary)
- `http_set_para(char *key, char *value)`  
	Wrapper around `sAPI_HttpPara` with debug logging.

- `HTTP_ParaForm(char *url)`  
	Sets HTTP parameters: `URL`, `CONTENT` (`application/json`), and `USERDATA` (the auth header).

- `HTTP_INIT(void)`  
	- Creates the response message queue `httpsUIResp_msgq`.  
	- Calls `sAPI_SslSetContextIdMsg` to set:
		- `"authmode" = "1"` (verify server certificate)
		- `"cabuf" = ca` (embedded CA)
		- `"sslversion" = "3"` (allowed SSL/TLS versions)
		- `"enableSNI" = "1"` (enable SNI)  
	- Initializes HTTPS via `sAPI_HttpInit(1, httpsUIResp_msgq)` and sets `"SSLCFG"`.

- `json_package_preparation(void)`  
	- Reads network RTC (`sAPI_NetworkGetRealTime`).  
	- Builds a compact JSON object with one `data` array element holding telemetry fields (name, description, pointKey, value, unit, phase, energySourceType, timestamp).  
	- Returns a heap-allocated string from `cJSON_Print` (caller must free).

- `HTTP_PostData(const char *data)`  
	- Calls `sAPI_HttpData((char *)data, strlen(data))` to set the POST body.  
	- Calls `sAPI_HttpAction(1)` to perform POST. On success, processes action result via `HTTP_Action`.  
	- Calls `sAPI_HttpHead()` and reads headers via `show_httphead_urc`.  
	- Calls `sAPI_HttpRead(1, 0, 512)` to read response body and processes via `HTTP_ReadAction`.  
	- Logs errors and frees response resources.

- `HTTP_Action`, `HTTP_ReadAction`, `show_httphead_urc`  
	Helpers that receive `SChttpApiTrans` objects from `httpsUIResp_msgq`, log status/content, print headers/body, and free buffers.

- `sTask_HTTPPOSTProcesser(void *argv)`  
	- Sets network time-zone and initializes network.  
	- Polls until network state is `"LTE,Online"` and PDP is active (calls `sAPI_NetworkSetCgact(1,1)` if needed).  
	- Calls `HTTP_INIT()` and `HTTP_ParaForm(HTTP_POST_URL)`.  
	- In an infinite loop: build JSON (`json_package_preparation()`), call `HTTP_PostData(json)` , free JSON, and sleep ~5 seconds.

- `sAPP_HTTPPOSTDemo(void)`  
	Creates the HTTP POST task with `sAPI_TaskCreate`.

## Memory & ownership notes
- `json_package_preparation()` returns a heap-allocated string (from `cJSON_Print`) — caller must free (the task calls `sAPI_Free`).
- Response data received via message queue is freed by the helper functions.
- `sAPI_HttpData` expects a non-const `char *`; the code casts away const when calling.

## Runtime flow (step-by-step)
1. `sAPP_HTTPPOSTDemo()` creates the background task.
2. Task initializes network and waits for `"LTE,Online"` and PDP activation.
3. SSL/HTTPS contexts are configured (CA, SNI, versions).
4. HTTP parameters (URL, content type, auth header) are set.
5. Task creates a timestamped JSON payload and posts it:
	 - supplies body, triggers POST,
	 - receives and logs HTTP status and headers via message queue,
	 - reads and logs response body.
6. Task sleeps and repeats.

## Usage & build notes
- This code targets the SIMCom OpenSDK environment and uses `sAPI_*` runtime APIs. It will not compile on a standard desktop toolchain without the SDK.
- Dependencies:
	- SIMCom OpenSDK headers and runtime.
	- `cJSON` library (included via `#include "cJSON.h"`).
- Configuration:
	- Replace `HTTP_POST_URL` and `AUTH_BEARER` before production use.
	- The CA is embedded in `ca[]`; replace or supply the correct CA if the server uses a different CA.
- The message queue length is 8; adjust if the application needs to handle more concurrent responses.
- The task stack is `1024*10` bytes and priority `150` — verify these values on target hardware.

## Edge cases and recommendations
- Use `cJSON_PrintUnformatted()` (if available) to reduce payload size (current code uses `cJSON_Print`, which may include whitespace).
- Improve error handling: add retry/backoff for repeated HTTP failures.
- Avoid hard-coded tokens or move them to secure storage / configuration.
- Validate stack size and task scheduling for production.
- Consider making the POST interval configurable (instead of fixed ~5 seconds).

---
