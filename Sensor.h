#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <cstdint>
#include "BmeBoard.h"

#if defined(ESP32) && (ESP32 == 0)
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
#endif

typedef struct __attribute__((packed)){
    uint8_t systolic;   // mmHg
    uint8_t diastolic;  // mmHg
} blood_pressure_t;

void initSensor();

blood_pressure_t getBloodPressure();
bool getSpO2AndHeartRate(uint8_t* spO2, uint8_t* hr);

#endif