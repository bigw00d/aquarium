#include "Arduino.h"

#ifndef COMMON_H
#define COMMON_H
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
  #define UPDATE_INTERVAL_SEC  (30) // valid range:1s, 2s, ... , 65535 (unit:sec)
  #define LIGHT_START_TIME_HOUR (11)
  #define LIGHT_END_TIME_HOUR (20)
  #define PIN_ACOUT 5  //IO5
  #define LOGFILE_NAME "log.txt"
  #define CSVFILE_NAME "save.csv"
  #define CSV_COUNT_MAX (2)
  #define FAN_ON_TEMPERATURE ((float)27.0)
#endif //COMMON_H

