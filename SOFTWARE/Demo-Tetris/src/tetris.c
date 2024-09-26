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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "keyboard.h"
#include "vga.h"

#define TETRIS_NUM_PIECES       7

#define TETRIS_SPRITE_ROWS      4
#define TETRIS_SPRITE_COLS      4

#define TETRIS_ROTATION_STATES  4
#define TETRIS_INITIAL_X        6

#define TETRIS_BIN_SPARE_TOP    4
#define TETRIS_BIN_SPARE_BOT    3

#define TETRIS_BIN_ROWS        ((VGA_NUM_ROWS - 2) + TETRIS_BIN_SPARE_TOP + TETRIS_BIN_SPARE_BOT)
#define TETRIS_BIN_COLUMNS     10

#define TETRIS_BIN_START_ROW   TETRIS_BIN_SPARE_TOP
#define TETRIS_BIN_END_ROW     (TETRIS_BIN_ROWS - TETRIS_BIN_SPARE_BOT)

#define TETRIS_BIN_FIRST_ROW    1
#define TETRIS_BIN_FIRST_COL    0

#define TETRIS_CHAR_EMPTY      0x20
#define TETRIS_CHAR_PIECE      0x02

#define TETRIS_CHAR_SOLID      0x01
#define TETRIS_CHAR_BORDER_H   0x03
#define TETRIS_CHAR_BORDER_V   0x04
#define TETRIS_CHAR_BORDER_BL  0x05
#define TETRIS_CHAR_BORDER_BR  0x06

#define TETRIS_MIN_SPEED        1
#define TETRIS_MAX_SPEED       10
#define TETRIS_SPEED_UP_LINES  10

uint8_t tetris_speed = TETRIS_MIN_SPEED;
uint8_t tetris_refreshes = 0;
uint8_t tetris_refreshes_limit = TETRIS_MAX_SPEED - TETRIS_MIN_SPEED + 1;

uint16_t tetris_score = 0;
uint16_t tetris_best = 0;

// Frame rate is 75 Hz, with pixel clock at 12 MHz
#define APP_SCREEN_FPS         75
#define APP_STARTUP_DELAY      (APP_SCREEN_FPS*6)        // 6 seconds
#define APP_REFRESH_DELAY      (APP_SCREEN_FPS/20)       // 1/20 second

uint16_t app_delay = APP_REFRESH_DELAY;
uint8_t  app_pause = 0;

const uint16_t TETRIS_PIECES[TETRIS_NUM_PIECES][TETRIS_SPRITE_ROWS] = {
	{
		0b0000010000000100,
		0b1111010011110100,
		0b0000010000000100,
		0b0000010000000100
	},
	{
		0b0000011000000010,
		0b1110010010000010,
		0b0010010011100110,
		0b0000000000000000
	},
	{
		0b0000011000000100,
		0b0010001011100100,
		0b1110001010000110,
		0b0000000000000000
	},
	{
		0b0000001000000100,
		0b0100011011100110,
		0b1110001001000100,
		0b0000000000000000
	},
	{
		0b0000001000000010,
		0b1100011011000110,
		0b0110010001100100,
		0b0000000000000000
	},
	{
		0b0000010000000100,
		0b0110011001100110,
		0b1100001011000010,
		0b0000000000000000
	},
	{
		0b0000000000000000,
		0b0110011001100110,
		0b0110011001100110,
		0b0000000000000000
	}
};

const uint16_t TETRIS_BIN_INIT[TETRIS_BIN_ROWS] = {
	0b1110000000000111,    // Spare top rows
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,

	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1110000000000111,
	0b1111111111111111,    // Spare bottom rows
	0b1111111111111111,
	0b1111111111111111,
};

typedef struct {
	uint8_t i;
	uint8_t r;
	uint8_t x;
	uint8_t y;
	uint16_t sprite[TETRIS_SPRITE_ROWS];
} TetrisPiece;

static TetrisPiece tetris_current_piece, tetris_next_piece;
static uint16_t tetris_bin[TETRIS_BIN_ROWS];
static uint16_t tetris_pieces_count = 0;

void splash() {
	// Clear screen
	vga_cls();

	uint8_t row = 0;
	vga_print_at(row++, 0, "===== RVPC Demo =====");
	vga_print_at(row++, 0, "VGA by Curtis Whitley");
	row += 2;
	vga_print_at(row,   7, "TETRIS");
	row += 3;
	vga_print_at(row++, 0, "<Up>    Rotate");
	vga_print_at(row++, 0, "<Left>  Move LEFT");
	vga_print_at(row++, 0, "<Right> Move RIGHT");
	vga_print_at(row++, 0, "<Down>  Drop");
	row++;
	vga_print_at(row++, 0, "<Space> Pause / Play");
	vga_print_at(row++, 0, "<Esc>   Reset");

	vga_print_at(VGA_NUM_ROWS-1, 0, "Press a key to start");
	
	kbd_wait_release();
	vga_cls();
	srand(SysTick->CNT);
}

#define PIECE_DROP()  \
 	buzz(880, 10)

#define LINE_DROP()   \
	buzz( 698, 50);   \
	buzz( 880, 50);   \
	buzz(1047, 50);   \
	buzz(1397, 50)

void piece_update_sprite(TetrisPiece* piece) {
	for (uint8_t row=0; row<TETRIS_SPRITE_ROWS; row++) {
		piece->sprite[row] = (TETRIS_PIECES[piece->i][row] << (piece->r * TETRIS_SPRITE_COLS)) & 0b1111000000000000;
		piece->sprite[row] >>= piece->x;
	}
}

void piece_init(TetrisPiece* piece) {
	piece->i = rand() % TETRIS_NUM_PIECES;
	piece->r = rand() % TETRIS_ROTATION_STATES;
	piece->x = TETRIS_INITIAL_X;
	piece->y = 0;
	piece_update_sprite(piece);
}

void piece_copy(TetrisPiece* dst, TetrisPiece* src) {
	memcpy(dst, src, sizeof(TetrisPiece));
}

void piece_move_left(TetrisPiece* piece) {
	piece->x--;
	piece_update_sprite(piece);
}

void piece_move_right(TetrisPiece* piece) {
	piece->x++;
	piece_update_sprite(piece);
}

void piece_move_down(TetrisPiece* piece) {
	piece->y++;
}

void piece_rotate(TetrisPiece* piece) {
	piece->r = (piece->r + 1) % TETRIS_ROTATION_STATES;
	piece_update_sprite(piece);
}

uint8_t piece_collision(TetrisPiece* piece, uint16_t* bin) {
	for (uint8_t row=0; row<TETRIS_SPRITE_ROWS; row++) {
		if (piece->sprite[row] & bin[piece->y + row]) {
			return 1;
		}
	}
	return 0;
}

void tetris_speed_up() {
	if (tetris_refreshes_limit > 0) {
		tetris_speed++;
		tetris_refreshes_limit--;
	}
}

void piece_to_bin(TetrisPiece* piece, uint16_t* bin) {
	tetris_pieces_count++;

	for (uint8_t row=0; row<TETRIS_SPRITE_ROWS; row++) {
		bin[piece->y + row] |= piece->sprite[row];
	}
	
	// Check for completed rows
	uint8_t found_completed = 0;
	for (uint8_t row=0; row<TETRIS_SPRITE_ROWS; row++) {
		if (bin[piece->y + row] == 0xFFFF && piece->sprite[row] != 0) {
			found_completed = 1;
			tetris_score++;
			if ((tetris_score % TETRIS_SPEED_UP_LINES) == 0) {
				tetris_speed_up();
			}

			for (uint8_t r = (piece->y + row); r > 0; r--) {
				bin[r] = bin[r-1];
			}
		}
	}

	if (found_completed) {
		LINE_DROP();
	}
}

void piece_drop(TetrisPiece* piece, uint16_t* bin) {
	while (1) {
		piece_move_down(piece);
		if (piece_collision(piece, bin)) {
			piece->y--;
			return;
		}
	}
}

void tetris_status() {
	static uint8_t paused = 0;
	if (app_pause) {
		vga_print_at(TETRIS_BIN_FIRST_ROW - 1, TETRIS_BIN_FIRST_COL, "   PAUSED   ");
	} else {
		if (tetris_pieces_count == 0 || paused != 0) {
			vga_print_at(TETRIS_BIN_FIRST_ROW - 1, TETRIS_BIN_FIRST_COL, "   Tetris   ");
		}
	}
	paused = app_pause;

	uint8_t row = TETRIS_BIN_FIRST_ROW - 1;
	uint8_t col = TETRIS_BIN_FIRST_COL + TETRIS_BIN_COLUMNS + 3;
	vga_printf_at (row++, col, "Speed %2d", tetris_speed);
	row++;
	vga_print_at (row++, col, "Blocks");
	vga_printf_at(row++, col, "%8d", tetris_pieces_count+1);
	row++;
	vga_print_at (row++, col, "Score");
	vga_printf_at(row++, col, "%8d", tetris_score);

	vga_print_at (VGA_NUM_ROWS - 2, col, "BEST");
	vga_printf_at(VGA_NUM_ROWS - 1, col, "%8d", tetris_best);
}

void tetris_display() {
	int8_t row_offset = TETRIS_BIN_FIRST_ROW - TETRIS_BIN_START_ROW;
	for (uint8_t row=TETRIS_BIN_START_ROW; row<TETRIS_BIN_END_ROW; row++) {
		vga_write_at(row + row_offset, TETRIS_BIN_FIRST_COL, TETRIS_CHAR_BORDER_V);
		vga_write_at(row + row_offset, TETRIS_BIN_FIRST_COL + TETRIS_BIN_COLUMNS + 1, TETRIS_CHAR_BORDER_V);
	}
	
	vga_write_at(TETRIS_BIN_END_ROW + row_offset, TETRIS_BIN_FIRST_COL, TETRIS_CHAR_BORDER_BL);
	for (uint8_t col=0; col<TETRIS_BIN_COLUMNS; col++) {
		vga_write_at(TETRIS_BIN_END_ROW + row_offset, TETRIS_BIN_FIRST_COL + col + 1, TETRIS_CHAR_BORDER_H);
	}
	vga_write_at(TETRIS_BIN_END_ROW + row_offset, TETRIS_BIN_FIRST_COL + TETRIS_BIN_COLUMNS + 1, TETRIS_CHAR_BORDER_BR);
}

void tetris_refresh() {
	for (uint8_t row=TETRIS_BIN_START_ROW; row<TETRIS_BIN_END_ROW; row++) {
		uint16_t data = tetris_bin[row];
		if (row >= tetris_current_piece.y && row < tetris_current_piece.y + TETRIS_SPRITE_ROWS) {
			data |= tetris_current_piece.sprite[row - tetris_current_piece.y];
		}

		data <<= 3;
		for (uint8_t col=0; col<TETRIS_BIN_COLUMNS; col++) {
			uint8_t d = (data & 0b1000000000000000) ?
				TETRIS_CHAR_PIECE
				:
				TETRIS_CHAR_EMPTY
			;

			vga_write_at(row - TETRIS_BIN_START_ROW + TETRIS_BIN_FIRST_ROW, col + TETRIS_BIN_FIRST_COL + 1, d);

			data <<= 1;
		}
	}

	tetris_status();
}

void tetris_init() {
	// Bin init
	memcpy(&tetris_bin, &TETRIS_BIN_INIT, sizeof(TETRIS_BIN_INIT));

	// Current piece
	piece_init(&tetris_current_piece);

	tetris_display();
	
	if (tetris_score > tetris_best) {
		tetris_best = tetris_score;
	}

	tetris_pieces_count = 0;
	tetris_score = 0;

	tetris_speed = TETRIS_MIN_SPEED;
	tetris_refreshes = 0;
	tetris_refreshes_limit = TETRIS_MAX_SPEED - TETRIS_MIN_SPEED + 1;
	
	app_delay = APP_REFRESH_DELAY;
	app_pause = 0;
}

void initialize_application() {
	kbd_init();
	splash();
	tetris_init();
}

void run_keyboard_state_machine() {
	uint32_t kbd_code = kbd_read();
	if (kbd_code == 0) {
		return;
	}

	TetrisPiece* current = &tetris_current_piece;
	TetrisPiece* next = &tetris_next_piece;
	uint16_t* bin = tetris_bin;

	piece_copy(next, current);
	
	switch (kbd_code) {
		case 0x43:   // I pressed
		case 0xE075: // Up pressed
			piece_rotate(next);
		break;

		case 0x42:   // K pressed
		case 0xE072: // Down pressed
			piece_drop(next, bin);
			PIECE_DROP();
		break;

		case 0x3B:   // J pressed
		case 0xE06B: // Left pressed
			piece_move_left(next);
		break;

		case 0x4B:   // L pressed
		case 0xE074: // Right pressed
			piece_move_right(next);
		break;

		case 0xF029: // Space released
			app_pause ^= 1;
			tetris_status();
		break;

		case 0xF076: // Esc released
			splash();
			tetris_init();
			return;
		break;

		default:
			return;
	}

	if (piece_collision(next, bin) || app_pause != 0) {
		return;
	}

	piece_copy(current, next);
	tetris_refresh();
}

void run_app_state_machine() {
	if (app_delay) {
		app_delay--;
		return;
	}
	app_delay = APP_REFRESH_DELAY;

	if (app_pause) {
		return;
	}

	tetris_refreshes++;
	if (tetris_refreshes < tetris_refreshes_limit) {
		return;		
	}

	tetris_refreshes = 0;

	TetrisPiece* current = &tetris_current_piece;
	TetrisPiece* next = &tetris_next_piece;
	uint16_t* bin = tetris_bin;

	piece_copy(next, current);
	piece_move_down(next);

	if (piece_collision(next, bin)) {
		piece_to_bin(current, bin);
		piece_init(current);
		if (piece_collision(current, bin)) {
			tetris_init();
			app_delay = APP_STARTUP_DELAY;
			vga_print_at(TETRIS_BIN_FIRST_ROW - 1, TETRIS_BIN_FIRST_COL, "  GAME OVER ");
			return;
		}
	} else {
		piece_copy(current, next);
	}
	
	tetris_refresh();
}
