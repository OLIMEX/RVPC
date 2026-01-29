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

#include "vga.h"
#include "chardefs.h"
#include "misc.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

extern void waste_time(uint8_t d);

// These are the character definitions in ROM (flash memory).
//
const uint8_t vga_character_defs[CHAR_HEIGHT][CHARS_COUNT] = {
	CHARSET_LINE(0),
	CHARSET_LINE(1),
	CHARSET_LINE(2),
	CHARSET_LINE(3),
	CHARSET_LINE(4),
	CHARSET_LINE(5),
	CHARSET_LINE(6),
	CHARSET_LINE(7)
};

// These are the screen characters in RAM (text frame buffer)
uint8_t vga_screen_chars[VGA_NUM_ROWS][VGA_NUM_COLS];

volatile uint8_t vga_state = VGA_STATE_BEGIN_FRAME;
volatile uint8_t vga_vsync = 0;

typedef struct {
	uint8_t row;
	uint8_t col;
	uint16_t line;
} vga_cursor_t;

volatile vga_cursor_t vga_cursor_pos = {
	.row = 0xFF, 
	.col = 0xFF, 
	.line = 0xFFFF
};

/*********************************************************************
 * @fn    vga_init
 * @brief Initializes VGA hardware
 */
void vga_init() {
	// VGA GPIOs Config
	GPIO_Config(VGA_DATA_GPIO,  VGA_DATA_PIN,  GPIO_Mode_Out_PP);
	GPIO_Config(VGA_HSYNC_GPIO, VGA_HSYNC_PIN, GPIO_Mode_AF_PP);
	GPIO_Config(VGA_VSYNC_GPIO, VGA_VSYNC_PIN, GPIO_Mode_AF_PP);

	// HSync
	// Timer Config
	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
	Timer_Config(VGA_HSYNC_TIM, VGA_HSYNC_PERIOD, (VGA_CLOCK_PRESCALER - 1), TIM_CounterMode_Up);
	// PWM Config
	PWM_Config(VGA_HSYNC_TIM, VGA_HSYNC_CH, VGA_HSYNC_PULSE, VGA_TIMER_OC_MODE);
	// Interrupt Config
	Timer_Interrupt(VGA_HSYNC_TIM);

	// VSync
	// Slave Timer Config
	GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM1, ENABLE);
	Slave_Timer_Config(VGA_VSYNC_TIM, VGA_HSYNC_TIM, TIM_TRGOSource_OC4Ref, VGA_VSYNC_PERIOD, TIM_CounterMode_Up);
	// PWM Config
	PWM_Config(VGA_VSYNC_TIM, VGA_VSYNC_CH, VGA_VSYNC_PULSE, VGA_TIMER_OC_MODE);
	// Interrupt Config
	Timer_Interrupt(VGA_VSYNC_TIM);

	// Start timers
	TIM_Cmd(VGA_VSYNC_TIM, ENABLE);
	TIM_Cmd(VGA_HSYNC_TIM, ENABLE);
}

/*********************************************************************
 * @fn    vga_is_frame_end
 * @brief Check if VGA frame is ended
 */
inline uint8_t vga_is_frame_end() {
	uint8_t res = (vga_vsync == 1 && vga_state == VGA_STATE_END_FRAME);
	vga_vsync = 0;
	return res;
}

/*********************************************************************
 * @fn    vga_cls
 * @brief Clear VGA screen
 */
void vga_cls() {
	memset(vga_screen_chars, ' ', sizeof(vga_screen_chars));
}

/*********************************************************************
 * @fn    vga_clear_rect
 * @brief Clear VGA rectangle
 * @param r1 - from row
 * @param c1 - from col
 * @param r2 - to row
 * @param c2 - to col
 */
void vga_clear_rect(uint8_t r1, uint8_t c1, uint8_t r2, uint8_t c2) {
	if (r1 > r2 || c1 > c2) {
		return;
	}
	for (uint8_t r=r1; r<=r2; r++) {
		memset(vga_screen_chars[r], 0x20, (c2 - c1 + 1));
	}
}

/*********************************************************************
 * @fn    vga_cursor_hide
 * @brief Hide the blinking cursor
 */
void vga_cursor_hide() {
	vga_cursor_pos.row = 0xFF;
	vga_cursor_pos.col = 0xFF;
	vga_cursor_pos.line = 0xFFFF;
}

/*********************************************************************
 * @fn    vga_cursor_set
 * @brief Show the blinking cursor at positon (row, col)
 * @param row - position row
 * @param col - position col
 */
void vga_cursor_set(uint8_t row, uint8_t col) {
	if (row < VGA_NUM_ROWS && col < VGA_NUM_COLS) {
		vga_cursor_pos.row = row;
		vga_cursor_pos.col = col;
		vga_cursor_pos.line = (row << 3) + 7;
	} else {
		vga_cursor_hide();
	}
}

/*********************************************************************
 * @fn    vga_cursor_row
 * @brief Get cursor position row
 */
uint8_t vga_cursor_row() {
	return vga_cursor_pos.row;
}

/*********************************************************************
 * @fn    vga_cursor_col
 * @brief Get cursor position col
 */
uint8_t vga_cursor_col() {
	return vga_cursor_pos.col;
}

/*********************************************************************
 * @fn    vga_char_at
 * @brief Returns char displayed at position (row, col)
 * @param row - position row
 * @param col - position col
 */
char vga_char_at(uint8_t row, uint8_t col) {
	return vga_screen_chars[row][col];
}

/*********************************************************************
 * @fn    vga_write_at
 * @brief Write char at position (row, col)
 * @param row - position row
 * @param col - position col
 * @param ch  - char to diaplay
 */
void vga_write_at(uint8_t row, uint8_t col, char ch) {
	if (row < VGA_NUM_ROWS && col < VGA_NUM_COLS) {
		vga_screen_chars[row][col] = ch;
	}
}

/*********************************************************************
 * @fn    vga_print_at
 * @brief Print text at position (row, col)
 * @param row  position row
 * @param col  position col
 * @param text pointer to null terminated text to diaplay
 */
void vga_print_at(uint8_t row, uint8_t col, const char* text) {
	if (row < VGA_NUM_ROWS && col < VGA_NUM_COLS) {
		strncpy((char *)(&vga_screen_chars[row][col]), text, strlen(text));
	}
}

/*********************************************************************
 * @fn    vga_printf_at
 * @brief Print formatted text at position (row, col)
 * @param row  position row
 * @param col  position col
 * @param format pointer to null terminated formatted text to diaplay
 */
void vga_printf_at(uint8_t row, uint8_t col, const char* format, ...) {
	va_list argptr;
	char tmp[VGA_NUM_COLS + 2];
	
	va_start(argptr, format);
	vsnprintf(tmp, sizeof(tmp), format, argptr);
	va_end(argptr);

	vga_print_at(row, col, tmp);
}

/*********************************************************************
 * @fn    vga_scroll_up
 * @brief Scroll one row up
 */
void vga_scroll_up() {
	for (uint8_t r=1; r<VGA_NUM_ROWS; r++) {
		memcpy(vga_screen_chars[r-1], vga_screen_chars[r], VGA_NUM_COLS);
	}
	memset(vga_screen_chars[VGA_NUM_ROWS - 1], ' ', VGA_NUM_COLS);
}

/*********************************************************************
 *                                                                   *
 *                    VGA hardware handling                          *
 *                                                                   *
 *********************************************************************/

static uint32_t vga_scan_line = 0;
static uint32_t frame_scan_line = 0;
static uint32_t frame_prepared_line = 0xFFFFFFFF;

// These are prepared frame scan line
uint32_t frame_line_bits[VGA_NUM_COLS];

inline static void vga_prepare_line(uint32_t line) {
	static const uint8_t* char_defs = NULL;
	static uint8_t* char_indexes = NULL;

	if (frame_prepared_line == line) {
		return;
	}

	char_defs    = vga_character_defs[line & CHAR_HEIGHT_MASK]; // point to fixed array of scan line words
	char_indexes = vga_screen_chars[line >> CHAR_HEIGHT_BITS];  // point to array of character codes (text line)

	#ifdef VGA_CURSOR
	static uint32_t MASK = 0;
	MASK = 0;
	if (line == vga_cursor_pos.line) {
		if (SysTick->CNT & (1 << 24)) {   
			MASK = (0x00FF00FF << 2);
		}
	} else {
		waste_time(VGA_PREPARE_WASTE);
	}

	frame_line_bits[ 0] = char_defs[char_indexes[ 0] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col ==  0 ? MASK : 0);
	frame_line_bits[ 1] = char_defs[char_indexes[ 1] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col ==  1 ? MASK : 0);
	frame_line_bits[ 2] = char_defs[char_indexes[ 2] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col ==  2 ? MASK : 0);
	frame_line_bits[ 3] = char_defs[char_indexes[ 3] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col ==  3 ? MASK : 0);
	frame_line_bits[ 4] = char_defs[char_indexes[ 4] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col ==  4 ? MASK : 0);
	frame_line_bits[ 5] = char_defs[char_indexes[ 5] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col ==  5 ? MASK : 0);
	frame_line_bits[ 6] = char_defs[char_indexes[ 6] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col ==  6 ? MASK : 0);
	frame_line_bits[ 7] = char_defs[char_indexes[ 7] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col ==  7 ? MASK : 0);
	frame_line_bits[ 8] = char_defs[char_indexes[ 8] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col ==  8 ? MASK : 0);
	frame_line_bits[ 9] = char_defs[char_indexes[ 9] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col ==  9 ? MASK : 0);
	frame_line_bits[10] = char_defs[char_indexes[10] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 10 ? MASK : 0);
	frame_line_bits[11] = char_defs[char_indexes[11] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 11 ? MASK : 0);
	frame_line_bits[12] = char_defs[char_indexes[12] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 12 ? MASK : 0);
	frame_line_bits[13] = char_defs[char_indexes[13] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 13 ? MASK : 0);
	frame_line_bits[14] = char_defs[char_indexes[14] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 14 ? MASK : 0);
	frame_line_bits[15] = char_defs[char_indexes[15] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 15 ? MASK : 0);
	frame_line_bits[16] = char_defs[char_indexes[16] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 16 ? MASK : 0);
	frame_line_bits[17] = char_defs[char_indexes[17] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 17 ? MASK : 0);
	frame_line_bits[18] = char_defs[char_indexes[18] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 18 ? MASK : 0);
	frame_line_bits[19] = char_defs[char_indexes[19] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 19 ? MASK : 0);
	frame_line_bits[20] = char_defs[char_indexes[20] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 20 ? MASK : 0);
	frame_line_bits[21] = char_defs[char_indexes[21] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 21 ? MASK : 0);
	frame_line_bits[22] = char_defs[char_indexes[22] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 22 ? MASK : 0);
	#if VGA_NUM_COLS > 23
	frame_line_bits[23] = char_defs[char_indexes[23] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 23 ? MASK : 0);
	frame_line_bits[24] = char_defs[char_indexes[24] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 24 ? MASK : 0);
	frame_line_bits[25] = char_defs[char_indexes[25] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 25 ? MASK : 0);
	frame_line_bits[26] = char_defs[char_indexes[26] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 26 ? MASK : 0);
	frame_line_bits[27] = char_defs[char_indexes[27] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 27 ? MASK : 0);
	frame_line_bits[28] = char_defs[char_indexes[28] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 28 ? MASK : 0);
	frame_line_bits[29] = char_defs[char_indexes[29] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 29 ? MASK : 0);
	frame_line_bits[30] = char_defs[char_indexes[30] & CHARS_COUNT_MASK] ^ (vga_cursor_pos.col == 30 ? MASK : 0);
	#endif
	#else
	frame_line_bits[ 0] = char_defs[char_indexes[ 0] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[ 1] = char_defs[char_indexes[ 1] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[ 2] = char_defs[char_indexes[ 2] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[ 3] = char_defs[char_indexes[ 3] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[ 4] = char_defs[char_indexes[ 4] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[ 5] = char_defs[char_indexes[ 5] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[ 6] = char_defs[char_indexes[ 6] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[ 7] = char_defs[char_indexes[ 7] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[ 8] = char_defs[char_indexes[ 8] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[ 9] = char_defs[char_indexes[ 9] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[10] = char_defs[char_indexes[10] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[11] = char_defs[char_indexes[11] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[12] = char_defs[char_indexes[12] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[13] = char_defs[char_indexes[13] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[14] = char_defs[char_indexes[14] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[15] = char_defs[char_indexes[15] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[16] = char_defs[char_indexes[16] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[17] = char_defs[char_indexes[17] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[18] = char_defs[char_indexes[18] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[19] = char_defs[char_indexes[19] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[20] = char_defs[char_indexes[20] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[21] = char_defs[char_indexes[21] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
	frame_line_bits[22] = char_defs[char_indexes[22] & CHARS_COUNT_MASK] << 2 | 0xfffffc00;
#if VGA_NUM_COLS > 23
	frame_line_bits[23] = char_defs[char_indexes[23] & CHARS_COUNT_MASK];
	frame_line_bits[24] = char_defs[char_indexes[24] & CHARS_COUNT_MASK];
	frame_line_bits[25] = char_defs[char_indexes[25] & CHARS_COUNT_MASK];
	frame_line_bits[26] = char_defs[char_indexes[26] & CHARS_COUNT_MASK];
	frame_line_bits[27] = char_defs[char_indexes[27] & CHARS_COUNT_MASK];
	frame_line_bits[28] = char_defs[char_indexes[28] & CHARS_COUNT_MASK];
	frame_line_bits[29] = char_defs[char_indexes[29] & CHARS_COUNT_MASK];
	frame_line_bits[30] = char_defs[char_indexes[30] & CHARS_COUNT_MASK];
	#endif
	#endif

	frame_prepared_line = line;
}

#define WRITE_GLYPH_LINE(offset)   \
	"c.lw   a3," #offset "(a2) \n"    /* load glyph scan line bits for column N */ \
	"and    a4,a3,t0 \n"              /* mask video-out pin                     */ \
	"c.sw   a4,0(a1) \n"              /* write to video-out pin                 */ \
	"c.srli a3,1     \n"              /* shift next bit into pin position       */ \
	"and    a4,a3,t0 \n"           \
	"c.sw   a4,0(a1) \n"           \
	"c.srli a3,1     \n"           \
	"and    a4,a3,t0 \n"           \
	"c.sw   a4,0(a1) \n"           \
	"c.srli a3,1     \n"           \
	"and    a4,a3,t0 \n"           \
	"c.sw   a4,0(a1) \n"           \
	"c.srli a3,1     \n"           \
	"and    a4,a3,t0 \n"           \
	"c.sw   a4,0(a1) \n"           \
	"c.srli a3,1     \n"           \
	"and    a4,a3,t0 \n"           \
	"c.sw   a4,0(a1) \n"           \
	"c.srli a3,1     \n"           \
	"and    a4,a3,t0 \n"           \
	"c.sw   a4,0(a1) \n"           \
	"c.srli a3,1     \n"           \
	"and    a4,a3,t0 \n"           \
	"c.sw   a4,0(a1) \n"           \

void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void) {
	// Hsync code
	VGA_DATA_GPIO->BCR = VGA_DATA_PIN;

	vga_scan_line = VGA_VSYNC_TIM->CNT;

	if (vga_scan_line < VGA_VBACK_PORCH) {
		vga_state = VGA_STATE_END_FRAME;
		goto done;
	}

	vga_scan_line -= VGA_VBACK_PORCH;
	if (vga_scan_line < 2) {
		frame_scan_line = 0;
		goto prepare;
	}

	if (vga_scan_line > VGA_VACTIVE_LINES)  {
		vga_state = VGA_STATE_END_FRAME;
		goto done;
	}

	vga_state = VGA_STATE_IN_FRAME;

	#if defined(VGA_QUAD_SCAN)
	frame_scan_line = (vga_scan_line >> 2);
	#elif defined(VGA_DUAL_SCAN)
	frame_scan_line = (vga_scan_line >> 1);
	#endif

	waste_time(VGA_HBACK_PORCH);
	__NOP();
	
	// Unroll the loop for columns and draw glyph bits
	__asm volatile (\
	"la     a2,frame_line_bits \n" // load a2(x12) with frame_line_bits array address
	"li     a1,0x40011010      \n" // load a1(x11) with BSHR address
	"li     t0,0x00040004      \n" // load mask
	WRITE_GLYPH_LINE(4*0)
	WRITE_GLYPH_LINE(4*1)
	WRITE_GLYPH_LINE(4*2)
	WRITE_GLYPH_LINE(4*3)
	WRITE_GLYPH_LINE(4*4)
	WRITE_GLYPH_LINE(4*5)
	WRITE_GLYPH_LINE(4*6)
	WRITE_GLYPH_LINE(4*7)
	WRITE_GLYPH_LINE(4*8)
	WRITE_GLYPH_LINE(4*9)
	WRITE_GLYPH_LINE(4*10)
	WRITE_GLYPH_LINE(4*11)
	WRITE_GLYPH_LINE(4*12)
	WRITE_GLYPH_LINE(4*13)
	WRITE_GLYPH_LINE(4*14)
	WRITE_GLYPH_LINE(4*15)
	WRITE_GLYPH_LINE(4*16)
	WRITE_GLYPH_LINE(4*17)
	WRITE_GLYPH_LINE(4*18)
	WRITE_GLYPH_LINE(4*19)
	WRITE_GLYPH_LINE(4*20)
	WRITE_GLYPH_LINE(4*21)
	WRITE_GLYPH_LINE(4*22)
	#if VGA_NUM_COLS > 23
	WRITE_GLYPH_LINE(4*23)
	WRITE_GLYPH_LINE(4*24)
	WRITE_GLYPH_LINE(4*25)
	WRITE_GLYPH_LINE(4*26)
	WRITE_GLYPH_LINE(4*27)
	WRITE_GLYPH_LINE(4*28)
	WRITE_GLYPH_LINE(4*29)
	WRITE_GLYPH_LINE(4*30)
	#endif
	"addi   a3,a0,4           \n" // Load video-out bit value
	"c.sw   a3,4(a1)          \n" // Clear video-out pin via BCR
	);

	prepare:
	vga_prepare_line(frame_scan_line);

	done:
	VGA_HSYNC_TIM->INTFR = (uint16_t)(~TIM_IT_Update);
}

void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void) {
	// Vsync code
	vga_vsync = 1;
	VGA_VSYNC_TIM->INTFR = (uint16_t)(~TIM_IT_Update);
}
