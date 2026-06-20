// ESP32 TFLite Micro sound classification with MQTT publishing.
// Model: distilled student v8  –  66.9% int8 accuracy, 64 KB, 12 classes.
//
// Captures 2 s of audio at 16 kHz, computes a 171x64 log-mel spectrogram,
// runs the distilled int8 model, and publishes the result via MQTT.
//
// Before flashing, obtain from Colab and update:
//   1. INPUT_SCALE / INPUT_ZERO_POINT  (interp.get_input_details()[0]['quantization'])
//   2. Verify model_data.h matches the Colab export size (Colab: 65824 B, local: 65808 B)
//      If they differ, download distilled_student_int8.cc from Drive and replace model_data.h.
//
// NOTE – sample-rate mismatch (known project limitation):
//   Training: 44.1 kHz audio, mel range 80–16000 Hz.
//   ESP32:    16 kHz capture, Nyquist = 8 kHz, mel range 80–8000 Hz.
//   STFT frame/step are time-scaled to keep the same 171×64 output shape.
//   Most safety-relevant sound energy is below 8 kHz; limitation is acceptable.

#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "model_data.h"

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
#define FRAME_LEN    371
#define FRAME_STEP   186
#define OVERLAP_LEN  (FRAME_LEN - FRAME_STEP)   // 185 — overlap kept between frames
#define FFT_SIZE     512
#define FFT_BINS     (FFT_SIZE / 2 + 1)          // 257

// ── Mel filter bank ────────────────────────────────────────────────────────
#define NUM_MEL   64
#define MEL_LO    80.0f
#define MEL_HI    8000.0f   // Nyquist for 16 kHz (notebook uses 16000 for 44.1 kHz)

// ── Model geometry ─────────────────────────────────────────────────────────
#define NUM_FRAMES  171
#define MODEL_IN    (NUM_FRAMES * NUM_MEL)   // 10944
#define NUM_LABELS  12
// Heap-allocated below. Print arena_used_bytes() at boot to tune.
// audioBuf removed from heap (streaming approach) so arena can be larger.
#define ARENA_KB    100

// ── Labels — v8 TARGET_LABELS order (cell 8 of sound_classification_v8.ipynb)
const char* LABELS[NUM_LABELS] = {
  "glass_breaking",
  "gunshot",
  "explosion_or_fireworks",
  "impact_or_thud",
  "scream_or_shout",
  "siren_or_alarm",
  "footsteps",
  "crying_or_sobbing",
  "pet_noise",
  "weather_noise",
  "mechanical_noise",
  "unknown_background"
};

// ── Quantization parameters ────────────────────────────────────────────────
// From: interp.get_input_details()[0]['quantization'] on distilled_student_int8.tflite
// Parsed from tinyml/models/distilled_student_int8.tflite (65808 bytes, TFL3, int8 I/O).
// float 0.0 → int8 11;  float -10.0 → int8 -90;  float +5.0 → int8 +61.
#define INPUT_SCALE      0.09936082f
#define INPUT_ZERO_POINT 11

// ── Globals ───────────────────────────────────────────────────────────────
// Streaming audio approach: instead of a 64 KB audioBuf for the full 2 s window,
// we keep a FRAME_LEN-sample sliding window and process each frame on the fly.
// Total samples read = FRAME_LEN + (NUM_FRAMES-1)*FRAME_STEP = 31991 ≈ same as before.
// audioBuf is gone; streamBuf is 371*2 = 742 bytes in BSS.
static int16_t   streamBuf[FRAME_LEN];   // sliding window: 742 bytes
static int32_t   tmpReadBuf[256];        // I2S DMA intermediary: 1 KB
static uint8_t*  tensorArena = nullptr;  // heap: ARENA_KB * 1024

float  frame[FFT_SIZE];          // 2 KB
float  fftRe[FFT_SIZE];          // 2 KB
float  fftIm[FFT_SIZE];          // 2 KB
float  hannWindow[FRAME_LEN];    // 1.5 KB
// melPoints[NUM_MEL+2] replaces the 64 KB melFilterBank[64][257].
// melSpec[171][64] (44 KB) is also gone: values are quantized on the fly.
float  melPoints[NUM_MEL + 2];   // mel triangle Hz endpoints: 264 bytes
float  binFreqs[FFT_BINS];       // FFT bin Hz values: 1 KB
int8_t modelInput[MODEL_IN];     // 10.7 KB

WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

static tflite::MicroErrorReporter microErrorReporter;
static tflite::AllOpsResolver     resolver;
static tflite::MicroInterpreter*  interpreter  = nullptr;
static TfLiteTensor*              inputTensor  = nullptr;
static TfLiteTensor*              outputTensor = nullptr;

// ── Mel helpers ────────────────────────────────────────────────────────────
static float hzToMel(float hz) { return 2595.0f * log10f(1.0f + hz / 700.0f); }
static float melToHz(float mel) { return 700.0f * (powf(10.0f, mel / 2595.0f) - 1.0f); }

void buildMelFilterBank() {
  float melLo = hzToMel(MEL_LO);
  float melHi = hzToMel(MEL_HI);
  for (int i = 0; i < NUM_MEL + 2; i++)
    melPoints[i] = melToHz(melLo + i * (melHi - melLo) / (NUM_MEL + 1));
  for (int k = 0; k < FFT_BINS; k++)
    binFreqs[k] = (float)k * SAMPLE_RATE / FFT_SIZE;
}

// ── Hann window (computed once) ────────────────────────────────────────────
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

// ── Helper: fill streamBuf[pos..pos+count-1] from I2S ─────────────────────
static void i2sFill(int16_t* buf, int pos, int count) {
  while (count > 0) {
    int chunk = min(count, 256);
    size_t got = 0;
    i2s_read(I2S_PORT, tmpReadBuf, chunk * sizeof(int32_t), &got, portMAX_DELAY);
    int n = got / sizeof(int32_t);
    for (int i = 0; i < n; i++)
      buf[pos + i] = (int16_t)(tmpReadBuf[i] >> 16);
    pos   += n;
    count -= n;
  }
}

// ── Process one mel frame from streamBuf → modelInput[f*NUM_MEL ..] ────────
static void processFrame(int f) {
  for (int i = 0; i < FRAME_LEN; i++)
    frame[i] = streamBuf[i] / 8192.0f * hannWindow[i];
  computeFFT(frame);
  for (int m = 0; m < NUM_MEL; m++) {
    float lo  = melPoints[m];
    float ctr = melPoints[m + 1];
    float hi  = melPoints[m + 2];
    float invAsc  = (ctr > lo) ? 1.0f / (ctr - lo)  : 0.0f;
    float invDesc = (hi > ctr)  ? 1.0f / (hi  - ctr) : 0.0f;
    float energy  = 0.0f;
    for (int k = 0; k < FFT_BINS; k++) {
      float f_hz = binFreqs[k];
      float w = 0.0f;
      if      (f_hz >= lo  && f_hz <= ctr) w = (f_hz - lo)  * invAsc;
      else if (f_hz >  ctr && f_hz <= hi)  w = (hi  - f_hz) * invDesc;
      energy += (fftRe[k]*fftRe[k] + fftIm[k]*fftIm[k]) * w;
    }
    float melVal = logf(energy + 1e-6f);
    int q = (int)roundf(melVal / INPUT_SCALE) + INPUT_ZERO_POINT;
    if (q < -128) q = -128;
    if (q >  127) q =  127;
    modelInput[f * NUM_MEL + m] = (int8_t)q;
  }
}

// ── Streaming capture + spectrogram + quantization ─────────────────────────
// Replaces the old "read 32000 samples into audioBuf" + computeAndQuantize().
// Memory: streamBuf[371] = 742 bytes (vs 64 KB audioBuf).
// Total samples read = FRAME_LEN + (NUM_FRAMES-1)*FRAME_STEP = 31991.
// Returns RMS of the first frame's samples (debug only).
float captureAndProcess() {
  // Fill initial window
  i2sFill(streamBuf, 0, FRAME_LEN);

  // RMS over first window (proxy for signal level)
  int64_t sumSq = 0;
  for (int i = 0; i < FRAME_LEN; i++) sumSq += (int64_t)streamBuf[i] * streamBuf[i];
  float rms = sqrtf((float)sumSq / FRAME_LEN);

  for (int f = 0; f < NUM_FRAMES; f++) {
    processFrame(f);
    if (f < NUM_FRAMES - 1) {
      // Slide window: keep OVERLAP_LEN old samples, read FRAME_STEP new ones
      memmove(streamBuf, streamBuf + FRAME_STEP, OVERLAP_LEN * sizeof(int16_t));
      i2sFill(streamBuf, OVERLAP_LEN, FRAME_STEP);
    }
  }
  return rms;
}

// ── I2S / Wi-Fi / MQTT ─────────────────────────────────────────────────────
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

  // Allocate tensorArena before Wi-Fi to get a contiguous pre-fragmentation block.
  // With audioBuf gone (streaming approach), only one large heap allocation is needed.
  tensorArena = (uint8_t*)malloc(ARENA_KB * 1024);
  if (!tensorArena) {
    Serial.printf("FATAL: malloc tensorArena failed (%d KB) — free: %u, max: %u\n",
                  ARENA_KB, ESP.getFreeHeap(), ESP.getMaxAllocHeap());
    while (true) delay(1000);
  }
  Serial.printf("Free heap after malloc: %u bytes\n", ESP.getFreeHeap());

  setupI2S();
  connectWifi();
  connectMqtt();

  const tflite::Model* model = tflite::GetModel(distilled_student_int8);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.printf("Model schema mismatch: %lu vs %d\n", model->version(), TFLITE_SCHEMA_VERSION);
    while (true) delay(1000);
  }
  static tflite::MicroInterpreter staticInterpreter(
      model, resolver, tensorArena, ARENA_KB * 1024, &microErrorReporter);
  interpreter = &staticInterpreter;
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.printf("AllocateTensors failed (%d KB arena) — increase ARENA_KB\n", ARENA_KB);
    while (true) delay(1000);
  }
  Serial.printf("Arena used: %u / %u bytes\n",
                interpreter->arena_used_bytes(), (unsigned)(ARENA_KB * 1024));
  inputTensor  = interpreter->input(0);
  outputTensor = interpreter->output(0);
  Serial.println("TFLite model loaded");
  Serial.println("ESP32 TinyML audio node ready");
}

// ── Loop ───────────────────────────────────────────────────────────────────
void loop() {
  if (!mqtt.connected()) connectMqtt();
  mqtt.loop();

  unsigned long t0 = millis();

  // 1. Capture 2 s of audio and compute mel spectrogram in one streaming pass
  float rms = captureAndProcess();

  // 2. Inference
  memcpy(inputTensor->data.int8, modelInput, MODEL_IN * sizeof(int8_t));
  interpreter->Invoke();

  unsigned long inferenceMs = millis() - t0;

  // 3. Decode best label
  int bestIdx = 0;
  for (int i = 1; i < NUM_LABELS; i++)
    if (outputTensor->data.int8[i] > outputTensor->data.int8[bestIdx]) bestIdx = i;

  // Softmax over raw int8 output scores
  float expSum = 0.0f;
  for (int i = 0; i < NUM_LABELS; i++) expSum += expf((float)outputTensor->data.int8[i]);
  float confidence = expf((float)outputTensor->data.int8[bestIdx]) / expSum;

  // 4. Publish classification
  char payload[200];
  snprintf(payload, sizeof(payload),
    "{\"label\":\"%s\",\"confidence\":%.2f,\"rms\":%.1f,\"inference_ms\":%lu,\"model\":\"distilled_student_v8_int8\"}",
    LABELS[bestIdx], confidence, rms, inferenceMs);
  mqtt.publish(MQTT_TOPIC, payload);

  Serial.printf("label=%s conf=%.2f rms=%.1f inf=%lums\n",
                LABELS[bestIdx], confidence, rms, inferenceMs);
}
