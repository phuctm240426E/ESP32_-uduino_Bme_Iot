#ifndef __BME_BOARD_H__
#define __BME_BOARD_H__

#define TEST_MODE 0

#define ESP32_S3 1
#define ESP32_C3 0
#define ESP32    0

#define LORA_NODE 0
#define LORA_GW 1

#if defined(ESP32) && (ESP32 == 1)
  #define DEVICE_ID 1
#endif

#if defined(ESP32_C3) && (ESP32_C3 == 1)
  #define DEVICE_ID 2
#endif

#endif