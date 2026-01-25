#ifndef __BME_LORA_H__
#define __BME_LORA_H__

#include <SPI.h>
#include <LoRa.h>
#include "Sensor.h"
#include <cstdint>
#include "BmeBoard.h"
#include "BmeLcd.h"

#if defined(ESP32) && (ESP32 == 1)
  #define SS 4
  #define RST 5
  #define DIO0 2
#endif

#if defined(ESP32_C3) && (ESP32_C3 == 1) //small
  #define L_SCK 4
  #define L_MISO 5
  #define L_MOSI 3
  #define SS 2
  #define RST 1
  #define DIO0 0
#endif

#if defined(ESP32_S3) && (ESP32_S3 == 1)
  #define L_SCK   5
  #define L_MISO  6
  #define L_MOSI  7
  #define SS   15
  #define RST   16
  #define DIO0  17 
#endif

#define MAX_LBT_RETRY      5
#define MAX_TX_RETRY      3
#define BACKOFF_MIN_MS    500
#define BACKOFF_MAX_MS    3000
#define ACK_TIMEOUT_MS   1500

#define LORA_DATA_PACKET_SIZE 6

typedef struct __attribute__((packed)) {
    uint8_t deviceId;   
    uint8_t spO2;  
    uint16_t  heartRate;  
    uint16_t crc;     
} lora_data_packet_t;

typedef struct __attribute__((packed)) {
    uint8_t deviceId;   
    uint8_t isActive;  
} alarm_t;

void initBmeLora(); 

uint16_t calculateCRC(const uint8_t *data, uint16_t len);

#if defined(LORA_NODE) && (LORA_NODE == 1)
bool setLoraPacket(lora_data_packet_t* data);
#endif        // LORA_NODE == 1

bool getLoraPacket(lora_data_packet_t* data);

#if defined(LORA_NODE) && (LORA_NODE == 1)
void setAndSendLoraPacket();
#endif        // LORA_NODE == 1

#endif