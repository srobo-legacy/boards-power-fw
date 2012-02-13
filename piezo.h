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

#ifndef __PIEZO_H
#define __PIEZO_H
#include <stdint.h>
#include <stdbool.h>

/* A variable of the following type called 'piezo_config' must be linked in */
typedef struct {
	/* Called when the playback buffer only has 5 notes remaining */
	void (*buf_low) (uint8_t free_space);
} piezo_config_t;

typedef struct {
	/* Frequency in Hertz */
	uint16_t f;
	/* Duration in milliseconds */
	uint16_t d;
	/* Volume (0-5) */
	uint8_t v;
} piezo_note_t;

void piezo_init(void);

/* Copy 'tune' into the internal playback buffer and begin playing.
 * Set 'notif' to true to enable notifications through the global
 * notification callback when the buffer is getting empty. */
void piezo_play(const piezo_note_t tune[], uint8_t len, bool notif);

/* Provides an easy way of making a 'beep' noise. */
void piezo_beep(void);

/* Output a string of long and short beeps, 1kHz 800ms/200ms. Blocking.
 * pattern consists of a string containing the characters '.' and '-'
 * where '.' is a short (200ms) beep and '-' is a long (800ms) beep.
 * A delay of 500ms is added between beeps. Maximum of 8 beeps. */
void piezo_beep_pattern(char *pattern);

#endif /* __PIEZO_H */
