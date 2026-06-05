# Hardware Inventory

This file lists the available hardware modules and how they fit into the safety-monitoring project.

## Available Modules

| # | Module | How to recognize it | Purpose | Project priority |
| --- | --- | --- | --- | --- |
| 1 | TTP223 touch sensor module | Blue board, large finger/spiral symbol, pins `VCC`, `GND`, `SIG` | Capacitive touch input, button replacement | Wired on ESP #2 as manual acknowledgement input |
| 2 | SW-520D tilt sensor module | Small black board, blue potentiometer, cylindrical tilt sensor | Detects tilt/movement as digital signal | Ignore for now |
| 3 | KY-025 reed switch / magnetic sensor | Red board, visible glass reed tube, often one screw hole | Door/window open/closed with magnet | Must-have, already used |
| 4 | KY-037 / KY-038 sound sensor | Red board, silver round microphone, blue potentiometer | Detects loud sound; digital threshold and analog output | Must-have later / TinyML candidate |
| 5 | HC-SR501 PIR motion sensor | White dome/bubble Fresnel lens | Detects motion using infrared | Must-have, already used |
| 6 | 1-channel relay module `SRD-05VDC-SL-C` | Blue relay cube, screw terminals | Switches external loads; useful as actuator | Must-have actuator option |
| 7-10 | ESP8266 D1 Mini boards | Small blue boards, metal Wi-Fi module, Micro-USB | Wi-Fi microcontrollers for Tasmota/MQTT/edge logic | Core hardware |
| 11 | Buzzer module | Black round buzzer, often marked `+`, usually 3 pins | Alarm sound output; in this setup it works as PWM-controlled output | Must-have actuator option |
| 12 | DS18B20 temperature sensor, TO-92 | Small black transistor-like part, 3 legs | Digital temperature sensor using 1-Wire | Nice context sensor, already used |
| 13 | HW-478 RGB / color light sensor | Black board, label like `3 Color`, small white sensor area | Detects light/color levels | Ignore for now |
| 14 | SW-420 vibration sensor module | Blue board, blue potentiometer, cylindrical vibration sensor | Detects shock/knock/vibration | Nice extension |

## Hardware Categories

| Category | Modules |
| --- | --- |
| Controller | ESP8266 D1 Mini |
| Motion | HC-SR501 PIR |
| Contact | KY-025 reed switch |
| Sound | KY-037 / KY-038 microphone |
| Environment | DS18B20 temperature |
| Disturbance | SW-420 vibration, SW-520D tilt |
| Light/color | HW-478 RGB/color sensor |
| Output | Buzzer, relay |
| Manual input | TTP223 touch sensor |

## Project-Relevant Selection

### Must-Have For Core Project

- ESP8266 D1 Mini
- PIR motion sensor
- reed switch / magnetic sensor
- relay or buzzer as alarm actuator

### Useful Next Extensions

- microphone for loud sound or TinyML sound classification, now wired on ESP #3
- vibration sensor for shock/knock detection, now wired on ESP #3
- touch sensor for acknowledge/silence/arm/disarm input, now wired on ESP #2
- DS18B20 temperature as context data

### Ignore For Now

- tilt sensor
- RGB/color sensor

These modules can be mentioned as available hardware, but they should not distract from the mid-term goal.

## Current Node Assignment

| Node | Role | Hardware |
| --- | --- | --- |
| ESP #1 | Monitoring node | PIR, reed switch |
| ESP #2 | Alarm and acknowledgement node | Relay actuator, PWM-controlled buzzer, and touch acknowledgement |
| ESP #3 | Safety context node | Vibration sensor, microphone, and DS18B20 temperature |
| ESP #4 | Optional node | Currently broken / not required |

## Mid-Term Hardware Priority

For the mid-term check, the most important hardware goal is:

```text
ESP #1 = monitoring node with sensors
ESP #2 = alarm node with actuator
```

The fourth D1 Mini is broken and is not needed for the mid-term or the stable core project.
