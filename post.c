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
#include "post.h"
#include "drivers/sched.h"
#include "piezo.h"
#include "input.h"
#include "power.h"
#include "leds.h"

#define reset_wdt() do { WDTCTL = WDTCNTCL | WDTPW; } while(0)

/* Decalred in main.c
 * Used here to hook into the input module callback */
extern input_conf_t input_conf;

static void post_test_mode(void);
static void post_input_cb(uint16_t flags, uint16_t edges);
static bool post_flash_cb(void* ud);

const sched_task_t flash_task = {.t = 1000,
                           .cb = post_flash_cb,
                           .udata = NULL};
bool flash_toggle;

static const piezo_note_t enter_test_mode[] = {
	{.f=100,  .d=100, .v=2},
	{.f=200,  .d=200, .v=2},
	{.f=500,  .d=300, .v=2},
	{.f=1000, .d=400, .v=2},
	{.f=2000, .d=500, .v=2},
	{.f=5000, .d=600, .v=2},
};

#define POST_ENTRY_BUTTONS (INPUT_B1 | INPUT_B2)

bool post(void) {
	/* Run 'test-mode' if the two buttons to the right of the screen
	 * are held in */
	if ( (input_get() & POST_ENTRY_BUTTONS) == POST_ENTRY_BUTTONS )
		post_test_mode();

	piezo_beep();
	piezo_beep();
	piezo_beep();
	return true;
}

static void post_test_mode(void) {
	input_conf.inp_cb = post_input_cb;

	piezo_play(enter_test_mode, 6, false);
	sched_add(&flash_task);

	while(1) {
		reset_wdt();
	}
}

/* Pressing a push button causes the LED next to it to toggle.
 * Pressing a rotary encoder causes the two user LEDs closest to it to toggle.
 *
 * Rotary enocder rotations (toggles LEDs):
 *  - Left encoder clockwise: middle user led
 *  - Left encoder anticlockwise: left user led
 *  - Right encoder clockwise: right user led
 *  - Right encoder anticlockwise: middle user led
 */
static void post_input_cb(uint16_t flags, uint16_t edges) {
	if (flags & INPUT_B0)
		led_toggle(0);
	if (flags & INPUT_B1)
		led_toggle(1);
	if (flags & INPUT_B2)
		led_toggle(2);

	if (flags & INPUT_R0B) {
		uled_toggle(0);
		uled_toggle(1);
	}
	if (flags & INPUT_R1B) {
		uled_toggle(1);
		uled_toggle(2);
	}

	if (flags & INPUT_R0CW)
		uled_toggle(1);
	if (flags & INPUT_R0CCW)
		uled_toggle(0);

	if (flags & INPUT_R1CW)
		uled_toggle(2);
	if (flags & INPUT_R1CCW)
		uled_toggle(1);

}

static bool post_flash_cb(void* ud) {
	dbg_toggle();
	chrg_toggle();

	if (flash_toggle) {
		flash_toggle = false;
		power_motor_enable();
	} else {
		flash_toggle = true;
		power_motor_disable();
	}

	return true;
}
