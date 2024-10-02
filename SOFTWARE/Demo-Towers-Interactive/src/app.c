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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "vga.h"
#include "misc.h"
#include "wait.h"
#include "keyboard.h"

// How portions the screen might look:
//
//    000000000011111111112
//    012345678901234567890
//   +---------------------+
// 00|===== RVPC Demo =====|
// 01|                     |
// 02|      /---\          |
// 03|      \_3_/          |
// 04|    P       P     P  |
// 05|    P       P     P  |
// 06|   /-\      P     P  |
// 07|   \1/      P     P  |
// 08|  /---\    /-\    P  |
// 09|  \_3_/    \1/    P  |
// 10| /-----\  /---\  /-\ |
// 11| \__5__/  \_3_/  \1/ |
// 12|/-------\/-----\/---\|
// 13|\___7___/\__5__/\_3_/|
// 14|   [=]     [=]   [=] |
// 15|    A       B     C  |
// 16|                     |
// 17| Move #5 from A to B |
//   +---------------------+
//

#define TITLE_ROW                  0
#define PEG_TOP_ROW                4
#define FIRST_RING_ROW             2
#define LAST_RING_ROW             13
#define PEG_BASE_ROW              14
#define LETTER_ROW                15
#define STATUS_ROW                (VGA_NUM_ROWS-1)

#define MOVE_COL                   6
#define FROM_COL                  14
#define TO_COL                    19
  
#define NUM_PEGS                   3
#define NUM_RINGS                  4
#define RING_HEIGHT                2
#define PEG_BASE_WIDTH             3

// Character codes:
#define CC_TOP_OF_PEG           0x00
#define CC_SHAFT_OF_PEG         0x01
#define CC_CENTER_OF_PEG_BASE   0x02
#define CC_LEFT_OF_PEG_BASE     0x03
#define CC_RIGHT_OF_PEG_BASE    0x04
#define CC_UP_LEFT_OF_RING      0x05
#define CC_UP_CENTER_OF_RING    0x06
#define CC_UP_RIGHT_OF_RING     0x07
#define CC_LW_LEFT_OF_RING      0x08
#define CC_LW_CENTER_OF_RING    0x09
#define CC_LW_RIGHT_OF_RING     0x0A

#define DIR_NONE                   0
#define DIR_UP                     1
#define DIR_RIGHT                  2
#define DIR_DOWN                   3
#define DIR_LEFT                   4

// Frame rate is 75 Hz, with pixel clock at 12 MHz
#define SCREEN_FPS      VGA_REFRESH
#define STARTUP_DELAY   (SCREEN_FPS*6)   // 6 seconds
#define SOLUTION_DELAY  (SCREEN_FPS*2)   // 2 seconds
#define MOVE_DELAY      (SCREEN_FPS/20)  // 1/20 second

typedef struct {
	uint8_t index;
	uint8_t row;
	uint8_t col;
	uint8_t start;
	uint8_t end;
	uint8_t width;
	uint8_t peg;
} Ring;

typedef struct {
	uint8_t name;
	uint8_t col;
	uint8_t count;
	uint8_t rings[NUM_RINGS];
} Peg;

typedef struct {
	uint8_t from;
	uint8_t to;
} Move;

const Ring rings_init[NUM_RINGS] = {
	{0, LAST_RING_ROW-RING_HEIGHT*3, 4, 3, 5, 3, 0 },
	{1, LAST_RING_ROW-RING_HEIGHT*2, 4, 2, 6, 5, 0 },
	{2, LAST_RING_ROW-RING_HEIGHT*1, 4, 1, 7, 7, 0 },
	{3, LAST_RING_ROW-RING_HEIGHT*0, 4, 0, 8, 9, 0 }
};

const Peg pegs_init[NUM_PEGS] = {
	{ 'A', 4, NUM_RINGS, { 3, 2, 1, 0 }},
	{ 'B', 13, 0, { 0, 0, 0, 0 }},
	{ 'C', 19, 0, { 0, 0, 0, 0 }}
};

Ring rings[NUM_RINGS];
Peg pegs[NUM_PEGS];

uint8_t start_peg = 0;
uint8_t current_move = 0;

uint8_t direction = DIR_NONE;
uint16_t delay = MOVE_DELAY;
Ring* active_ring = NULL;

Move interactive = {0};
uint8_t in_progress = 0;

uint8_t dest_row;
uint8_t dest_col;

uint8_t app_pause = 0;

void draw_peg(const Peg* peg) {
	uint8_t row = PEG_TOP_ROW;
	vga_write_at(row++, peg->col, CC_TOP_OF_PEG);
	while (row < PEG_BASE_ROW) {
		vga_write_at(row++, peg->col, CC_SHAFT_OF_PEG);
	}
	vga_write_at(row, peg->col-1, CC_LEFT_OF_PEG_BASE);
	vga_write_at(row, peg->col, CC_CENTER_OF_PEG_BASE);
	vga_write_at(row, peg->col+1, CC_RIGHT_OF_PEG_BASE);
	vga_write_at(row+1, peg->col, peg->name);
}

void draw_ring(const Ring* ring) {
	vga_write_at(ring->row-1, ring->start, CC_UP_LEFT_OF_RING);
	vga_write_at(ring->row, ring->start, CC_LW_LEFT_OF_RING);
	uint8_t col = ring->start;
	while (++col != ring->end) {
		vga_write_at(ring->row-1, col, CC_UP_CENTER_OF_RING);
		vga_write_at(ring->row, col, CC_LW_CENTER_OF_RING);
	}
	vga_write_at(ring->row-1, col, CC_UP_RIGHT_OF_RING);
	vga_write_at(ring->row, col, CC_LW_RIGHT_OF_RING);
}

void animate_frame() {
	// Fill middle section of screen with blank characters
	vga_clear_rect(1, 0, VGA_NUM_ROWS-2, VGA_NUM_COLS-1);

	// Draw pieces
	draw_peg(&pegs[0]);
	draw_peg(&pegs[1]);
	draw_peg(&pegs[2]);
	draw_ring(&rings[0]); // 1+7+1 wide
	draw_ring(&rings[1]); // 1+5+1 wide
	draw_ring(&rings[2]); // 1+3+1 wide
	draw_ring(&rings[3]); // 1+1+1 wide
}

uint8_t get_peg_footprint(const Peg* peg, uint8_t width) {
	if (peg->count) {
		Ring* ring = &rings[peg->rings[0]];
		if (ring->width > width) {
			width = ring->width;
		}
	}
	return width;
}

void adjust_peg_columns(Move* move) {
	// Because of limited screen columns, bottom-most
	// ring widths and currently moving ring width determine
	// required widths of pegs, and positions of all pegs.
	uint8_t width0 = get_peg_footprint(&pegs[0], (move->to == 0 ? active_ring->width : PEG_BASE_WIDTH));
	uint8_t width1 = get_peg_footprint(&pegs[1], (move->to == 1 ? active_ring->width : PEG_BASE_WIDTH));
	uint8_t width2 = get_peg_footprint(&pegs[2], (move->to == 2 ? active_ring->width : PEG_BASE_WIDTH));

	pegs[0].col = width0 / 2;
	pegs[1].col = width0 + width1 / 2;
	pegs[2].col = width0 + width1 + width2 / 2;

	// Relocate any rings that move with a peg
	for (uint8_t r = 0; r < NUM_RINGS; r++) {
		uint8_t old_col = rings[r].col;
		uint8_t new_col = pegs[rings[r].peg].col;
		rings[r].col = new_col;
		rings[r].start += new_col - old_col;
		rings[r].end += new_col - old_col;
	}

	// Determine where the moving ring will stop
	Peg* peg = &pegs[move->to];
	dest_row = LAST_RING_ROW - peg->count * RING_HEIGHT;
	dest_col = peg->col;
}

void start_move() {
	vga_cursor_hide();
	in_progress = 1;

	// Determine which ring to move.
	Peg* peg = &pegs[interactive.from];
	uint8_t ring_index = peg->rings[peg->count - 1];
	active_ring = &rings[ring_index];

	// Start the move.
	delay = MOVE_DELAY;
	direction = DIR_UP;
}

uint8_t success_check() {
	for (uint8_t p=0; p<NUM_PEGS; p++) {
		if (p != start_peg && pegs[p].count == NUM_RINGS) {
			start_peg = p;
			return 1;
		}
	}
	return 0;
}

void splash() {
	// Clear screen
	vga_cursor_hide();
	vga_cls();

	uint8_t row = 0;
	vga_print_at(row++, 0, "====== RVPC Demo ======");
	vga_print_at(row++, 0, "VGA by Curtis Whitley");
	row += 2;
	vga_print_at(row,   5, "Hanoi Towers");
	row += 4;
	vga_print_at(row++, 0, "A,B,C Set FROM and TO");
	vga_print_at(row++, 0, "Enter Make the move");
	vga_print_at(row++, 0, "Esc   Reset the game");

	vga_print_at(STATUS_ROW, 1, "Press a key to start");
	
	buzz_ok();

	kbd_wait_release();
}

void print_status() {
	uint8_t cursor_col = vga_cursor_col();
	vga_cursor_hide();

	vga_clear_rect(STATUS_ROW, 0, STATUS_ROW, VGA_NUM_COLS-1);
	if (app_pause) {
		vga_print_at(STATUS_ROW, 8, "PAUSED");
	} else if (success_check()) {
		vga_printf_at(STATUS_ROW, 0, "SUCCSESS in %d moves", current_move);
		current_move = 0;
		delay = STARTUP_DELAY;
	} else {
		current_move++;
		if (current_move < 100) {
			vga_print_at(STATUS_ROW, 0, "Move #   from   to   ");
			vga_printf_at(STATUS_ROW, MOVE_COL, "%d", current_move);
			vga_cursor_set(STATUS_ROW, cursor_col > VGA_NUM_COLS ? FROM_COL : cursor_col);
		} else {
			vga_print_at(STATUS_ROW, 8, "FAILURE");
		}
	}
}

void init_move() {
	interactive.from = 0xFF;
	interactive.to = 0xFF;
}

void move_done() {
	init_move();
	print_status();

	direction = DIR_NONE;
	in_progress = 0;
}

void towers_init() {
	// Init rings
	memcpy(rings, rings_init, NUM_RINGS * sizeof(Ring));
	
	// Init pegs
	memcpy(pegs, pegs_init, NUM_PEGS * sizeof(Peg));
	start_peg = 0;
	current_move = 0;

	// Init move
	init_move();

	// Prepare screen
	delay = MOVE_DELAY;
	animate_frame();
}

void initialize_application() {
	kbd_init();
	splash();
	towers_init();
}

#define KEY_OK() buzz(880, 10)

void KEY_ERR() {
	buzz(440, 50);
	buzz(220, 150);
}

uint8_t peg_check() {
	if (interactive.from == 0xFF) {
		return 1;
	}

	Peg* peg_from = &pegs[interactive.from];
	if (peg_from->count == 0) {
		KEY_ERR();
		return 0;
	}

	if (interactive.to == 0xFF) {
		return 1;
	}
	Peg* peg_to = &pegs[interactive.to];
	if (peg_to->count > 0) {
		uint8_t ring_from = peg_from->rings[peg_from->count - 1];
		uint8_t ring_to = peg_to->rings[peg_to->count - 1];

		if (ring_to < ring_from) {
			KEY_ERR();
			return 0;
		}
	}

	return 1;
}

void key_Letter(char letter) {
	if (in_progress) {
		KEY_ERR();
		return;
	}

	uint8_t target = letter - 'A';
	vga_write_at(STATUS_ROW, vga_cursor_col(), letter);

	if (vga_cursor_col() == FROM_COL) {
		interactive.from = target;
	   
		// Check
		if (peg_check() == 0) {
			return;
		}

		vga_cursor_set(STATUS_ROW, TO_COL);
	} else if (vga_cursor_col() == TO_COL) {
		interactive.to = target;

		// Check
		if (peg_check() == 0) {
			return;
		}

		vga_cursor_set(STATUS_ROW, FROM_COL);
	}

	KEY_OK();
}

void key_Escape() {
	if (in_progress == 0) {
		splash();
		towers_init();
		KEY_OK();
	} else {
		KEY_ERR();
	}
}

void key_Space() {
	if (in_progress == 1) {
		app_pause ^= 1;
		print_status();
		if (app_pause == 0) {
			vga_cursor_set(STATUS_ROW, FROM_COL);
		}
		KEY_OK();
	} else {
		KEY_ERR();
	}
}

void key_Enter() {
	if (in_progress) {
		KEY_ERR();
		return;
	}

	// Check
	if (peg_check() == 0) {
		return;
	}

	if (interactive.from == interactive.to) {
		KEY_ERR();
		return;
	}

	KEY_OK();
	start_move();
}

void run_keyboard_state_machine() {
	uint32_t kbd_code = kbd_read();
	if ((kbd_code & 0xF000) != 0xF000) {
		return;
	}

	// key released
	switch (kbd_code) {
		case 0xF029:
			key_Space();
		break;

		case 0xF01C:
			key_Letter('A');
		break;

		case 0xF032:
			key_Letter('B');
		break;

		case 0xF021:
			key_Letter('C');
		break;

		case 0xF05A:
			key_Enter();
		break;

		case 0xF076:
			key_Escape();
		break;

		default:
			KEY_ERR();
	}
}

void run_app_state_machine() {
	bool redraw;
	Move* move;

	if (app_pause) {
		return;
	}

	redraw = false;

	if (delay) {
		delay--;
	} else {
		delay = MOVE_DELAY;
		move = &interactive;

		switch (direction) {
			case DIR_NONE: {
				if (in_progress == 0) {
					if (current_move == 0) {
						print_status();
					}
				}
			} break;

			case DIR_UP: {
				if (active_ring->row == FIRST_RING_ROW) {
					Peg* peg = &pegs[move->from];
					peg->count--;
					adjust_peg_columns(move);
					if (move->from < move->to) {
						direction = DIR_RIGHT;
					} else {
						direction = DIR_LEFT;
					}
				} else {
					active_ring->row--;
					redraw = true;
				}
			} break;

			case DIR_RIGHT: {
				if (active_ring->col == dest_col) {
					direction = DIR_DOWN;
				} else {
					active_ring->col++;
					active_ring->start++;
					active_ring->end++;
					redraw = true;
				}
			} break;

			case DIR_DOWN: {
				if (active_ring->row == dest_row) {
					active_ring->peg = move->to;
					Peg* peg = &pegs[move->to];
					peg->rings[peg->count++] = active_ring->index;
					move_done();
				} else {
					active_ring->row++;
					redraw = true;
				}
			} break;

			case DIR_LEFT: {
				if (active_ring->col == dest_col) {
					direction = DIR_DOWN;
				} else {
					active_ring->col--;
					active_ring->start--;
					active_ring->end--;
					redraw = true;
				}
			} break;
		}
	}

	if (redraw) {
		// Draw the next animation frame (modify array of character codes).
		animate_frame();
	}
}