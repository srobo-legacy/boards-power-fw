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

void but_isr(uint16_t flags);

void input_init(void) {
	pinint_conf[PININT_BUTTON].mask = (BUT0 | BUT1 | BUT2);
	pinint_conf[PININT_BUTTON].int_cb = but_isr;

	P1DIR &= ~(BUT0 | BUT1 | BUT2); /* Set to inputs */
	P1SEL &= ~(BUT0 | BUT1 | BUT2); /* GPIO function */
	P1IES |=  (BUT0 | BUT1 | BUT2); /* Int on High-Low transition */
	P1IFG &= ~(BUT0 | BUT1 | BUT2); /* Clear interrupt flags */
	P1IE  |=  (BUT0 | BUT1 | BUT2); /* Enable interrupts on those pins */
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
}
