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

extern int main(void);

#define MON_PROMPT       '>'
#define MON_SANDBOX_SIZE 512

typedef void function(void);

void key_ok() {
	buzz(880, 10);
}

void key_err() {
	buzz(440, 50);
	buzz(220, 150);
}

uint8_t app_pending_reset = 0;

uint8_t mon_sandbox[MON_SANDBOX_SIZE];
char mon_cmdline[VGA_NUM_COLS+2];
char mon_cmd = 'R';
uint32_t mon_address[2];

inline static uint8_t is_hex_digit(char letter) {
	return (letter >= '0' && letter <= '9') || (letter >= 'A' && letter <= 'F');
}

void addresses() {
	printfln("reset    @0x%08lX", (uint32_t)main);
	printfln("buzz_ok  @0x%08lX", (uint32_t)buzz_ok);
	printfln("buzz_err @0x%08lX", (uint32_t)key_err);
	printfln("sandbox  @0x%08lX", (uint32_t)mon_sandbox);
}

void help() {
	println("[addr][cmd][bytes]");
	println("Commands");
	println("[\n] Get addr");
	println("[.] Dump TO addr");
	println("[+] Dump 16 bytes");
	println("[:] Write bytes");
	println("[G] Go/Run");
	println("[@] Addresses");
}

void prompt() {
	// Clear command line	
	mon_cmdline[0] = 0x00;
	// Show cursor
	cursor_down();
	write(MON_PROMPT);
}

uint8_t parse() {
	static uint8_t cmd_pos = 0;

	uint8_t cmd_len = strlen(mon_cmdline);
	uint8_t start_pos = cmd_pos;

	uint8_t hex_index = (mon_cmd == 'W' ? 1 : 0);
	mon_address[1] = mon_address[0];

	while (cmd_pos < cmd_len) {
		if (is_hex_digit(mon_cmdline[cmd_pos])) {
			uint8_t hex_digit = (mon_cmdline[cmd_pos] <= '9') ?
				mon_cmdline[cmd_pos] - '0'
				:
				0x0A + mon_cmdline[cmd_pos] - 'A'
			;

			if (cmd_pos == start_pos) {
				mon_address[hex_index] = 0;
			}
			mon_address[hex_index] = (mon_address[hex_index] << 4) | hex_digit;
		} else {
			switch (mon_cmdline[cmd_pos]) {
				case ' ':
					cmd_pos++;
					goto done;
				break;

				case ':':
					mon_cmd = 'W';
				case '.':
					hex_index = 1;
					mon_address[1] = 0;
				break;

				case '+':
					cmd_pos++;
					hex_index = 1;
					mon_address[1] = mon_address[0] + 0x0F;
					goto done;
				break;

				case '?':
				case '@':
				case 'G':
					mon_cmd = mon_cmdline[cmd_pos];
					cmd_pos++;
					goto done;
				break;
			}
		}
		cmd_pos++;
	}

	done:

	if (cmd_pos >= cmd_len) {
		cmd_pos = 0;
	}

	if (hex_index == 0) {
		mon_address[1] = mon_address[0];
	}

	return (cmd_pos != 0);
}

void execute() {
	mon_cmd = 'R';
	uint8_t cmd_len = strlen(mon_cmdline);
	if (cmd_len == 0) {
		sprintf(mon_cmdline, "%lX", mon_address[0]);
		printfstr("%lX", mon_address[0]);
		return;
	}

	cursor_down();

	uint8_t more;
	do {
		more = parse();

		switch (mon_cmd) {
			case '?': // Help
				help();
			break;

			case '@': // Help
				addresses();
			break;

			case 'R': { // Read
				uint32_t offset = 0;
				printfln("0x%08lX", mon_address[0] + offset);
				while (mon_address[0] + offset <= mon_address[1]) {
					printfstr("%02X ", *(uint8_t *)(mon_address[0] + offset));
					offset++;
					if ((offset & 0x03) == 0) {
						if ((mon_address[0] & 0x03) == 0) {
							printfln("%08lX", *(uint32_t *)(mon_address[0] + offset - 4));
						} else {
							cursor_down();
						}
					}
				}
				if ((offset & 0x03) != 0) {
					cursor_down();
				}
			} break;

			case 'W': // Write
				printfln("0x%08lX = 0x%02X", mon_address[0], (uint8_t)mon_address[1]);
				*(uint8_t *)(mon_address[0]) = (uint8_t)mon_address[1];
				mon_address[0]++;
			break;
		
			case 'G': // Go
				printfln("0x%08lX [Go]", mon_address[0]);
				if (mon_address[0] == (uint32_t)main) {
					app_pending_reset = 1;
				} else {
					function* f = (function *)(mon_address[0]);
					f();
				}
			break;
		}
	} while (more);

	prompt();
}

void key(char letter) {
	uint8_t cmd_len = strlen(mon_cmdline);
	switch (letter) {
		case 0x08:
			if (cmd_len > 0) {
				mon_cmdline[cmd_len - 1] = 0x00;
				uint8_t col = vga_cursor_col() - 1;
				vga_cursor_set(vga_cursor_row(), col);
				vga_write_at(vga_cursor_row(), col, ' ');
			}
		break;

		default:
			if (cmd_len < VGA_NUM_COLS - 2) {
				if (cmd_len > 0 && !is_hex_digit(letter) && !is_hex_digit(mon_cmdline[cmd_len - 1])) {
					key_err();
					return;
				}
				mon_cmdline[cmd_len] = letter;
				mon_cmdline[cmd_len + 1] = 0x00;
				write(letter);
			}
		break;
	}
	key_ok();
}

void app_init() {
	// Initialize cmd_line
	memset(mon_cmdline, 0, sizeof(mon_cmdline));

	// Initialize mon_sandbox
	memset(mon_sandbox, 0, sizeof(mon_sandbox));

	// Initialize cmd_address
	mon_address[0] = (uint32_t)mon_sandbox;
	mon_address[1] = (uint32_t)mon_sandbox;

	vga_cls();
	vga_cursor_set(0, 0);
	
	println("RVMON 1.0 (c) Olimex");
	println("VGA by Curtis Whitley");
	cursor_down();

	help();
	cursor_down();
	addresses();

	prompt();
	
	buzz_ok();
}

void app_run() {
	static uint8_t shift = 0;

	uint32_t key_code = kbd_wait();
	uint8_t key_char = kbd_to_ascii(key_code);

	switch (key_code) {
		case 0xF05A:  // Enter
			key_ok();
			execute();
		break;

		case 0x66:   // Backspace press
			key(0x08);
		case 0xF066: // Backspace release
		break;

		case 0x12:    // Left shift
		case 0x59:    // Right shift
			shift = 1;
		break;

		case 0xF012:  // Left shift released
		case 0xF059:  // Right shift released
			shift = 0;
		break;

		default:
			// Key released
			if ((key_code & 0xFF00) == 0xF000) {
				if (key_char >= '0' && key_char <= '9') {
					if (key_char == '2' && shift) {
						key('@');
					} else {
						key(key_char);
					}
				} else if ((key_char >= 'a' && key_char <= 'f') || key_char  == 'g') {
					key(key_char - 'a' + 'A');
				} else if (key_char == ' ' || key_char == '.') {
					key(key_char);
				} else if (key_char == '/' && shift) {
					key('?');
				} else if (key_char == '=' && shift) {
					key('+');
				} else if (key_char == ';' && shift) {
					key(':');
				} else {
					key_err();
				}
			}
		break;
	}
}

uint8_t app_reset() {
	if (app_pending_reset != 0) {
		app_pending_reset = 0;
		return 1;
	}
	return 0;
}
