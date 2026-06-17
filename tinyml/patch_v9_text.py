"""
patch_v9_text.py  —  apply text/markdown fixes to v9 notebook without touching outputs

Run this AFTER downloading the finished Colab v9 notebook to restore text changes
that were made locally after the notebook was uploaded to Colab.

Only markdown cells are patched. Code cells and all outputs are left untouched.

Usage:
    python tinyml/patch_v9_text.py
"""

import json, pathlib, sys

NB_PATH = pathlib.Path(__file__).parent / "notebooks" / "sound_classification_v9.ipynb"

with open(NB_PATH, encoding="utf-8") as f:
    nb = json.load(f)

def patch_markdown(cell_id, old, new):
    for cell in nb["cells"]:
        if cell.get("id") != cell_id:
            continue
        if cell.get("cell_type") != "markdown":
            print(f"  SKIP {cell_id}: not a markdown cell")
            return
        src = "".join(cell["source"])
        if old not in src:
            print(f"  SKIP {cell_id}: string not found (may already be applied)")
            return
        cell["source"] = [src.replace(old, new, 1)]
        print(f"  Patched {cell_id}")
        return
    print(f"  WARNING: cell {cell_id} not found")


# ── Cell 4f3b3d40: intro ──────────────────────────────────────────────────────

patch_markdown("4f3b3d40",
    "This notebook is both an experiment and a written explanation for the Wahlfachprojekt comparison. It compares a normal TensorFlow training flow, a quantized TensorFlow Lite/TinyML-style model, a YAMNet transfer-learning teacher, and a distilled TinyML student model for a safety-relevant audio task.",
    "This notebook is both an experiment and a written explanation for a three-way comparison on a safety-relevant audio task: a regular TensorFlow model trained from scratch, a YAMNet transfer-learning teacher, and a distilled TinyML student model. The regular model is also quantized to int8 as a baseline reference to show the cost of quantization without distillation."
)

patch_markdown("4f3b3d40",
    "The comparison does **not** require the physical microphone yet. It uses a reproducible audio dataset on the laptop or in Colab. Later, the same output idea can be connected to openHAB through MQTT with `label`, `confidence`, `risk_category`, `rms`, and `inference_ms` instead of raw audio.",
    "The comparison uses a reproducible audio dataset on the laptop or in Colab. The INMP441 microphone is soldered and the ESP32 is running, but the TinyML model is not yet deployed on the device. Once deployed, the ESP32 publishes `label`, `confidence`, `rms`, and `inference_ms` to openHAB via MQTT instead of raw audio."
)


# ── Cell c71f5191: comparison idea ───────────────────────────────────────────

patch_markdown("c71f5191",
    "This avoids training the model directly on hand-made categories that may be semantically useful but acoustically inconsistent. For example, `fireworks` and `glass_breaking` can both be risk-relevant, but they do not necessarily sound like one class.\n\nThe notebook compares four things:\n\n1. A compact CNN trained from scratch on log-mel features.\n2. The scratch CNN converted to quantized TFLite as a plain TinyML baseline.\n3. A YAMNet transfer-learning teacher to get stronger class probabilities from pretrained audio embeddings.\n4. A distilled compact CNN student that learns from both the true labels and the YAMNet teacher, then exports to int8 TFLite for ESP32-style deployment.",
    "This avoids training the model directly on risk categories, which are semantically useful for the safety system but acoustically inconsistent — a gunshot, a siren, and a scream might all be high risk, but they sound completely different and cannot be learned reliably as a single acoustic class. Training on what things actually sound like and mapping to risk separately keeps both tasks solvable.\n\nThe notebook compares three things:\n\n1. A compact CNN trained from scratch on log-mel features (also quantized to int8 as a baseline reference).\n2. A YAMNet transfer-learning teacher to get stronger class probabilities from pretrained audio embeddings.\n3. A distilled compact CNN student that learns from both the true labels and the YAMNet teacher, then exports to int8 TFLite for ESP32-style deployment."
)


# ── Cell 1bc4cac0: audio preprocessing ───────────────────────────────────────

patch_markdown("1bc4cac0",
    "The ESP32 would later receive raw samples from the INMP441 microphone.",
    "The ESP32 receives raw samples from the INMP441 microphone; the model is not yet deployed on the device."
)


# ── Cell 2a125cf5: dataset — Donate-a-cry clip count + crying cap count ───────

patch_markdown("2a125cf5",
    "- **Donate-a-cry**: auto-cloned from GitHub; ~1 000 infant cry recordings that fill the `crying_or_sobbing` class which FSD50K covers only thinly (~151 clips).",
    "- **Donate-a-cry**: auto-cloned from GitHub; ~450 infant cry recordings that supplement the `crying_or_sobbing` class which FSD50K covers only thinly (~151 clips)."
)

patch_markdown("2a125cf5",
    "crying (~800 with Donate-a-cry)",
    "crying (~632 with Donate-a-cry)"
)


# ── Cell dd41a6da: Colab dataset setup — add ESC-50 ──────────────────────────

patch_markdown("dd41a6da",
    "- **UrbanSound8K** is downloaded automatically (~5 min).",
    "- **ESC-50** is downloaded automatically (~1 min).\n- **UrbanSound8K** is downloaded automatically (~5 min)."
)


# ── Cell 1e4d508c: if accuracy is low — rewrite for multi-dataset ─────────────

patch_markdown("1e4d508c",
    "### If The Regular Accuracy Is Low\n\nLow accuracy is a useful result, but it changes the conclusion. It means the current baseline setup is too simple for the task, not that TinyML is impossible. Likely reasons:\n\n- ESC-50 has only 2000 examples total. With 5 folds, the test fold is intentionally separate.\n- The selected ESC-50 classes are broad. One label can contain very different sound events.\n- This notebook uses short cropped windows rather than full 5-second clips. The important sound may not always be in the center crop.\n- The model is intentionally small and trained quickly so the comparison stays manageable.\n\nIf needed, improve the baseline before final reporting: use longer/random crops, log-mel features, more epochs, a slightly larger CNN, or pretrained audio embeddings. For the course comparison, the key is to compare regular vs quantized TinyML under the **same** preprocessing/task setup.",
    "### If The Regular Accuracy Is Low\n\nLow accuracy is a useful result, but it changes the conclusion. It means the current baseline setup is too weak for the task, not that TinyML is impossible. Likely reasons with the multi-dataset setup:\n\n- Some target labels are acoustically broad. `mechanical_noise` covers motorcycles, drills, aircraft, and trains — the model has to generalise across very different sounds in one class.\n- Some classes have fewer than the 1 200-clip cap (gunshot ~850, footsteps ~850, crying ~632). The cap limits high-volume classes but cannot add clips that do not exist.\n- The two-second center crop may miss the relevant sound event in longer clips from FSD50K or UrbanSound8K.\n- The regular model is intentionally compact (no BatchNorm, no pretrained weights) to make the comparison meaningful. A weaker baseline makes the teacher and distilled student gains more visible, not less.\n\nIf the model collapses to predicting one class, check the confusion matrix distribution first. Class collapse usually means a training bug (wrong class weights or a badly imbalanced split), not a model-capacity issue. For the comparison, the key is that the regular model and the distilled student use the **same** preprocessing and label set."
)


# ── Cell 2d71c5a6: openHAB connection — hardware is working ──────────────────

patch_markdown("2d71c5a6",
    "In the existing openHAB project, the ESP8266/Tasmota nodes publish summarized sensor states such as motion, vibration, temperature, and touch acknowledgement. The future ESP32 audio node should follow the same idea: it should not stream raw audio into openHAB.\n\nThe deployed audio node should publish summarized inference data:",
    "In the existing openHAB project, the ESP8266/Tasmota nodes publish summarized sensor states such as motion, vibration, temperature, and touch acknowledgement. The ESP32 audio node follows the same idea: it does not stream raw audio into openHAB.\n\nOnce the TinyML model is deployed, the audio node will publish summarized inference data:"
)


# ── Save ──────────────────────────────────────────────────────────────────────

with open(NB_PATH, "w", encoding="utf-8") as f:
    json.dump(nb, f, indent=1, ensure_ascii=False)

print(f"\nSaved {NB_PATH}")
print("Outputs preserved. Done.")
