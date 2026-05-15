# Project Evidence

Paste screenshots and photos directly below each heading.

Tip: after pasting an image, keep the short caption below it. The captions can be reused in the final report.

## 1. Monitoring Node Wiring

![Monitoring node current wiring](docs/evidence/01-monitor-node-wiring-current.png)

Figure 1 shows ESP #1, the monitoring node, connected to the PIR sensor, reed switch module, DS18B20 temperature sensor, and shared 3.3V/GND breadboard rails.

Status:

```text
Captured: yes
```

## 2. Monitoring Node Tasmota Module Configuration

![Monitoring node current Tasmota module configuration](docs/evidence/02-monitor-node-tasmota-current.png)
Figure 2 shows the Tasmota GPIO configuration for ESP #1:

```text
D2 / GPIO4  -> Switch1, PIR motion sensor
D4 / GPIO2  -> DS18x20 temperature sensor
D6 / GPIO12 -> Switch2, reed switch
```

Status:

```text
Captured: yes
```

## 3. Tasmota Console: PIR And Reed Events

![Tasmota console with PIR and reed events](docs/evidence/03-tasmota-console-switch-events.png)

Figure 3 shows the Tasmota console publishing `Switch1` events from the PIR sensor and `Switch2` events from the reed switch.

Status:

```text
Captured: yes
```

## 4. MQTT Explorer: Switch Events

![MQTT Explorer switch result topic](docs/evidence/04-mqtt-explorer-switch-result.png)

Figure 4 shows MQTT Explorer receiving switch events from the monitoring node under:

```text
stat/safety_monitor_1/RESULT
```

This proves that the sensor node publishes PIR and reed events to the MQTT broker.

Status:

```text
Captured: yes
```

## 5. MQTT Explorer: Temperature Sensor

![MQTT Explorer temperature sensor telemetry](docs/evidence/05-mqtt-explorer-temperature-sensor.png)

Figure 5 shows MQTT Explorer receiving DS18B20 temperature telemetry under:

```text
tele/safety_monitor_1/SENSOR
```

This proves that the monitoring node also publishes periodic sensor telemetry.

Status:

```text
Captured: yes
```

## 6. Alarm Node Wiring

![Alarm node wiring](docs/evidence/06-alarm-node-wiring.png)

Figure 6 shows ESP #2, the alarm node, connected to the relay and PWM buzzer actuator.

Status:

```text
Captured: yes
```

## 7. Alarm Node Tasmota Module Configuration

![Alarm node Tasmota module configuration](docs/evidence/07-alarm-node-tasmota-module.png)

Figure 7 shows the Tasmota GPIO configuration for ESP #2:

```text
D1 / GPIO5  -> Relay1
D5 / GPIO14 -> PWM1 buzzer
Topic       -> safety_alarm_1
```

Status:

```text
Captured: yes
```

## 8. Safety Context Node Wiring

![Safety context node wiring](docs/evidence/08-context-node-wiring.png)

Figure 8 shows ESP #3, the safety context node, connected to the vibration sensor, microphone module, and touch sensor.

Status:

```text
Captured: yes
```

## 9. Safety Context Node Tasmota Module Configuration

![Safety context node current Tasmota module configuration](docs/evidence/09-context-node-tasmota-current.png)
Figure 9 shows the Tasmota GPIO configuration for ESP #3:

```text
D1 / GPIO5 -> Switch1, vibration sensor
D2 / GPIO4 -> None
D5 / GPIO14 -> Switch2, touch / acknowledge sensor
A0 / GPIO17 -> ADC Input / Analog, microphone AO
Topic      -> safety_context_1
```

Status:

```text
Captured: yes
```

## 10. MQTT Explorer: Context Sensor Events And Telemetry

![Safety context node telemetry](docs/evidence/10-context-node-telemetry-current.png)

Figure 10 shows MQTT Explorer receiving context-node sensor events and analog microphone telemetry under:

```text
stat/safety_context_1/RESULT
tele/safety_context_1/SENSOR
```

Status:

```text
Captured: yes
```

Current observation:

```text
tele/safety_context_1/SENSOR includes Switch1, Switch2, and ANALOG.A0
SetOption114 1 detached the context-node switches from generic POWER behavior
SwitchMode1 1 and SwitchMode2 1 were applied
touch sensor now publishes clean Switch2 action messages
microphone is available as analog A0 telemetry
vibration is not yet verified as a clean Switch1 action
```

This proves the context node is online, the touch input is working as a clean MQTT switch event, and the microphone analog value is visible in telemetry. The vibration threshold still needs tuning before openHAB integration.

## 11. openHAB Basic UI: Current Safety-Monitoring View

Paste image here.

Figure 11 shows the current openHAB Basic UI with the monitoring sensors and ESP #2 relay/buzzer controls.

Status:

```text
Captured: no
```

## 12. Device 1 Or 3 Triggers Device 2

Paste image here.

Figure 12 shows the final distributed automation: an event from ESP #1 or ESP #3 is received by openHAB and causes an actuator output on ESP #2.

Status:

```text
Captured: no
```

## Evidence Chain

The final proof chain for the project is:

```text
PIR/reed sensor
  -> Tasmota on ESP #1
vibration/sound/touch sensor
  -> Tasmota on ESP #3
  -> MQTT broker
  -> openHAB item/rule
  -> MQTT command
  -> Tasmota on ESP #2
  -> relay/buzzer actuator
```
