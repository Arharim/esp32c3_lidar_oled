// #include "vl53l0x.h"
// #include "i2c_master.h"
// #include <string.h>

// #define VL53L0X_I2C_ADDR 0x29

// static VL53L0X_Dev_t dev;

// VL53L0X_Error i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data,
//                         uint8_t len) {
//   uint8_t buffer[16];
//   buffer[0] = reg_addr;
//   memcpy(&buffer[1], data, len);
//   esp_err_t res = i2c_master_write_to_device(I2C_NUM_0, dev_addr, buffer,
//                                              len + 1, pdMS_TO_TICKS(1000));
//   return res == ESP_OK ? 0 : 1;
// }

// VL53L0X_Error i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data,
//                        uint8_t len) {
//   esp_err_t res = i2c_master_write_read_device(
//       I2C_NUM_0, dev_addr, &reg_addr, 1, data, len, pdMS_TO_TICKS(1000));
//   return res == ESP_OK ? 0 : 1;
// }

// void vl53l0x_init(void) {
//   dev.I2cDevAddr = VL53L0X_I2C_ADDR;
//   dev.comms_type = 1; // I2C
//   dev.comms_speed_khz = 400;
//   dev.read = i2c_read;
//   dev.write = i2c_write;

//   VL53L0X_Error status = VL53L0X_DataInit(&dev);
//   if (status != 0) {
//     printf("VL53L0X init failed: %d\n", status);
//     return;
//   }

//   status = VL53L0X_StaticInit(&dev);
//   if (status != 0) {
//     printf("VL53L0X static init failed: %d\n", status);
//     return;
//   }

//   VL53L0X_SetDistanceMode(&dev, VL53L0X_DISTANCEMODE_SHORT);
//   VL53L0X_StartMeasurement(&dev);
// }

// uint16_t vl53l0x_read_distance_mm(void) {
//   VL53L0X_RangingMeasurementData_t data;
//   VL53L0X_Error status = VL53L0X_PerformSingleRangingMeasurement(&dev,
//   &data); if (status == 0) {
//     return data.RangeMilliMeter;
//   } else {
//     return 0xFFFF;
//   }
// }
