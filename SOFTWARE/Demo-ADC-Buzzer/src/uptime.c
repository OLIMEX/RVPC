#include "uptime.h"

static uint32_t uptime_seconds = 0;

static uint32_t p_ms = 0;
static uint32_t p_us = 0;

void Uptime_Init() {
	if (p_ms != 0) {
		// Already initialized
		return;
	}

	p_ms = (uint32_t)(SystemCoreClock / 1000);
	p_us = (uint32_t)(SystemCoreClock / 1000000);

	// Interrupt every second
	NVIC_EnableIRQ(SysTicK_IRQn);

	SysTick->SR &= ~(1 << 0);
	SysTick->CMP = SystemCoreClock-1;
	SysTick->CNT = 0;
	SysTick->CTLR = 0xF;
}

uint32_t Uptime_S() {
	return uptime_seconds;
}

uint32_t Uptime_Ms() {
	return uptime_seconds * 1000 + (uint32_t)(SysTick->CNT / p_ms);
}

uint32_t Uptime_Us() {
	return uptime_seconds * 1000000 + (uint32_t)(SysTick->CNT / p_us);
}

void Wait_Ms(uint32_t delay) {
	uint32_t start = Uptime_Ms();
	while((Uptime_Ms() - start) < delay);
}

void Wait_Us(uint32_t delay) {
	uint32_t start = Uptime_Us();
	while((Uptime_Us() - start) < delay);
}

void SysTick_Handler(void) {
	uptime_seconds++;
	SysTick->SR = 0;
}