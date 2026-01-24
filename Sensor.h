#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <cstdint>
#include "BmeBoard.h"

#if defined(LORA_NODE) && (LORA_NODE == 1)        // LORA_NODE == 1
#include "MAX30105.h"
#include "spo2_algorithm.h"

extern MAX30105 particleSensor;

extern uint32_t irBuffer[100]; 
extern uint32_t redBuffer[100];
extern int32_t spo2, heartRate;
extern int8_t  validSpO2, validHR;

extern int dispBPM;
extern int dispSpO2;
extern bool tick;

void initSensor();

bool getSpO2AndHeartRate(uint8_t* spO2, uint16_t* hr);
#endif        // LORA_NODE == 1

#endif