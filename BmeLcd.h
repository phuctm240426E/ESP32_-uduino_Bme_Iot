#ifndef __BME_LCD_H__
#define __BME_LCD_H__

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BmeBoard.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define I2C_SDA 8
#define I2C_SCL 9

#if defined(ESP32) && (ESP32 == 0)
extern Adafruit_SSD1306 display;
#endif

void initOLED();

#endif