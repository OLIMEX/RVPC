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

#include "keyboard.h"
#include "misc.h"
#include "wait.h"

char kbd_to_ascii(uint32_t key_code) {
    static const uint8_t kbd_map[] = {
        0x00, // 0x00
        0x00, // 0x01   <F9>
        0x00, // 0x02
        0x00, // 0x03   <F5>
        0x00, // 0x04   <F3>
        0x00, // 0x05   <F1>
        0x00, // 0x06   <F2>
        0x00, // 0x07   <F12>
        0x00, // 0x08
        0x00, // 0x09   <F10>
        0x00, // 0x0A   <F8>
        0x00, // 0x0B   <F6>
        0x00, // 0x0C   <F4>
        0x09, // 0x0D	<Tab>
        '`',  // 0x0E
        0x00, // 0x0F

        0x00, // 0x10
        0x00, // 0x11	<Alt Left>
        0x00, // 0x12	<Shift Left>
        0x00, // 0x13
        0x00, // 0x14	<Ctrl Left>
        'q',  // 0x15
        '1',  // 0x16
        0x00, // 0x17
        0x00, // 0x18
        0x00, // 0x19
        'z',  // 0x1A
        's',  // 0x1B
        'a',  // 0x1C
        'w',  // 0x1D
        '2',  // 0x1E
        0x00, // 0x1F

        0x00, // 0x20
        'c',  // 0x21
        'x',  // 0x22
        'd',  // 0x23
        'e',  // 0x24
        '4',  // 0x25
        '3',  // 0x26
        0x00, // 0x27
        0x00, // 0x28
        ' ',  // 0x29
        'v',  // 0x2A
        'f',  // 0x2B
        't',  // 0x2C
        'r',  // 0x2D
        '5',  // 0x2E
        0x00, // 0x2F

        0x00, // 0x30
        'n',  // 0x31
        'b',  // 0x32
        'h',  // 0x33
        'g',  // 0x34
        'y',  // 0x35
        '6',  // 0x36
        0x00, // 0x37
        0x00, // 0x38
        0x00, // 0x39
        'm',  // 0x3A
        'j',  // 0x3B
        'u',  // 0x3C
        '7',  // 0x3D
        '8',  // 0x3E
        0x00, // 0x3F

        0x00, // 0x40
        ',',  // 0x41
        'k',  // 0x42
        'i',  // 0x43
        'o',  // 0x44
        '0',  // 0x45
        '9',  // 0x46
        0x00, // 0x47
        0x00, // 0x48
        '.',  // 0x49
        '/',  // 0x4A
        'l',  // 0x4B
        ';',  // 0x4C
        'p',  // 0x4D
        '-',  // 0x4E
        0x00, // 0x4F

        0x00, // 0x50
        0x00, // 0x51
        '\'', // 0x52
        0x00, // 0x53
        '[',  // 0x54
        '=',  // 0x55
        0x00, // 0x56
        0x00, // 0x57
        0x00, // 0x58	<CapsLock>
        0x00, // 0x59
        0x0D, // 0x5A	<Enter>
        ']',  // 0x5B
        0x00, // 0x5C
        '\\', // 0x5D
        0x00, // 0x5E
        0x00, // 0x5F

        0x00, // 0x60
        0x00, // 0x61
        0x00, // 0x62
        0x00, // 0x63
        0x00, // 0x64
        0x00, // 0x65
        0x08, // 0x66	<Backspace>
        0x00, // 0x67
        0x00, // 0x68
        0x00, // 0x69
        0x00, // 0x6A
        0x00, // 0x6B
        0x00, // 0x6C
        0x00, // 0x6D
        0x00, // 0x6E
        0x00, // 0x6F

        0x00, // 0x70
        0x00, // 0x71
        0x00, // 0x72
        0x00, // 0x73
        0x00, // 0x74
        0x00, // 0x75
        0x1B, // 0x76   <Esc>
        0x00, // 0x77
        0x00, // 0x78   <F11>
        0x00, // 0x79
        0x00, // 0x7A
        0x00, // 0x7B
        0x00, // 0x7C
        0x00, // 0x7D
        0x00, // 0x7E
        0x00, // 0x7F
    };

	uint8_t index = (uint8_t)key_code & 0xFF;
	if (index < sizeof(kbd_map)) {
		return kbd_map[index];
	}
	return 0;
}

void buzz(uint32_t hz, uint32_t timeMS) {
	#ifdef KBD_USE_BUZZ
	static uint8_t buzz_state = 0;
	uint32_t delay = 500000 / hz;
	
	setTimeout_Ms(timeMS);
	while (isTimeout_Ms() == 0) {
		buzz_state ^= 1;
		Wait_Us(delay);
		if (buzz_state) {
			BUZZ_PORT->BSHR = BUZZ_PIN;
		} else {
			BUZZ_PORT->BCR = BUZZ_PIN;
		}
	}
	#endif
}

void buzz_ok() {
	#ifdef KBD_USE_BUZZ
	buzz( 698, 50);
	buzz( 880, 50);
	buzz(1047, 50);
	buzz(1397, 50);
	#endif
}

void kbd_init() {
   	Wait_Init();

	GPIO_Config(KBD_CLOCK_PORT, KBD_CLOCK_PIN, GPIO_Mode_IN_FLOATING);
	GPIO_Config(KBD_DATA_PORT, KBD_DATA_PIN, GPIO_Mode_IN_FLOATING);

	#ifdef KBD_USE_BUZZ
	GPIO_Config(BUZZ_PORT, BUZZ_PIN, GPIO_Mode_Out_PP);
	#endif

	GPIO_Interrupt(KBD_INT_PORT_SRC, KBD_INT_PIN_SRC, KBD_INT_LINE, EXTI_Trigger_Falling);
}

static uint16_t _KBD_ = 0;
static uint8_t _KBD_BIT_ = 0;
static uint16_t _KBD_READY_ = 0;

static uint8_t KBD_DATA_READY = 0;
static uint32_t KBD_DATA = 0;

void kbd_handle() {
	if (_KBD_READY_ == 0) {
		// No data
		return;
	}

	if (KBD_DATA_READY) {
		// keyboard is not read skip
		goto done;
	}
	
	uint16_t mask = 0b0000010000000000;

	// Check start bit
	if (_KBD_ & mask) {
		// Wrong start bit
		goto done;
	}

	// Get data bits
	uint8_t buff = 0;
	uint8_t one = 0;
	for (uint8_t bit=0; bit<8; bit++) {
		mask >>= 1;
		if (_KBD_ & mask) {
			buff |= (1 << bit);
			one ^= 1;
		}
	}

	// Get parity bit
	mask >>= 1;
	uint8_t parity = (_KBD_ & mask) ^ one;

	// Check stop bit
	mask >>= 1;
	if ((_KBD_ & mask) == 0) {
		// Wrong stop bit
		goto done;
	}

	if (parity) {
		KBD_DATA <<= 8;
		KBD_DATA |= ((uint32_t)buff);
		KBD_DATA_READY = ((buff & 0xE0) != 0xE0);
	}

	done:
		_KBD_ = 0;
		_KBD_BIT_ = 0;
		_KBD_READY_ = 0;
}

uint32_t kbd_read() {
	kbd_handle();
	if (KBD_DATA_READY) {
		uint32_t kbd_code = KBD_DATA;
		KBD_DATA = 0;
		KBD_DATA_READY = 0;
		return kbd_code;
	}
	return 0;
}

uint32_t kbd_wait() {
	uint32_t code = 0;
	do {
		code = kbd_read();
	} while (code == 0x0000);
	return code;
}

uint32_t kbd_wait_press() {
	uint32_t code = 0;
	do {
		code = kbd_read();
	} while (code == 0 || (code & 0xFF00) == 0xF000);
	return code;
}

uint32_t kbd_wait_release() {
	uint32_t code = 0;
	do {
		code = kbd_read();
	} while ((code & 0xFF00) != 0xF000);
	return code;
}


void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI7_0_IRQHandler(void) {
	if (_KBD_READY_ == 0) {
		_KBD_BIT_++;
		_KBD_ <<= 1;
		if (KBD_DATA_PORT->INDR & KBD_DATA_PIN) {
			_KBD_ |= 1;
		}

		_KBD_READY_ = (_KBD_BIT_ == 11);
	}
	
	EXTI->INTFR = KBD_INT_LINE;
}
