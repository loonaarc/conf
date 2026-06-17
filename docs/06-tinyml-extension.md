# TinyML Extension

The TinyML extension adds an ESP32 audio node to the existing openHAB/MQTT safety-monitoring system.

## Goal

Use an ESP32 WROOM and INMP441 microphone to classify short sound windows locally. The node publishes only summarized inference results to MQTT, not raw audio.

```text
INMP441 microphone
  -> ESP32 audio capture
  -> TinyML model
  -> label/confidence
  -> MQTT
  -> openHAB risk score
```

## Audio Node

| Field | Value |
| --- | --- |
| Topic | `safety_audio_1` |
| Hardware | ESP32 WROOM + INMP441 |
| Firmware | Arduino/PlatformIO or ESP-IDF, not Tasmota |
| Model | Distilled quantized TensorFlow Lite / TFLite Micro audio classifier |
| Output | Sound class, confidence, inference time |

Current hardware status:

```text
ESP32 upload test:         verified
Arduino board profile:     ESP32 Dev Module
Local serial port (test):  COM3
INMP441 I2S capture:       verified — changing avg_abs/peak serial output confirmed
MQTT feature topic:        verified — ESP32 connects to broker and publishes audio features
Working pins:              SCK=D14/GPIO14, WS=D15/GPIO15, SD=D32/GPIO32, L/R=GND, VDD=3V3

Model deployment:          pending — TinyML model not yet flashed to device
```

## Later Voice Command Node

A second ESP32 can be used as a separate voice-command node. This should be treated as a later extension after the basic audio-classification comparison is working.

| Field | Value |
| --- | --- |
| Topic | `safety_voice_1` |
| Hardware | Second ESP32 WROOM + microphone |
| Model | Tiny speech-command classifier |
| Output | Command label, confidence, inference time |

Planned voice commands:

```text
arm
silence
disarm_code
unknown
```

The voice node is a convenience interface, not the highest-trust safety control. Touch on the physical alarm node remains the trusted local override.

Control hierarchy:

```text
physical touch disarm > voice command > automatic risk rule
```

Recommended behavior:

```text
Touch on alarm node
  -> relay OFF
  -> buzzer OFF
  -> AlarmAutomation OFF
  -> AlarmState IDLE

voice "arm" with high confidence
  -> AlarmAutomation ON

voice "silence" with high confidence
  -> relay OFF
  -> buzzer OFF

voice "disarm_code" with high confidence
  -> optional convenience disarm
  -> should be documented as weaker than physical touch because replay or false recognition is possible
```

Planned MQTT topic:

```text
tele/safety_voice_1/COMMAND
```

Example payload:

```json
{
  "command": "arm",
  "confidence": 0.91,
  "inference_ms": 22,
  "model": "voice_cmd_v1_int8"
}
```

## MQTT Payload

Target deployment labels (12 classes, v6+) trained on ESC-50, UrbanSound8K, FSD50K, and the Donate-a-cry corpus:

```text
glass_breaking
gunshot
explosion_or_fireworks
impact_or_thud
scream_or_shout
siren_or_alarm
footsteps
crying_or_sobbing
pet_noise
weather_noise
mechanical_noise
unknown_background
```

`impact_or_thud` is kept clean of household sounds — risk scores are assigned per class in firmware, so mixing a door slam with a structural thud would pollute the risk category. `unknown_background` is the explicit escape valve for sounds the model does not recognise.

Topic:

```text
tele/safety_audio_1/CLASSIFICATION
```

Payload:

```json
{
  "label": "glass_breaking",
  "confidence": 0.84,
  "rms": 0.62,
  "inference_ms": 38,
  "model": "distilled_student_int8"
}
```

The class-to-risk mapping is not part of the model output. It belongs in openHAB rules so risk policy can change without retraining the model.

## Debug And Raw Data Access

The deployed system publishes only summarized inference results to openHAB. During development, raw microphone samples or feature previews can still be inspected locally.

This keeps the operational system privacy-preserving while still making the audio pipeline testable.

Recommended output levels:

| Level | Channel | Purpose |
| --- | --- | --- |
| Normal operation | MQTT `CLASSIFICATION` topic | Send label, confidence, RMS, and inference time to openHAB |
| Debug MQTT | MQTT `FEATURES` or `AUDIO_DEBUG` topic | Inspect selected features or a short sample preview |
| Local development | USB serial monitor | Inspect raw sample windows, feature values, and inference results |
| Optional advanced debug | ESP32 local web page | Show Wi-Fi/MQTT status, latest class, confidence, and a small live feature chart |

Normal operation topic:

```text
tele/safety_audio_1/CLASSIFICATION
```

Optional debug topics:

```text
tele/safety_audio_1/FEATURES
tele/safety_audio_1/AUDIO_DEBUG
```

Example debug payload:

```json
{
  "rms": 0.42,
  "peak": 0.77,
  "zcr": 0.18,
  "samples_preview": [-120, -98, -105, -80, -60]
}
```

The debug topic should not stream continuous full raw audio. It should be used temporarily for development, calibration, and evidence screenshots. For longer raw-data inspection, use the serial monitor or short recorded test windows outside the normal MQTT/openHAB path.

## Planned openHAB Items

```text
String TinyMLSoundClass "TinyML Sound Class [%s]"
Number TinyMLSoundConfidence "TinyML Sound Confidence [%.2f]"
Number TinyMLSoundRms "TinyML Sound RMS [%.2f]"
Number TinyMLInferenceMs "TinyML Inference Time [%.0f ms]"
String VoiceCommand "Voice Command [%s]"
Number VoiceCommandConfidence "Voice Command Confidence [%.2f]"
Number VoiceInferenceMs "Voice Inference Time [%.0f ms]"
```

## Scope

In scope:

- small sound classification task
- local inference on ESP32
- MQTT summary payload
- local or temporary debug access to raw samples and feature previews
- integration into openHAB risk score
- comparison with regular TensorFlow, YAMNet teacher, and distilled TinyML student models
- later voice-command node for arming/silencing/disarm-code experiments

Out of scope:

- continuous raw audio streaming
- cloud audio processing
- permanent raw audio storage as part of the deployed system
- production-grade alarm certification
- large multi-class acoustic scene recognition
- real speaker authentication or secure voice-password security

## Implementation Order

1. Solder and test ESP32 + INMP441. **Done**
2. Verify I2S audio capture with a simple serial monitor sketch. **Done**
3. Add serial debug output for stable RMS/peak and sample window stats. **Done**
4. Verify MQTT connection and debug feature topic from ESP32. **Done**
5. Build the notebook model comparison in `tinyml/notebooks/`. **Done** (v8: 66.9% int8 @ 64 KB; v9 Mixup regression 64.0%; v10 in progress: YAMNet fine-tuning + BatchNorm)
6. Train the YAMNet teacher and distilled compact student. **Done** (v8: teacher 75.7%, student 66.9% int8)
7. Export the distilled int8 model to `tinyml/exported/distilled_student_int8.cc`.
8. Integrate model into ESP32 firmware under `tinyml/esp32_audio_node/` and flash to device.
9. Publish the normal MQTT classification payload on `tele/safety_audio_1/CLASSIFICATION`.
10. Add optional debug MQTT payloads for feature/sample previews.
11. Add openHAB MQTT channels and Items.
12. Add TinyML sound evidence to the risk-score rule.
13. Later, use the second ESP32 as `safety_voice_1` for voice commands.
14. Keep physical touch as the unconditional local disarm override.
