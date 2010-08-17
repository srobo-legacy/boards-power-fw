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
#include "piezo.h"

#define PIEZO_PIN (1<<2)

void piezo_init(void) {
	P1OUT &= ~PIEZO_PIN;
	P1DIR |= PIEZO_PIN;
}

void piezo_play(void) {
}

void piezo_beep(void) {
	int i, j;
	for (i = 0; i < 200; i++) {
		P1OUT |= PIEZO_PIN;
		for (j=0; j<100; j++) {
			nop();
		}
		P1OUT &= ~PIEZO_PIN;
		for (j=0; j<100; j++) {
			nop();
		}
	}
}

void piezo_beep_pattern(char *pattern) {
}
