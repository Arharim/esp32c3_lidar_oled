idf_component_register (SRCS "main.cpp" # "vl53l0x_api_core.c" "vl53l0x_api.c" "vl53l0x_platform.c" "vl53l0x_api_calibration.c" "vl53l0x_api_ranging.c" "vl53l0x_api_strings.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver ssd1306 vl53l0x_s)
