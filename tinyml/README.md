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
|   |-- tiny_model_int8.tflite
|   |-- distilled_student_model.keras
|   |-- distilled_student_float32.tflite
|   `-- distilled_student_int8.tflite
|-- exported/
|   |-- distilled_student_int8.cc
|   `-- figures/
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

Run it on the laptop or in Colab first. It downloads an audio dataset, trains a regular TensorFlow model, trains a YAMNet transfer-learning teacher, trains a distilled compact student model, converts the student to TensorFlow Lite, quantizes it to int8, and compares size/accuracy/inference time.

The real ESP32 + INMP441 microphone is not required for this notebook. The regular and YAMNet models are laptop/Colab training references and do not need to fit on the microcontroller. The ESP32 candidate is the distilled student model exported as quantized int8 TFLite.

The notebook builds one combined training manifest. ESC-50 is downloaded automatically. UrbanSound8K and FSD50K are used automatically when their audio folders are present under `tinyml/data/raw/` or Colab `/content/data/raw/`.

The notebook uses `soundfile` for loading WAV files instead of only TensorFlow's built-in WAV decoder. This is intentional: some UrbanSound8K/FSD50K files use WAV header variants that can fail with `tf.audio.decode_wav`, while `soundfile` handles them more reliably.

The current target labels are:

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
household_noise
unknown_background
```

The reason for this split is that the audio node should focus on sounds the existing IoT sensors cannot detect well. For example, door opening is already covered by the reed switch, while gunshot, thud, screaming, glass shatter, alarm sounds, weather noise, household noise, and mechanical false positives are better handled by audio classification.

The model should learn the most accurate acoustic label it can. Later, the ESP32 firmware or openHAB rules can map the predicted class to a risk category. This keeps machine learning separate from safety policy:

```text
ML model -> glass_breaking
ESP32/openHAB mapping -> impact / high risk contribution
```

The notebook also includes a YAMNet transfer-learning teacher. This uses pretrained audio embeddings from TensorFlow Hub to produce stronger class probabilities. A compact student CNN then learns from both the true ESC-50 labels and the YAMNet teacher probabilities. Only the student is exported for ESP32-style deployment.

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

The notebook has a dedicated **Colab Dataset Setup** section.

Recommended order in Colab:

1. Run the dependency/setup cell.
2. Run the **Colab Dataset Setup** cell.
3. Run the normal dataset-loading and training cells.

UrbanSound8K:

```text
The notebook downloads and extracts UrbanSound8K automatically in Colab.
It becomes available at /content/data/raw/UrbanSound8K/
```

FSD50K:

FSD50K audio is larger, so upload or extract it in Google Drive once.

Create this folder in Google Drive:

```text
My Drive/tinyml_datasets/FSD50K/
```

Put the extracted FSD50K `.wav` files somewhere inside that folder. The exact subfolder names do not matter because the notebook searches recursively. These are both fine:

```text
My Drive/tinyml_datasets/FSD50K/12345.wav
My Drive/tinyml_datasets/FSD50K/FSD50K.dev_audio/12345.wav
My Drive/tinyml_datasets/FSD50K/FSD50K.eval_audio/67890.wav
```

In Colab, the setup cell mounts Drive and links:

```text
/content/drive/MyDrive/tinyml_datasets/FSD50K
-> /content/data/raw/FSD50K
```

Then the normal dataset loader can find FSD50K audio automatically.

The generated model files can be downloaded from Colab and copied back into `tinyml/models/` and `tinyml/exported/`. Audio files downloaded in Colab stay in the remote Colab filesystem; they will not appear automatically in the local Windows project folder.

If you connect VS Code to a Colab runtime, treat the Colab runtime as the execution kernel and the local `.venv` as the local fallback/editing kernel. The notebook comparison itself is the same in both cases.

Expected generated artifacts:

```text
models/regular_model.keras
models/regular_model_float32.tflite
models/tiny_model_int8.tflite
models/distilled_student_model.keras
models/distilled_student_float32.tflite
models/distilled_student_int8.tflite
exported/distilled_student_int8.cc
exported/figures/*.png
```

Later, after the INMP441 is soldered, the ESP32 audio node can publish only summarized results to:

```text
tele/safety_audio_1/CLASSIFICATION
```

with fields such as `label`, `confidence`, `rms`, and `inference_ms`.
