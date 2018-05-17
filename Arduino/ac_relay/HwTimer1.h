#include "Arduino.h"

#ifndef INCLUDED_HWTIMER1

#define INCLUDED_HWTIMER1

typedef void (*TimeoutFunc)(void);

#define HW_TIMER1_TICKS_PER_US (APB_CLK_FREQ / 1000000L)

void HwTimer1_Init(void);

void HwTimer1_Start(uint16_t timer_sec, TimeoutFunc pAfterTimeout);

void HwTimer1_Stop(void);

#endif // INCLUDED_TIMER1
