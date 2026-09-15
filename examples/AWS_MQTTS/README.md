# AWS IoT MQTTS Demo — Mutual Authentication with Certificate Buffers

This example demonstrates how to connect a SIMCom A76XX module to **AWS IoT Core** over
**MQTT over TLS** using **mutual (two-way) authentication**, where the certificates are
supplied to the SSL context as **in-memory buffers** rather than as file names.

This is the "certificate buffer" approach (approach 2). For the file-name approach
(`"cacert"` / `"clientcert"` / `"clientkey"`), see the [MQTTS demo](../mqtts/README.md).

## File overview

`demo_mqtt_certbuf.c` runs as an RTOS task. It waits for network registration, reads three
PEM files (CA root, client certificate, client private key) from the module file system into
heap buffers, injects them into an SSL context through `sAPI_SslSetContextIdMsg`, and then
acquires an MQTT client, binds it to that SSL context and connects to the AWS IoT endpoint.

## Prerequisites

### 1. AWS IoT Core setup

1. Create a **Thing** in the AWS IoT console and generate a certificate for it.
2. Download the certificate package and keep these three files:
   - `AmazonRootCA1.pem` — the AWS root CA
   - `<thing>-certificate.pem.crt` — the client certificate
   - `<thing>-private.pem.key` — the client private key
3. Attach an **IoT policy** that allows `iot:Connect` for your client ID, and — if you enable
   `DEMO_MQTT_PUB_TEST` — `iot:Publish` for the test topic.
4. Note your **endpoint** (AWS IoT console → Settings → Device data endpoint).

### 2. Push the certificates onto the module

Copy the three PEM files to a writable drive of the module file system and update the
`CERT_PATH_*` macros to match. The paths must be reachable by `sAPI_fopen`, i.e. they live on
a mounted drive such as `C:/` or `D:/`.

> These files are **not** part of this repository — they are device-specific secrets. Never
> commit a private key.

### 3. Network

The demo assumes the module is already able to register on the network; it polls
`sAPI_NetworkGetCgreg` until the state is `1` or `5`. Bring up the PDP context (as done in
the other examples of this SDK) before calling the demo entry point.

## Configuration

Edit the macros at the top of `demo_mqtt_certbuf.c`:

| Macro | Default | Description |
|-------|---------|-------------|
| `MQTT_CLIENT_INDEX` | `0` | MQTT client index, range 0~1 |
| `MQTT_SSL_CTX_ID` | `0` | SSL context index, range 0~9 |
| `MQTT_BROKER_URL` | AWS test endpoint | `tcp://<endpoint>:8883`, port 8883 for MQTTS |
| `MQTT_CLIENT_ID` | `xxxxxx` | Device client ID; must be consistent with the client certificate and the AWS IoT policy |
| `CERT_PATH_CA` | `C:/AmazonRootCA1.pem` | Path of the root CA on the module file system |
| `CERT_PATH_CLIENT` | `C:/certificatePem.pem` | Path of the client certificate |
| `CERT_PATH_KEY` | `C:/privateKeyPem.pem` | Path of the client private key |
| `MQTT_SSL_VERSION` | `"4"` | `0`:SSL3.0 `1`:TLS1.0 `2`:TLS1.1 `3`:TLS1.2 `4`:ALL |
| `MQTT_AUTH_MODE` | `"2"` | `2`: mutual auth (verify the server *and* present a client certificate) |
| `MQTT_ENABLE_SNI` | `"1"` | SNI is mandatory for AWS IoT |
| `MQTT_IGNORE_LOCALTIME` | `1` | `1` skips certificate validity-period checking |
| `MQTT_KEEPALIVE_S` | `60` | MQTT keep-alive interval, in seconds |
| `MQTT_CLEAN_SESSION` | `1` | MQTT clean-session flag |
| `DEMO_MQTT_PUB_TEST` | `0` | `1`: publish one test message after connecting |
| `DEMO_TEARDOWN_AFTER_S` | `0` | Keep the connection for N seconds, then disconnect/release/stop; `0` keeps it connected |

> `MQTT_IGNORE_LOCALTIME` is set to `1` by default because the module clock is often not yet
> synchronised, which makes the broker reject a valid certificate as "not yet valid". Set it
> back to `0` in production so that the validity period is actually enforced.

## Runtime flow

1. **Entry point** — `MqttCertBufDemoInit()` creates the URC message queue and the demo task.
2. **Network wait** — the task polls `sAPI_NetworkGetCgreg()` every 10 s until `cgreg` is `1` or `5`.
3. **MQTT start** — `sAPI_MqttStart(-1)` initializes the MQTT client.
4. **Certificates to RAM** — `cert_read_file()` reads each PEM file into a heap buffer
   (the core step of this demo, see below).
5. **SSL context setup** — `ssl_ctx_setup_with_buffers()` sets `sslversion`, `enableSNI`,
   `authmode`, `ignorelocaltime`, and injects the three buffers via `cabuf` / `certbuf` / `keybuf`.
6. **Buffers released** — the SSL context keeps its own copy, so the local buffers are freed
   immediately with `cert_buf_free()`.
7. **MQTT client setup** — `sAPI_MqttAccq()` acquires the client, `sAPI_MqttSslCfg()` binds the
   SSL context to it.
8. **Connect** — `sAPI_MqttConnect()` connects to `MQTT_BROKER_URL`.
9. **Optional test publish** — if `DEMO_MQTT_PUB_TEST` is `1`, publish one message to
   `certbuf/demo/hello`.
10. **Optional teardown** — if `DEMO_TEARDOWN_AFTER_S > 0`, disconnect (60 s), release the
    client and stop MQTT after the delay, following the AT command sequence.
    On any failure the `cleanup` label releases the buffers and unwinds whatever was started.

## Certificate buffer handling

`cert_read_file(const char *path, cert_buf_t *out)` is the part worth reusing:

- Opens the file in binary mode and gets its size with `sAPI_fseek` / `sAPI_ftell`.
- Rejects files larger than **32 KB** — enough for a single PEM certificate or key, and safely
  below the **INT16 length limit** of the SSL context APIs.
- Allocates `size + 1` bytes, reads the content and appends a `'\0'` terminator.
- Returns `0` on success and `-1` on failure; on failure nothing leaks.

The `'\0'` terminator is required: `cabuf` / `certbuf` / `keybuf` derive their length from
`strlen()`, so the buffer must contain **NUL-terminated PEM text**. For binary DER
certificates use the file-name form instead:

```c
sAPI_SslSetContextIdMsg("cacert", ctx_id, "xxx.der");
```

`cert_buf_free()` releases a buffer and is safe to call more than once, which is why the
`cleanup` path can free all three unconditionally.

## Key symbols

| Symbol | Description |
|--------|-------------|
| `cert_buf_t` | Certificate descriptor: `data` (PEM text) and `len` (bytes read) |
| `g_ca`, `g_cert`, `g_key` | Global buffers for the root CA, client certificate and private key |
| `g_mqtt_urc_msgq` | Message queue receiving the MQTT URCs |
| `cert_read_file()` | File system → RAM buffer |
| `cert_buf_free()` | Release a certificate buffer |
| `ssl_ctx_setup_with_buffers()` | Configure the SSL context and inject the buffers |
| `mqtt_certbuf_demo_task()` | Main demo task |
| `MqttCertBufDemoInit()` | Public entry point |
| `mqtt_err_str()` | Result code → readable string, used in every log line |

## Integration notes

- **Call `MqttCertBufDemoInit()` once** from your application entry point or demo menu.
- **Drain the URC queue.** It is created with a depth of 4 and is private to this file. The
  demo task does not read from it, so for long-running use expose the queue (or a callback)
  and consume `SCmqttData` messages, freeing the three pointers with `sAPI_Free` — otherwise
  the queue fills up.
- The demo task stack is `SC_DEFAULT_THREAD_STACKSIZE * 2` bytes at `SC_DEFAULT_TASK_PRIORITY`.
  The SDK recommends a task priority in the 150–250 range for customer applications.
- Log output uses `sAPI_Debug` with the `[certbuf]` prefix, so it is easy to filter.

## Troubleshooting

| Log / symptom | Likely cause |
|---------------|--------------|
| `fopen fail` | Wrong `CERT_PATH_*`, or the file was not pushed to a drive reachable by `sAPI_fopen` |
| `file too large` | The file is not the expected single PEM certificate/key |
| `ssl handshake fail` | Certificate content truncated, module clock not synchronised, or SNI disabled |
| `certs not set` | One of `cabuf` / `certbuf` / `keybuf` was rejected — check for NUL-terminated PEM text |
| `not authorized` | IoT policy does not allow this client ID, or the certificate is not attached to the policy |
| `waiting network` loop | SIM not registered, or the PDP context is not active |

For more details on the underlying APIs, refer to the SIMCom OpenSDK documentation.
