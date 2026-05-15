# Mid-Term Project Check Checklist

This checklist is based on `IoT Lab Requirements mid-term project check.pdf`.

Maximum score: 20 points.

## Required For Mid-Term

| Requirement | Current project status | Next action |
| --- | --- | --- |
| Configure 2x D1 mini devices | ESP #1 works; ESP #2 is flashed/named and relay works in Tasmota | Add ESP #2 to openHAB |
| At least one actuator or sensor per D1 mini | ESP #1 has PIR, reed, DS18B20; ESP #2 has relay actuator | Show both devices during demo |
| Network setup shown | Partly documented | Add a simple network plan diagram |
| Mosquitto broker installed | Done | Show Mosquitto running during demo |
| Device 1 MQTT access | Done: `safety_monitor_1` | Show in MQTT Explorer |
| Device 2 MQTT access | Relay works in Tasmota; MQTT proof should be captured | Show `safety_alarm_1` in MQTT Explorer |
| openHAB installed | Done | Start openHAB and show Basic UI |
| Basic UI sitemap configured | Done: `safety_monitor` | Show Safety Monitor sitemap |
| Display at least one device value/status in openHAB UI | Done for ESP #1: motion, door, temperature | Add ESP #2 relay state/switch |
| Manual action on device actuator via openHAB UI | Next target: ESP #2 relay | Add manual ESP #2 relay switch |
| External webservice via HTTP binding | Missing | Add one HTTP value to UI |
| Basic rules in openHAB | Started | Keep simple rule; later use ESP #1 event to trigger ESP #2 |

## Recommended Mid-Term Demo Target

The strongest realistic mid-term target is:

```text
ESP #1 monitoring node
  PIR/reed/temperature
  -> MQTT
  -> openHAB UI

ESP #2 alarm node
  relay or buzzer
  <- MQTT command
  <- openHAB manual UI switch
```

If time allows, add the rule:

```text
ESP #1 motion/reed event
  -> openHAB rule
  -> ESP #2 relay/buzzer ON
```

## Relay Decision

The relay has been moved to ESP #2.

For mid-term, this is cleaner than keeping the main actuator on ESP #1. ESP #1 can now be described as the monitoring node, and ESP #2 as the alarm actuator node:

```text
ESP #1 = PIR + reed + temperature
ESP #2 = relay actuator
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
- HTTP binding value in openHAB UI, if implemented

## Next Practical Steps

1. Prove ESP #2 MQTT access in MQTT Explorer.
2. Add ESP #2 manual actuator control to openHAB.
3. Remove the old ESP #1 relay config after ESP #2 control works.
4. Update Basic UI with the ESP #2 actuator state/switch.
5. Add a simple HTTP-binding value.
6. Add or adjust a basic rule for the mid-term demo.
