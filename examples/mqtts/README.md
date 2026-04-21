
# MQTTS Demo

This example demonstrates how to establish a secure MQTT connection (MQTT over SSL/TLS) on SIMCom modules. The main workflow is as follows:

## 1. Certificate Preparation
- The CA root certificate, client certificate, and private key are embedded in the code and written to the file system using `mqtt_write_file`, for SSL/TLS connection usage.

## 2. Network Initialization
- Calls `sAPI_NetworkInit` to initialize the network and loops to check the network status, ensuring registration to the LTE network.

## 3. File System Operations
- Writes certificate contents to local files and switches to the appropriate directory to ensure SSL usage.

## 4. SSL Parameter Configuration
- Configures SSL parameters via `sAPI_SslSetContextIdMsg`, including protocol version, authentication mode, certificate paths, SNI enable, negotiation timeout, etc.

## 5. MQTT Client Initialization and Connection
- Calls `sAPI_MqttStart` to initialize the MQTT client.
- Configures MQTT to use SSL.
- Acquires an MQTT client instance (`sAPI_MqttAccq`).
- Connects to the MQTT server (e.g., `tcp://dev.rightech.io:8883`) and performs username/password authentication.

## 6. Task and Debugging
- The main workflow runs in the `sTask_HelloWorldProcesser` task, and all debug information is output via `simcom_printf` for easy debugging and log collection.

## Application Scenario
- Suitable for IoT developers who need to implement secure MQTT communication on SIMCom modules, helping to quickly build a TLS-encrypted MQTT connection process.

For more detailed API documentation, please refer to the SIMCom OpenSDK documentation.
