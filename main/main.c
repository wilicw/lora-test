#include <bsp.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <ra01s.h>
#include <stddef.h>
#include <stdint.h>

static const char *TAG = "main";
static const size_t RECV_LEN = 255;

void spi_init(spi_host_device_t device, uint32_t mosi, uint32_t miso,
              uint32_t sck) {
  esp_err_t ret;
  const spi_bus_config_t spi_bus_config = {
      .sclk_io_num = sck,
      .mosi_io_num = mosi,
      .miso_io_num = miso,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
  };
  ret = spi_bus_initialize(device, &spi_bus_config, SPI_DMA_CH_AUTO);
  ESP_LOGI(TAG, "SPI%d_HOST spi_bus_initialize=%d", device + 1, ret);
}

void indicator_init() {
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = BIT(CONFIG_INDI_LED);
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;
  gpio_config(&io_conf);
}

void indicator_toggle() {
  static uint8_t state = 0;
  state ^= 1;
  gpio_set_level(CONFIG_INDI_LED, state);
}

void task(void *pvParameters) {
  uint8_t message[] = "ME0W!";
  size_t len;
  uint8_t data[RECV_LEN];
  uint32_t last_send = xTaskGetTickCount();
  while (1) {
    if ((len = LoRaReceive(&data[0], RECV_LEN)) > 0) {
      int8_t rssi, snr;
      GetPacketStatus(&rssi, &snr);
      ESP_LOGI(TAG, "RSSI=%d SNR=%d", rssi, snr);
      ESP_LOG_BUFFER_HEXDUMP(TAG, data, len, ESP_LOG_INFO);
      indicator_toggle();
    }
    if (xTaskGetTickCount() - last_send >= pdMS_TO_TICKS(3000)) {
      LoRaSend(message, sizeof(message), SX126x_TXMODE_SYNC);
      last_send = xTaskGetTickCount();
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void app_main(void) {
  indicator_init();
  for (int i = 0; i < 50; i++) {
    indicator_toggle();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  spi_init(LORA_SPI_HOST, CONFIG_LORA_MOSI_GPIO, CONFIG_LORA_MISO_GPIO,
           CONFIG_LORA_SCK_GPIO);
  lora_init();
  xTaskCreate(task, "task", 4096, NULL, 1, NULL);
}
