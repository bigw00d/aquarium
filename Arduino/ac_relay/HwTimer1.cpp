#include "HwTimer1.h"

uint16_t timer1_cnt = 0;
uint16_t timer1_end_cnt;
TimeoutFunc pHwTimer1Func;
#define TIMER1_INTERVAL_US (1000000) //1sec

void ICACHE_RAM_ATTR HwTimer1_ISR();
uint32_t usToTicks(uint32_t us);

void HwTimer1_Init() {
    timer1_disable();
    timer1_isr_init();
    pHwTimer1Func = NULL;
}

void HwTimer1_Start(uint16_t timer_sec, TimeoutFunc pAfterTimeout)
{
    timer1_cnt = 0;
    timer1_end_cnt = timer_sec;
    pHwTimer1Func = pAfterTimeout;

    timer1_disable();
    timer1_attachInterrupt(HwTimer1_ISR);
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP); //roop timer
    timer1_write(usToTicks(TIMER1_INTERVAL_US));
}

void HwTimer1_Stop(void)
{
    timer1_disable();
}

void disableTimer(uint8_t _index) {
      timer1_disable();
}

uint32_t usToTicks(uint32_t us)
{
    return (HW_TIMER1_TICKS_PER_US / 16 * us);     // converts microseconds to tick count
}

void ICACHE_RAM_ATTR HwTimer1_ISR() {
  timer1_cnt++;
  if(timer1_cnt >= timer1_end_cnt) {
    timer1_cnt = 0;
    if(pHwTimer1Func != NULL) {
      pHwTimer1Func();
    }
  }
}
