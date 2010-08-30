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

#define field_set(x, val, mask) do { x &= ~mask; x |= val; } while(0)

#define BUT0 (1<<1)
#define BUT1 (1<<3)
#define BUT2 (1<<4)

#define RBUT0 (1<<(2+8)) /* On P2 so shift by 8 for flags */
#define RBUT1 (1<<7)

#define R0A (1<<(0+8))
#define R0B (1<<(1+8))
#define R1A (1<<5)
#define R1B (1<<6)

/* Don't include rotary encoder 'B' inputs, don't want interrupts on them */
#define P1SIGNALS (BUT0 | BUT1 | BUT2 | RBUT1 | R1A)
#define P2SIGNALS ((RBUT0 | R0A) >> 8)

#define test_rotary_b(n) (n ? P1IN & R1B : P2IN & (R0B>>8))

/* n = rotary encoder (0 or 1)
 * e = edge (0 = rising, 1 = falling) */
#define set_rotary_edge_a(n, e) do { \
                                    if (!n) \
                                        field_set(P2IES, e ? (R0A>>8):0, (R0A>>8)); \
                                    else \
                                        field_set(P1IES, e ? R1A:0, R1A); \
                                } while(0)

typedef enum {
	S_IDLE, /* A high */
	S_CW,   /* B high when entered from S_IDLE */
	S_CCW,  /* B low, when entered from S_IDLE */
} input_rot_state_t;

static input_rot_state_t input_rot_state[2] = {S_IDLE, S_IDLE};

void but_isr(uint16_t flags);
static void input_rot_fsm(uint8_t n);

static const pinint_conf_t button_int = {
	.mask = BUT0 | BUT1 | BUT2 | RBUT0 | RBUT1 | R0A | R1A,
	.int_cb = but_isr,
};

void input_init(void) {
	pinint_add( &button_int );

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

void but_isr(uint16_t flags) {
	if (flags & BUT0) {
		led_toggle(0);
	}
	if (flags & BUT1) {
		led_toggle(1);
	}
	if (flags & BUT2) {
		led_toggle(2);
	}
	if (flags & RBUT0) {
		uled_toggle(0);
	}
	if (flags & RBUT1) {
		uled_toggle(1);
	}

	if (flags & R0A) {
		input_rot_fsm(0);
	}
	if (flags & R1A) {
		input_rot_fsm(1);
	}
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
				TACCR0 -= 20;
			else
				TACCR1 += 1;
			input_rot_state[n] = S_IDLE;
		}
		set_rotary_edge_a(n, IO_IESPIN_FALLING);
		break;
	case S_CCW:
		if (test_rotary_b(n)) {
			/* Count-clockwise event */
			if (n)
				TACCR0 += 20;
			else
				TACCR1 -= 1;
			input_rot_state[n] = S_IDLE;
		} else {
			/* Again must have bounced back and forth */
			input_rot_state[n] = S_IDLE;
		}
		set_rotary_edge_a(n, IO_IESPIN_FALLING);
		break;
	}
}
