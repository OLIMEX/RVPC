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

#ifndef __MISC_H
#define __MISC_H

#include <ch32v00x.h>

// Timers
void GPIO_Config(GPIO_TypeDef *GPIO_port, uint16_t GPIO_pin, GPIOMode_TypeDef GPIO_mode);
void GPIO_Interrupt(uint8_t GPIO_PortSource, uint8_t GPIO_PinSource, uint32_t GPIO_Line, EXTITrigger_TypeDef GPIO_Trigger);

// Timers
void Timer_Config(TIM_TypeDef *TIM, uint16_t period, uint16_t prescaler, uint16_t mode);
void Slave_Timer_Config(TIM_TypeDef *TIM_Slave, TIM_TypeDef *TIM_Master, uint16_t TIM_TRGOSource, uint16_t period, uint16_t mode);
void Timer_Interrupt(TIM_TypeDef *TIM);

// PWM
void PWM_Config(TIM_TypeDef *TIM, uint8_t channel, uint16_t pulse, uint16_t mode);

#endif // __MISC_H