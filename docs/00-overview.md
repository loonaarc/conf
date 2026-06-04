# Project Overview

This project combines a distributed IoT safety-monitoring prototype with an optional TinyML sound-classification extension.

The IoT part is the main final-project demonstrator:

```text
D1 Mini / Tasmota sensors
  -> MQTT / Mosquitto
  -> openHAB Items, UI, and rules
  -> risk score
  -> D1 Mini alarm actuator
```

The TinyML part is a focused extension for the Wahlfachprojekt:

```text
ESP32 + INMP441 microphone
  -> local sound classification
  -> MQTT label/confidence summary
  -> openHAB risk score
```

The two parts are intentionally separated. The openHAB system should still work without TinyML, while the TinyML work can be evaluated as a small regular-vs-tiny model comparison.

## Current Node Roles

| Node | Topic | Role | Status |
| --- | --- | --- | --- |
| ESP #1 | `safety_monitor_1` | PIR, reed switch, DS18B20 temperature | Implemented |
| ESP #2 | `safety_alarm_1` | Relay and PWM buzzer actuator | Implemented |
| ESP #3 | `safety_context_1` | Vibration, analog microphone value, touch acknowledgement | Implemented |
| ESP32 audio node | `safety_audio_1` | INMP441 microphone and TinyML sound class | Planned |

## Work Packages

| Area | Next work |
| --- | --- |
| openHAB final project | Per-node UI layout, risk score, JDBC/SQLite persistence, security notes, final demo flow |
| TinyML Wahlfachprojekt | Notebook, dataset plan, regular TensorFlow model, quantized TFLite model, comparison table |
| ESP32 integration | INMP441 audio capture, TinyML inference, MQTT payload, debug observability, openHAB channel integration |
| Documentation | Keep root README short and put detailed documentation under `docs/` |

## Documentation Map

| File | Purpose |
| --- | --- |
| [01-architecture.md](01-architecture.md) | System architecture and data flow |
| [02-hardware.md](02-hardware.md) | Hardware inventory and node assignment |
| [03-wiring.md](03-wiring.md) | Wiring notes |
| [04-openhab-mqtt.md](04-openhab-mqtt.md) | Setup, Tasmota, MQTT, openHAB troubleshooting |
| [05-risk-score.md](05-risk-score.md) | Risk-score design |
| [06-tinyml-extension.md](06-tinyml-extension.md) | ESP32 TinyML audio node |
| [07-model-comparison.md](07-model-comparison.md) | Regular TensorFlow vs TinyML comparison |
| [08-security.md](08-security.md) | Security notes |
| [09-final-requirements.md](09-final-requirements.md) | Final project requirement mapping |
| [10-demo-flow.md](10-demo-flow.md) | Demo and video flow |
| [11-persistence-history.md](11-persistence-history.md) | JDBC/SQLite persistence and history charts |
| [evidence.md](evidence.md) | Screenshots, photos, and proof chain |
| [project-plan.md](project-plan.md) | Detailed project plan |
