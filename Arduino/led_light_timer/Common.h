#include "Arduino.h"

#ifndef COMMON_H
#define COMMON_H
  #define LIGHT_SW (14)
  #define EEP_PREAMBLE (0x5A)
  #define MAX_LIGHT_SLEEP_SEC  (30)
  #define MAX_DEEP_SLEEP_SEC  (60*60)
  #define START_TIME_HOUR (11)
  #define END_TIME_HOUR (21)
  #define INIT_MODE (0xFF)
  #define LIGHT_ON_MODE (1)
  #define LIGHT_OFF_MODE (2)
//  #define USE_TEST_PRINTOUT  
#endif //COMMON_H

