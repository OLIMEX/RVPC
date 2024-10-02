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

#include <stdio.h>
#include <string.h>

#include "vga.h"
#include "misc.h"
#include "wait.h"
#include "keyboard.h"
#include "print.h"

void key_ok() {
	buzz(880, 10);
}

void key_err() {
	buzz(440, 50);
	buzz(220, 150);
}

uint8_t app_pending_reset = 0;

#define CHAR_TOP          1
#define CHAR_BOT          2
#define CHAR_TOP_LEFT     3
#define CHAR_TOP_RIGHT    4
#define CHAR_BOT_LEFT     5
#define CHAR_BOT_RIGHT    6
#define CHAR_LEFT         7
#define CHAR_RIGHT        8

void app_init() {
	vga_cls();
	vga_write_at(0, 0, CHAR_TOP_LEFT);
	for (uint8_t c=1; c<VGA_NUM_COLS-1; c++) {
		vga_write_at(0, c, CHAR_TOP);
	}
	vga_write_at(0, VGA_NUM_COLS-1, CHAR_TOP_RIGHT);

	for (uint8_t r=1; r<VGA_NUM_ROWS-1; r++) {
		vga_write_at(r, 0, CHAR_LEFT);
		vga_write_at(r, VGA_NUM_COLS-1, CHAR_RIGHT);
	}

	vga_write_at(VGA_NUM_ROWS-1, 0, CHAR_BOT_LEFT);
	for (uint8_t c=1; c<VGA_NUM_COLS-1; c++) {
		vga_write_at(VGA_NUM_ROWS-1, c, CHAR_BOT);
	}
	vga_write_at(VGA_NUM_ROWS-1, VGA_NUM_COLS-1, CHAR_BOT_RIGHT);

	vga_printf_at(1, 1, "%d x %d @ %dHz", VGA_NUM_COLS, VGA_NUM_ROWS, VGA_REFRESH);

	for (uint8_t r=0; r<VGA_NUM_ROWS; r++) {
		vga_printf_at(r, VGA_NUM_COLS-4, "%02d", r);
	}
	for (uint8_t c=0; c<VGA_NUM_COLS; c++) {
		vga_printf_at(VGA_NUM_ROWS-5, c, "%d", c / 10);
		vga_printf_at(VGA_NUM_ROWS-4, c, "%d", c % 10);
	}

	vga_print_at(VGA_NUM_ROWS-5, VGA_NUM_COLS-4, "  ");
	vga_print_at(VGA_NUM_ROWS-4, VGA_NUM_COLS-4, "  ");

	vga_cursor_set(VGA_NUM_ROWS / 2 - 2, 1);

	buzz_ok();
}

void app_run() {
	static uint8_t shift = 0;
	uint32_t key_code = kbd_wait();
	switch (key_code) {
		case 0x12:    // Left shift
		case 0x59:    // Right shift
			shift = 1;
		break;

		case 0xF012:  // Left shift released
		case 0xF059:  // Right shift released
			shift = 0;
		break;
	}

	uint8_t kbd_char = kbd_to_ascii(key_code);
	if (kbd_char == 0) {
		kbd_char = ' ';
	} else if (shift) {
		if (kbd_char >= 'a' && kbd_char <= 'z') {
			kbd_char -= ('a'-'A');
		}
	}
	vga_printf_at(vga_cursor_row(), vga_cursor_col(), "%c", kbd_char);
	vga_printf_at(vga_cursor_row(), vga_cursor_col() + 5, "0x%08lX", key_code);
}

uint8_t app_reset() {
	if (app_pending_reset != 0) {
		app_pending_reset = 0;
		return 1;
	}
	return 0;
}
