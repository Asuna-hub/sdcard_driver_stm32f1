#include "systick.h"

uint32_t SysTick_CNT = 0;

void SysTick_Init(void){
	SysTick->LOAD = (SYSCLOCK / 1000) - 1;
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void){
	if (SysTick_CNT > 0)
		SysTick_CNT--;
}

void delay_ms(uint32_t ms) {
    SysTick_CNT = ms;
    while (SysTick_CNT);
}

void delay_s(uint32_t s) {
    for (uint32_t i = 0; i < s; i++) {
        delay_ms(1000);
    }
}