// ESP32 TFLite Micro sound classification with MQTT publishing.
//
// Captures 2 s of audio at 16 kHz, computes a 171x64 log-mel spectrogram,
// runs the distilled int8 model, and publishes the result via MQTT.
//
// NOTE – sample-rate mismatch:
//   The notebook trained on 44.1 kHz audio (mel range 80–16000 Hz).
//   The ESP32 captures at 16 kHz (Nyquist = 8 kHz).
//   STFT frame length/step are scaled to keep the same time resolution,
//   producing the same 171x64 shape. Frequency coverage is 80–8000 Hz.
//   This is a known project limitation noted in the report.

#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <EloquentTinyML.h>
#include <eloquent_tinyml/tensorflow.h>

#include "model_data.h"   // <-- replace placeholder with Colab output

// ── Wi-Fi / MQTT ───────────────────────────────────────────────────────────
#define WIFI_SSID     "realme C3"
#define WIFI_PASSWORD "easy098!"
#define MQTT_HOST     "192.168.43.26"
#define MQTT_PORT     1883
#define MQTT_TOPIC    "tele/safety_audio_1/CLASSIFICATION"
#define MQTT_DEBUG    "tele/safety_audio_1/FEATURES"
#define MQTT_CLIENT   "esp32_audio_1"

// ── I2S / INMP441 ──────────────────────────────────────────────────────────
#define I2S_WS   15
#define I2S_SD   32
#define I2S_SCK  14
#define I2S_PORT I2S_NUM_0

// ── Audio window ───────────────────────────────────────────────────────────
#define SAMPLE_RATE      16000
#define WINDOW_SECONDS   2
#define WINDOW_SAMPLES   (SAMPLE_RATE * WINDOW_SECONDS)  // 32000

// ── STFT parameters (time-scaled from notebook's 44100 Hz values) ──────────
// Notebook: frame=1024, step=512 @ 44100 Hz  →  23.2 ms frame, 11.6 ms hop
// ESP32:    frame=371,  step=186 @ 16000 Hz  →  same durations
// FFT size: nearest power-of-2 ≥ 371 = 512
#define FRAME_LEN   371
#define FRAME_STEP  186
#define FFT_SIZE    512
#define FFT_BINS    (FFT_SIZE / 2 + 1)   // 257

// ── Mel filter bank ────────────────────────────────────────────────────────
#define NUM_MEL   64
#define MEL_LO    80.0f
#define MEL_HI    8000.0f   // Nyquist for 16 kHz (notebook uses 16000 for 44.1 kHz)

// ── Model geometry ─────────────────────────────────────────────────────────
#define NUM_FRAMES  171
#define MODEL_IN    (NUM_FRAMES * NUM_MEL)   // 10944 floats
#define NUM_LABELS  11
#define ARENA_KB    80

// ── Labels (must match notebook LABELS order) ─────────────────────────────
// Fill from Colab cell 8 output: "Active model labels in this run:"
const char* LABELS[NUM_LABELS] = {
  "glass_breaking",
  "gunshot",
  "explosion_or_fireworks",
  "siren_or_alarm",
  "footsteps",
  "crying_or_sobbing",
  "pet_noise",
  "weather_noise",
  "mechanical_noise",
  "household_noise",
  "unknown_background"
};

// ── Quantization parameters ────────────────────────────────────────────────
// Fill from Colab after model export:
//   interpreter.get_input_details()[0]['quantization']
// Placeholders — update before final deployment:
#define INPUT_SCALE      0.1f
#define INPUT_ZERO_POINT 0

// ── Globals ───────────────────────────────────────────────────────────────
int32_t   audioBuf[WINDOW_SAMPLES];
float     frame[FFT_SIZE];
float     melSpec[NUM_FRAMES][NUM_MEL];
int8_t    modelInput[MODEL_IN];
float     modelOutput[NUM_LABELS];
float     melFilterBank[NUM_MEL][FFT_BINS];

WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

Eloquent::TinyML::TensorFlow::TensorFlow<MODEL_IN, NUM_LABELS, ARENA_KB * 1024> tflite;

// ── Mel helpers ────────────────────────────────────────────────────────────
static float hzToMel(float hz) { return 2595.0f * log10f(1.0f + hz / 700.0f); }
static float melToHz(float mel) { return 700.0f * (powf(10.0f, mel / 2595.0f) - 1.0f); }

void buildMelFilterBank() {
  float melLo = hzToMel(MEL_LO);
  float melHi = hzToMel(MEL_HI);
  float melPoints[NUM_MEL + 2];
  for (int i = 0; i < NUM_MEL + 2; i++)
    melPoints[i] = melLo + i * (melHi - melLo) / (NUM_MEL + 1);

  float binFreqs[FFT_BINS];
  for (int k = 0; k < FFT_BINS; k++)
    binFreqs[k] = (float)k * SAMPLE_RATE / FFT_SIZE;

  for (int m = 0; m < NUM_MEL; m++) {
    float lo  = melToHz(melPoints[m]);
    float ctr = melToHz(melPoints[m + 1]);
    float hi  = melToHz(melPoints[m + 2]);
    for (int k = 0; k < FFT_BINS; k++) {
      float f = binFreqs[k];
      if (f >= lo && f <= ctr)
        melFilterBank[m][k] = (f - lo) / (ctr - lo);
      else if (f > ctr && f <= hi)
        melFilterBank[m][k] = (hi - f) / (hi - ctr);
      else
        melFilterBank[m][k] = 0.0f;
    }
  }
}

// ── Hann window (computed once) ────────────────────────────────────────────
float hannWindow[FRAME_LEN];
void buildHannWindow() {
  for (int i = 0; i < FRAME_LEN; i++)
    hannWindow[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (FRAME_LEN - 1)));
}

// ── Bit-reversal permutation for in-place FFT ─────────────────────────────
static void fftBitReverse(float* re, float* im, int n) {
  int j = 0;
  for (int i = 1; i < n; i++) {
    int bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) { float t = re[i]; re[i] = re[j]; re[j] = t;
                  t = im[i]; im[i] = im[j]; im[j] = t; }
  }
}

// ── Cooley-Tukey radix-2 FFT ──────────────────────────────────────────────
float fftRe[FFT_SIZE], fftIm[FFT_SIZE];
void computeFFT(const float* x) {
  for (int i = 0; i < FFT_SIZE; i++) { fftRe[i] = (i < FRAME_LEN) ? x[i] : 0.0f; fftIm[i] = 0.0f; }
  fftBitReverse(fftRe, fftIm, FFT_SIZE);
  for (int len = 2; len <= FFT_SIZE; len <<= 1) {
    float ang = -2.0f * M_PI / len;
    float wRe = cosf(ang), wIm = sinf(ang);
    for (int i = 0; i < FFT_SIZE; i += len) {
      float curRe = 1.0f, curIm = 0.0f;
      for (int j = 0; j < len / 2; j++) {
        float uRe = fftRe[i+j], uIm = fftIm[i+j];
        float vRe = fftRe[i+j+len/2]*curRe - fftIm[i+j+len/2]*curIm;
        float vIm = fftRe[i+j+len/2]*curIm + fftIm[i+j+len/2]*curRe;
        fftRe[i+j] = uRe+vRe; fftIm[i+j] = uIm+vIm;
        fftRe[i+j+len/2] = uRe-vRe; fftIm[i+j+len/2] = uIm-vIm;
        float tmp = curRe*wRe - curIm*wIm;
        curIm = curRe*wIm + curIm*wRe; curRe = tmp;
      }
    }
  }
}

// ── Compute full mel spectrogram from audio buffer ─────────────────────────
void computeMelSpectrogram() {
  for (int f = 0; f < NUM_FRAMES; f++) {
    int start = f * FRAME_STEP;
    // Window the frame
    for (int i = 0; i < FRAME_LEN; i++) {
      float s = (start + i < WINDOW_SAMPLES)
                ? (audioBuf[start + i] >> 14) / 32768.0f
                : 0.0f;
      frame[i] = s * hannWindow[i];
    }
    computeFFT(frame);
    // Power spectrum → mel → log
    for (int m = 0; m < NUM_MEL; m++) {
      float energy = 0.0f;
      for (int k = 0; k < FFT_BINS; k++) {
        float power = fftRe[k]*fftRe[k] + fftIm[k]*fftIm[k];
        energy += power * melFilterBank[m][k];
      }
      melSpec[f][m] = logf(energy + 1e-6f);
    }
  }
}

// ── Quantize float mel spec to int8 model input ───────────────────────────
void quantizeMelToInt8() {
  for (int f = 0; f < NUM_FRAMES; f++)
    for (int m = 0; m < NUM_MEL; m++) {
      int q = (int)roundf(melSpec[f][m] / INPUT_SCALE) + INPUT_ZERO_POINT;
      q = max(-128, min(127, q));
      modelInput[f * NUM_MEL + m] = (int8_t)q;
    }
}

// ── Wi-Fi / MQTT ───────────────────────────────────────────────────────────
void setupI2S() {
  i2s_config_t cfg = {
    .mode                = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate         = SAMPLE_RATE,
    .bits_per_sample     = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format      = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format= I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags    = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count       = 4,
    .dma_buf_len         = 256,
    .use_apll            = false,
    .tx_desc_auto_clear  = false,
    .fixed_mclk          = 0
  };
  i2s_pin_config_t pins = {
    .bck_io_num   = I2S_SCK, .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE, .data_in_num = I2S_SD
  };
  i2s_driver_install(I2S_PORT, &cfg, 0, NULL);
  i2s_set_pin(I2S_PORT, &pins);
  i2s_zero_dma_buffer(I2S_PORT);
}

void connectWifi() {
  Serial.printf("Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.printf("\nIP: %s\n", WiFi.localIP().toString().c_str());
}

void connectMqtt() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  while (!mqtt.connected()) {
    Serial.print("Connecting MQTT...");
    if (mqtt.connect(MQTT_CLIENT)) Serial.println(" ok");
    else { Serial.printf(" failed (%d), retry 3s\n", mqtt.state()); delay(3000); }
  }
}

// ── Setup ──────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1000);

  buildHannWindow();
  buildMelFilterBank();
  setupI2S();
  connectWifi();
  connectMqtt();

  if (!tflite.begin(distilled_student_int8)) {
    Serial.println("TFLite init failed — check model_data.h");
    while (true) delay(1000);
  }
  Serial.println("TFLite model loaded");
  Serial.println("ESP32 TinyML audio node ready");
}

// ── Loop ───────────────────────────────────────────────────────────────────
void loop() {
  if (!mqtt.connected()) connectMqtt();
  mqtt.loop();

  // 1. Capture 2 s of audio
  size_t total = 0;
  while (total < WINDOW_SAMPLES) {
    size_t got = 0;
    i2s_read(I2S_PORT, audioBuf + total,
             (WINDOW_SAMPLES - total) * sizeof(int32_t), &got, portMAX_DELAY);
    total += got / sizeof(int32_t);
  }

  unsigned long t0 = millis();

  // 2. Features
  computeMelSpectrogram();
  quantizeMelToInt8();

  // 3. Inference
  tflite.predict(modelInput, modelOutput);

  unsigned long inferenceMs = millis() - t0;

  // 4. Decode best label
  int bestIdx = 0;
  for (int i = 1; i < NUM_LABELS; i++)
    if (modelOutput[i] > modelOutput[bestIdx]) bestIdx = i;

  // Softmax confidence
  float expSum = 0.0f;
  for (int i = 0; i < NUM_LABELS; i++) expSum += expf(modelOutput[i]);
  float confidence = expf(modelOutput[bestIdx]) / expSum;

  // RMS for debug
  int64_t sumSq = 0;
  for (int i = 0; i < WINDOW_SAMPLES; i++) {
    int32_t s = audioBuf[i] >> 14;
    sumSq += (int64_t)s * s;
  }
  float rms = sqrtf((float)sumSq / WINDOW_SAMPLES);

  // 5. Publish classification
  char payload[200];
  snprintf(payload, sizeof(payload),
    "{\"label\":\"%s\",\"confidence\":%.2f,\"rms\":%.1f,\"inference_ms\":%lu,\"model\":\"distilled_student_int8\"}",
    LABELS[bestIdx], confidence, rms, inferenceMs);
  mqtt.publish(MQTT_TOPIC, payload);

  Serial.printf("label=%s conf=%.2f rms=%.1f inf=%lums\n",
                LABELS[bestIdx], confidence, rms, inferenceMs);
}
