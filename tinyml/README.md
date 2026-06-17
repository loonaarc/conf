# TinyML Workspace

This folder contains the TinyML work for the ESP32 audio-node extension of the openHAB safety system.

## Structure

```text
tinyml/
|-- notebooks/
|   |-- sound_classification_v8.ipynb   <- best student result so far (66.9% int8, 64 KB)
|   |-- sound_classification_v9.ipynb   <- v9 ran: Mixup regression to 64.0%, v8 student remains best
|   `-- sound_classification_v10.ipynb  <- current: YAMNet fine-tuning + student BatchNorm
|-- data/
|   |-- raw/                            <- ESC-50 auto-downloaded; US8K/FSD50K/DonateACry optional
|   `-- processed/
|-- models/                             <- local copies of exported artifacts
|   |-- regular_model.keras
|   |-- regular_model_float32.tflite
|   |-- tiny_model_int8.tflite
|   |-- distilled_student_model.keras
|   |-- distilled_student_float32.tflite
|   `-- distilled_student_int8.tflite
|-- exported/
|   |-- distilled_student_int8.cc       <- drop into ESP32 firmware as model_data.h
|   `-- figures/
`-- esp32_audio_node/
    |-- platformio.ini
    `-- src/
        `-- main.cpp
```

## What the notebook does

Each versioned notebook trains and compares four models on the same safety-relevant audio task:

1. **Regular TensorFlow model** — compact Conv2D CNN, laptop/Colab baseline
2. **YAMNet teacher** — 1024-dim pretrained embeddings + deep Dense head, Colab reference only
3. **Distilled TinyML student** — four-block SeparableConv2D, guided by teacher soft labels during training, exported as int8 TFLite for ESP32
4. **Quantized regular model** — int8 TFLite version of the baseline, for comparison

The regular and YAMNet models are training references. Only the distilled student is exported for ESP32 deployment.

## Target Labels (12 classes, v6+)

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

`household_noise` was dissolved into `unknown_background` in v6. `impact_or_thud` is kept clean of household sounds because risk scores are assigned per class in firmware. `unknown_background` is the explicit escape valve for sounds the model does not recognise.

The model predicts the acoustic class. Risk policy lives in the ESP32 firmware or openHAB rules, not in the model.

## Datasets

| Dataset | How it arrives | Approx clips used |
|---|---|---|
| ESC-50 | Auto-downloaded by notebook | ~1 750 |
| UrbanSound8K | Auto-downloaded in Colab | ~6 000 |
| FSD50K | Upload to Google Drive once | ~30 000+ |
| Donate-a-cry | Auto-cloned from GitHub (`git clone --depth 1`) | ~450 |

Per-class cap is 1 200 clips to avoid one source dominating. Classes below the cap use all available clips.

## Drive Checkpoint Strategy

Models are saved to `My Drive/tinyml_models/` with version-aware names:

```text
regular_model_best_v8.keras
yamnet_teacher_best_v8.keras
distilled_student_checkpoint_v9.keras    <- v9 student (Mixup, regressed to 64.0%)
regular_model_best_v10.keras
yamnet_teacher_best_v10.keras            <- v10: fine-tuned YAMNet teacher
distilled_student_checkpoint_v10.keras   <- v10 student (BatchNorm, no Mixup)
```

`FEATURE_CACHE_VERSION` controls the feature cache and teacher/regular checkpoint names. `STUDENT_VERSION` controls only the student checkpoint. Bumping only `STUDENT_VERSION` reuses cached features and the trained teacher, so only the student retrains. Bumping `FEATURE_CACHE_VERSION` (as in v10) forces the teacher to retrain and embeddings to be re-extracted — necessary when the teacher or audio preprocessing changes.

## VS Code Kernel Setup

Use a local virtual environment for editing and light runs. From the `tinyml` folder:

```powershell
cd C:\Users\lil\openhab-5.1.4\conf\tinyml
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
python -m ipykernel install --user --name safety-tinyml --display-name "Safety TinyML (.venv)"
```

Then open any versioned notebook in VS Code and select the kernel:

```text
Safety TinyML (.venv)
```

## Colab Use

For full training, use Google Colab. The notebook detects Colab automatically and installs its own dependencies.

Recommended order:

1. Run the dependency/setup cell.
2. Run the **Colab Dataset Setup** cell (mounts Drive, downloads UrbanSound8K and Donate-a-cry, links FSD50K).
3. Run the remaining cells top to bottom.

**FSD50K** audio must be uploaded once to Google Drive:

```text
My Drive/tinyml_datasets/FSD50K/
```

The notebook searches recursively, so subfolder names do not matter.

**Saved checkpoints** land in:

```text
My Drive/tinyml_models/
```

Download the generated artifacts after training and copy them to `tinyml/models/` and `tinyml/exported/` locally.

## Expected Artifacts

```text
models/regular_model.keras
models/regular_model_float32.tflite
models/tiny_model_int8.tflite
models/distilled_student_model.keras
models/distilled_student_float32.tflite
models/distilled_student_int8.tflite
exported/distilled_student_int8.cc      <- rename to model_data.h for ESP32 firmware
exported/figures/*.png
```

After the model is deployed on INMP441 microphone, the ESP32 audio node publishes summarised inference results to:

```text
tele/safety_audio_1/CLASSIFICATION
```

with fields `label`, `confidence`, `rms`, and `inference_ms`. Raw audio does not enter the openHAB data path.
