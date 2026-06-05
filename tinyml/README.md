# TinyML Workspace

This folder contains the planned TinyML work for the ESP32 audio-node extension.

## Structure

```text
tinyml/
|-- notebooks/
|   `-- sound_classification_comparison.ipynb
|-- data/
|   |-- raw/
|   `-- processed/
|-- models/
|   |-- regular_model.keras
|   |-- regular_model_float32.tflite
|   `-- tiny_model_int8.tflite
|-- exported/
|   `-- tiny_model_int8.cc
`-- esp32_audio_node/
    |-- platformio.ini
    `-- src/
        `-- main.cpp
```

The notebook trains and compares the models. The ESP32 firmware later uses the exported TinyML model and publishes MQTT classification summaries to openHAB.

## Sound Classification Comparison

The current notebook is:

```text
notebooks/sound_classification_comparison.ipynb
```

Run it on the laptop first. It downloads a small audio dataset, trains a regular TensorFlow model, converts it to TensorFlow Lite, quantizes it to int8, and compares size/accuracy/inference time.

The real ESP32 + INMP441 microphone is not required for this notebook. The regular model is a laptop baseline and does not need to fit on the microcontroller. The TinyML candidate is the quantized TFLite model.

## VS Code Kernel Setup

Use a local virtual environment for editing and light runs in VS Code. From the `tinyml` folder:

```powershell
cd C:\Users\lil\openhab-5.1.4\conf\tinyml
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
python -m ipykernel install --user --name safety-tinyml --display-name "Safety TinyML (.venv)"
```

Then open:

```text
notebooks/sound_classification_comparison.ipynb
```

In VS Code, choose the kernel:

```text
Safety TinyML (.venv)
```

This keeps the dependencies separate from your openHAB project and from your global Python installation.

## Colab Use

For heavier training, use Google Colab. The notebook is written so paths work both from the repo and from the `tinyml/notebooks` folder. In Colab, upload or open the same notebook and run the dependency cell/command:

The first code cell detects whether the notebook is running in Colab. If it is, it installs the notebook dependencies in the remote Colab runtime. If it is running locally, it does not install anything and expects the local `.venv` from the setup above.

The generated model files can be downloaded from Colab and copied back into `tinyml/models/` and `tinyml/exported/`.

If you connect VS Code to a Colab runtime, treat the Colab runtime as the execution kernel and the local `.venv` as the local fallback/editing kernel. The notebook comparison itself is the same in both cases.

Expected generated artifacts:

```text
models/regular_model.keras
models/regular_model_float32.tflite
models/tiny_model_int8.tflite
exported/tiny_model_int8.cc
```

Later, after the INMP441 is soldered, the ESP32 audio node can publish only summarized results to:

```text
tele/safety_audio_1/CLASSIFICATION
```

with fields such as `label`, `confidence`, `rms`, and `inference_ms`.
