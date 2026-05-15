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
ESP #1 sensor event
  -> MQTT
  -> openHAB rule
  -> MQTT command
  -> ESP #2 alarm actuator
```

That later step proves that one D1 Mini device can trigger a second D1 Mini device.

## System Components

```text
+-----------------------------+
| ESP #1 Safety Monitor Node  |
| Tasmota topic:              |
| safety_monitor_1            |
|                             |
| D2 -> PIR / Switch1         |
| D6 -> Reed / Switch2        |
| D4 -> DS18B20 temperature   |
+--------------+--------------+
               |
               | MQTT
               v
+--------------+--------------+
| Mosquitto MQTT broker       |
| host: localhost for openHAB |
| port: 1883                  |
+--------------+--------------+
               ^
               |
               | MQTT binding
               v
+--------------+--------------+
| openHAB                     |
| Things / Items / Rules      |
| Basic UI sitemap            |
+--------------+--------------+
               |
               | user view/control
               v
+--------------+--------------+
| Browser / Basic UI          |
| sitemap=safety_monitor      |
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

Tasmota uses the topic name:

```text
safety_monitor_1
```

Important topics:

```text
tele/safety_monitor_1/LWT
tele/safety_monitor_1/SENSOR
stat/safety_monitor_1/RESULT
stat/safety_monitor_1/POWER
cmnd/safety_monitor_1/POWER
```

Meaning:

| Topic | Direction | Purpose |
| --- | --- | --- |
| `tele/safety_monitor_1/LWT` | Tasmota -> MQTT | Online/offline status |
| `tele/safety_monitor_1/SENSOR` | Tasmota -> MQTT | Periodic sensor telemetry, including DS18B20 temperature |
| `stat/safety_monitor_1/RESULT` | Tasmota -> MQTT | Immediate switch events from PIR/reed |
| `stat/safety_alarm_1/POWER` | Tasmota -> MQTT | ESP #2 relay state |
| `cmnd/safety_alarm_1/POWER` | openHAB/MQTT -> Tasmota | ESP #2 relay command |

## Thing Layer

[things/mqtt.things](things/mqtt.things) contains:

```text
Bridge mqtt:broker:mosquitto
Thing mqtt:topic:d1mini
```

The bridge connects openHAB to Mosquitto on:

```text
localhost:1883
```

The `d1mini` Thing currently represents ESP #1 and exposes the sensor channels. The relay has physically moved to ESP #2, so the next openHAB integration step is to move relay control from `safety_monitor_1` to `safety_alarm_1`.

| Channel | MQTT topic | Transformation | Meaning |
| --- | --- | --- | --- |
| `relay` | currently still configured for `safety_monitor_1`; should move to `safety_alarm_1` next | none | Reads and commands the alarm relay |
| `motion` | `stat/safety_monitor_1/RESULT` | `JSONPATH:$.Switch1.Action` | PIR motion from Switch1 |
| `door` | `stat/safety_monitor_1/RESULT` | `JSONPATH:$.Switch2.Action` | Reed switch from Switch2 |
| `temperature` | `tele/safety_monitor_1/SENSOR` | `JSONPATH:$.DS18B20.Temperature` | DS18B20 temperature |

The reed switch originally produced `TOGGLE` actions. The Tasmota command below was needed so openHAB receives clean `ON` and `OFF` states:

```text
SwitchMode2 1
```

## Item Layer

[items/safety_monitor.items](items/safety_monitor.items) defines the openHAB Items:

| Item | Type | Source |
| --- | --- | --- |
| `Relay` | `Switch` | MQTT relay channel, next target is ESP #2 |
| `Motion` | `Switch` | PIR / Switch1 channel |
| `Door` | `Switch` | Reed / Switch2 channel |
| `Temperature` | `Number` | DS18B20 temperature channel |
| `MotionAutomation` | `Switch` | Local openHAB control, not MQTT-linked |

Items are the values that the UI displays and the rules use.

## UI Layer

[sitemaps/safety_monitor.sitemap](sitemaps/safety_monitor.sitemap) defines the Basic UI page:

```text
http://localhost:8080/basicui/app?sitemap=safety_monitor
```

The current UI shows:

- relay switch, next target is ESP #2
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

The relay hardware has moved to ESP #2. The next config step is to point the relay MQTT channel and rule behavior at `safety_alarm_1`, then remove the old relay setting from ESP #1.

## Helper Script

[scripts/start_safety_monitor.ps1](scripts/start_safety_monitor.ps1) opens the local demo tools:

- Tasmota web UI
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
