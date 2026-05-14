# Wiring Guide

This guide is for the first hardware milestone:

```text
PIR + reed switch -> D1 Mini -> MQTT -> openHAB
```

## Breadboard Basics

A breadboard lets you connect components without soldering.

The long side rails are usually used for power:

```text
red rail  -> 3.3V or 5V
blue rail -> GND
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
| `D5` | GPIO14 | PIR motion input |
| `D6` | GPIO12 | Reed switch input |
| `D7` | GPIO13 | Vibration or extra input |
| `D2` | GPIO4 | Alternative input/output |

Avoid using `D3`, `D4`, and `D8` for beginner sensor wiring unless needed, because they can affect boot behavior if pulled to the wrong state.

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
| `VCC` | `5V` |
| `OUT` | `D5` / GPIO14 |
| `GND` | `GND` |

The HC-SR501 usually works well with 5V power, and its output is often 3.3V-compatible, but check your module. If the output is 5V, use a voltage divider or level shifter before connecting it to the ESP8266.

Recommended Tasmota module setting:

```text
GPIO14 (D5) -> Switch1
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

Use a pull-up input mode in Tasmota if available, for example:

```text
GPIO12 (D6) -> Switch2
```

Depending on how the reed switch behaves, the state may be inverted. That is normal and can be adjusted later in Tasmota or openHAB.

## Alarm Node

The alarm node should be separate from the monitoring node.

Recommended first version:

| Component | D1 Mini pin |
| --- | --- |
| Relay module signal | `D1` / GPIO5 |
| Active buzzer signal | `D5` / GPIO14 or `D6` / GPIO12 |
| GND | `GND` |

Be careful with current:

- The ESP8266 GPIO pins are signal pins, not power outputs.
- A relay module usually needs external 5V power.
- The D1 Mini and relay/buzzer power supply must share GND.
- Do not power high-current loads directly from a GPIO pin.

For a simple active buzzer module, Tasmota can often control it like a relay output:

```text
GPIO14 (D5) -> Relay2
```

Then openHAB can send commands through MQTT just like it does for a relay.

## First Wiring Target

Build only the monitoring node first:

```text
D1 Mini
  D5 -> PIR OUT
  D6 -> Reed switch
  GND -> PIR GND + reed GND
  5V -> PIR VCC
```

Then verify in this order:

1. Tasmota web page opens.
2. Tasmota console shows sensor events.
3. MQTT Explorer shows `stat/safety_monitor_1/RESULT`.
4. openHAB receives the item state.
5. openHAB rule calculates risk score.

The core final-check build only needs the monitoring node and the alarm node to be stable. The advanced node can be added after that, and the fourth D1 Mini is optional if it cannot be recovered.
