# Architecture

This document describes the current architecture of the distributed edge-based safety monitoring project. For setup and startup commands, see [README.md](README.md). For wiring details, see [WIRING_GUIDE.md](WIRING_GUIDE.md).

## Current Goal

The current milestone proves this chain:

```text
ESP #1 sensors
  -> Tasmota
  -> Mosquitto MQTT broker
  -> openHAB MQTT binding
  -> openHAB Items / Basic UI / rule
```

The next larger milestone is:

```text
ESP #1 or ESP #3 sensor event
  -> MQTT
  -> openHAB rule
  -> MQTT command
  -> ESP #2 alarm actuator
```

That step proves that one D1 Mini device can trigger a second D1 Mini device, while a third D1 Mini adds extra context for a stronger risk score.

## System Components

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
                                  | MQTT events
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
              user view    |     | MQTT commands
              / control    v     v
             +-------------+     +-----------------------------+
             | Browser /   |     | ESP #2 Safety Alarm Node    |
             | Basic UI    |     | topic: safety_alarm_1       |
             +-------------+     | D1 -> Relay1                |
                                 | D5 -> PWM1 buzzer           |
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
```

The bridge connects openHAB to Mosquitto on:

```text
localhost:1883
```

The `safety_monitor_1` Thing represents ESP #1 and exposes the monitoring sensor channels. The `safety_alarm_1` Thing represents ESP #2 and exposes the relay and buzzer actuator channels. ESP #3 is wired in hardware and should be added to openHAB next as `safety_context_1`.

| Channel | MQTT topic | Transformation | Meaning |
| --- | --- | --- | --- |
| `motion` | `stat/safety_monitor_1/RESULT` | `JSONPATH:$.Switch1.Action` | PIR motion from Switch1 |
| `door` | `stat/safety_monitor_1/RESULT` | `JSONPATH:$.Switch2.Action` | Reed switch from Switch2 |
| `temperature` | `tele/safety_monitor_1/SENSOR` | `JSONPATH:$.DS18B20.Temperature` | DS18B20 temperature |
| `relay` | `stat/safety_alarm_1/POWER`, `cmnd/safety_alarm_1/POWER` | none | Reads and commands the alarm relay |
| `buzzerPower` | `stat/safety_alarm_1/POWER2`, `cmnd/safety_alarm_1/POWER2` | none | Stops or enables the PWM buzzer output |
| `buzzerLevel` | `cmnd/safety_alarm_1/Dimmer` | none | Sends the PWM buzzer intensity |
| context vibration, next | `stat/safety_context_1/RESULT` | `JSONPATH:$.Switch1.Action` | Vibration event from ESP #3 |
| context touch, next | `stat/safety_context_1/RESULT` | `JSONPATH:$.Switch2.Action` | Manual acknowledge/silence input from ESP #3 |
| context microphone, next | `tele/safety_context_1/SENSOR` | `JSONPATH:$.ANALOG.A0` | Analog microphone value from ESP #3 |

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

## Item Layer

[items/safety_monitor.items](items/safety_monitor.items) defines the openHAB Items:

| Item | Type | Source |
| --- | --- | --- |
| `Relay` | `Switch` | ESP #2 MQTT relay channel |
| `Motion` | `Switch` | PIR / Switch1 channel |
| `Door` | `Switch` | Reed / Switch2 channel |
| `Temperature` | `Number` | DS18B20 temperature channel |
| `Buzzer` | `Switch` | ESP #2 buzzer power channel |
| `BuzzerLevel` | `Dimmer` | ESP #2 buzzer PWM level channel |
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

## Rule Layer

[rules/safety_monitor.rules](rules/safety_monitor.rules) currently contains one rule:

```text
Motion changed to ON
```

If `MotionAutomation` is ON, the current rule toggles the relay item:

```text
Motion ON -> openHAB rule -> Relay command
```

The relay and buzzer hardware have moved to ESP #2. The next rule step is to command ESP #2 when ESP #1 or ESP #3 reports a risky event.

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
binding = mqtt
transformation = jsonpath
ui = basic
```

The MQTT binding connects openHAB to Mosquitto. JSONPATH extracts values from Tasmota JSON payloads. Basic UI renders the sitemap.

## Lab Origin

This project continues from the IoT Applications Lab 1 and Lab 3 setup. The current configuration keeps the useful lab chain but renames and extends it toward the safety-monitoring project:

```text
D1 Mini / Tasmota -> Mosquitto -> openHAB -> rule/UI -> actuator
```
