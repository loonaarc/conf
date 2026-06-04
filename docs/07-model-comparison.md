# Regular vs TinyML Model Comparison

This document defines the Wahlfachprojekt comparison. The comparison should be small, reproducible, and directly connected to the safety-monitoring use case.

## Research Question

How does a regular TensorFlow audio classifier differ from a quantized TinyML/TFLite version when both solve the same small sound-classification task?

## Task

Classify short audio windows into a small number of classes.

Candidate classes:

```text
silence
normal_noise
clap_or_knock
alarm_like
```

If using TensorFlow Speech Commands for a more reproducible first version, use fewer classes such as:

```text
yes
no
unknown
silence
```

## Notebook

Planned notebook:

```text
tinyml/notebooks/sound_classification_comparison.ipynb
```

The notebook should contain:

1. Dataset loading or dataset preparation.
2. Feature extraction, for example spectrograms or MFCCs.
3. Regular TensorFlow/Keras model training.
4. Evaluation with accuracy and confusion matrix.
5. TensorFlow Lite conversion.
6. Int8 quantization.
7. Model-size comparison.
8. Summary table for regular vs TinyML.

## Comparison Table

| Metric | Regular TensorFlow | TinyML / TFLite int8 |
| --- | --- | --- |
| Target device | Laptop / Colab | ESP32 |
| Numeric format | float32 | int8 |
| Model size | TBD | TBD |
| Accuracy | TBD | TBD |
| Confusion matrix | TBD | TBD |
| Inference time | TBD | TBD |
| Memory constraint | Low relevance | Important |
| Data sent to openHAB | optional raw/central data | label/confidence only |
| Debug data access | direct notebook arrays/plots | serial monitor or temporary debug MQTT |

## Expected Interpretation

Commonalities:

- same classification task
- same input concept
- same output labels
- same model-training pipeline before conversion

Differences:

- TinyML model must fit into microcontroller memory
- quantization reduces size and may reduce accuracy
- inference runs locally on the ESP32
- MQTT publishes only summarized results
- local inference improves privacy and reduces bandwidth
- raw microphone samples can still be inspected during development through serial output or temporary debug topics

## Artifacts To Produce

```text
tinyml/models/regular_model.h5
tinyml/models/regular_model.tflite
tinyml/models/tiny_model_int8.tflite
tinyml/exported/tiny_model_int8.cc
```

The `.h5`, `.tflite`, and exported `.cc` files can be generated later and do not need to exist before the notebook is implemented.

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
