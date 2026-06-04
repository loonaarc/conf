# Distributed Edge-Based Safety Monitoring

Project name:

```text
Distributed Edge-Based Safety Monitoring with optional TinyML Extension
```

This project continues from the existing IoT Applications Lab 3 openHAB configuration. The current setup already proves the basic chain:

```text
D1 Mini / Tasmota -> Mosquitto MQTT -> openHAB -> rule/UI -> relay
```

The next goal is to turn this into a distributed safety-monitoring system with several ESP8266 nodes.

## Strategy

Build the stable IoT infrastructure first. TinyML is optional and comes later, after sensor events are collected reliably.

Current order:

1. Identify and name all ESP8266 devices.
2. Build one clean monitoring node with PIR and reed switch.
3. Publish stable MQTT events.
4. Integrate those events into openHAB items.
5. Add the alarm node with buzzer and/or relay.
6. Implement the first cross-device rule.
7. Implement the first risk-score rule.
8. Add vibration and sound sensors on ESP #3.
9. Integrate ESP #3 events into openHAB.
10. Later, use collected data for a TinyML experiment.
11. Add ESP32 audio-node debug observability with serial output and optional temporary MQTT feature previews.

For the mid-term check, prioritize the second D1 Mini before polishing the risk score. The mid-term requirements explicitly ask for two D1 Mini devices with at least one actuator or sensor per device and MQTT access for both devices.

## Device Roles

Available modules and hardware priorities are documented in [02-hardware.md](02-hardware.md).

| ESP | Role | Components | Purpose |
| --- | --- | --- | --- |
| ESP #1 | Monitoring node | PIR, reed switch | Detect motion and door/window opening |
| ESP #2 | Alarm node | Relay, PWM-controlled buzzer | React to alerts from openHAB |
| ESP #3 | Safety context node | Vibration sensor, microphone, touch sensor | Detect shock/loud sound and provide manual acknowledge input |
| ESP #4 | Optional auxiliary/status node | Built-in LED first, optional extra sensor later | Optional extension if the fourth board works |

## Recommended Device Names

Use clear Tasmota topics instead of the default generated topic names.

| Role | Tasmota topic |
| --- | --- |
| Monitoring node | `safety_monitor_1` |
| Alarm node | `safety_alarm_1` |
| Safety context node | `safety_context_1` |
| Optional auxiliary/status node | `safety_aux_1` |

In Tasmota, set the MQTT topic under:

```text
Configuration -> Configure MQTT -> Topic
```

Set the visible device/friendly name separately under:

```text
Configuration -> Configure Other
```

Keep a small inventory table while identifying the boards:

| Role | Topic | IP address | MAC address | Notes |
| --- | --- | --- | --- | --- |
| Monitoring node | `safety_monitor_1` | `192.168.43.223` | `A8:48:FA:C1:7D:DD` | ESP #1, label `1`, PIR + reed |
| Alarm node | `safety_alarm_1` | `192.168.43.110` | `C4:D8:D5:12:B5:63` | ESP #2, label `2`, relay + PWM buzzer |
| Safety context node | `safety_context_1` | `192.168.43.240` | `EC:64:C9:DF:12:9B` | ESP #3, label `3`, vibration + microphone + touch |
| Optional auxiliary/status node | `safety_aux_1` | not working yet | TBD | optional extension |

The fourth D1 Mini is useful but not required for the core project. The final check requires at least two D1 Mini devices with sensors/actuators. The stable core should therefore use ESP #1 as monitoring node and ESP #2 as alarm node. ESP #3 adds extra sensor capability, and ESP #4 remains an optional extension if it can be recovered later.

## Identified Devices

### ESP #1: Safety Monitor 1

This is the first confirmed D1 Mini and will be used as the monitoring node.

| Field | Value |
| --- | --- |
| Physical label | `1` |
| Tasmota module | `Generic` |
| Device name | `Safety Monitor 1` |
| Friendly name | `Safety Monitor 1` |
| Tasmota version | `15.3.0 (release-tasmota)` |
| Hostname | `safety-monitor-1-7645` |
| MAC address | `A8:48:FA:C1:7D:DD` |
| IP address | `192.168.43.223` |
| Wi-Fi SSID | `realme C3` |
| MQTT host | `192.168.43.26` |
| MQTT port | `1883` |
| MQTT user | `DVES_USER` |
| MQTT topic | `safety_monitor_1` |
| MQTT full topic | `cmnd/safety_monitor_1/` |
| HTTP API | Enabled |
| Planned sensors | PIR motion sensor, reed switch |

### ESP #2: Safety Alarm 1

This D1 Mini will be used as the actuator/alarm node.

| Field | Value |
| --- | --- |
| Physical label | `2` |
| Tasmota module | `Sonoff Basic` |
| Device name | `Safety Alarm 1` |
| Friendly name | `Safety Alarm 1` |
| Tasmota version | `15.3.0 (release-tasmota)` |
| Hostname | `safety-alarm-1-5475` |
| MAC address | `C4:D8:D5:12:B5:63` |
| IP address | `192.168.43.110` |
| Wi-Fi SSID | `realme C3` |
| MQTT host | `192.168.43.26` |
| MQTT port | `1883` |
| MQTT user | `DVES_USER` |
| MQTT client | `DVES_12B563` |
| MQTT topic | `safety_alarm_1` |
| MQTT full topic | `cmnd/safety_alarm_1/` |
| HTTP API | Enabled |
| Planned actuators | relay working, buzzer working with `PWM1` / `Dimmer 50` |

### ESP #3: Safety Context 1

This D1 Mini was flashed with Tasmotizer and configured through `Send config`.

| Field | Value |
| --- | --- |
| Physical label | `3` |
| Tasmota module | `Sonoff Basic` |
| Device name | `Safety Context 1` |
| Friendly name | `Safety Context 1` |
| Tasmota version | `15.3.0 (release-tasmota)` |
| Hostname | `safety-advanced-1-4763` |
| MAC address | `EC:64:C9:DF:12:9B` |
| IP address | `192.168.43.240` |
| Wi-Fi SSID | `realme C3` |
| MQTT host | `192.168.43.26` |
| MQTT port | `1883` |
| MQTT user | `DVES_USER` |
| MQTT client | `DVES_DF129B` |
| MQTT topic | `safety_context_1` |
| MQTT full topic | `cmnd/safety_context_1/` |
| HTTP API | Enabled |
| Sensors | vibration sensor, microphone, touch sensor, optional TinyML experiment later |

## MQTT Topic Design

Tasmota has a built-in topic structure:

```text
tele/<topic>/...
stat/<topic>/...
cmnd/<topic>/...
```

Use this first because it works well with Tasmota and openHAB.

Examples:

```text
tele/safety_monitor_1/LWT
stat/safety_monitor_1/RESULT
cmnd/safety_alarm_1/POWER
cmnd/safety_alarm_1/POWER2
cmnd/safety_alarm_1/Dimmer
stat/safety_context_1/RESULT
```

Meaning:

| Prefix | Direction | Meaning |
| --- | --- | --- |
| `tele` | device -> broker | Telemetry, sensor status, availability |
| `stat` | device -> broker | State/result messages |
| `cmnd` | broker -> device | Commands sent to the device |

Later, if custom firmware or Node-RED is added, a cleaner project namespace can be introduced:

```text
safety/node/<node-id>/event/<event-name>
safety/node/<node-id>/score
safety/alarm/cmd
```

For now, stay with Tasmota topics to avoid unnecessary complexity.

## First Milestone

The immediate milestone is:

```text
PIR/Reed -> MQTT -> openHAB -> risk score -> buzzer/relay
```

Minimum events:

| Event | Sensor | Example openHAB item |
| --- | --- | --- |
| `motion_detected` | PIR | `Monitor1_Motion` |
| `door_open` | Reed switch | `Monitor1_Door` |
| `alarm_active` | Buzzer/relay node | `Alarm1_Alarm` |

## Risk Score Concept

openHAB is the fusion layer. It receives separate sensor events and calculates the risk score.

Initial scoring:

| Event | Score |
| --- | --- |
| Motion detected | +1 |
| Door/reed open | +2 |
| Vibration detected | +3 |
| Loud sound detected | +3 |
| Multiple events in a short time | +2 |

Initial risk levels:

| Score | Level | Action |
| --- | --- | --- |
| 0-1 | Low | Log only |
| 2-3 | Medium | Show warning |
| 4-5 | High | Trigger local alarm |
| 6+ | Critical | Escalation placeholder |

The first real rule should only use PIR and reed:

```text
motion + door open = high risk
```

That is enough to prove the architecture before adding vibration, sound, or TinyML.

## Final Check Alignment

The final IoT Applications check requires more than just a working sensor. The architecture should therefore grow toward these deliverables:

| Requirement area | Project answer |
| --- | --- |
| At least 2 D1 Mini devices with sensors/actuators | Monitoring node + alarm node |
| Additional sensors/actuators | PIR, reed, vibration, microphone, buzzer, relay |
| File-based openHAB config | Keep using `.things`, `.items`, `.rules`, `.sitemap`, `addons.cfg` |
| Device connection status in UI | Add LWT/availability items for each node |
| Manual actuator control in UI | Add switches for buzzer/relay alarm node |
| Sensor/actuator values in UI | Show motion, door, vibration, sound, alarm state, risk score |
| Rule where device 1 triggers device 2 | Monitoring node events trigger alarm node actuator |
| Security documentation | Document Wi-Fi, MQTT, openHAB access, and known lab tradeoffs |
| HTTP webservice value and rule | GeoSphere Austria warning values are shown; later use warning count/level as a risk modifier |
| Geolocation map | Add device or monitored-area location later |
| Persistence and historical values | Add non-default persistence later, for example InfluxDB or JDBC |
| Customized UI | Rename sitemap/pages around the safety-monitoring use case |
| Demo video | Record final flow: event -> MQTT -> openHAB -> risk score -> alarm |

## Mid-Term Check Alignment

The Lab 4 mid-term check is a status check, not the final project. The project should show a stable vertical slice and a clear next step.

| Mid-term requirement | Current project answer |
| --- | --- |
| 2x D1 mini devices | ESP #1 works; ESP #2 is flashed/named and relay/buzzer work in Tasmota |
| At least one actuator or sensor per D1 mini | ESP #1 has PIR, reed, DS18B20; ESP #2 has relay and buzzer actuators |
| Network setup | D1 nodes -> Mosquitto -> openHAB -> browser/MQTT Explorer |
| Mosquitto broker | Installed and used locally |
| Devices 1 and 2 MQTT access | ESP #1 proven; ESP #2 should be shown in MQTT Explorer with `safety_alarm_1` |
| openHAB installed | Done |
| Basic UI sitemap | `safety_monitor` sitemap exists |
| Display sensor/actuator value in UI | ESP #1 motion, door, temperature shown; ESP #2 relay/buzzer controls added |
| Manual actuator action in UI | ESP #2 relay/buzzer can be controlled from openHAB |
| External webservice via HTTP binding | Done: GeoSphere Austria warnings in Basic UI |
| Basic openHAB rules | Done: door AND (motion OR vibration) triggers ESP #2; touch acknowledges |

Mid-term target:

```text
ESP #1 sensor values shown in openHAB
ESP #2 actuator manually controllable from openHAB
optional rule: ESP #1 event triggers ESP #2 actuator
```

ESP #2 now has the relay and PWM buzzer, and openHAB has separate relay, buzzer power, and buzzer level items for `safety_alarm_1`. ESP #1 should remain sensor-only.

ESP #3 is now wired as the safety context node with vibration, microphone, and touch modules. For the mid-term, ESP #3 can be shown as extra progress. For the final project, it should become part of the risk-score and acknowledgement flow:

```text
PIR or reed event from ESP #1
+ vibration event or high analog sound value from ESP #3
-> higher risk score
-> relay/buzzer command to ESP #2

touch event from ESP #3
-> acknowledge, silence, arm, or disarm action in openHAB
```

The context-node switch inputs are detached from Tasmota's default `POWER` behavior with:

```text
SetOption114 1
SwitchMode1 1
SwitchMode2 1
```

## TinyML Extension Later

TinyML should not block the current build.

Potential later use:

- anomaly detection
- event classification
- sound/vibration pattern classification
- edge inference on the safety context node

Possible comparison:

| Model type | Purpose |
| --- | --- |
| TensorFlow float32 | Baseline model |
| Quantized TensorFlow Lite | Smaller TinyML-style model |

Compare:

- model size
- inference latency
- memory usage
- accuracy

TinyML should become an enhancement to the event detection layer, not a replacement for the MQTT/openHAB infrastructure.

## TinyML Debug Observability

The ESP32 audio node should have two operating views:

```text
normal operation:
ESP32 publishes label/confidence/inference time to openHAB

development/debug:
ESP32 exposes raw sample previews or feature values through serial output
and optionally through temporary MQTT debug topics
```

This is similar in purpose to using the Tasmota console and MQTT Explorer for the D1 Mini nodes. The difference is that the ESP32 audio node will use custom firmware, so the debug interface must be implemented explicitly.

Planned debug outputs:

| Output | Purpose |
| --- | --- |
| Serial monitor | Raw sample preview, RMS/peak, feature values, predicted class |
| `tele/safety_audio_1/FEATURES` | Temporary feature/debug MQTT payload |
| `tele/safety_audio_1/AUDIO_DEBUG` | Optional short sample preview, not continuous raw audio |

The normal deployed data path should still use:

```text
tele/safety_audio_1/CLASSIFICATION
```

with summarized data only.
