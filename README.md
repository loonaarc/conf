# Distributed Edge-Based Safety Monitoring

This repository contains the openHAB configuration and documentation for a distributed safety-monitoring prototype built with ESP8266 D1 Mini nodes, Tasmota, MQTT, Mosquitto, openHAB, and an external HTTP safety-context feed.

The project demonstrates a local safety system where multiple sensor nodes publish events through MQTT, openHAB combines those events in rules, and a separate alarm node reacts with a relay and buzzer. In addition, openHAB reads official GeoSphere Austria warning data through the HTTP binding.

## Current Prototype

| Node / Component | Role | Implemented Hardware / Data |
| --- | --- | --- |
| ESP #1 `safety_monitor_1` | Monitoring node | PIR motion, reed/door sensor, DS18B20 temperature |
| ESP #2 `safety_alarm_1` | Alarm actuator node | Relay, PWM buzzer |
| ESP #3 `safety_context_1` | Context and acknowledgement node | Vibration sensor, analog microphone value, touch sensor |
| Mosquitto | MQTT broker | Local message broker for all Tasmota/openHAB MQTT traffic |
| openHAB | UI and rule engine | Basic UI, MQTT binding, HTTP binding, automation rules |
| GeoSphere Austria | External safety context | Official warning area, warning count, warning level, warning text |

## Implemented Behavior

The current rule logic reduces false alarms by requiring combined evidence:

```text
door AND (motion OR vibration)
  -> relay ON
  -> buzzer ON at selected intensity

touch acknowledgement
  -> relay OFF
  -> buzzer OFF
```

The buzzer intensity slider stores the selected intensity. When the buzzer is switched on, openHAB sends that selected value to the Tasmota PWM output.

## Mid-Term Requirement Coverage

| Requirement | Status |
| --- | --- |
| 2x D1 Mini devices with at least one sensor/actuator each | Done, with 3 nodes implemented |
| Network setup and network plan | Done, see `docs/network-plan.puml` and evidence image |
| Mosquitto broker | Done |
| Device 1 and 2 MQTT access | Done |
| openHAB installed | Done |
| Basic UI sitemap | Done |
| Display sensor/actuator value in openHAB UI | Done |
| Manual actuator action via openHAB UI | Done |
| External webservice through HTTP binding | Done with GeoSphere Austria warnings |
| Basic openHAB rules | Done |

## Demo Flow

1. Start Mosquitto, MQTT Explorer, openHAB, and the Tasmota web UIs.
2. Open the Basic UI:

```text
http://localhost:8080/basicui/app?sitemap=safety_monitor
```

3. Show the three node groups in the UI:
   - motion, door/reed, temperature
   - vibration, sound level, touch
   - relay, buzzer power, buzzer intensity
4. Show the GeoSphere Austria external safety context.
5. Enable `Motion automation`.
6. Trigger door + motion, or door + vibration.
7. Show that openHAB commands the ESP #2 relay and buzzer.
8. Touch the touch sensor to acknowledge and silence the alarm.
9. Show `events.log` and/or MQTT Explorer as proof of the data flow.

## Start Script

From the openHAB root folder:

```powershell
cd C:\Users\lil\openhab-5.1.4
.\conf\scripts\start_safety_monitor.ps1
```

The script opens:

- Tasmota UI for ESP #1, ESP #2, and ESP #3
- Mosquitto
- MQTT Explorer
- openHAB
- openHAB Basic UI

MQTT Explorer may still require selecting the saved broker connection manually.

## Important Files

| File | Purpose |
| --- | --- |
| `things/mqtt.things` | MQTT broker and Tasmota channels |
| `things/http.things` | GeoSphere Austria HTTP warning channels |
| `items/safety_monitor.items` | openHAB Items used by UI and rules |
| `rules/safety_monitor.rules` | Combined safety-event rules and buzzer behavior |
| `sitemaps/safety_monitor.sitemap` | Basic UI layout |
| `services/addons.cfg` | Required openHAB add-ons |
| `docs/network-plan.puml` | PlantUML network plan source |
| `EVIDENCE.md` | Screenshots, photos, and proof chain |

Required add-ons:

```text
binding = mqtt,http
transformation = jsonpath
ui = basic
```

## Evidence And Documentation

- [EVIDENCE.md](EVIDENCE.md): screenshots/photos for wiring, Tasmota, MQTT Explorer, openHAB UI, rule proof, HTTP binding, and network plan.
- [ARCHITECTURE.md](ARCHITECTURE.md): architecture and data-flow explanation.
- [MIDTERM_CHECKLIST.md](MIDTERM_CHECKLIST.md): requirement coverage for the mid-term check.
- [WIRING_GUIDE.md](WIRING_GUIDE.md): wiring and pinout notes.
- [HARDWARE_INVENTORY.md](HARDWARE_INVENTORY.md): available modules and chosen hardware.
- [PROJECT_PLAN.md](PROJECT_PLAN.md): project direction toward final check and TinyML/risk scoring.
- [FINAL_CHECKLIST.md](FINAL_CHECKLIST.md): remaining work for the final project.
- [SETUP_NOTES.md](SETUP_NOTES.md): detailed setup, flashing, MQTT, and troubleshooting notes.

## Current Limitations And Final Direction

The mid-term prototype focuses on a stable vertical slice: distributed nodes, MQTT, openHAB UI, HTTP binding, and rules. For the final project, the planned extensions are:

- risk score item and clearer alarm states
- persistence/history for selected values
- LWT/online status for each node
- security documentation
- optional TinyML or smarter classification using sound/vibration context
