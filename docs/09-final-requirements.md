# Final Project Check Checklist

This checklist is based on the IoT Applications final project check requirements. It translates the grading points into concrete tasks for this project.

## Requirement Status

The final check is graded with 30 points and uses the MoSCoW categories from the assignment PDF.

| Requirement | Category | Points | Current status | Next action |
| --- | --- | ---: | --- | --- |
| Project documentation, wiring plan, network plan, configuration screenshots, security documentation, openHAB config files | Must | 4 | In progress | Add wiring diagrams and finish security/config screenshots |
| Implementation of 2 D1 Mini devices including actuators and sensors | Must | 1 | Done, with extra third node | Keep ESP #1 monitoring, ESP #2 alarm, ESP #3 context |
| openHAB config via config files, not graphical UI | Must | 2 | Done | Keep `.things`, `.items`, `.rules`, `.sitemap`, `addons.cfg` |
| Security configuration for devices, MQTT, and openHAB | Must | 3 | Documented | Capture security evidence screenshots |
| Device online/offline status in Sitemap UI | Must | 1 | Done | Evidence captured in [evidence.md](evidence.md) |
| Manual actuator control via Sitemap UI | Must | 1 | Done | Relay and buzzer controls are in Basic UI |
| Display actuator/sensor values | Must | 1 | Done | Values are grouped by node |
| Display webservice values via HTTP binding | Must | 1 | Done | GeoSphere Austria warning context is shown |
| openHAB rule: device 1 value change triggers device 2 actuator | Must | 1 | Done | Risk level now triggers the alarm node |
| Webservice-based rule to control device actuator | Must | 1 | Done | Warning count/level are used as risk-score modifiers |
| Display historical values in openHAB UI | Must | 1 | Done | Capture history chart evidence |
| Internet access to openHAB UI | Must | 1 | Done | Capture myopenHAB online evidence |
| Short demonstration video | Should | 2 | Missing | Record final flow after UI/history is stable |
| Additional actuators/sensors used | Should | 1 | Done | ESP #3 context node plus sound/vibration/temperature and ESP #2 touch support this |
| Geolocation-based map | Should | 1 | Missing | Add if time remains |
| Persistence implementation, not default rrd4j | Should | 2 | Done | Capture JDBC/SQLite proof |
| Customization of UI according to use case | Should | 2 | Done/in progress | Per-node sitemap is implemented; final screenshot still needed |
| Additional MainUI page or HABPanel | Could | 2 | Missing | Optional dashboard if time remains |
| Extra effort | Could | 2 | In progress | Risk scoring and TinyML extension can support this |

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
| Device status | Implemented MQTT LWT Items for all current Tasmota nodes | [evidence.md](evidence.md) |
| Per-node UI | Implemented sitemap frames for monitoring, alarm, and context nodes | Capture final UI after layout stabilizes |
| Risk score | Implemented openHAB RiskScore/RiskLevel/AlarmState Items and rules | Capture Basic UI and events.log evidence |
| Historical values | JDBC/SQLite persistence with Basic UI charts for RiskScore, Temperature, and SoundLevel | Capture history chart evidence |
| Security documentation | Local lab setup, MQTT/openHAB/device security, risks, and improvements | [08-security.md](08-security.md) |
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
