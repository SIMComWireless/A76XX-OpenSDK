# demo_mqtt

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_mqtt.c

## Overview

This demo file provides interactive, menu-driven MQTT client demonstrations for connecting to multiple Chinese IoT cloud platforms (Aliyun, Tencent Cloud, OneNET, and Twing/CTWing). Each cloud platform has its own dedicated demo function with a step-by-step menu allowing the user to start, connect, subscribe, publish, unsubscribe, disconnect, release, and stop MQTT sessions via UART input. The file also contains a static task-based MQTT processor and receiver for automated message handling.

## Main Features

- Multi-cloud MQTT support: Aliyun, Tencent Cloud, OneNET, and Twing (CTWing)
- Interactive UART-based menu for each cloud platform demo
- MQTT lifecycle management: Start, Acquire, Configure, Connect, Subscribe, Publish, Unsubscribe, Disconnect, Release, Stop
- Asynchronous message reception via message queues (`sMsgQRef`)
- Lost-connection callback with automatic reconnection logic
- Separate receive task for processing subscribed topic data
- Support for multiple MQTT client indices (0 and 1)
- Integration with `core_auth` library for Aliyun credential generation
- Integration with external `Cfg_mqtt_service` for OneNET credential generation
- Integration with external `Get_Cfg_Parameters` for Tencent credential generation

## Key Header Dependencies

- `simcom_mqtts_client.h` -- Provides all MQTT SDK API function declarations (`sAPI_MqttStart`, `sAPI_MqttAccq`, `sAPI_MqttConnect`, etc.), result code enums (`SCmqttResultType`), the `SCmqttData` structure for received messages, and the `mqtt_connlost_cb` callback type
- `simcom_debug.h` -- Provides `sAPI_Debug()` for debug logging
- `simcom_uart.h` -- Provides UART write functions
- `simcom_network.h` -- Provides `sAPI_NetworkGetCgreg()` for network registration status checking
- `core_auth.h` -- Provides Aliyun MQTT authentication functions (`core_auth_mqtt_username`, `core_auth_mqtt_password`, `core_auth_mqtt_clientid`)
- `uart_api.h` -- Provides `UartReadValue()` and `UartReadLine()` for UART input

## SDK APIs Observed

- `sAPI_MqttStart(channel)` -- Start the MQTT service; `channel` controls URC output channel (-1 to ignore)
- `sAPI_MqttStop()` -- Stop the MQTT service entirely
- `sAPI_MqttAccq(type, str, index, clientID, serverType, msgQ)` -- Acquire an MQTT client instance; assigns a message queue for incoming subscribed data
- `sAPI_MqttRel(index)` -- Release a previously acquired MQTT client
- `sAPI_MqttCfg(type, str, index, configType, value)` -- Configure MQTT context (e.g., UTF8 check, operation timeout)
- `sAPI_MqttConnect(type, str, index, addr, keepalive, cleanSession, user, pass)` -- Connect to an MQTT broker
- `sAPI_MqttDisConnect(type, str, index, timeout)` -- Disconnect from the broker with timeout
- `sAPI_MqttConnLostCb(callback)` -- Register a callback for connection-lost events
- `sAPI_MqttTopic(index, topic, len)` -- Set the publish topic (used before `sAPI_MqttPub`)
- `sAPI_MqttPayload(index, data, len)` -- Set the publish payload (used before `sAPI_MqttPub`)
- `sAPI_MqttPub(index, qos, timeout, retained, dup)` -- Publish a message to the broker
- `sAPI_MqttSubTopic(index, topic, len, qos)` -- Set a subscribe topic (for batch subscribe with `sAPI_MqttSub(NULL,...)`)
- `sAPI_MqttSub(index, topic, len, qos, dup)` -- Subscribe to a topic; when `topic=NULL` and `len=0`, subscribes all topics set via `sAPI_MqttSubTopic`
- `sAPI_MqttUNSubTopic(index, topic, len)` -- Set an unsubscribe topic (for batch unsubscribe)
- `sAPI_MqttUnsub(index, topic, len, dup)` -- Unsubscribe from a topic; when `topic=NULL`, unsubscribes all topics set via `sAPI_MqttUNSubTopic`
- `sAPI_MsgQCreate(ref, name, msgSize, queueSize, mode)` -- Create a message queue
- `sAPI_MsgQSend(ref, msg)` -- Send a message to a queue
- `sAPI_MsgQRecv(ref, msg, timeout)` -- Receive a message from a queue
- `sAPI_TaskCreate(ref, stack, stackSize, priority, name, func, arg)` -- Create a task/thread
- `sAPI_Malloc(size)` -- Allocate memory
- `sAPI_Free(ptr)` -- Free allocated memory
- `sAPI_NetworkGetCgreg(status)` -- Get network GPRS registration status
- `sAPI_TaskSleep(ticks)` -- Sleep the current task

## Usage

1. Call `MqttDemo()` to enter the main menu.
2. Select a cloud platform (Aliyun=1, Tencent=2, OneNET=3, Twing=4).
3. Within the selected cloud demo, follow the step-by-step menu:
   - **Step 1 (Start):** Call `sAPI_MqttStart(-1)` to initialize MQTT service.
   - **Step 2 (Connect):** Depending on the platform, generate credentials (using `core_auth`, `Cfg_mqtt_service`, `Get_Cfg_Parameters`, or hardcoded values), then call `sAPI_MqttAccq()` to acquire a client, register the lost-connection callback via `sAPI_MqttConnLostCb()`, and connect via `sAPI_MqttConnect()`.
   - **Step 3 (Subscribe):** Enter a topic via UART and call `sAPI_MqttSub()`.
   - **Step 4 (Publish):** Enter topic and payload via UART; call `sAPI_MqttTopic()` then `sAPI_MqttPayload()` then `sAPI_MqttPub()`.
   - **Step 5 (Unsubscribe):** Enter topic and call `sAPI_MqttUnsub()`.
   - **Step 6 (Disconnect):** Call `sAPI_MqttDisConnect()`.
   - **Step 7 (Release):** Call `sAPI_MqttRel(0)`.
   - **Step 8 (Stop):** Call `sAPI_MqttStop()`.
4. Option 99 returns to the main menu and performs cleanup of all clients.

The separate receive task (`sTask_MqttRecvProcesser`) runs in a loop, reading from `urc_mqtt_msgq_1` and printing received topic/payload data. Topic and payload pointers in `SCmqttData` must be freed after use.

## Implementation Notes

- Two message queues are created at initialization: `urc_mqtt_msgq_1` for incoming subscribed data (from the MQTT SDK) and `uart_mqtt_msgq_1` for UART input data to be published.
- The `app_MqttLostConnCb` callback checks network registration status (`sAPI_NetworkGetCgreg`) in a loop. If the network was down and recovered, it advises releasing and re-initializing the full MQTT session rather than just reconnecting.
- The demo supports up to 2 concurrent MQTT client indices (0 and 1). The Tencent, OneNET, and Twing demos only use index 0.
- For Aliyun, the connection URL is constructed as `tcp://<productkey>.iot-as-mqtt.cn-shanghai.aliyuncs.com:443` with `clean_session=0`.
- For OneNET, the URL is `tcp://183.230.40.96:1883` (or `183.230.40.39:6002` in the auto processor) with `clean_session=1`.
- For Twing, the URL is `tcp://mqtt.ctwing.cn:1883` with `clean_session=1`.
- The Tencent demo uses `Get_Cfg_Parameters()` from `mqtt_tencent.c` for credential generation.
- Topic buffer is limited to 1025 bytes and payload buffer to 10241 bytes.
- The `sTask_MqttProcesser` function (commented-out task version) demonstrates a non-interactive automated flow using a `while(1)/switch(branch)` state machine.

## Best Practices

- Always create message queues and receive tasks before starting MQTT operations to avoid missing incoming messages.
- Register the lost-connection callback (`sAPI_MqttConnLostCb`) immediately after `sAPI_MqttAccq` and before `sAPI_MqttConnect`.
- In the lost-connection callback, do NOT directly call reconnection functions; instead, send a message to another task via `sAPI_MsgQSend()` to handle reconnection asynchronously.
- Free `topic_P`, `payload_P`, and the `SCmqttData` struct after processing received messages, in that order.
- Always pair `sAPI_MqttStart`/`sAPI_MqttStop` and `sAPI_MqttAccq`/`sAPI_MqttRel` properly. Call `Disconnect` before `Rel`, and `Rel` before `Stop`.
- Increase the message queue size if you see "message queue full" warnings in logs; the default size of 4 may be insufficient for high-throughput scenarios.
- Check return codes from every MQTT API call and handle failures gracefully.
- For production use, store credentials securely rather than hardcoding them in source.
