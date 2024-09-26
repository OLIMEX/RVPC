/**
 * This file is part of the RVPC project 
 * https://github.com/OLIMEX/RVPC
 * https://www.olimex.com/Products/Retro-Computers/RVPC/open-source-hardware
 * 
 * Copyright (c) 2024 Olimex Ltd
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "wait.h"
#include <ch32v00x.h>

static uint32_t p_ms = 0;
static uint32_t p_us = 0;

void Wait_Init() {
	if (p_ms != 0) {
		// Already initialized
		return;
	}

	p_us = (uint32_t)(SystemCoreClock / 1000000);
	p_ms = (uint32_t)(SystemCoreClock / 1000);

	SysTick->SR &= ~(1 << 0);
	SysTick->CMP = 0xFFFFFFFF;
	SysTick->CNT = 0;
	SysTick->CTLR = 0xF;
}

void Wait_Ms(uint32_t delay) {
	static volatile uint32_t start;
	delay = delay * p_ms - 193;
	start = SysTick->CNT;
	while (SysTick->CNT - start < delay);
}

void Wait_Us(uint32_t delay) {
	static volatile uint32_t start;
	delay = delay * p_us - 83;
	start = SysTick->CNT;
	while (SysTick->CNT - start < delay);
}

static volatile uint32_t startMS;
static volatile uint32_t delayMS;
void setTimeout_Ms(uint32_t timeoutMS) {
	delayMS = timeoutMS * p_ms;
	startMS = SysTick->CNT;
}

uint8_t isTimeout_Ms() {
	return (SysTick->CNT - startMS > delayMS);
}
