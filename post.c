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
#include "piezo.h"
#include "input.h"
#include "leds.h"

static void post_test_mode(void);

bool post(void) {
	/* Run 'test-mode' if the two buttons to the right of the screen
	 * are held in */
	if (input_get() == (INPUT_B1 | INPUT_B2))
		post_test_mode();

	piezo_beep();
	return true;
}

static void post_test_mode(void) {
	while(1);
}
