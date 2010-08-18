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
#include <signal.h>
#include "leds.h"
#include "piezo.h"
#include "power.h"
#include "monitor.h"

void init(void) {
	leds_init();
	piezo_init();
	power_init();
	monitor_init();

	eint();
}

int main(void) {
	/* Disable watchdog timer */
	WDTCTL = WDTHOLD | WDTPW;

	init();
	led_set(0, 1);
	piezo_beep();
	power_motor_enable();
	power_bb_enable();
	power_bl_enable();

	while(1);
}
