# Final Project Check Checklist

This checklist is based on the IoT Applications final project check requirements. It translates the grading points into concrete tasks for this project.

## Must-Have Direction

These items should be treated as required for the final check.

| Requirement | Current status | Next action |
| --- | --- | --- |
| Project documentation | Started | Keep README, architecture, wiring guide, project plan, final checklist |
| Wiring plan of Tasmota devices | Missing | Create wiring diagram for each D1 Mini, e.g. with EasyEDA |
| Network plan | Partly documented | Add diagram with D1 nodes, Mosquitto, openHAB, laptop/network |
| Configuration screenshots | In progress | Continue adding Tasmota, MQTT Explorer, openHAB UI, and openHAB file screenshots to [evidence.md](evidence.md) |
| Security documentation | Missing | Document Wi-Fi, MQTT access, openHAB access, risks, improvements |
| openHAB config files | Present | Keep all config file based, not MainUI-only |
| 2 D1 Mini devices with sensors/actuators | Done, with extra third node | ESP #1 monitoring, ESP #2 alarm, ESP #3 context |
| File-based openHAB configuration | Present | Continue using `.things`, `.items`, `.rules`, `.sitemap` |
| Device online/offline status in UI | Done | Capture online/offline screenshots for evidence |
| Manual actuator control in UI | Done | Relay and buzzer controls are in Basic UI |
| Sensor/actuator values in UI | Done for mid-term | Final extension: split sitemap into one frame per node and add risk score |
| Rule: device 1 event controls device 2 actuator | Done for mid-term | Final extension: risk scoring/TinyML |
| HTTP webservice value in UI | Done | GeoSphere Austria warning context is shown in Basic UI |
| HTTP webservice based rule | Planned | Use warning count/level as a risk modifier later |
| Historical values in UI | Missing | Add persistence and display history |
| Internet access to openHAB UI | Missing | Decide safe access method and document it |
| Risk score | Planned | Add `RiskScore`, `RiskLevel`, and a recalculation rule |
| TinyML extension | Planned | Add ESP32 audio-node MQTT summary and model-comparison evidence |
| UI organization | Planned | Group status, sensors, and actuators by physical node in the sitemap |

## Should/Could-Have Direction

These items can improve the grade if there is time.

| Requirement | Possible project implementation |
| --- | --- |
| Short demo video | Record a 1-2 minute flow with sensor trigger, MQTT, openHAB UI, alarm |
| Additional sensors/actuators | Add vibration sensor, sound sensor, active buzzer |
| Geolocation map | Show monitored area or device location |
| Non-default persistence | Use InfluxDB, JDBC, MongoDB, or DynamoDB; not only default rrd4j |
| Custom UI | Safety-monitoring sitemap or MainUI page with risk score and device status |
| Additional MainUI/HABPanel | Add a safety dashboard if time allows |
| Extra effort | Dynamic risk scoring and optional TinyML experiment |

## Recommended Final Node Names

| ESP | Role | Tasmota topic |
| --- | --- | --- |
| ESP #1 | Monitoring node | `safety_monitor_1` |
| ESP #2 | Alarm node | `safety_alarm_1` |
| ESP #3 | Safety context node | `safety_context_1` |
| ESP #4 | Optional auxiliary/status node | `safety_aux_1` |

## Documentation Evidence To Collect

Capture these while building, so the final report is easier:

- photo of each labeled D1 Mini
- screenshot of each Tasmota information page
- screenshot of MQTT Explorer topics
- screenshot of openHAB Basic UI or MainUI
- screenshot or copy of relevant `.things`, `.items`, `.rules`, `.sitemap`
- wiring diagram for each node
- short explanation of security choices and limitations
- short explanation of risk score logic
- notebook screenshots or plots for the regular-vs-TinyML model comparison
- MQTT payload screenshot for the optional ESP32 audio node

## Requirement Mapping

| Requirement area | Project implementation | Evidence location |
| --- | --- | --- |
| Distributed devices | ESP #1 monitoring, ESP #2 alarm, ESP #3 context | [evidence.md](evidence.md) |
| MQTT communication | Tasmota topics through Mosquitto and openHAB MQTT binding | MQTT Explorer screenshots |
| Device-to-device rule | Sensor events in openHAB trigger relay/buzzer on alarm node | `events.log` screenshot |
| External webservice | GeoSphere Austria HTTP warning feed | Basic UI screenshot |
| Device status | Planned MQTT LWT Items | To capture after implementation |
| Per-node UI | Planned sitemap frames for monitoring, alarm, and context nodes | To capture after sitemap cleanup |
| Risk score | Planned openHAB score and level Items | To capture after implementation |
| Historical values | Planned persistence for risk, sound, temperature, alarm | To capture after implementation |
| Security documentation | Local lab setup, risks, and improvements | [08-security.md](08-security.md) |
| TinyML extension | Optional ESP32 audio classifier sends label/confidence | [06-tinyml-extension.md](06-tinyml-extension.md), [07-model-comparison.md](07-model-comparison.md) |

## First Final-Check Milestone

The first milestone that clearly satisfies the main architecture requirement is:

```text
safety_monitor_1 PIR/reed event
  -> MQTT
  -> openHAB risk score rule
  -> MQTT command
  -> safety_alarm_1 buzzer/relay
```

This proves distributed sensing, MQTT communication, openHAB rule processing, and actuator control across two D1 Mini devices.
