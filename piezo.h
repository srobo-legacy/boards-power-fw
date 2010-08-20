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

typedef struct {
	/* Frequency in Hertz */
	uint16_t f;
	/* Duration in milliseconds */
	uint16_t d;
	/* Volume (0-5) */
	uint8_t v;
} piezo_note_t;

typedef struct {
	piezo_note_t notes[PIEZO_BUFFER_LEN];
	uint8_t in;
	uint8_t out;
} piezo_buffer_t;

void piezo_init(void);

/* Begins playback of a tune.
 * Notes to be played will be obtained through the 'gen_notes' callback.
 * It must return false when there are no more notes to play */
void piezo_play(bool (*gen_notes)(piezo_buffer_t *buf));

/* Provides an easy way of making a 'beep' noise. Gives a 200ms beep at 1kHz
 * and a period of silence for 100ms. Blocks until finished.*/
void piezo_beep(void);

/* Output a string of long and short beeps, 1kHz 800ms/200ms. Blocking.
 * pattern consists of a string containing the characters '.' and '-'
 * where '.' is a short (200ms) beep and '-' is a long (800ms) beep.
 * A delay of 500ms is added between beeps */
void piezo_beep_pattern(char *pattern);

#endif /* __PIEZO_H */
