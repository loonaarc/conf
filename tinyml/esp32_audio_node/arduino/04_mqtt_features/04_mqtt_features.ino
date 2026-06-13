#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ── Wi-Fi ──────────────────────────────────────────────────────────────────
#define WIFI_SSID     "realme C3"
#define WIFI_PASSWORD "easy098!"   // <-- fill this in

// ── MQTT ───────────────────────────────────────────────────────────────────
#define MQTT_HOST     "192.168.43.26"
#define MQTT_PORT     1883
#define MQTT_TOPIC    "tele/safety_audio_1/FEATURES"
#define MQTT_CLIENT   "esp32_audio_1"

// ── I2S / INMP441 ──────────────────────────────────────────────────────────
#define I2S_WS   15
#define I2S_SD   32
#define I2S_SCK  14

#define I2S_PORT    I2S_NUM_0
#define SAMPLE_RATE 16000
#define BUFFER_LEN  512

// ── Thresholds (same as sketch 03) ─────────────────────────────────────────
#define RMS_THRESHOLD  300
#define PEAK_THRESHOLD 30000

// ── Publish interval ───────────────────────────────────────────────────────
#define PUBLISH_INTERVAL_MS 1000

int32_t   samples[BUFFER_LEN];
WiFiClient    wifiClient;
PubSubClient  mqtt(wifiClient);
unsigned long lastPublish = 0;

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
    .bck_io_num   = I2S_SCK,
    .ws_io_num    = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = I2S_SD
  };
  i2s_driver_install(I2S_PORT, &cfg, 0, NULL);
  i2s_set_pin(I2S_PORT, &pins);
  i2s_zero_dma_buffer(I2S_PORT);
}

void connectWifi() {
  Serial.printf("Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nWi-Fi connected, IP: %s\n", WiFi.localIP().toString().c_str());
}

void connectMqtt() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqtt.connect(MQTT_CLIENT)) {
      Serial.println(" connected");
    } else {
      Serial.printf(" failed (state=%d), retrying in 3s\n", mqtt.state());
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  setupI2S();
  connectWifi();
  connectMqtt();
  Serial.println("ESP32 audio MQTT node ready");
}

void loop() {
  if (!mqtt.connected()) {
    connectMqtt();
  }
  mqtt.loop();

  // Read one buffer from the microphone
  size_t bytes_read = 0;
  i2s_read(I2S_PORT, samples, sizeof(samples), &bytes_read, portMAX_DELAY);
  int samples_read = bytes_read / sizeof(int32_t);

  int64_t sum_sq = 0;
  int32_t peak   = 0;
  for (int i = 0; i < samples_read; i++) {
    int32_t s    = samples[i] >> 14;
    int32_t abss = abs(s);
    sum_sq += (int64_t)s * s;
    if (abss > peak) peak = abss;
  }
  float rms = sqrt((float)sum_sq / samples_read);
  const char* audio_state = (rms > RMS_THRESHOLD || peak > PEAK_THRESHOLD) ? "sound" : "quiet";

  // Publish at most once per PUBLISH_INTERVAL_MS
  unsigned long now = millis();
  if (now - lastPublish >= PUBLISH_INTERVAL_MS) {
    lastPublish = now;

    char payload[128];
    snprintf(payload, sizeof(payload),
      "{\"rms\":%.1f,\"peak\":%d,\"audio_state\":\"%s\",\"samples_read\":%d}",
      rms, peak, audio_state, samples_read);

    if (mqtt.publish(MQTT_TOPIC, payload)) {
      Serial.printf("Published: %s\n", payload);
    } else {
      Serial.println("Publish failed");
    }
  }
}
