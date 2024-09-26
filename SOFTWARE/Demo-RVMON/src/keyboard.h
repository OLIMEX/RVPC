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

#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <ch32v00x.h>

#define KBD_USE_BUZZ

#define KBD_PERIPHERY     (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD)
#define KBD_CLOCK_PORT    GPIOD
#define KBD_CLOCK_PIN     GPIO_Pin_1

#define KBD_INT_PORT_SRC  GPIO_PortSourceGPIOD
#define KBD_INT_PIN_SRC   GPIO_PinSource1
#define KBD_INT_LINE      EXTI_Line1

#define KBD_DATA_PORT     GPIOA
#define KBD_DATA_PIN      GPIO_Pin_2

#define BUZZ_PORT         GPIOC
#define BUZZ_PIN          GPIO_Pin_4

void buzz(uint32_t hz, uint32_t timeMS);
void kbd_init();
uint32_t kbd_read();

uint32_t kbd_wait();
uint32_t kbd_wait_press();
uint32_t kbd_wait_release();

#endif // __KEYBOARD_H