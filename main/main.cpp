#include "../components/ssd1306/font8x8_basic.h"
#include "../components/ssd1306/ssd1306.h"
#include "VL53L0X.h"
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
#define SDA_GPIO GPIO_NUM_21
#define SCL_GPIO GPIO_NUM_20
#define I2C_PORT I2C_NUM_0
#define TAG "app_main"
#define tag "SSD1306"
#define OFFSET_CALIBRATION -20
#define FILTER_WINDOW 5
#define MIN_DISTANCE 0
#define MAX_DISTANCE 2000

int32_t mesurements[FILTER_WINDOW] = {0};
uint8_t mesurement_index = 0;

extern "C" void app_main(void)
{
	SSD1306_t dev;
	int center, top, bottom;
	char lineChar[20];

#if CONFIG_I2C_INTERFACE
	ESP_LOGI(tag, "INTERFACE is i2c");
	ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",SDA_GPIO );
	ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",SCL_GPIO);
	ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(&dev, SDA_GPIO, SCL_GPIO, CONFIG_RESET_GPIO);
#endif // CONFIG_I2C_INTERFACE

#if CONFIG_SPI_INTERFACE
	ESP_LOGI(tag, "INTERFACE is SPI");
	ESP_LOGI(tag, "CONFIG_MOSI_GPIO=%d",CONFIG_MOSI_GPIO);
	ESP_LOGI(tag, "CONFIG_SCLK_GPIO=%d",CONFIG_SCLK_GPIO);
	ESP_LOGI(tag, "CONFIG_CS_GPIO=%d",CONFIG_CS_GPIO);
	ESP_LOGI(tag, "CONFIG_DC_GPIO=%d",CONFIG_DC_GPIO);
	ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
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
	ssd1306_display_text_x3(&dev, 0, "Hello", 5, false);
	vTaskDelay(3000 / portTICK_PERIOD_MS);

#if CONFIG_SSD1306_128x64
	top = 2;
	center = 3;
	bottom = 8;
	ssd1306_display_text(&dev, 0, "SSD1306 128x64", 14, false);
	ssd1306_display_text(&dev, 1, "ABCDEFGHIJKLMNOP", 16, false);
	ssd1306_display_text(&dev, 2, "abcdefghijklmnop",16, false);
	ssd1306_display_text(&dev, 3, "Hello World!!", 13, false);
	//ssd1306_clear_line(&dev, 4, true);
	//ssd1306_clear_line(&dev, 5, true);
	//ssd1306_clear_line(&dev, 6, true);
	//ssd1306_clear_line(&dev, 7, true);
	ssd1306_display_text(&dev, 4, "SSD1306 128x64", 14, true);
	ssd1306_display_text(&dev, 5, "ABCDEFGHIJKLMNOP", 16, true);
	ssd1306_display_text(&dev, 6, "abcdefghijklmnop",16, true);
	ssd1306_display_text(&dev, 7, "Hello World!!", 13, true);
#endif // CONFIG_SSD1306_128x64

#if CONFIG_SSD1306_128x32
	top = 1;
	center = 1;
	bottom = 4;
	ssd1306_display_text(&dev, 0, "SSD1306 128x32", 14, false);
	ssd1306_display_text(&dev, 1, "Hello World!!", 13, false);
	//ssd1306_clear_line(&dev, 2, true);
	//ssd1306_clear_line(&dev, 3, true);
	ssd1306_display_text(&dev, 2, "SSD1306 128x32", 14, true);
	ssd1306_display_text(&dev, 3, "Hello World!!", 13, true);
#endif // CONFIG_SSD1306_128x32
	vTaskDelay(3000 / portTICK_PERIOD_MS);

  VL53L0X vl(I2C_PORT);
  if (!vl.init()) {
    ESP_LOGE(TAG, "Failed to initialize VL53L0X :(");
    vTaskDelay(portMAX_DELAY);
  }

  ssd1306_clear_screen(&dev, false);

while (1) {
    uint16_t raw_result_mm = 0;
    char buf[20];
    if (vl.read(&raw_result_mm)) {
        int32_t calibrated_raw = (int32_t)raw_result_mm + OFFSET_CALIBRATION;
        calibrated_raw = (calibrated_raw < MIN_DISTANCE) ? MIN_DISTANCE : calibrated_raw;
        calibrated_raw = (calibrated_raw > MAX_DISTANCE) ? MAX_DISTANCE : calibrated_raw;
        mesurements[mesurement_index] = calibrated_raw;
        mesurement_index = (mesurement_index + 1) % FILTER_WINDOW;

        int32_t sorted[FILTER_WINDOW];
        memcpy(sorted, mesurements, sizeof(mesurements));
        for (int i = 0; i < FILTER_WINDOW - 1; i++) {
            for (int j = i + 1; j < FILTER_WINDOW; j++) {
                if (sorted[j] < sorted[i]) {
                    int32_t temp = sorted[i];
                    sorted[i] = sorted[j];
                    sorted[j] = temp;
                }
            }
        }
        int32_t final_distance_mm = sorted[FILTER_WINDOW / 2];

        if (FILTER_WINDOW >= 3) {
            final_distance_mm = (sorted[FILTER_WINDOW/2 - 1] + sorted[FILTER_WINDOW/2] + sorted[FILTER_WINDOW/2 + 1]) / 3;
        }

        ESP_LOGI(TAG, "Raw: %d, Calibrated: %d, Final: %d [mm]", raw_result_mm, calibrated_raw, final_distance_mm);
        sprintf(buf, "%ld mm", final_distance_mm);
        ssd1306_clear_screen(&dev, false);
        ssd1306_display_text_x3(&dev, 0, buf, strlen(buf), false);
    } else {
        ESP_LOGE(TAG, "Failed to measure :(");
        ssd1306_clear_screen(&dev, false);
        ssd1306_display_text(&dev, 0, "Failed to", 9, false);
        ssd1306_display_text(&dev, 1, "measure", 7, false);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

#if 0
	// Fade Out
	for(int contrast=0xff;contrast>0;contrast=contrast-0x20) {
		ssd1306_contrast(&dev, contrast);
		vTaskDelay(40);
	}
#endif

	// Restart module
	// esp_restart();
}