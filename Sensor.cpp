#include "Sensor.h"
#include <Arduino.h>

blood_pressure_t getBloodPressure() {
  blood_pressure_t data;
  data.diastolic = random(10, 200);
  data.systolic = random(10, 100);
  return data;
}

uint8_t getSpO2() {
  return random(10, 100);
}

uint8_t getHeartRate() {
  return random(10, 200);
}