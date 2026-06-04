# TinyML Extension

The TinyML extension adds an ESP32 audio node to the existing openHAB/MQTT safety-monitoring system.

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
```

## Scope

In scope:

- small sound classification task
- local inference on ESP32
- MQTT summary payload
- local or temporary debug access to raw samples and feature previews
- integration into openHAB risk score
- comparison with regular TensorFlow model

Out of scope:

- continuous raw audio streaming
- cloud audio processing
- permanent raw audio storage as part of the deployed system
- production-grade alarm certification
- large multi-class acoustic scene recognition

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
