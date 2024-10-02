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
#include <stdarg.h>

#include "print.h"
#include "vga.h"

void cursor_down() {
	uint8_t row = vga_cursor_row();
	if (row < VGA_NUM_ROWS - 1) {
		row++;
	} else {
		vga_scroll_up();
	}
	vga_cursor_set(row, 0);
}

void write(char c) {
	uint8_t col = vga_cursor_col();
	uint8_t row = vga_cursor_row();
	vga_write_at(row, col, c);
	if (col < VGA_NUM_COLS - 1) {
		vga_cursor_set(row, col+1);
	}
}

void print(const char *str) {
	uint8_t col = vga_cursor_col();
	uint8_t row = vga_cursor_row();
	uint8_t len = strlen(str);
	
	vga_print_at(row, col, str);
	
	col += len;
	if (col >= VGA_NUM_COLS) {
		col = VGA_NUM_COLS - 1;
	}
	vga_cursor_set(row, col);
}

void println(const char *str) {
	print(str);
	cursor_down();
}

void printfstr(const char* format, ...) {
	va_list argptr;
	char tmp[VGA_NUM_COLS + 2];

	va_start(argptr, format);
	vsnprintf(tmp, sizeof(tmp), format, argptr);
	va_end(argptr);

	print(tmp);
}

void printfln(const char* format, ...) {
	va_list argptr;
	char tmp[VGA_NUM_COLS + 2];

	va_start(argptr, format);
	vsnprintf(tmp, sizeof(tmp), format, argptr);
	va_end(argptr);

	println(tmp);
}
