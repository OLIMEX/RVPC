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

#ifndef	__WAIT_H
#define	__WAIT_H

#include <ch32v00x.h>

#ifdef __cplusplus
extern "C" {
#endif

void Wait_Init();

void Wait_Us(uint32_t delay);

void Wait_Ms(uint32_t delay);

static inline void Wait_Ticks(uint32_t delay) {
    static volatile uint32_t start;
	start = SysTick->CNT;
	while (SysTick->CNT - start < delay);
}

void setTimeout_Ms(uint32_t timeoutMS);
uint8_t isTimeout_Ms();

#ifdef __cplusplus
}
#endif

#endif // __WAIT_H