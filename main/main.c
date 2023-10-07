#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <ra01s.h>
#include <stdint.h>

void spi_init(spi_host_device_t device, uint32_t mosi, uint32_t miso, uint32_t sck) {
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

void app_main(void) {
  spi_init(LORA_SPI_HOST, CONFIG_LORA_MOSI_GPIO, CONFIG_LORA_MISO_GPIO, CONFIG_LORA_SCK_GPIO);
  lora_init();

  uint8_t message[] = "Hello World";
  while (1) {
    LoRaSend(message, sizeof(message), SX126x_TXMODE_SYNC);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
