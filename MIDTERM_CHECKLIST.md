# Mid-Term Project Check Checklist

This checklist is based on `IoT Lab Requirements mid-term project check.pdf`.

Maximum score: 20 points.

## Required For Mid-Term

| Requirement | Current project status | Next action |
| --- | --- | --- |
| Configure 2x D1 mini devices | Done: ESP #1 monitoring node and ESP #2 alarm node; ESP #3 context node is extra progress | Show nodes during demo |
| At least one actuator or sensor per D1 mini | Done: ESP #1 has PIR/reed/DS18B20; ESP #2 has relay/buzzer; ESP #3 has vibration/touch/microphone | Show hardware and UI |
| Network setup shown | Partly documented | Add a simple network plan diagram |
| Mosquitto broker installed | Done | Show Mosquitto running during demo |
| Device 1 MQTT access | Done: `safety_monitor_1` | Show in MQTT Explorer |
| Device 2 MQTT access | Done: `safety_alarm_1` relay, buzzer power, and dimmer topics are visible | Show `safety_alarm_1` in MQTT Explorer |
| openHAB installed | Done | Start openHAB and show Basic UI |
| Basic UI sitemap configured | Done: `safety_monitor` | Show Safety Monitor sitemap |
| Display at least one device value/status in openHAB UI | Done: sensor values, actuator states, and external warning values are shown | Capture UI evidence |
| Manual action on device actuator via openHAB UI | Done: relay and buzzer can be controlled from Basic UI | Demonstrate in UI |
| External webservice via HTTP binding | Done: GeoSphere Austria warnings shown in External Safety Context | Capture UI evidence |
| Basic rules in openHAB | Done: door AND (motion OR vibration) triggers ESP #2; touch acknowledges alarm | Show `events.log` |

## Recommended Mid-Term Demo Target

The strongest realistic mid-term target is:

```text
ESP #1 monitoring node
  PIR/reed/temperature
  -> MQTT
  -> openHAB UI/rules

ESP #2 alarm node
  relay and PWM-controlled buzzer
  <- MQTT command
  <- openHAB manual UI switch/rule

ESP #3 context node
  vibration/touch/microphone
  -> MQTT
  -> openHAB UI/rules

GeoSphere Austria warnings
  -> HTTP binding
  -> openHAB External Safety Context
```

Implemented rule:

```text
ESP #1 door/motion event or ESP #3 vibration event
  -> openHAB rule
  -> ESP #2 relay/buzzer ON

ESP #3 touch event
  -> openHAB rule
  -> ESP #2 relay/buzzer OFF
```

## Relay Decision

The relay has been moved to ESP #2.

For mid-term, this is cleaner than keeping the main actuator on ESP #1. ESP #1 can now be described as the monitoring node, and ESP #2 as the alarm actuator node:

```text
ESP #1 = PIR + reed + temperature
ESP #2 = relay + buzzer actuators
```

Next cleanup step: after openHAB controls ESP #2 successfully, remove the old relay GPIO setting from ESP #1 Tasmota so ESP #1 is sensor-only.

## Evidence To Prepare

Capture or show:

- ESP #1 Tasmota page and console
- ESP #2 Tasmota page and console
- MQTT Explorer showing `safety_monitor_1`
- MQTT Explorer showing `safety_alarm_1`
- openHAB Basic UI `safety_monitor`
- Manual actuator control from openHAB
- Basic rule behavior
- Network plan with D1 nodes, Mosquitto, openHAB, browser, and MQTT Explorer
- HTTP binding value in openHAB UI

## Next Practical Steps

1. Capture the final Basic UI including External Safety Context.
2. Capture or keep the `events.log` proof for the rule behavior.
3. Show MQTT Explorer with all three node topics.
4. Prepare a short verbal walkthrough of MQTT vs HTTP roles.
5. Mark the evidence screenshots as captured in `EVIDENCE.md`.
6. Keep remaining final-project work separate: persistence, security notes, LWT status, and TinyML/risk scoring.
