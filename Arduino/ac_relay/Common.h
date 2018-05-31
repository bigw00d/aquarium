#include "Arduino.h"

#ifndef COMMON_H
#define COMMON_H
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
  // #define UPDATE_INTERVAL_SEC  (30) // valid range:1s, 2s, ... , 65535 (unit:sec)
  #define UPDATE_INTERVAL_SEC  (5) // valid range:1s, 2s, ... , 65535 (unit:sec)
  #define START_TIME_HOUR (11)
  #define END_TIME_HOUR (20)
  #define PIN_ACOUT 5  //IO5
  #define LOGFILE_NAME "log.txt"

#endif //COMMON_H

