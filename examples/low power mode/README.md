# Low power mode

This source file demonstrates a complete low power mode application for SIMCom A76XX OpenSDK solution, focusing on network initialization, MQTT communication, data packaging, power management, and wakeup mechanisms. Below is a detailed overview of its main functions and implementation highlights.

## Network Initialization and Connection
Network Registration & Activation
The code initializes the cellular network using sAPI_NetworkInit.
It continuously checks the network status (LTE,Online) and activates the PDP context if needed.
Automatic reconnection is attempted if the network is not active.
## MQTT Client Setup and Communication
MQTT Client Initialization
Initializes the MQTT service (sAPI_MqttStart) and acquires a client (sAPI_MqttAccq).
Connection & Subscription
Connects to a public MQTT broker (test.mosquitto.org) and subscribes to a topic (test/download).
Message Reception
Creates a message queue and a dedicated thread (MqttRecvTaskProcesser) to handle incoming MQTT messages asynchronously.
Received messages are printed and memory is properly freed.
## Data Packaging and Publishing
JSON Data Packaging
The function mqtt_package_preparation collects module information (version, IMEI, CPU usage, RTC time, etc.) and packages it into a JSON string using cJSON.
Data Publishing
Data is published to the MQTT broker either periodically (via timer) or upon external wakeup (via GPIO).
After publishing, the module can enter sleep mode to save power.
## Power Management and Wakeup Mechanisms
Timer-Based Wakeup
Uses a timer (sAPI_TimerStart) to periodically wake up and trigger data publishing.
GPIO-Based Wakeup
Configures a GPIO pin as an external wakeup source. When triggered, it wakes the module and initiates data publishing.
Sleep and RTC Wakeup
After a set number of data updates, the module enters RTC mode, sets an alarm, and powers off. It will wake up when the RTC alarm triggers.
## Task and Synchronization Management
Main Task Flow
The main process (sTask_HelloWorldProcesser) manages network setup, MQTT initialization, data publishing, and power management.
Inter-Task Communication
Uses message queues and flags for synchronization between tasks, ensuring reliable data flow and event handling.
## Robustness and Extensibility
Error Handling
All critical API calls are checked for errors, with informative logging for debugging and maintenance.
Memory Management
Dynamically allocated memory for MQTT messages and JSON strings is properly freed to prevent leaks.
Modular Design
Functions for network, MQTT, GPIO, and RTC are well separated, making the code easy to extend and maintain.
## Summary
This code provides a comprehensive demonstration of how to build an MQTT-based IoT application on SIMCom modules. It covers network connectivity, MQTT communication, data collection and packaging, low-power operation, and wakeup mechanisms, making it a solid reference for embedded IoT development.