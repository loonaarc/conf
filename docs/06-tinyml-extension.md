# TinyML Extension

The TinyML extension adds ESP32 audio nodes to the existing openHAB/MQTT safety-monitoring system.

## Goal

Use an ESP32 WROOM and INMP441 microphone to classify short sound windows locally. The node should publish only summarized inference results to MQTT, not raw audio.

```text
INMP441 microphone
  -> ESP32 audio capture
  -> TinyML model
  -> label/confidence
  -> MQTT
  -> openHAB risk score
```

## Planned Node

| Field | Value |
| --- | --- |
| Topic | `safety_audio_1` |
| Hardware | ESP32 WROOM + INMP441 |
| Firmware | Arduino/PlatformIO or ESP-IDF, not Tasmota |
| Model | Quantized TensorFlow Lite / TFLite Micro audio classifier |
| Output | Sound class, confidence, inference time |

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
silence
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

Topic:

```text
tele/safety_audio_1/CLASSIFICATION
```

Payload:

```json
{
  "label": "alarm_like",
  "confidence": 0.84,
  "rms": 0.62,
  "inference_ms": 38,
  "model": "sound_v1_int8"
}
```

## Debug And Raw Data Access

The deployed system should publish only summarized inference results to openHAB. During development, raw microphone samples or feature previews can still be inspected locally.

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
- comparison with regular TensorFlow model
- later voice-command node for arming/silencing/disarm-code experiments

Out of scope:

- continuous raw audio streaming
- cloud audio processing
- permanent raw audio storage as part of the deployed system
- production-grade alarm certification
- large multi-class acoustic scene recognition
- real speaker authentication or secure voice-password security

## Implementation Order

1. Solder and test ESP32 + INMP441.
2. Verify I2S audio capture with a simple serial monitor sketch.
3. Add serial debug output for sample windows, RMS/peak, and inference result.
4. Build the notebook model comparison in `tinyml/notebooks/`.
5. Export the quantized model to `tinyml/models/` and `tinyml/exported/`.
6. Add ESP32 firmware under `tinyml/esp32_audio_node/`.
7. Publish the normal MQTT classification payload.
8. Add optional debug MQTT payloads for feature/sample previews.
9. Add openHAB MQTT channels and Items.
10. Add TinyML sound evidence to the risk-score rule.
11. Later, use the second ESP32 as `safety_voice_1` for voice commands.
12. Keep physical touch as the unconditional local disarm override.
