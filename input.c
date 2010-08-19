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
#include "pinint.h"
#include "leds.h"

#define BUT0 (1<<1)
#define BUT1 (1<<3)
#define BUT2 (1<<4)

#define RBUT0 (1<<(2+8)) /* On P2 so shift by 8 for flags */
#define RBUT1 (1<<7)

#define P1SIGNALS (BUT0 | BUT1 | BUT2 | RBUT1)
#define P2SIGNALS ((RBUT0) >> 8)

void but_isr(uint16_t flags);

void input_init(void) {
	pinint_conf[PININT_BUTTON].mask =
	        (BUT0 | BUT1 | BUT2 | RBUT0 | RBUT1);
	pinint_conf[PININT_BUTTON].int_cb = but_isr;

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
}
