# Architecture

This document describes the current architecture of the distributed edge-based safety monitoring project. For setup and startup commands, see [README.md](README.md). For wiring details, see [WIRING_GUIDE.md](WIRING_GUIDE.md).

The PlantUML network plan is available in [docs/network-plan.puml](docs/network-plan.puml).

## Current Goal

The current milestone proves two communication chains.

Local distributed IoT chain:

```text
ESP #1 / ESP #3 sensors
  -> Tasmota MQTT
  -> Wi-Fi router / hotspot
  -> Mosquitto MQTT broker
  -> openHAB MQTT binding
  -> openHAB Items / Basic UI / rules
  -> MQTT commands
  -> Wi-Fi router / hotspot
  -> ESP #2 relay and buzzer
```

External safety-context chain:

```text
GeoSphere Austria Warn API
  -> openHAB HTTP binding
  -> warning Items
  -> Basic UI External Safety Context frame
```

Together, these chains prove local distributed sensing/actuation and an external webservice input that can later become part of the risk score.

## System Components

The network plan can be rendered from [docs/network-plan.puml](docs/network-plan.puml). It shows the Wi-Fi router/hotspot, the laptop/openHAB host, Mosquitto, MQTT Explorer, all three ESP/Tasmota nodes, the browser, and the GeoSphere Austria HTTP feed.

```text
+-----------------------------+        +-----------------------------+
| ESP #1 Safety Monitor Node  |        | ESP #3 Safety Context Node  |
| topic: safety_monitor_1     |        | topic: safety_context_1     |
| D2 -> PIR / Switch1         |        | D1 -> Vibration / Switch1   |
| D6 -> Reed / Switch2        |        | D5 -> Touch / Switch2       |
| D4 -> DS18B20 temperature   |        | A0 -> Microphone analog     |
|                             |        +--------------+--------------+
+--------------+--------------+                       |
               |                                      |
               +------------------+-------------------+
                                  |
                                  | Wi-Fi MQTT events
                                  v
                    +-------------+-------------+
                    | Wi-Fi Router / Hotspot    |
                    | local network: 192.168.43.x|
                    +-------------+-------------+
                                  |
                                  | local network
                                  v
                    +-------------+-------------+
                    | Mosquitto MQTT broker     |
                    | host: localhost for       |
                    | openHAB, port: 1883       |
                    +-------------+-------------+
                                  ^
                                  |
                                  | MQTT binding
                                  v
                    +-------------+-------------+
                    | openHAB                   |
                    | Things / Items / Rules    |
                    | Basic UI sitemap          |
                    +------+------+-------------+
                           |     |
              user view    |     | MQTT commands over Wi-Fi
              / control    v     v
             +-------------+     +-----------------------------+
             | Browser /   |     | ESP #2 Safety Alarm Node    |
             | Basic UI    |     | topic: safety_alarm_1       |
             +-------------+     | D1 -> Relay1                |
                                 | D5 -> PWM1 buzzer           |
                                 +-----------------------------+

                    +-----------------------------+
                    | GeoSphere Austria Warn API  |
                    | HTTP JSON warning data      |
                    +--------------+--------------+
                                   |
                                   | HTTP binding
                                   v
                    +--------------+--------------+
                    | openHAB warning Items       |
                    | External Safety Context UI  |
                    +-----------------------------+
```

MQTT Explorer is used beside openHAB as a debugging and evidence tool. It connects to the same broker and shows the raw Tasmota topics before openHAB transforms them into Items.

## File-Based Configuration

The project uses openHAB text configuration files:

```text
things/mqtt.things
  defines MQTT broker and MQTT channels
        |
        v
things/http.things
  defines GeoSphere Austria warning channels
        |
        v
items/safety_monitor.items
  links channels to openHAB Items
        |
        +--> sitemaps/safety_monitor.sitemap
        |      displays Items in Basic UI
        |
        +--> rules/safety_monitor.rules
               reacts to Item state changes
```

This keeps the configuration inspectable and suitable for documentation/screenshots.

## MQTT Topics

The Tasmota nodes use these topic names:

```text
safety_monitor_1
safety_alarm_1
safety_context_1
```

Important topics:

```text
tele/safety_monitor_1/LWT
tele/safety_monitor_1/SENSOR
stat/safety_monitor_1/RESULT
stat/safety_context_1/RESULT
tele/safety_context_1/SENSOR
stat/safety_alarm_1/POWER
cmnd/safety_alarm_1/POWER
cmnd/safety_alarm_1/Dimmer
cmnd/safety_alarm_1/POWER2
```

Meaning:

| Topic | Direction | Purpose |
| --- | --- | --- |
| `tele/safety_monitor_1/LWT` | Tasmota -> MQTT | Online/offline status |
| `tele/safety_monitor_1/SENSOR` | Tasmota -> MQTT | Periodic sensor telemetry, including DS18B20 temperature |
| `stat/safety_monitor_1/RESULT` | Tasmota -> MQTT | Immediate switch events from PIR/reed |
| `stat/safety_context_1/RESULT` | Tasmota -> MQTT | Immediate switch events from vibration and touch |
| `tele/safety_context_1/SENSOR` | Tasmota -> MQTT | Periodic context telemetry, including microphone `ANALOG.A0` |
| `stat/safety_alarm_1/POWER` | Tasmota -> MQTT | ESP #2 relay state |
| `cmnd/safety_alarm_1/POWER` | openHAB/MQTT -> Tasmota | ESP #2 relay command |
| `cmnd/safety_alarm_1/Dimmer` | openHAB/MQTT -> Tasmota | ESP #2 PWM buzzer intensity |
| `cmnd/safety_alarm_1/POWER2` | openHAB/MQTT -> Tasmota | ESP #2 buzzer off/on state |

## Thing Layer

[things/mqtt.things](things/mqtt.things) contains:

```text
Bridge mqtt:broker:mosquitto
Thing mqtt:topic:safety_monitor_1
Thing mqtt:topic:safety_alarm_1
Thing mqtt:topic:safety_context_1
```

The bridge connects openHAB to Mosquitto on:

```text
localhost:1883
```

The `safety_monitor_1` Thing represents ESP #1 and exposes the monitoring sensor channels. The `safety_alarm_1` Thing represents ESP #2 and exposes the relay and buzzer actuator channels. The `safety_context_1` Thing represents ESP #3 and exposes vibration, touch, and analog sound context.

| Channel | MQTT topic | Transformation | Meaning |
| --- | --- | --- | --- |
| `motion` | `stat/safety_monitor_1/RESULT` | `JSONPATH:$.Switch1.Action` | PIR motion from Switch1 |
| `door` | `stat/safety_monitor_1/RESULT` | `JSONPATH:$.Switch2.Action` | Reed switch from Switch2 |
| `temperature` | `tele/safety_monitor_1/SENSOR` | `JSONPATH:$.DS18B20.Temperature` | DS18B20 temperature |
| `relay` | `stat/safety_alarm_1/POWER`, `cmnd/safety_alarm_1/POWER` | none | Reads and commands the alarm relay |
| `buzzerPower` | `stat/safety_alarm_1/POWER2`, `cmnd/safety_alarm_1/POWER2` | none | Stops or enables the PWM buzzer output |
| `buzzerLevel` | `cmnd/safety_alarm_1/Dimmer` | none | Sends the PWM buzzer intensity |
| `vibration` | `stat/safety_context_1/RESULT` | `JSONPATH:$.Switch1.Action` | Vibration event from ESP #3 |
| `touch` | `stat/safety_context_1/RESULT` | `JSONPATH:$.Switch2.Action` | Manual acknowledge/silence input from ESP #3 |
| `soundLevel` | `tele/safety_context_1/SENSOR` | `JSONPATH:$.ANALOG.A0` | Analog microphone value from ESP #3 |

The context node should not be treated as an actuator. Its switch inputs are detached from Tasmota's default power-control behavior with:

```text
SetOption114 1
SwitchMode1 1
SwitchMode2 1
```

The intended integration is based on `Switch1` and `Switch2` sensor events plus `ANALOG.A0` telemetry.

The reed switch originally produced `TOGGLE` actions. The Tasmota command below was needed so openHAB receives clean `ON` and `OFF` states:

```text
SwitchMode2 1
```

[things/http.things](things/http.things) contains the HTTP thing:

```text
Thing http:url:geosphere_warnings
```

It fetches official GeoSphere Austria warning data:

```text
https://warnungen.zamg.at/wsapp/api/getWarningsForCoords?lon=16.3738&lat=48.2082&lang=en
```

The channels extract the warning area, number of active warnings, first warning level, and first warning text.

## Item Layer

[items/safety_monitor.items](items/safety_monitor.items) defines the openHAB Items:

| Item | Type | Source |
| --- | --- | --- |
| `Relay` | `Switch` | ESP #2 MQTT relay channel |
| `Motion` | `Switch` | PIR / Switch1 channel |
| `Door` | `Switch` | Reed / Switch2 channel |
| `Temperature` | `Number` | DS18B20 temperature channel |
| `Buzzer` | `Switch` | ESP #2 buzzer power channel |
| `BuzzerLevel` | `Dimmer` | Local openHAB buzzer intensity preset |
| `BuzzerCommand` | `Dimmer` | ESP #2 buzzer PWM command channel |
| `Vibration` | `String` | ESP #3 vibration event channel |
| `Touch` | `String` | ESP #3 touch acknowledgement channel |
| `SoundLevel` | `Number` | ESP #3 microphone analog channel |
| `AustriaWarningLocation` | `String` | GeoSphere Austria HTTP warning area |
| `AustriaWarningCount` | `Number` | GeoSphere Austria active warning count |
| `AustriaWarningLevel` | `Number` | GeoSphere Austria first warning level |
| `AustriaWarningText` | `String` | GeoSphere Austria first warning text |
| `MotionAutomation` | `Switch` | Local openHAB control, not MQTT-linked |

Items are the values that the UI displays and the rules use.

## UI Layer

[sitemaps/safety_monitor.sitemap](sitemaps/safety_monitor.sitemap) defines the Basic UI page:

```text
http://localhost:8080/basicui/app?sitemap=safety_monitor
```

The current UI shows:

- relay switch for ESP #2
- buzzer switch and buzzer level slider for ESP #2
- PIR motion state
- reed/door state
- DS18B20 temperature
- motion automation enable switch
- external GeoSphere Austria warning context

## Rule Layer

[rules/safety_monitor.rules](rules/safety_monitor.rules) contains the current alarm logic:

```text
door AND (motion OR vibration)
```

If `MotionAutomation` is ON, combined sensor evidence triggers the alarm node:

```text
Door + Motion -> openHAB rule -> Relay/Buzzer ON
Door + Vibration -> openHAB rule -> Relay/Buzzer ON
```

The touch sensor on ESP #3 acknowledges the alarm:

```text
Touch ON -> openHAB rule -> Relay/Buzzer OFF
```

The buzzer intensity is handled through a local preset item and a separate MQTT command item so changing the preset does not accidentally act as the only buzzer power control.

## Helper Script

[scripts/start_safety_monitor.ps1](scripts/start_safety_monitor.ps1) opens the local demo tools:

- Tasmota web UI for ESP #1 Safety Monitor
- Tasmota web UI for ESP #2 Safety Alarm
- Tasmota web UI for ESP #3 Safety Context
- Mosquitto
- MQTT Explorer
- openHAB
- Basic UI sitemap

Run from the openHAB root:

```powershell
cd C:\Users\lil\openhab-5.1.4
.\conf\scripts\start_safety_monitor.ps1
```

## Add-ons

[services/addons.cfg](services/addons.cfg) installs the required openHAB add-ons:

```text
binding = mqtt,http
transformation = jsonpath
ui = basic
```

The MQTT binding connects openHAB to Mosquitto. The HTTP binding reads the GeoSphere Austria warning feed. JSONPATH extracts values from Tasmota and GeoSphere JSON payloads. Basic UI renders the sitemap.

## Lab Origin

This project continues from the IoT Applications Lab 1 and Lab 3 setup. The current configuration keeps the useful lab chain but renames and extends it toward the safety-monitoring project:

```text
D1 Mini / Tasmota -> Mosquitto -> openHAB -> rule/UI -> actuator
```
