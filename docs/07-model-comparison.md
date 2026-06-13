# Regular vs TinyML Model Comparison

This document defines the Wahlfachprojekt comparison. The comparison should be small, reproducible, and directly connected to the safety-monitoring use case.

## Research Question

How can a strong regular audio model help train a compact quantized TinyML model that can run on the ESP32 audio node?

## Task

Classify short audio windows into selected real sound-event classes that fit the safety-monitoring story.

The notebook uses ESC-50 automatically and can add UrbanSound8K and FSD50K when those audio folders are present. This keeps the first run reproducible while supporting the more useful final label split with gunshot, thud, screaming, alarm, and mechanical false-positive classes.

### Target Multi-Dataset Deployment Split

The current notebook target labels are:

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

This version removes weak access-context labels such as door knock and door creak from the core deployment target. They are not useless, but they are less important than impact, distress, alarm, and false-positive classes. Door contact is already covered better by the reed switch in the IoT system, while audio should focus on events the existing sensors cannot see.

Useful verified source labels:

| Target label | Dataset labels to use |
| --- | --- |
| `glass_breaking` | ESC-50 `glass_breaking`; FSD50K `Glass`, `Shatter` |
| `gunshot` | UrbanSound8K `gun_shot`; FSD50K `Gunshot_and_gunfire` |
| `explosion_or_fireworks` | ESC-50 `fireworks`; FSD50K `Explosion`, `Fireworks` |
| `impact_or_thud` | FSD50K `Thump_and_thud`, `Boom`, `Slam`, `Crack`, `Hammer` |
| `scream_or_shout` | FSD50K `Screaming`, `Shout`, `Yell` |
| `siren_or_alarm` | ESC-50 `siren`, `clock_alarm`; UrbanSound8K `siren`; FSD50K `Siren`, `Alarm`, `Doorbell`, `Ringtone` |
| `footsteps` | ESC-50 `footsteps`; FSD50K `Walk_and_footsteps` |
| `crying_or_sobbing` | ESC-50 `crying_baby`; FSD50K `Crying_and_sobbing` |
| `pet_noise` | ESC-50 `dog`, `cat`; UrbanSound8K `dog_bark`; FSD50K `Dog`, `Bark`, `Cat`, `Meow`, `Purr` |
| `weather_noise` | ESC-50 `rain`, `thunderstorm`, `water_drops`, `wind`; FSD50K `Rain`, `Raindrop`, `Thunder`, `Thunderstorm`, `Wind` |
| `mechanical_noise` | UrbanSound8K `drilling`, `jackhammer`, `engine_idling`; FSD50K `Drill`, `Power_tool`, `Sawing`, `Tools`, `Mechanical_fan` |
| `household_noise` | ESC-50 `vacuum_cleaner`, `washing_machine`, `clock_tick`; FSD50K `Domestic_sounds_and_home_sounds`, `Dishes_and_pots_and_pans`, `Cupboard_open_or_close`, `Drawer_open_or_close`, `Microwave_oven`, `Computer_keyboard`, `Typing` |
| `unknown_background` | non-target ESC-50/FSD50K/UrbanSound8K classes and own normal-room recordings |

The model predicts the actual acoustic event class. The ESP32 firmware or openHAB rules later map that class to a risk category. This avoids training the model on categories that are semantically useful but acoustically inconsistent.

Example later mapping:

```text
glass_breaking -> impact -> higher risk
gunshot -> critical_impulse -> critical risk
explosion_or_fireworks -> explosive_context -> medium/high risk depending on context
impact_or_thud -> impact -> medium/high risk depending on confidence
scream_or_shout -> distress -> high risk
siren_or_alarm -> alarm_context -> medium/high risk depending on confidence
weather_noise -> weather_context -> low risk modifier
pet_noise -> ignore_or_low -> no direct alarm
unknown_background -> ignore -> no direct alarm
```

The class-to-risk mapping is not part of model training. It belongs in firmware/openHAB logic so that risk policy can change without retraining the model.

## Notebook

The prepared notebook is:

```text
tinyml/notebooks/sound_classification_comparison.ipynb
```

It uses a laptop/Colab-side audio dataset first. This means the comparison is not blocked by soldering the INMP441 microphone. A laptop microphone can be added later for calibration or extra experiments, but the main comparison should use a fixed dataset so the results are reproducible.

The intended development setup is:

```text
VS Code + local .venv kernel for editing/light runs
Google Colab runtime for heavier TensorFlow training if needed
```

The local kernel is documented in `tinyml/README.md` as `Safety TinyML (.venv)`. The Colab route uses the same notebook, so results and generated model artifacts remain comparable.

The first notebook cell detects the execution environment:

```text
Colab kernel -> install packages in the remote Colab runtime
Local VS Code .venv kernel -> use the already installed local requirements
```

For Colab dataset setup:

```text
UrbanSound8K -> downloaded automatically by the notebook setup cell
FSD50K -> upload/extract audio to Google Drive at My Drive/tinyml_datasets/FSD50K/
```

The notebook mounts Google Drive and links that folder to `/content/data/raw/FSD50K` when running in Colab. The FSD50K metadata zip is small and downloaded automatically; only the large audio files need to be provided.

The notebook contains:

1. Dataset loading or dataset preparation.
2. Feature extraction with log-mel spectrograms.
3. Regular TensorFlow/Keras model training as the laptop baseline.
4. Training curves for checking underfitting or overfitting.
5. Evaluation with accuracy and readable saved confusion matrices.
6. TensorFlow Lite conversion.
7. Int8 quantization as the TinyML-style model.
8. YAMNet transfer-learning teacher with pretrained audio embeddings.
9. Distilled compact student model trained from hard labels and YAMNet soft probabilities.
10. Int8 quantization of the distilled student as the ESP32 candidate.
11. Model-size comparison.
12. Summary table for scratch, teacher, student, and quantized student results.

The baseline should be reasonably meaningful before the TinyML comparison is interpreted. With the current 13 model labels, chance accuracy is about 7.7%. If regular TensorFlow accuracy is only slightly above chance, or if the confusion matrix shows that nearly all samples are predicted as one class, the conclusion should be that the feature/model setup is weak, not that TinyML itself is unsuitable. The current notebook therefore uses log-mel features, prints true/predicted label distributions, saves readable confusion-matrix PNGs, includes a YAMNet teacher, and adds a distilled student model as the ESP32 candidate.

## Comparison Table

| Metric | Scratch TensorFlow | YAMNet Teacher | Distilled TinyML Student |
| --- | --- | --- | --- |
| Target device | Laptop / Colab | Laptop / Colab training reference | ESP32 candidate |
| Numeric format | float32 | float32 embeddings/classifier | int8 after quantization |
| Model size | TBD | TBD | TBD |
| Accuracy | TBD | TBD | TBD |
| Confusion matrix | TBD | TBD | TBD |
| Inference time | TBD | TBD | TBD |
| Memory constraint | Low relevance | Too large/heavy for direct ESP32 WROOM deployment | Important |
| Data sent to openHAB | optional raw/central data | not deployed | label/confidence only |
| Debug data access | direct notebook arrays/plots | direct notebook arrays/plots | serial monitor or temporary debug MQTT |

The YAMNet result is not the ESP32 deployment target. It is the teacher/reference model. The deployable result is the distilled compact student converted to int8 TFLite.

## Expected Interpretation

Commonalities:

- same classification task
- same input concept
- same output labels
- same model-training pipeline before conversion

Differences:

- TinyML student model must fit into microcontroller memory
- quantization reduces size and may reduce accuracy
- inference runs locally on the ESP32
- MQTT publishes only summarized results
- local inference improves privacy and reduces bandwidth
- raw microphone samples can still be inspected during development through serial output or temporary debug topics
- teacher-assisted training can improve the small model without deploying the teacher

Important interpretation:

```text
The regular/YAMNet models do not need to fit on the microcontroller.
They are training references used to improve and evaluate the compact student.
The deployable edge version is the distilled int8 TFLite/TFLite Micro style model.
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
The deployed MQTT/openHAB integration uses summarized inference data only.
```
