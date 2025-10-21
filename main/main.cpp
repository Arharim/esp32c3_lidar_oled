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
	
	// // Display Count Down
	// uint8_t image[24];
	// memset(image, 0, sizeof(image));
	// ssd1306_display_image(&dev, top, (6*8-1), image, sizeof(image));
	// ssd1306_display_image(&dev, top+1, (6*8-1), image, sizeof(image));
	// ssd1306_display_image(&dev, top+2, (6*8-1), image, sizeof(image));
	// for(int font=0x39;font>0x30;font--) {
	// 	memset(image, 0, sizeof(image));
	// 	ssd1306_display_image(&dev, top+1, (7*8-1), image, 8);
	// 	memcpy(image, font8x8_basic_tr[font], 8);
	// 	if (dev._flip) ssd1306_flip(image, 8);
	// 	ssd1306_display_image(&dev, top+1, (7*8-1), image, 8);
	// 	vTaskDelay(1000 / portTICK_PERIOD_MS);
	// }
	
	// // Scroll Up
	// ssd1306_clear_screen(&dev, false);
	// ssd1306_contrast(&dev, 0xff);
	// ssd1306_display_text(&dev, 0, "---Scroll  UP---", 16, true);
	// //ssd1306_software_scroll(&dev, 7, 1);
	// ssd1306_software_scroll(&dev, (dev._pages - 1), 1);
	// for (int line=0;line<bottom+10;line++) {
	// 	lineChar[0] = 0x01;
	// 	sprintf(&lineChar[1], " Line %02d", line);
	// 	ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
	// 	vTaskDelay(500 / portTICK_PERIOD_MS);
	// }
	// vTaskDelay(3000 / portTICK_PERIOD_MS);
	
  VL53L0X vl(I2C_PORT);
  if (!vl.init()) {
    ESP_LOGE(TAG, "Failed to initialize VL53L0X :(");
    vTaskDelay(portMAX_DELAY);
  }

  ssd1306_clear_screen(&dev, false);

  while (1) {
    uint16_t raw_result_mm = 0;

    // TickType_t tick_start = xTaskGetTickCount();
    // TickType_t tick_end = xTaskGetTickCount();
    // int took_ms = ((int)tick_end - tick_start) / portTICK_PERIOD_MS;
	char buf[20];
	if(vl.read(&raw_result_mm)){
		int32_t calibrated_raw = (int32_t)raw_result_mm + OFFSET_CALIBRATION;
		calibrated_raw = (calibrated_raw < MIN_DISTANCE) ? MIN_DISTANCE : calibrated_raw;
		calibrated_raw = (calibrated_raw > MAX_DISTANCE) ? MAX_DISTANCE : calibrated_raw;

		mesurements[mesurement_index] = calibrated_raw;
		mesurement_index = (mesurement_index + 1) % FILTER_WINDOW;

		int32_t sorted[FILTER_WINDOW];
		memcpy(sorted, mesurements, sizeof(mesurements));
		for (int i = 0; i < FILTER_WINDOW - 1; i++)
		{
			for (int j = i + 1; j < FILTER_WINDOW; j++)
			{
				if (sorted[j] < sorted[i])
				{
					uint16_t temp = sorted[i];
					sorted[i] = sorted[j];
					sorted[j] = temp;
				}
			}
		}
		int32_t final_distance_mm = sorted[FILTER_WINDOW/2];
		ESP_LOGI(TAG, "Range: %d [mm]", (int)final_distance_mm);
		sprintf(buf, "%ld mm ", final_distance_mm);
		ssd1306_display_text_x3(&dev, 0, buf, strlen(buf), false);
	}
	else{
        ESP_LOGE(TAG, "Failed to measure :(");
	    ssd1306_display_text(&dev, 0, "Failed to", 9, false);
	    ssd1306_display_text(&dev, 1, "measure", 7, false);
  	}
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