# mqtt_tencent

Location: SIMCOM_SDK_SET/sc_demo/V1/src/mqtt_tencent.c

## Overview

This file provides MQTT credential configuration functions for connecting to the Tencent Cloud IoT platform (QCloud IoT Hub). It implements the complete Tencent Cloud MQTT authentication mechanism including client ID generation, URL construction with region-based domain selection, username composition with connection ID and timestamps, and HMAC-SHA1 password signing. The file includes a full SHA1 implementation and base64 decoder. This function is called by `TencentDemo()` in `demo_mqtt.c`.

## Main Features

- Generates Tencent Cloud MQTT connection parameters: client ID, host URL, username, and HMAC-SHA1 password
- Region-based MQTT domain resolution (China: `iotcloud.tencentdevices.com`, US-East: `us-east.iotcloud.tencentdevices.com`)
- Random connection ID generation for session uniqueness
- Complete SHA-1 hash implementation (init, starts, process, update, finish)
- HMAC-SHA1 authentication for MQTT password generation
- Base64 decoding for device secret processing
- Proper memory management with dynamic allocation and cleanup

## Key Header Dependencies

- `simcom_os.h` -- Provides OS primitives: `sAPI_Malloc()`, `sAPI_Free()`, `sAPI_GetTicks()`, `sAPI_Debug()`
- `simcom_debug.h` -- Provides `sAPI_Debug()` for debug logging

## SDK APIs Observed

- `sAPI_Malloc(size)` -- Allocate memory for credential buffers
- `sAPI_Free(ptr)` -- Free allocated memory
- `sAPI_GetTicks()` -- Get system tick count (used for random seed)
- `sAPI_Debug(fmt, ...)` -- Print debug messages

## Usage

This function is not called directly by end users. It is invoked internally by `TencentDemo()` in `demo_mqtt.c`:

1. The caller provides product ID, device name, device secret, and region.
2. The function fills output buffers with MQTT connection parameters:

### Parameter Generation Process

**1. Client ID:**
- Format: `<product_id><device_name>` (concatenated)
- Max length: 80 characters

**2. Host Address:**
- Format: `tcp://<product_id>.<region_domain>:1883`
- Domain is resolved via `iot_get_mqtt_domain(region)`:
  - "china" maps to `iotcloud.tencentdevices.com`
  - "us-east" maps to `us-east.iotcloud.tencentdevices.com`
  - Default is China domain

**3. Username:**
- Format: `<client_id>;<sdk_app_id>;<conn_id>;<timestamp>`
- SDK App ID: "12010126" (hardcoded IoT C-SDK APPID)
- Connection ID: 5 random alphanumeric characters
- Timestamp: epoch seconds (currently hardcoded to 1608336000)

**4. Password:**
- Base64-decode the device secret to get the raw key
- Compute HMAC-SHA1 of the username using the decoded key
- Format: `<hex_hmacsha1_result>;hmacsha1`

The generated parameters are then used with `sAPI_MqttAccq()` and `sAPI_MqttConnect()` in `demo_mqtt.c`.

## Implementation Notes

- The SHA1 implementation is a complete, self-contained version using big-endian 32-bit operations with the standard four-round, 80-step algorithm.
- The `KEY_IOPAD_SIZE` is 64 bytes, matching the standard HMAC block size.
- The `base64_dec_map` array maps ASCII characters to their base64 values for decoding.
- The timestamp is currently hardcoded (`long time = 1608336000`) rather than obtained from system time. This should be updated to use `sAPI_Time()` or `sAPI_GetSysLocalTime()` for production use.
- The `get_next_conn_id()` function uses `srand((unsigned)HAL_GetTimeMs())` seeded by `sAPI_GetTicks()` for pseudo-random connection ID generation.
- Memory is carefully allocated and freed for `client_id`, `host_addr`, `username`, and `password` intermediaries.
- Constants `MAX_SIZE_OF_CLIENT_ID=80`, `HOST_STR_LENGTH=64`, `MAX_CONN_ID_LEN=6` define buffer sizes.
- Two regions are supported: China and US-East, defined in the `sg_iot_mqtt_domain` array.

## Best Practices

- Replace the hardcoded timestamp with actual system time using `sAPI_Time()` for production deployments.
- Ensure the module's clock is synchronized before connecting, as the timestamp is part of the username and affects authentication.
- The hardcoded `QCLOUD_IOT_DEVICE_SDK_APPID` ("12010126") should be verified against your Tencent Cloud project's actual APPID.
- For production, use SSL/TLS by changing the URL scheme to `ssl://` and port to `8883`, combined with `sAPI_MqttSslCfg()`.
- Store device secrets securely; they should not be hardcoded in source code.
- The HMAC-SHA1 password has the format `<hash>;hmacsha1` -- the ";hmacsha1" suffix tells the Tencent Cloud broker which algorithm was used.
