#ifndef __BME_LORA_H__
#define __BME_LORA_H__

#include <SPI.h>
#include <LoRa.h>
#include "Sensor.h"
#include <cstdint>

//define the pins used by the transceiver module
#define SS 5
#define RST 14
#define DIO0 2

#define LORA_DATA_PACKET_SIZE 9

typedef struct __attribute__((packed)) {
    uint8_t deviceId;   
    uint8_t spO2;  
    uint8_t  heartRate;       
    blood_pressure_t  bloodPressure; 
    uint8_t  CRC;
} lora_data_packet_t;

void initBmeLora(); 

uint8_t* getLoraPacket();

#endif