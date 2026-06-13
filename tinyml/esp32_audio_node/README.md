# ESP32 Audio Node

This folder contains the ESP32 + INMP441 work for the TinyML audio extension.

## Arduino Sketches

Use the `arduino/` folder for early hardware tests in Arduino IDE.

```text
arduino/
|-- 02_inmp441_i2s_debug/
|   `-- 02_inmp441_i2s_debug.ino
`-- 03_audio_features_serial/
    `-- 03_audio_features_serial.ino
```

Arduino IDE expects the sketch folder and `.ino` file to have the same name.

## Verified Hardware

```text
Board profile -> ESP32 Dev Module
Port during local test -> COM3
Microphone -> INMP441
Upload test -> verified in Arduino IDE, sketch not kept
I2S debug sketch -> arduino/02_inmp441_i2s_debug/
Audio feature sketch -> arduino/03_audio_features_serial/
```

Working wiring:

```text
INMP441 VDD -> ESP32 3V3
INMP441 GND -> ESP32 GND
INMP441 L/R -> ESP32 GND
INMP441 SCK -> ESP32 D14 / GPIO14
INMP441 WS  -> ESP32 D15 / GPIO15
INMP441 SD  -> ESP32 D32 / GPIO32
```

## Planned Firmware Path

The Arduino sketches are hardware/debug milestones. The later deployable firmware can move into `platformio/` once the MQTT and TinyML model integration starts.
