/*   Copyright (C) 2010 Richard Barlow

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <io.h>
#include "input.h"
#include "drivers/pinint.h"
#include "leds.h"

/* 'input_conf' must be linked in.
 * Non-const as the callback is changed during POST */
extern input_conf_t input_conf;

static const uint16_t buttons[] = {
	1<<1, /* B0 */
	1<<3, /* B1 */
	1<<4, /* B2 */
	1<<(2+8), /* B3 (Left rotary encoder) */
	1<<7, /* B4 (Right rotary encoder) */
};
const uint8_t num_buttons = sizeof(buttons)/sizeof(uint16_t);

#define field_set16(reg, bit) do { \
                                  if (bit > 0xff) \
                                      P2##reg |= (bit)>>8; \
                                  else \
                                      P1##reg |= (bit); \
                                 } while(0)

#define field_clear16(reg, bit) do { \
                                    if (bit > 0xff) \
                                        P2##reg &= ~((bit)>>8); \
                                    else \
                                        P1##reg &= ~(bit); \
                                   } while(0)
#define field_test16(reg, bit) (!((bit > 0xff) ? P2##reg & ((bit)>>8) : P1##reg & (bit)))

#define field_set(x, val, mask) do { x &= ~mask; x |= val; } while(0)

#define R0A (1<<0)
#define R0B (1<<1)
#define R1A (1<<5)
#define R1B (1<<6)

#define P1SIGNALS (R1A)
#define P2SIGNALS (R0A)

#define test_rotary_b(n) (n ? P1IN & R1B : P2IN & R0B)

/* n = rotary encoder (0 or 1)
 * e = edge (0 = rising, 1 = falling) */
#define set_rotary_edge_a(n, e) do { \
                                    if (!n) \
                                        field_set(P2IES, e ? R0A:0, R0A); \
                                    else \
                                        field_set(P1IES, e ? R1A:0, R1A); \
                                } while(0)

typedef enum {
	S_IDLE, /* A high */
	S_CW,   /* B high when entered from S_IDLE */
	S_CCW,  /* B low, when entered from S_IDLE */
} input_rot_state_t;

static input_rot_state_t input_rot_state[2] = {S_IDLE, S_IDLE};

static uint16_t input_cb_flags;

void but_isr(uint16_t flags);
static void input_rot_fsm(uint8_t n);

static const pinint_conf_t button_int = {
	.mask = (1<<1) | (1<<3) | (1<<4) | (1<<(8+2)) | (1<<7) | (R0A<<8) | R1A,
	.int_cb = but_isr,
};

void input_init(void) {
	pinint_add( &button_int );

	int i;
	for (i = 0; i < num_buttons; i++) {
		field_clear16(DIR, buttons[i]); /* Inputs */
		field_clear16(SEL, buttons[i]); /* GPIO */
		field_set16(IES, buttons[i]);   /* Int on falling edge */
		field_clear16(IFG, buttons[i]); /* Clear int flag */
		field_set16(IE, buttons[i]);    /* Enable interrupt */
	}

	P1DIR &= ~P1SIGNALS; /* Set to inputs */
	P2DIR &= ~P2SIGNALS;

	P1SEL &= ~P1SIGNALS; /* GPIO function */
	P2SEL &= ~P2SIGNALS;

	P1IES |=  P1SIGNALS; /* Int on High-Low transition */
	P2IES |=  P2SIGNALS;

	P1IFG &= ~P1SIGNALS; /* Clear interrupt flags */
	P2IFG &= ~P2SIGNALS;

	P1IE  |=  P1SIGNALS; /* Enable interrupts on those pins */
	P2IE  |=  P2SIGNALS;
}

uint16_t input_get() {
	uint16_t state = 0;
	int i;
	for (i = 0; i < num_buttons; i++) {
		if (field_test16(IN, buttons[i]))
			state |= 1<<i;
	}
	return state;
}

void but_isr(uint16_t flags) {
	input_cb_flags = 0;

	int i;
	for (i = 0; i < num_buttons; i++) {
		if (field_test16(IN, buttons[i]))
			input_cb_flags |= 1<<i;
	}

	if (flags & (R0A<<8)) {
		input_rot_fsm(0);
	}
	if (flags & R1A) {
		input_rot_fsm(1);
	}

	/* Finished checking all button states and calculating
	 * rotary encoder rotations, invoke input callback */
	if (input_conf.inp_cb)
		input_conf.inp_cb(input_cb_flags);
}

static void input_rot_fsm(uint8_t n) {
	switch (input_rot_state[n]) {
	case S_IDLE:
		if (test_rotary_b(n)) {
			input_rot_state[n] = S_CW;
		} else {
			input_rot_state[n] = S_CCW;
		}
		set_rotary_edge_a(n, IO_IESPIN_RISING);
		break;
	case S_CW:
		if (test_rotary_b(n)) {
			/* Must have bounced back and forth */
			input_rot_state[n] = S_IDLE;
		} else {
			/* Clockwise event */
			if (n)
				input_cb_flags |= INPUT_R1CW;
			else
				input_cb_flags |= INPUT_R0CW;
			input_rot_state[n] = S_IDLE;
		}
		set_rotary_edge_a(n, IO_IESPIN_FALLING);
		break;
	case S_CCW:
		if (test_rotary_b(n)) {
			/* Count-clockwise event */
			if (n)
				input_cb_flags |= INPUT_R1CCW;
			else
				input_cb_flags |= INPUT_R0CCW;
			input_rot_state[n] = S_IDLE;
		} else {
			/* Again must have bounced back and forth */
			input_rot_state[n] = S_IDLE;
		}
		set_rotary_edge_a(n, IO_IESPIN_FALLING);
		break;
	}
}
