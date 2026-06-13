#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

#define I2S_WS 15
#define I2S_SD 32
#define I2S_SCK 14

#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000
#define BUFFER_LEN 512

int32_t samples[BUFFER_LEN];

void setup() {
  Serial.begin(115200);
  delay(1000);

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);

  Serial.println("INMP441 I2S audio feature test started");
}

void loop() {
  size_t bytes_read = 0;
  i2s_read(I2S_PORT, samples, sizeof(samples), &bytes_read, portMAX_DELAY);

  int samples_read = bytes_read / sizeof(int32_t);

  int64_t sum_squares = 0;
  int32_t peak = 0;

  for (int i = 0; i < samples_read; i++) {
    int32_t sample = samples[i] >> 14;
    int32_t abs_sample = abs(sample);

    sum_squares += (int64_t)sample * sample;

    if (abs_sample > peak) {
      peak = abs_sample;
    }
  }

  float rms = sqrt((float)sum_squares / samples_read);

  const char* audio_state = "quiet";

  if (rms > 300 || peak > 30000) {
    audio_state = "sound";
  }

  Serial.print("samples_read=");
  Serial.print(samples_read);

  Serial.print(" rms=");
  Serial.print(rms, 1);

  Serial.print(" peak=");
  Serial.print(peak);

  Serial.print(" audio_state=");
  Serial.println(audio_state);

  delay(200);
}
