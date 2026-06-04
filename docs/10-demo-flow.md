# Final Demo Flow

This file is the script for the final live demo or short demo video.

## Preparation

Start from the openHAB root folder:

```powershell
cd C:\Users\lil\openhab-5.1.4
.\conf\scripts\start_safety_monitor.ps1
```

Open:

```text
http://localhost:8080/basicui/app?sitemap=safety_monitor
```

Also keep MQTT Explorer and `events.log` visible for proof.

## Demo Steps

1. Show the hardware nodes and explain the roles:
   - ESP #1 monitoring node
   - ESP #2 alarm node
   - ESP #3 context node
   - optional ESP32 TinyML audio node
2. Show MQTT Explorer with live topics for `safety_monitor_1`, `safety_alarm_1`, and `safety_context_1`.
3. Show the openHAB Basic UI with node availability from MQTT LWT.
4. Optional: unplug one node briefly and show the UI changing to `Offline`.
5. Enable `Motion automation`.
6. Trigger door/reed and motion on ESP #1.
7. Show that openHAB receives the Item updates.
8. Show the relay and buzzer on ESP #2 activating.
9. Trigger vibration on ESP #3 as additional context evidence.
10. Trigger touch on ESP #3 to acknowledge and silence the alarm.
11. Show GeoSphere Austria warning values in the external safety context frame.
12. If TinyML is ready, trigger a sound event and show `label/confidence` arriving through MQTT.
13. Point to the risk score or planned risk-score behavior.

## Evidence To Capture

| Evidence | File or view |
| --- | --- |
| Hardware wiring | Photos in `docs/evidence/` |
| MQTT topics | MQTT Explorer screenshot |
| openHAB UI | Basic UI screenshot |
| Node availability | Basic UI screenshot with all LWT values online |
| Offline detection | Optional Basic UI screenshot after one node disconnects |
| Rule execution | `events.log` screenshot |
| External webservice | GeoSphere warning UI screenshot |
| TinyML comparison | Notebook plots and model-comparison table |

## UI Layout Direction

For the final UI, group values by node rather than mixing all sensors into one generic device frame.

Recommended frame structure:

```text
Monitoring Node
  status
  motion
  door/reed
  temperature

Alarm Node
  status
  relay
  buzzer power
  buzzer intensity

Context Node
  status
  vibration
  sound level
  touch acknowledgement

External Safety Context
  GeoSphere warning values
```

This makes the distributed architecture easier to understand during the demo.

## Short Explanation

Use this wording for the final presentation:

```text
The system separates sensing, decision logic, and actuation across multiple edge devices.
Tasmota nodes publish sensor events through MQTT.
openHAB combines those events into safety logic and controls the alarm node.
The optional TinyML audio node extends the system by classifying sound locally and sending only summarized results.
```
