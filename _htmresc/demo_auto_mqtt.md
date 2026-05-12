# demo_auto_mqtt

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_auto_mqtt.c

## Overview

This file provides an automated (non-interactive) MQTT test implementation for the SIMCom SDK. Unlike `demo_mqtt.c` which requires UART menu interaction, this demo runs a complete MQTT lifecycle (start, acquire, connect, subscribe, publish, unsubscribe, wait for offline notification, disconnect/release/stop) in a background task. It is designed for automated testing and regression verification, and is conditionally compiled with `FEATURE_SIMCOM_MQTT`.

## Main Features

- Fully automated MQTT test sequence running in a dedicated task
- Offline connection-lost notification via message queue (instead of direct reconnection)
- Two message queues: one for subscribed data (`urc_mqtt_msgq_1_autotest`) and one for offline notification (`notice_mqtt_offline`)
- Separate receive task for processing incoming subscribed topic data
- Timeout-based wait for offline notification (6 minutes) to distinguish between normal disconnect and connection loss
- One-time initialization function `demo_mqtt_test_init()` suitable for calling from `userSpace_Main()`
- Clean memory management with proper pointer freeing

## Key Header Dependencies

- `simcom_mqtts_client.h` -- Provides all MQTT API functions, result codes, `SCmqttData` struct, and `mqtt_connlost_cb` type
- `simcom_os.h` -- Provides OS primitives: task creation, message queues, task sleep

## SDK APIs Observed

- `sAPI_MqttStart(channel)` -- Start the MQTT service
- `sAPI_MqttStop()` -- Stop the MQTT service
- `sAPI_MqttAccq(type, str, index, clientID, serverType, msgQ)` -- Acquire MQTT client
- `sAPI_MqttRel(index)` -- Release MQTT client
- `sAPI_MqttCfg(type, str, index, configType, value)` -- Configure MQTT context
- `sAPI_MqttConnect(type, str, index, addr, keepalive, cleanSession, user, pass)` -- Connect to broker
- `sAPI_MqttDisConnect(type, str, index, timeout)` -- Disconnect from broker
- `sAPI_MqttConnLostCb(callback)` -- Register connection-lost callback
- `sAPI_MqttTopic(index, topic, len)` -- Set publish topic
- `sAPI_MqttPayload(index, data, len)` -- Set publish payload
- `sAPI_MqttPub(index, qos, timeout, retained, dup)` -- Publish message
- `sAPI_MqttSubTopic(index, topic, len, qos)` -- Set subscribe topic for batch subscribe
- `sAPI_MqttSub(index, topic, len, qos, dup)` -- Subscribe to topic
- `sAPI_MqttUNSubTopic(index, topic, len)` -- Set unsubscribe topic for batch unsubscribe
- `sAPI_MqttUnsub(index, topic, len, dup)` -- Unsubscribe from topic
- `sAPI_MsgQCreate(ref, name, msgSize, queueSize, mode)` -- Create message queue
- `sAPI_MsgQSend(ref, msg)` -- Send message to queue
- `sAPI_MsgQRecv(ref, msg, timeout)` -- Receive message from queue (supports timeout)
- `sAPI_TaskCreate(ref, stack, stackSize, priority, name, func, arg)` -- Create task
- `sAPI_TaskSleep(ticks)` -- Sleep task for specified ticks

## Usage

1. Include this file in your build with `FEATURE_SIMCOM_MQTT` defined.
2. Call `demo_mqtt_test_init()` from your main initialization function (e.g., `userSpace_Main()`).
3. The function creates two message queues and two tasks:
   - `gMqttRecvTask` -- continuously receives subscribed data from `urc_mqtt_msgq_1_autotest`
   - `gMqttdemo` -- runs the automated test sequence after a 10-second delay (waiting for network)
4. The test sequence automatically:
   - Starts MQTT service (with retry loop on failure, waiting 10s between attempts)
   - Acquires client with ID "614393569" and configures it
   - Connects to `tcp://47.108.134.22:1883` with credentials "znn"/"znn11"
   - Subscribes to three topics ("topic11" at QoS 1, "apitest1" at QoS 0, "apitest2" at QoS 0)
   - Publishes "aMsgsend123" to "topic11"
   - Unsubscribes from all topics
   - Waits up to 6 minutes for an offline notification via `notice_mqtt_offline` queue
   - If offline notification received: skips disconnect, directly releases and stops
   - If timeout: performs normal disconnect, then release and stop

## Implementation Notes

- The offline callback `mqtt_notice_offline` sends a message to the `notice_mqtt_offline` queue with `msg_id=11`, `arg1=index`, and `arg2=cause`, rather than attempting reconnection directly.
- The receive task `sTask_MqttRecvProcesser_auto` uses `free()` (not `sAPI_Free()`) for `topic_P`, `payload_P`, and `sub_data` -- this is a notable difference from `demo_mqtt.c` which uses `sAPI_Free()`.
- The `msgQ_data_recv.arg1` check in the receive task has the `SC_SRV_MQTT` check commented out, accepting all message IDs.
- The automated test has a 10-second initial delay (`sAPI_TaskSleep(200*10)`) before starting MQTT operations to allow network registration to complete.
- The test uses a single client index (0) with hardcoded server address and credentials.
- The `sAPI_MqttStart` call has a retry mechanism: if it fails, it waits 10 seconds and retries indefinitely.

## Best Practices

- Use this automated approach for CI/CD testing and regression verification rather than interactive demos.
- The offline notification pattern (sending to a message queue from the callback) is the recommended production pattern for handling disconnections. Do not attempt reconnection directly within the callback.
- Always check the return value of `sAPI_MsgQRecv` with timeout to distinguish between normal operation completion and unexpected connection loss.
- Use consistent memory allocation/deallocation functions -- prefer `sAPI_Free()` over `free()` for SDK-allocated memory for consistency.
- For production deployments, replace hardcoded credentials with secure storage.
