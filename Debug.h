#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <Arduino.h>

#define DEBUG_MODE 1

#if defined(DEBUG_MODE) && (DEBUG_MODE == 1)
  #define DEBUG(...)  Serial.println(__VA_ARGS__)
#else
  #define DEBUG(...) ((void)0)
#endif

#endif