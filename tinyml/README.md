# TinyML Workspace

This folder contains the planned TinyML work for the ESP32 audio-node extension.

## Structure

```text
tinyml/
├─ notebooks/
│  └─ sound_classification_comparison.ipynb
├─ data/
│  ├─ raw/
│  └─ processed/
├─ models/
│  ├─ regular_model.h5
│  ├─ regular_model.tflite
│  └─ tiny_model_int8.tflite
├─ exported/
│  └─ tiny_model_int8.cc
└─ esp32_audio_node/
   ├─ platformio.ini
   └─ src/
      └─ main.cpp
```

The notebook trains and compares the models. The ESP32 firmware later uses the exported TinyML model and publishes MQTT classification summaries to openHAB.
