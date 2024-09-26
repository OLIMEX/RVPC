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

/**
 * This app implements a solution to the Towers-of-Hanoi problem.
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "misc.h"
#include "vga.h"
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
#define STATUS_ROW                17

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
#define SCREEN_FPS      75
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
	uint8_t count;
	uint8_t from;
	uint8_t to;
	uint8_t spare;
} Move;

Ring rings[NUM_RINGS] = {
	{ 0, LAST_RING_ROW-RING_HEIGHT*3, 4, 3, 5, 3, 0 },
	{ 1, LAST_RING_ROW-RING_HEIGHT*2, 4, 2, 6, 5, 0 },
	{ 2, LAST_RING_ROW-RING_HEIGHT*1, 4, 1, 7, 7, 0 },
	{ 3, LAST_RING_ROW-RING_HEIGHT*0, 4, 0, 8, 9, 0 }
};

Peg pegs[NUM_PEGS] = {
	{ 'A', 4, NUM_RINGS, { 3, 2, 1, 0 }},
	{ 'B', 13, 0, { 0, 0, 0, 0 }},
	{ 'C', 19, 0, { 0, 0, 0, 0 }}
};

uint8_t start_peg = 0;
uint8_t current_move = 0;

uint8_t direction = DIR_NONE;
uint16_t delay = STARTUP_DELAY;
Ring* active_ring = NULL;
Move stack[12];
uint8_t moves;
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

uint8_t success_check() {
	for (uint8_t p=0; p<NUM_PEGS; p++) {
		if (p != start_peg && pegs[p].count == NUM_RINGS) {
			start_peg = p;
			return 1;
		}
	}
	return 0;
}

void print_status() {
	if (app_pause) {
		vga_print_at(STATUS_ROW, 0, "        PAUSED       ");
	} else if (success_check()) {
		vga_printf_at(STATUS_ROW, 0, "SUCCSESS in %d moves", current_move);
		current_move = 0;
		delay = STARTUP_DELAY;
	} else {
		Move* move = &stack[moves - 1];
		current_move++;
		vga_print_at(STATUS_ROW, 0, "Move #   from   to   ");
		vga_printf_at(STATUS_ROW, MOVE_COL, "%d", current_move);
		vga_write_at(STATUS_ROW, FROM_COL, move->from + 'A');
		vga_write_at(STATUS_ROW, TO_COL, move->to + 'A');
	}
}

void start_move() {
	// Determine which ring to move.
	Move* move = &stack[moves - 1];
	Peg* peg = &pegs[move->from];
	uint8_t ring_index = peg->rings[peg->count - 1];
	active_ring = &rings[ring_index];

	// Update the status info on screen
	print_status();

	// Start the move.
	delay = MOVE_DELAY;
	direction = DIR_UP;
}

void push_move(uint8_t count, uint8_t from, uint8_t spare, uint8_t to) {
	// Push a move onto the stack.
	Move* move = &stack[moves++];
	move->count = count;
	move->from = from;
	move->to = to;
	move->spare = spare;

	if (count > 1) {
		// Move all but the bottom ring, so that
		// we can get to the bottom ring.
		push_move(count - 1, from, to, spare);
	} else {
		start_move();
	}
}

void pop_move() {
	Move* move = &stack[moves - 1]; // points to move just completed
	moves--;
	if (move->count > 1) {
		push_move(move->count - 1, move->spare, move->from, move->to);
	} else if (moves) {
		start_move();
	} else {
		moves = 0;
		push_move(NUM_RINGS, move->to, move->from, move->spare);
		delay = STARTUP_DELAY;
	}
}

void splash() {
	// Clear screen
	vga_cls();

	uint8_t row = 0;
	vga_print_at(row++, 0, "===== RVPC Demo =====");
	vga_print_at(row++, 0, "VGA by Curtis Whitley");
	row += 2;
	vga_print_at(row++, 4, "Hanoi Towers");
	row++;
	vga_print_at(row++, 5, "(solution)");
	row += 3;
	vga_print_at(row++, 0, "Use <Space> to pause");
	vga_print_at(STATUS_ROW, 0, "Press a key to start");

	kbd_wait_release();
}

void initialize_application() {
	kbd_init();

	splash();

	// Fill entire screen with blank characters
	vga_cls();

	// Write constant texts
	vga_print_at(TITLE_ROW, 0, "===== RVPC Demo =====");

	// Prepare for first move
	push_move(NUM_RINGS, 0, 1, 2);
	delay = SOLUTION_DELAY;
	animate_frame();
}

void run_keyboard_state_machine() {
	uint32_t kbd_code = kbd_read();
	if ((kbd_code & 0xF000) != 0xF000) {
		return;
	}

	// key released
	switch (kbd_code) {
		case 0xF029:
			// Space released
			app_pause ^= 1;
			buzz(420, 30);
			print_status();
		break;

		default:
			buzz(840, 50);
			buzz(420, 50);
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
		move = &stack[moves - 1];

		switch (direction) {
			case DIR_NONE: {
			} break;

			case DIR_UP: {
				if (current_move == 0) {
					print_status();
				}
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
					pop_move();
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