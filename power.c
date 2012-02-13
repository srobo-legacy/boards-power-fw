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
#include "power.h"

#define MOTOR_EN_PIN (1<<0)
#define BL_EN_PIN    (1<<1)
/* BB 5V rail enable line is active low */
#define BB_EN_PIN    (1<<2)
#define BB_REGEN_PIN (1<<5)

void power_init(void) {
	P5OUT &= ~(MOTOR_EN_PIN | BL_EN_PIN);
	if (power_bb_status())
		P5OUT &= ~BB_EN_PIN; /* Leave BB powered on */
	else
		P5OUT |= BB_EN_PIN; /* Prevent BB from being powered for now */
	P5DIR |= MOTOR_EN_PIN | BL_EN_PIN | BB_EN_PIN;
}

void power_bl_enable(void) {
	P5OUT |= BL_EN_PIN;
}
void power_bl_disable(void) {
	P5OUT &= ~BL_EN_PIN;
}

void power_motor_enable(void) {
	P5OUT |= MOTOR_EN_PIN;
}
void power_motor_disable(void) {
	P5OUT &= ~MOTOR_EN_PIN;
}

void power_bb_enable(void) {
	P5OUT &= ~BB_EN_PIN;
}
void power_bb_disable(void) {
	P5OUT |= BB_EN_PIN;
}

bool power_bb_status(void) {
	return P2IN & BB_REGEN_PIN;
}
