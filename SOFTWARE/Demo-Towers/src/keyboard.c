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
		KBD_DATA_READY = ((buff & 0x80) == 0);
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
