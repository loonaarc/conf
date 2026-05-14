# Wiring Guide

This guide is for the first hardware milestone:

```text
PIR + reed switch -> D1 Mini -> MQTT -> openHAB
```

## Breadboard Basics

A breadboard lets you connect components without soldering.

The long side rails are usually used for power:

```text
red rail  -> 3.3V in this project
blue rail -> GND
```

For this project, use the D1 Mini `3V3` pin for the breadboard power rail unless a specific module explicitly requires 5V. This keeps the sensor outputs safer for the ESP8266 GPIO pins, which are 3.3V logic pins.

On most breadboards, each long red rail is connected along its length, and each long blue rail is connected along its length. Some breadboards split the long rails in the middle, so check the printed line on the breadboard or test continuity if a sensor does not power on.

Use the rails to share the one D1 Mini power pin with several sensors:

```text
D1 Mini 3V3       -> red rail
D1 Mini GND       -> blue rail
sensor VCC/+      -> red rail
sensor GND/-      -> blue rail
```

The middle area is split into rows. On a typical breadboard, five holes in the same row are connected:

```text
a b c d e     connected together
f g h i j     connected together
```

The gap in the middle separates the two sides. A component placed across the middle gap has its pins separated, which is useful for modules and chips.

Important rule:

```text
GND must be shared.
```

If a sensor and the D1 Mini use different power sources, their GND pins still need to be connected together for the signal to make sense.

## D1 Mini Pin Names

The D1 Mini labels pins as `D1`, `D2`, etc. Internally, the ESP8266 uses GPIO numbers.

| D1 Mini label | ESP8266 GPIO | Suggested use |
| --- | --- | --- |
| `D1` | GPIO5 | Relay, already used in Lab 1 |
| `D2` | GPIO4 | PIR motion input / `Switch1` in the current lab setup |
| `D4` | GPIO2 | DS18x20 temperature sensor in the current lab setup |
| `D6` | GPIO12 | Reed switch input |
| `D5` | GPIO14 | Alternative PIR motion input |
| `D7` | GPIO13 | Vibration or extra input |

Avoid using `D3`, `D4`, and `D8` for beginner sensor wiring unless needed, because they can affect boot behavior if pulled to the wrong state.

The current working monitoring-node setup is:

```text
D4 / GPIO2 -> DS18x20 temperature sensor
D2 / GPIO4 -> PIR motion sensor as Switch1
D1 / GPIO5 -> Relay1 from the older lab setup, current test actuator
D6 / GPIO12 -> Reed switch as Switch2
```

Recommended Tasmota module settings for this setup:

```text
GPIO2  (D4) -> DS18x20
GPIO4  (D2) -> Switch1
GPIO5  (D1) -> Relay1
GPIO12 (D6) -> Switch2
```

## DS18B20 Temperature Sensor

For the DS18B20 temperature sensor, hold the sensor with the flat side facing you and the legs pointing downward:

```text
Left   -> GND
Middle -> DATA
Right  -> VCC 3.3V
```

Current wiring:

| DS18B20 pin | D1 Mini / breadboard |
| --- | --- |
| `GND` | blue rail / `GND` |
| `DATA` | `D4` / GPIO2 |
| `VCC` | red rail / `3V3` |

Tasmota publishes the temperature periodically under:

```text
tele/safety_monitor_1/SENSOR
```

Example payload:

```json
{"DS18B20":{"Temperature":23.8},"TempUnit":"C"}
```

## PIR Sensor HC-SR501

Typical pins:

```text
VCC
OUT
GND
```

Recommended wiring for ESP #1 monitoring node:

| PIR pin | D1 Mini |
| --- | --- |
| `VCC` | red rail / `3V3` |
| `OUT` | `D2` / GPIO4 in the current lab setup |
| `GND` | `GND` |

Some HC-SR501 PIR modules are often powered with 5V, but for this project start with 3.3V if the module works reliably. If a module must use 5V, check that its `OUT` pin is still 3.3V-safe before connecting it to the ESP8266.

Recommended Tasmota module setting:

```text
GPIO4 (D2) -> Switch1
```

The current openHAB config already expects a Switch1-style JSON field:

```text
$.Switch1.Action
```

## Reed Switch / Magnetic Sensor

A reed switch is a simple magnetic contact. It behaves like a button:

```text
magnet near     -> contact closed or open, depending on module
magnet away     -> opposite state
```

For a bare two-wire reed switch, use:

| Reed wire | D1 Mini |
| --- | --- |
| one side | `D6` / GPIO12 |
| other side | `GND` |

A bare reed switch does not need VCC. It is only a magnetic contact, like a button.

If the reed sensor is a small module with pins marked `+`, `G`, `DO`, and sometimes `AO`, wire it like this:

| Reed module pin | D1 Mini / breadboard |
| --- | --- |
| `+` or `VCC` | red rail / `3V3` |
| `G` or `GND` | blue rail / `GND` |
| `DO` | `D6` / GPIO12 |
| `AO` | not used |

Use `DO`, not `AO`, for Tasmota switch input. `DO` is the digital on/off output. `AO` is an analog output and is not needed for a door/window open/closed state.

Use a pull-up input mode in Tasmota if available, for example:

```text
GPIO12 (D6) -> Switch2
```

The reed module has been verified with a magnet. If Tasmota reports `Switch2` as `TOGGLE`, that still proves the input is working. For a cleaner openHAB door item later, configure Switch2 to report explicit `ON` and `OFF` states:

```text
SwitchMode2 1
```

If the displayed door/reed state is backwards, use inverted follow mode instead:

```text
SwitchMode2 2
```

Depending on how the reed switch behaves, the state may be inverted. That is normal and can be adjusted later in Tasmota or openHAB.

If Tasmota telemetry shows `"Switch2":"OFF"` but no `Switch2` change appears in the console:

1. Confirm Tasmota has `GPIO12 (D6) -> Switch2`.
2. Confirm the reed module `DO` pin goes to `D6`, not `AO`.
3. Confirm the module `GND` is connected to the same GND rail as the D1 Mini.
4. Try both magnet positions and both sides of the reed sensor, because the active area can be small.
5. Watch for the module LED, if it has one. It should usually change when the magnet moves.
6. Briefly test the D6 input directly by connecting `D6` to `GND`. If Tasmota logs `Switch2`, the D1/Tasmota side works and the issue is the reed module or magnet position.

## Alarm Node

The alarm node should be separate from the monitoring node.

During the step-by-step build, the old Lab 3 relay can stay on ESP #1 as a local test actuator. In the final architecture, the main alarm relay/buzzer should move to ESP #2 so that a sensor event on device 1 can trigger an actuator on device 2.

Recommended first version:

| Component | D1 Mini pin |
| --- | --- |
| Relay module signal | `D1` / GPIO5 |
| Active buzzer signal | `D5` / GPIO14 or `D6` / GPIO12 |
| GND | `GND` |

Be careful with current:

- The ESP8266 GPIO pins are signal pins, not power outputs.
- A relay module usually needs external 5V power.
- If a separate 5V supply is used for a relay module, the D1 Mini and relay/buzzer power supply must share GND.
- Do not power high-current loads directly from a GPIO pin.

If the relay used to click when it was plugged directly on top of the D1 Mini but does not click when wired on the breadboard, check that the relay has all required connections, not only the signal pin:

```text
relay signal/input -> D1 / GPIO5
relay VCC/+        -> required relay supply
relay GND/-        -> D1 Mini GND / shared ground rail
```

A mechanical relay clicks only when its coil is powered and switched. If only the signal wire is connected, Tasmota can publish `POWER ON/OFF` but the relay hardware will not move.

For a simple active buzzer module, Tasmota can often control it like a relay output:

```text
GPIO14 (D5) -> Relay2
```

Then openHAB can send commands through MQTT just like it does for a relay.

## First Wiring Target

Build only the monitoring node first:

```text
D1 Mini
  D2 -> PIR OUT
  D4 -> DS18x20 data
  D6 -> reed DO, or one side of a bare reed switch
  GND -> blue breadboard rail
  3V3 -> red breadboard rail
```

Then verify in this order:

1. Tasmota web page opens.
2. Tasmota console shows sensor events.
3. Open MQTT Explorer and connect it to the broker.
4. MQTT Explorer shows `stat/safety_monitor_1/RESULT` when `Switch1` or `Switch2` changes.
5. MQTT Explorer shows `tele/safety_monitor_1/SENSOR` for the DS18x20 temperature value.
6. openHAB receives the item state.
7. openHAB rule calculates risk score.

Current verified result:

```text
MQTT connected: yes
PIR on Switch1: yes, publishes stat/safety_monitor_1/RESULT
DS18B20 temperature: yes, publishes tele/safety_monitor_1/SENSOR
Switch2 in telemetry: yes
Reed change event: yes, publishes Switch2 when triggered by magnet
Relay shield on D1: yes, clicks when toggled
```

Important MQTT Explorer note:

- `stat/safety_monitor_1/RESULT` updates immediately when `Switch1` or `Switch2` changes.
- `tele/safety_monitor_1/SENSOR` is periodic. With the current `TelePeriod` of 300 seconds, temperature may only update about every 5 minutes unless Tasmota is restarted or telemetry is requested.

The core final-check build only needs the monitoring node and the alarm node to be stable. The advanced node can be added after that, and the fourth D1 Mini is optional if it cannot be recovered.
