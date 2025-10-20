#include "../components/ssd1306/font8x8_basic.h"
#include "../components/ssd1306/ssd1306.h"
#include "../components/vl53l0x/include/vl53l0x_api.h"
#include "../components/vl53l0x/include/vl53l0x_platform.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 You have to set this config value with menuconfig
 CONFIG_INTERFACE
 for i2c
 CONFIG_MODEL
 CONFIG_SDA_GPIO
 CONFIG_SCL_GPIO
 CONFIG_RESET_GPIO
 for SPI
 CONFIG_CS_GPIO
 CONFIG_DC_GPIO
 CONFIG_RESET_GPIO
*/
#define SDA_GPIO 21
#define SCL_GPIO 20
#define tag "SSD1306"

void app_main(void) {
  SSD1306_t dev;
#if CONFIG_I2C_INTERFACE
  ESP_LOGI(tag, "INTERFACE is i2c");
  ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d", SDA_GPIO);
  ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d", SCL_GPIO);
  ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d", CONFIG_RESET_GPIO);
  i2c_master_init(&dev, SDA_GPIO, SCL_GPIO, CONFIG_RESET_GPIO);
#endif // CONFIG_I2C_INTERFACE
#if CONFIG_SPI_INTERFACE
  ESP_LOGI(tag, "INTERFACE is SPI");
  ESP_LOGI(tag, "CONFIG_MOSI_GPIO=%d", CONFIG_MOSI_GPIO);
  ESP_LOGI(tag, "CONFIG_SCLK_GPIO=%d", CONFIG_SCLK_GPIO);
  ESP_LOGI(tag, "CONFIG_CS_GPIO=%d", CONFIG_CS_GPIO);
  ESP_LOGI(tag, "CONFIG_DC_GPIO=%d", CONFIG_DC_GPIO);
  ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d", CONFIG_RESET_GPIO);
  spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO,
                  CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
#endif // CONFIG_SPI_INTERFACE
#if CONFIG_FLIP
  dev._flip = true;
  ESP_LOGW(tag, "Flip upside down");
#endif
#if CONFIG_SSD1306_128x64
  ESP_LOGI(tag, "Panel is 128x64");
  ssd1306_init(&dev, 128, 64);
#endif // CONFIG_SSD1306_128x64
#if CONFIG_SSD1306_128x32
  ESP_LOGI(tag, "Panel is 128x32");
  ssd1306_init(&dev, 128, 32);
#endif // CONFIG_SSD1306_128x32
  ssd1306_clear_screen(&dev, false);
  ssd1306_contrast(&dev, 0xff);

  // Initialize VL53L0X using STM API
  VL53L0X_Dev_t dev_vl;
  VL53L0X_Error Status = VL53L0X_ERROR_NONE;
  VL53L0X_Version_t version;
  VL53L0X_DeviceInfo_t device_info;

  memset(&dev_vl, 0, sizeof(dev_vl));
  dev_vl.I2cDevAddr = 0x29;

  if (Status == VL53L0X_ERROR_NONE) {
    Status = VL53L0X_DataInit(&dev_vl);
  }
  if (Status == VL53L0X_ERROR_NONE) {
    Status = VL53L0X_GetDeviceInfo(&dev_vl, &device_info);
    if (Status == VL53L0X_ERROR_NONE && device_info.ProductId[0] != 0) {
      ESP_LOGI(tag, "VL53L0X detected");
    } else {
      ESP_LOGE(tag, "VL53L0X not detected");
      while (1)
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Halt on error
    }
  }
  if (Status == VL53L0X_ERROR_NONE) {
    Status = VL53L0X_StaticInit(&dev_vl);
  }
  if (Status == VL53L0X_ERROR_NONE) {
    Status = VL53L0X_SetDeviceMode(&dev_vl, VL53L0X_DEVICEMODE_SINGLE_RANGING);
  }
  if (Status == VL53L0X_ERROR_NONE) {
    Status = VL53L0X_SetLimitCheckEnable(
        &dev_vl, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
  }
  if (Status == VL53L0X_ERROR_NONE) {
    Status = VL53L0X_SetLimitCheckEnable(
        &dev_vl, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
  }
  if (Status == VL53L0X_ERROR_NONE) {
    Status = VL53L0X_SetLimitCheckValue(
        &dev_vl, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
        (FixPoint1616_t)(0.1 * 65536));
  }
  if (Status == VL53L0X_ERROR_NONE) {
    Status = VL53L0X_SetLimitCheckValue(&dev_vl,
                                        VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
                                        (FixPoint1616_t)(60 * 65536));
  }
  if (Status == VL53L0X_ERROR_NONE) {
    Status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&dev_vl, 33000);
  }
  if (Status != VL53L0X_ERROR_NONE) {
    ESP_LOGE(tag, "VL53L0X init error: %d", Status);
    while (1)
      vTaskDelay(1000 / portTICK_PERIOD_MS); // Halt on error
  }

  // Loop to read and display distance
  while (1) {
    VL53L0X_RangingMeasurementData_t measure;
    Status = VL53L0X_PerformSingleRangingMeasurement(&dev_vl, &measure);
    if (Status == VL53L0X_ERROR_NONE) {
      char buf[20];
      if (measure.RangeStatus != 4) { // 4 = out of range
        sprintf(buf, "%d mm", measure.RangeMilliMeter);
      } else {
        sprintf(buf, "Out of range");
      }
      ssd1306_clear_screen(&dev, false);
      ssd1306_display_text(&dev, 0, buf, strlen(buf), false);
    } else {
      ESP_LOGE(tag, "Measurement error: %d", Status);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}