# Regular vs TinyML Model Comparison

This document defines the Wahlfachprojekt comparison. The comparison should be small, reproducible, and directly connected to the safety-monitoring use case.

## Research Question

How can a strong regular audio model help train a compact quantized TinyML model that can run on the ESP32 audio node?

## Task

Classify short audio windows into selected real sound-event classes that fit the safety-monitoring story.

The notebook uses ESC-50 automatically and can add UrbanSound8K and FSD50K when those audio folders are present. This keeps the first run reproducible while supporting the more useful final label split with gunshot, thud, screaming, alarm, and mechanical false-positive classes.

### Target Labels (v6+, 12 classes)

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

`household_noise` was dissolved into `unknown_background` in v6 because household sounds are acoustically inconsistent as a single class and the category had weak performance. `unknown_background` is an explicit escape valve — events the model does not recognise are labeled there rather than forced into a wrong safety-relevant class. Risk scores are assigned per-category in firmware/openHAB, not in the model, so keeping `impact_or_thud` clean (no household slams mixed in) is essential.

Useful verified source labels (v7):

| Target label | Dataset labels to use |
| --- | --- |
| `glass_breaking` | ESC-50 `glass_breaking`; FSD50K `Glass`, `Shatter`, `Chink_and_clink` |
| `gunshot` | UrbanSound8K `gun_shot`; FSD50K `Gunshot_and_gunfire` |
| `explosion_or_fireworks` | ESC-50 `fireworks`; FSD50K `Explosion`, `Fireworks`, `Boom` |
| `impact_or_thud` | FSD50K `Thump_and_thud`, `Crack`, `Hammer` |
| `scream_or_shout` | FSD50K `Screaming`, `Shout`, `Yell`, `Screech` |
| `siren_or_alarm` | ESC-50 `siren`, `clock_alarm`; UrbanSound8K `siren`; FSD50K `Siren`, `Alarm`, `Doorbell`, `Ringtone` |
| `footsteps` | ESC-50 `footsteps`; FSD50K `Walk_and_footsteps`, `Run` |
| `crying_or_sobbing` | ESC-50 `crying_baby`; FSD50K `Crying_and_sobbing` |
| `pet_noise` | ESC-50 `dog`, `cat`; UrbanSound8K `dog_bark`; FSD50K `Dog`, `Bark`, `Cat`, `Meow`, `Purr`, `Growling`, `Domestic_animals_and_pets` |
| `weather_noise` | ESC-50 `rain`, `thunderstorm`, `water_drops`, `wind`, `sea_waves`; FSD50K `Rain`, `Raindrop`, `Thunder`, `Thunderstorm`, `Wind`, `Stream`, `Ocean`, `Waves_and_surf` |
| `mechanical_noise` | ESC-50 `chainsaw`, `engine`, `helicopter`, `train`, `airplane`; UrbanSound8K `drilling`, `jackhammer`, `engine_idling`; FSD50K `Drill`, `Power_tool`, `Sawing`, `Tools`, `Mechanical_fan`, `Engine`, `Motor_vehicle_(road)`, `Aircraft`, `Motorcycle`, `Bus`, `Truck`, `Train`, `Rail_transport`, `Idling`, `Vehicle` |
| `unknown_background` | ESC-50 `vacuum_cleaner`, `washing_machine`, `clock_tick`, `laughing`, `clapping`; FSD50K `Domestic_sounds_and_home_sounds`, `Cupboard_open_or_close`, `Drawer_open_or_close`, `Microwave_oven`, `Computer_keyboard`, `Typing`, `Speech`, `Music`, `Laughter`, `Chatter`, `Crowd`, `Clapping`, `Applause`, `Singing`, `Slam`, `Knock`, `Telephone`, `Water`, `Hiss`, `Giggle`, `Cheering`; UrbanSound8K `air_conditioner`, `car_horn`, `street_music` |

The model predicts the acoustic event class. The ESP32 firmware or openHAB rules later map that class to a risk category. This avoids training the model on categories that are semantically useful but acoustically inconsistent.

Example later mapping:

```text
glass_breaking          -> critical_impulse  -> high risk
gunshot                 -> critical_impulse  -> critical risk
explosion_or_fireworks  -> explosive_context -> medium/high risk depending on context
impact_or_thud          -> impact            -> medium/high risk depending on confidence
scream_or_shout         -> distress          -> high risk
siren_or_alarm          -> alarm_context     -> medium/high risk depending on confidence
weather_noise           -> weather_context   -> low risk modifier
pet_noise               -> ignore_or_low     -> no direct alarm
unknown_background      -> ignore            -> no direct alarm
```

The class-to-risk mapping is not part of model training. It belongs in firmware/openHAB logic so that risk policy can change without retraining the model.

## Notebook

The prepared notebook is at:

```text
tinyml/notebooks/sound_classification_v7.ipynb
```

It uses a laptop/Colab-side audio dataset. This means the comparison is not blocked by soldering the INMP441 microphone. A laptop microphone can be added later for calibration or extra experiments, but the main comparison uses a fixed dataset so results are reproducible.

Development setup:

```text
VS Code + local .venv kernel for editing/light runs
Google Colab runtime for heavier TensorFlow training
```

The local kernel is documented in `tinyml/README.md` as `Safety TinyML (.venv)`. The Colab route uses the same notebook, so results and generated artifacts remain comparable.

For Colab dataset setup:

```text
UrbanSound8K -> downloaded automatically by the notebook setup cell
FSD50K       -> upload/extract audio to Google Drive at My Drive/tinyml_datasets/FSD50K/
```

The notebook mounts Google Drive and links that folder to `/content/data/raw/FSD50K` when running in Colab. The FSD50K metadata zip is small and downloaded automatically; only the large audio files need to be provided.

## Comparison Table

Results from v7 (best run to date).

| Metric | Scratch TensorFlow | YAMNet Teacher | Distilled TinyML Student |
| --- | --- | --- | --- |
| Target device | Laptop / Colab | Laptop / Colab training reference | ESP32 candidate |
| Numeric format | float32 | float32 embeddings/classifier | int8 after quantization |
| Model size | ~2.4 MB (int8: 216 KB) | ~13 MB | **41 KB** |
| Test accuracy | 59.4% (float32) / 59.2% (int8) | 74.5% | **58.6%** (int8) |
| Confusion matrix | Saved as PNG | Saved as PNG | Saved as PNG |
| Inference time | ~4.0 ms/window (laptop int8) | N/A | ~0.3 ms/window (ESP32 est.) |
| Memory constraint | Low relevance | Too large for ESP32 WROOM | Fits in ESP32 flash |
| Data sent to openHAB | optional raw/central | not deployed | label + confidence only |
| Debug data access | notebook arrays/plots | notebook arrays/plots | serial monitor or debug MQTT |

Chance accuracy with 12 classes is 8.3%. The YAMNet teacher result is not the ESP32 deployment target — it is the reference model whose soft probability output guides student training. The deployable result is the distilled compact student converted to int8 TFLite.

## Version Training History

Each version is a separate notebook. Key changes and their measured impact on distilled student int8 test accuracy:

| Version | Labels | Key Change | Regular | Teacher | Student int8 | Student size | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| v2 | 13 | Initial student: 32/64/96 SeparableConv, T=5.0, alpha=0.35 | 47.9% | 63.9% | 43.5% | 41 KB | Baseline |
| v3 | 13 | SpecAugment on regular model; 70/15/15 random split | 60.2% | 76.6% | 40.0% | 41 KB | SpecAugment improved regular model but broke distillation: masked student input vs full-spectrogram teacher soft labels caused loss 16.5 (vs 6.3 in v2) |
| v4 | 13 | FSD50K full 51K-file extraction; Gaussian noise replaces SpecAugment for student augmentation | 57.6% | 74.2% | 13.4% | 41 KB | Class weights from imbalanced regular dataset applied to balanced distillation split — scream weight 14.7×, student collapsed to predicting scream for 430/1455 test examples |
| v5 | 13 | Uniform weights in distillation; T=3.0 (down from 5.0) | 60.2% | 74.2% | **57.3%** | 41 KB | Fixed weight collapse; best student result to date |
| v6 | 12 | `household_noise` dissolved; two-stage training (Stage 1: 30 ep hard labels, Stage 2: distillation); label smoothing | 60.2% | 75.1% | ~40% | 41 KB | Regression: Stage 1 created hard-label local minimum; Stage 2 LR cut to 7.5e-5 by epoch 12, model stuck at 39–40% for 100 epochs |
| v7 | 12 | Reverted to v5 single-stage; corrected FSD50K label names from official website; added ~20 new FSD50K mappings (Music 14 K, Vehicle, etc.) | 59.4% | 74.5% | **58.6%** | 41 KB | New best student. Removed non-existent FSD50K labels (Chainsaw, Baby_cry, Smoke_detector); added high-volume background sources. 6 labels unmatched (Motorcycle, Bus, Train, Truck, Doorbell, Waves_and_surf) — audio still captured via parent categories (Vehicle, Rail_transport) |

**Key lessons learned:**

- SpecAugment breaks knowledge distillation: the teacher sees a full spectrogram but the student sees a masked one, so the soft labels do not correspond to the student's input.
- Class weights must match the distribution of the dataset they are applied to. The distillation split is intentionally balanced; importing weights computed on the imbalanced regular split amplified minority classes by up to 14.7× and caused class collapse.
- Two-stage training (hard labels then distillation) can trap the student in a local minimum that low-LR distillation cannot escape.
- FSD50K compound label display names (`Crying, sobbing`) map to `_and_` in the ground-truth CSV (`Crying_and_sobbing`), not to comma-separated tokens. Labels not in the official 200-class list (Chainsaw, Baby_cry, Smoke_detector) simply do not exist in FSD50K and should be removed from the mapping.

## Expected Interpretation

Commonalities:

- same classification task
- same input concept (log-mel spectrogram)
- same output labels
- same model-training pipeline before conversion

Differences:

- TinyML student model must fit into microcontroller memory
- quantization reduces size and may reduce accuracy
- inference runs locally on the ESP32
- MQTT publishes only summarised results
- local inference improves privacy and reduces bandwidth
- raw microphone samples can still be inspected during development through serial output or temporary debug topics
- teacher-assisted training can improve the small model without deploying the teacher

Important interpretation:

```text
The regular/YAMNet models do not need to fit on the microcontroller.
They are training references used to improve and evaluate the compact student.
The deployable edge version is the distilled int8 TFLite/TFLite Micro model.
```

## Artifacts To Produce

```text
tinyml/models/regular_model.keras
tinyml/models/regular_model_float32.tflite
tinyml/models/tiny_model_int8.tflite
tinyml/models/distilled_student_model.keras
tinyml/models/distilled_student_float32.tflite
tinyml/models/distilled_student_int8.tflite
tinyml/exported/distilled_student_int8.cc
tinyml/exported/figures/*.png
```

The `.keras`, `.tflite`, and exported `.cc` files are generated by the notebook.

## Debug Evidence

For the project report, capture evidence that the ESP32 audio node can be inspected without making raw audio part of the normal openHAB data path.

Possible evidence:

- notebook plot of one audio window or spectrogram
- serial monitor screenshot with a short raw sample preview
- MQTT Explorer screenshot of `tele/safety_audio_1/CLASSIFICATION`
- optional MQTT Explorer screenshot of `tele/safety_audio_1/FEATURES`

The explanation should be:

```text
Raw or feature-level data is available during development for calibration and verification.
The deployed MQTT/openHAB integration uses summarised inference data only.
```
