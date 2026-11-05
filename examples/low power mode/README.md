# Low Power Mode Demo

This example shows a complete MQTT telemetry application for the SIMCom A76XX OpenSDK that
cycles through three power states:

1. **Normal mode** – register on LTE, connect to an MQTT broker, publish a JSON payload.
2. **System sleep** – after each of the first four publishes the module is allowed to enter
   system sleep and is woken again either by a 20 s timer or by a GPIO edge.
3. **RTC mode** – after the fifth publish the MQTT client is shut down, an RTC alarm is armed
   for one minute later, and the module powers off. The alarm restarts the module, and the
   cycle repeats from zero.

The demo source is `demo_helloworld.c`.

## At a glance

| Item | Value in the code |
|------|-------------------|
| Entry point | `sAPP_HelloWorldDemo()` |
| Main task | `helloWorldProcesser` – 30 KB stack, priority 150 |
| MQTT broker | `tcp://test.mosquitto.org:1883` (plain TCP, no TLS, no credentials) |
| MQTT client | index `0`, client ID `simcom` |
| Publish topic | `test/upload`, QoS 1 |
| Subscribe topic | `test/download`, QoS 1 |
| Publish period | `MQTT_UPDATE_PERIOD` = `20 * 200` ticks = **20 s** (1 tick = 5 ms) |
| Wakeup GPIO | `SC_MODULE_GPIO_03` (CTS), input, pull-up, **falling edge** |
| Publish limit per cycle | 5 (hard-coded as `mqtt_update_count < 5`) |
| RTC alarm | current time + 1 minute, then `sAPI_SysPowerOff()` |

## Runtime flow

### 1. Task creation

`sAPP_HelloWorldDemo()` creates `sTask_HelloWorldProcesser` with a 30 KB stack at priority
150 (the SDK recommends 150–250 for customer tasks).

### 2. Network registration and PDP activation

- `sAPI_NetworkSetCtzu(1)` – take the time zone from the network.
- `sAPI_NetworkInit()` – initialize the network service.
- Poll `sAPI_NetworkGetCpsi()` / `sAPI_NetworkGetCgreg()` every 200 ticks (1 s) until
  `networkmode` equals `"LTE,Online"`.
- Poll `sAPI_NetworkGetCgact()` every 100 ticks (500 ms). If `SCact[0].isActived != 1`, call
  `sAPI_NetworkSetCgact(1, 1)` to activate the first PDP context.

### 3. Inter-task objects

| Object | Creation | Purpose |
|--------|----------|---------|
| `urc_mqtt_msgq_1` | `sAPI_MsgQCreate(..., 4, SC_FIFO)` | Carries MQTT URCs (`SCmqttData`), depth 4 |
| `gMqttRecvTask` | `sAPI_TaskCreate(..., SC_DEFAULT_THREAD_STACKSIZE * 2, SC_DEFAULT_TASK_PRIORITY, ...)` | Receives and prints subscribed messages |
| `mqtt_data_update` | `sAPI_FlagCreate()` | Signals "publish now" to the main loop |
| `mqtt_data_timer` | `sAPI_TimerCreate()` | One-shot 20 s wakeup timer, restarted after every publish |

### 4. MQTT setup

```c
sAPI_MqttStart(-1);
sAPI_MqttAccq(0, NULL, 0, "simcom", 0, urc_mqtt_msgq_1);
sAPI_MqttConnLostCb(app_MqttLostConnCb);
sAPI_MqttConnect(0, NULL, 0, "tcp://test.mosquitto.org:1883", 60, 1, NULL, NULL);
sAPI_MqttSub(0, "test/download", strlen("test/download"), 1, 0);
```

Keep-alive is 60 s and the clean-session flag is `1`. The lost-connection callback only logs
`index` and `cause`; it does not reconnect.

### 5. Wakeup sources

- **Timer** – `sAPI_TimerStart(mqtt_data_timer, MQTT_UPDATE_PERIOD, 0, timer_callback, 0)`.
  `timer_callback` first disables system sleep, then sets flag bit `0x01` when
  `System_Mode == SYSTEM_MODE_NORMAL`.
- **GPIO** – `sAPI_GpioConfig()` configures `SC_MODULE_GPIO_03` as an input with pull-up and
  a falling-edge wakeup handler (`GPIO_WakeupHandler`), then
  `sAPI_GpioWakeupEnable(SC_MODULE_GPIO_03, SC_GPIO_FALL_EDGE)` arms it. The handler
  disables system sleep, **stops the pending timer**, and sets flag bit `0x02`.

### 6. Main event loop

```c
sAPI_FlagWait(mqtt_data_update, 0x01 | 0x02, SC_FLAG_OR_CLEAR, &flag, SC_SUSPEND);
```

The task blocks until either wakeup source fires, then:

1. `mqtt_package_preparation()` collects module information and builds a JSON string.
2. `sAPI_MqttTopic(0, "test/upload", ...)`, `sAPI_MqttPayload(0, json, strlen(json))`,
   `sAPI_MqttPub(0, 1, 60, 0, 0)`, then `free(json)`.
3. Logs `publish data triggered by timer` or `... by gpio` according to `flag`.
4. Increments `mqtt_update_count`.

### 7. Power decision

| After publish | Action |
|---------------|--------|
| Count 1–4 (`< 5`) | Restart the 20 s timer, then `sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_ENABLE)` – module may sleep until the next timer expiry or GPIO edge |
| Count 5 | Stop the timer, `sAPI_MqttDisConnect()` → `sAPI_MqttRel()` → `sAPI_MqttStop()`, read the RTC, add 1 minute, `sAPI_RtcSetAlarm()` + `sAPI_RtcEnableAlarm(1)`, then `sAPI_SysPowerOff()` – module powers off in RTC mode |

The RTC alarm restarts the module. Because `mqtt_update_count` is an ordinary `static` in RAM,
it starts from 0 again and the whole cycle (register → connect → 5 publishes → power off)
repeats every ~5 × 20 s plus reboot time.

## Power-state diagram

```text
        NORMAL (network + MQTT connected)
                  │
      publish 1..4│                         publish 5
   restart timer  │                         stop timer, disconnect MQTT
   sleep ENABLE   ▼                         RTC alarm = now + 1 min
        SYSTEM SLEEP                        │
   wake: 20 s timer / GPIO03 falling        ▼
                  │                     RTC MODE (SysPowerOff)
                  └────► publish            │
                                            │ alarm fires (module restarts)
                                            └──► NORMAL, count = 0
```

## Published payload

`mqtt_package_preparation()` builds the JSON with cJSON:

| Field | Source |
|-------|--------|
| `manufacture_namestr`, `Module_modelstr`, `Revision`, `GCAP`, `cgmr`, `internal_verstr`, `version_tagstr`, `SDK_Version` | `sAPI_SysGetVersion()` |
| `rfversion1`, `rfversion2` | `sAPI_SysGetRFVersion()` |
| `cusversion` | `sAPI_SysGetCusVersion()` |
| `ImeiValue` | `sAPI_SysGetImei()` |
| `cpuUsedRate`, `heapFreeSize` | `sAPI_GetSystemInfo()` |
| `Date` | `sAPI_NetworkGetRealTime()`, formatted as `20YY-MM-DD HH:MM:SS:±HH` |

Example:

```json
{
  "manufacture_namestr": "SIMCOM",
  "Module_modelstr": "A7670E",
  "Revision": "LE20B04SIM7600M22",
  "GCAP": "A7670E_GL",
  "cgmr": "LE20B04SIM7600M22",
  "internal_verstr": "...",
  "version_tagstr": "...",
  "SDK_Version": "...",
  "rfversion1": "...",
  "rfversion2": "...",
  "cusversion": "...",
  "ImeiValue": "86xxxxxxxxxxxxxxx",
  "cpuUsedRate": 12,
  "heapFreeSize": 123456,
  "Date": "2026-11-05 14:03:21:+08"
}
```

The whole JSON string is printed once through `SYSINFO()` before it is published.

## Subscribed messages

`MqttRecvTaskProcesser` blocks on `urc_mqtt_msgq_1`, ignores any message where
`msg_id != SC_SRV_MQTT`, `arg1 != 0` or `arg3 == NULL`, and for valid `SCmqttData` items:

- prints client index, topic and payload,
- frees `topic_P`, `payload_P` and the `SCmqttData` structure with `sAPI_Free()`.

## Key functions

| Function | Role |
|----------|------|
| `sAPP_HelloWorldDemo()` | Creates the main demo task; call once at startup |
| `sTask_HelloWorldProcesser()` | Network bring-up, MQTT setup, wakeup configuration, publish/sleep loop |
| `mqtt_package_preparation()` | Collects versions, IMEI, CPU/heap usage and network time into JSON |
| `MqttRecvTaskProcesser()` | Drains the MQTT URC queue and prints subscribed data |
| `timer_callback()` | 20 s timer handler: disable sleep, set flag `0x01` |
| `GPIO_WakeupHandler()` | GPIO wakeup handler: disable sleep, stop timer, set flag `0x02` |
| `system_printf()` / `SYSINFO()` | Formats a message and splits it into ≤180-character lines for `PrintfResp()` |
| `add_time_complete()` | Adds time to a `t_rtc` value with full carry handling (used for the RTC alarm) |
| `days_in_month()`, `is_leap_year()` | Calendar helpers used by `add_time_complete()` |
| `zellers_weekday()` | Weekday calculation utility; present in the file but not called by the demo flow |

## Configuration

Most values are hard-coded in `demo_helloworld.c`:

| What to change | Where |
|----------------|-------|
| Broker URL / port | `sAPI_MqttConnect(..., "tcp://test.mosquitto.org:1883", ...)` |
| Publish / subscribe topics | `"test/upload"`, `"test/download"` |
| Publish period | `#define MQTT_UPDATE_PERIOD 20*200` (ticks, 5 ms each) |
| Wakeup pin | `#define WAKEUP_GPIO_PIN SC_MODULE_GPIO_03` |
| Number of publishes before power-off | `if (mqtt_update_count < 5)` |
| RTC alarm offset | `add_time_complete(&timeval, 0, 1, 0)` – 0 h, 1 min, 0 s |
| Task stack / priority | `helloWorldProcesserStack[1024*30]`, priority `150` |

## Expected log output (excerpt)

```text
Task runs successfully
NetworkMode->LTE,Online,cgreg->1
cid=1,isActived=1,isdefine=1
Network is connected
MQTT Init Successful!
MQTT accquire Successful!
MQTT connect Successful!
MQTT Sub Successful!
Config OK!
Gpio set wake up OK !
{"manufacture_namestr":"SIMCOM", ... }
publish data triggered by timer
mqtt update count = 1
Module will go to sleep mode
...
mqtt update count = 5
Current RTC time:26-11-05 14:05:10 3
Module will go to RTC mode
```

## Notes and limitations

- The broker is the public Mosquitto test server over **plain TCP**; there is no TLS,
  authentication or payload privacy. Replace it with your own endpoint for production.
- `System_Mode` is initialized to `SYSTEM_MODE_NORMAL` and never changed in this demo, so the
  `System_Mode` check in `timer_callback()` always passes. `SYSTEM_MODE_SLEEP` is reserved
  for extending the demo.
- `sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_ENABLE)` only *permits* sleep; the module enters sleep
  once all tasks are idle. UART communication is unavailable while the module sleeps.
- A GPIO wakeup stops the 20 s timer, so a pending periodic publish is cancelled; the timer is
  restarted after the next publish (while the count is below 5).
- If the MQTT connection is lost, `app_MqttLostConnCb()` only prints the cause. No
  reconnect/back-off logic is implemented; a failure to reconnect does not block the
  sleep/power-off cycle.
- The 5-publish limit and 1-minute RTC alarm are compile-time constants in the code, not
  macros.
- `A7671_V101_GPIO.h` provides `SC_MODULE_GPIO_03`; the physical pin depends on the module
  variant. Verify the mapping against the hardware design guide before wiring the wakeup
  source.

## Integration

Copy `demo_helloworld.c` into the SDK application project (as with the other examples in this
repository), include the required SDK headers, and call `sAPP_HelloWorldDemo()` once from the
application entry point or demo menu. The demo requires a registered LTE network and an
active PDP context; it activates PDP context 1 itself if needed.

For detailed API signatures, refer to the SIMCom A76XX OpenSDK API documentation.
