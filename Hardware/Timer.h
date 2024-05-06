#ifndef __TIMER_H__
#define __TIMER_H__

extern uint16_t timecount;

void Timer_Init(void);
void TIM3_IRQHandler(void);
uint16_t timecountGet(void);

#endif
