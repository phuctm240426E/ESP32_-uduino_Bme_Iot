#ifndef __BME_LORA_H__
#define __BME_LORA_H__

#include <SPI.h>
#include <LoRa.h>
#include "Sensor.h"
#include <cstdint>
#include "BmeLcd.h"

//define the pins used by the transceiver module
#define ESP32_S3 1
#define ESP32_C3 0
#define ESP32    0

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

#define LORA_DATA_PACKET_SIZE 6

typedef struct __attribute__((packed)) {
    uint8_t deviceId;   
    uint8_t spO2;  
    uint8_t  heartRate;       
    blood_pressure_t  bloodPressure; 
    uint8_t  CRC;
} lora_data_packet_t;

void initBmeLora(); 

void setLoraPacket(lora_data_packet_t* data);

bool getLoraPacket(lora_data_packet_t* data);

void setAndSendLoraPacket();

#endif