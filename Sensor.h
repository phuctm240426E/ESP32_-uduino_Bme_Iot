#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <cstdint>

typedef struct __attribute__((packed)){
    uint8_t systolic;   // mmHg
    uint8_t diastolic;  // mmHg
} blood_pressure_t;

blood_pressure_t getBloodPressure();
uint8_t getSpO2();
uint8_t getHeartRate();

#endif