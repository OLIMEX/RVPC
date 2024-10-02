/**
 * This file is part of the RVPC project 
 * https://github.com/OLIMEX/RVPC
 * https://www.olimex.com/Products/Retro-Computers/RVPC/open-source-hardware
 * 
 * Copyright (c) 2024 Olimex Ltd
 * Copyright (c) 2024 Curtis Whitley
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

#ifndef __VGA_H
#define __VGA_H

#include <ch32v00x.h>

#include "chardefs.h"

#if !defined(VGA_MODE_31x18_56Hz) && !defined(VGA_MODE_23x18_75Hz) && !defined(VGA_MODE_31x36_56Hz) && !defined(VGA_MODE_23x36_75Hz)
#define VGA_MODE_23x18_75Hz
#endif

#if defined(VGA_MODE_31x18_56Hz)
#define VGA_REFRESH             56
#define VGA_QUAD_SCAN
#define VGA_NUM_COLS            31
#elif defined(VGA_MODE_23x18_75Hz)
#define VGA_REFRESH             75
#define VGA_QUAD_SCAN
#define VGA_NUM_COLS            23
#elif defined(VGA_MODE_31x36_56Hz)
#define VGA_REFRESH             56
#define VGA_DUAL_SCAN
#define VGA_NUM_COLS            31
#elif defined(VGA_MODE_23x36_75Hz)
#define VGA_REFRESH             75
#define VGA_DUAL_SCAN
#define VGA_NUM_COLS            23
#endif

#if !defined(VGA_QUAD_SCAN) && !defined(VGA_DUAL_SCAN)
#define VGA_QUAD_SCAN
#endif

#if defined(VGA_QUAD_SCAN)
#define VGA_NUM_ROWS            18
#define VGA_VACTIVE_LINES       (VGA_NUM_ROWS * CHAR_HEIGHT * 4)
#elif defined(VGA_DUAL_SCAN)
#define VGA_NUM_ROWS            36
#define VGA_VACTIVE_LINES       (VGA_NUM_ROWS * CHAR_HEIGHT * 2)
#endif

#define VGA_FRAME_LINES         (VGA_NUM_ROWS * CHAR_HEIGHT)

#define VGA_STATE_BEGIN_FRAME    0
#define VGA_STATE_IN_FRAME       1
#define VGA_STATE_END_FRAME      2

#define VGA_DATA_GPIO           GPIOC
#define VGA_DATA_PIN            GPIO_Pin_2

#define VGA_HSYNC_GPIO          GPIOC
#define VGA_HSYNC_PIN           GPIO_Pin_1
#define VGA_HSYNC_TIM           TIM2
#define VGA_HSYNC_CH            4

#define VGA_VSYNC_GPIO          GPIOA
#define VGA_VSYNC_PIN           GPIO_Pin_1
#define VGA_VSYNC_TIM           TIM1
#define VGA_VSYNC_CH            2

#ifdef SYSCLK_FREQ_48MHZ_HSI
#define VGA_CLOCK_PRESCALER     4 // 48MHz / 4 = 12MHz
#if defined(VGA_MODE_31x18_56Hz) || defined(VGA_MODE_31x36_56Hz)
#define VGA_TICK_CNT_DIVIDER    3 // Converts 36MHz tick counts to 12MHz tick counts
#elif defined(VGA_MODE_23x18_75Hz) || defined(VGA_MODE_23x36_75Hz)
#define VGA_TICK_CNT_DIVIDER    4 // Converts 48MHz tick counts to 12MHz tick counts
#endif
#define VGA_TIMER_OC_MODE       TIM_OCMode_PWM1
#else
#define VGA_CLOCK_PRESCALER     2 // 24MHz / 2 = 12MHz
#define VGA_TICK_CNT_DIVIDER    3 // Converts 36MHz tick counts to 12MHz tick counts
#define VGA_TIMER_OC_MODE       TIM_OCMode_PWM1
#endif

#define VGA_HACTIVE_PIXELS      800
#define VGA_HSYNC_PERIOD        ((1024)/VGA_TICK_CNT_DIVIDER)
#define VGA_HSYNC_PULSE           ((72)/VGA_TICK_CNT_DIVIDER)

#define VGA_VSYNC_PERIOD        625
#define VGA_VSYNC_PULSE         2

#ifdef SYSCLK_FREQ_48MHZ_HSI
#if defined(VGA_MODE_31x18_56Hz) || defined(VGA_MODE_31x36_56Hz)
#define VGA_HBACK_PORCH         36
#define VGA_PREPARE_WASTE       4
#elif defined(VGA_MODE_23x18_75Hz) || defined(VGA_MODE_23x36_75Hz)
#define VGA_HBACK_PORCH         23
#define VGA_PREPARE_WASTE       3
#endif
#else
#define VGA_HBACK_PORCH         18
#endif

#define VGA_VBACK_PORCH         36 // helps with vertical positioning
#define VGA_VFRONT_PORCH        (VGA_VSYNC_PERIOD - VGA_VACTIVE_LINES - VGA_VSYNC_PULSE - VGA_VBACK_PORCH)

void vga_init();

uint8_t vga_is_frame_end();

void vga_cls();
void vga_clear_rect(uint8_t r1, uint8_t c1, uint8_t r2, uint8_t c2);

#ifdef VGA_CURSOR
void vga_cursor_hide();
void vga_cursor_set(uint8_t row, uint8_t col);
uint8_t vga_cursor_row();
uint8_t vga_cursor_col();
#endif

char vga_char_at(uint8_t row, uint8_t col);
void vga_write_at(uint8_t row, uint8_t col, char ch);
void vga_print_at(uint8_t row, uint8_t col, const char* text);

void vga_printf_at(uint8_t row, uint8_t col, const char* format, ...) __attribute__((__format__ (__printf__, 3, 4)));;

void vga_scroll_up();

#endif // __VGA_H