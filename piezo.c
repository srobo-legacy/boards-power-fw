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
#include "drivers/sched.h"
#include "piezo.h"

#define PIEZO_PIN (1<<2)

#define PIEZO_EN() P1SEL |= PIEZO_PIN
#define PIEZO_DIS() P1SEL &= ~PIEZO_PIN
#define FREQ_TO_DELAY(f) (1000000UL/f)

typedef struct {
	piezo_note_t notes[PIEZO_BUFFER_LEN];
	uint8_t in;
	uint8_t out;
} piezo_buffer_t;
static volatile piezo_buffer_t piezo_buffer;

bool piezo_play_cb(void *p);
sched_task_t piezo_task = {.cb = piezo_play_cb};

extern piezo_config_t piezo_config;
volatile bool playing;
volatile bool requires_notif;

void piezo_init(void) {
	P1OUT &= ~PIEZO_PIN;
	P1DIR |= PIEZO_PIN;

	TACTL = TASSEL_SMCLK   /* Source clock from 'SMCLK' */
	      | ID_DIV8        /* Divide SMCLK by 8 */
	      | MC_UPTO_CCR0;  /* Count up to TACCR0 */
	      /* No interrupts */

	TACCTL1 = OUTMOD_SET_RESET; /* Toggle output over a full timer cycle */

	TACCR0 = 1000;
	TACCR1 = 10;
}

void piezo_play(piezo_note_t tune[], uint8_t len, bool notif) {
	if (notif)
		requires_notif = true;

	for (uint8_t i = 0; i < len; i++) {
		if ((piezo_buffer.in + 1) % PIEZO_BUFFER_LEN == piezo_buffer.out) {
			/* Buffer full, give up */
			break;
		}

		piezo_buffer.notes[piezo_buffer.in] = tune[i];

		uint8_t tmp_in = piezo_buffer.in + 1;
		if (tmp_in == PIEZO_BUFFER_LEN) tmp_in = 0;
		piezo_buffer.in = tmp_in;
	}

	bool restoreint = ((READ_SR & GIE) != 0);
	dint();
	if (!playing) {
		playing = true;
		piezo_task.t = 2;
		sched_add(&piezo_task);
		PIEZO_EN();
	}
	if (restoreint)
		eint();
}

bool piezo_play_cb(void *p) {
	if (piezo_buffer.in != piezo_buffer.out) {
		piezo_note_t *n = &(piezo_buffer.notes[piezo_buffer.out]);

		TACCR0 = FREQ_TO_DELAY(n->f);
		piezo_task.t = n->d;
		TACCR1 = (1<<(n->v));

		piezo_buffer.out++;
		if (piezo_buffer.out == PIEZO_BUFFER_LEN) piezo_buffer.out = 0;

	} else {
		PIEZO_DIS();
		playing = false;
		requires_notif = false;
	}

	if (requires_notif && (piezo_buffer.out + 5) % PIEZO_BUFFER_LEN  == piezo_buffer.in) {
		piezo_config.buf_low();
	}
	return playing;
}

void piezo_beep(void) {
}

void piezo_beep_pattern(char *pattern) {
}
