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
#include "drivers/sched.h"
#include "piezo.h"

#define PIEZO_PIN (1<<2)

#define PIEZO_EN() P1SEL |= PIEZO_PIN
#define PIEZO_DIS() P1SEL &= ~PIEZO_PIN
#define FREQ_TO_DELAY(f) (1000000UL/f)

sched_task_t piezo_delay;
bool piezo_stop_cb(void *p);
bool piezo_play_cb(void *p);

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

void piezo_play(bool (*gen_note)(piezo_note_t *note)) {
	piezo_delay.cb = piezo_play_cb;
	piezo_delay.t = 10;
	piezo_delay.udata = gen_note;
	PIEZO_EN();
}

bool piezo_play_cb(void *p) {
	bool (*gen_note) (piezo_note_t *n) = p;
	piezo_note_t note;
	if (gen_note(&note)) {
		TACCR0 = FREQ_TO_DELAY(note.f);
		piezo_delay.t = (note.d) * 100;
		return true;
	} else {
		PIEZO_DIS();
		return false;
	}
}

void piezo_beep(void) {
	TACCR0 = FREQ_TO_DELAY(1000);
	PIEZO_EN();
	piezo_delay.cb = piezo_stop_cb;
	piezo_delay.t = 150;
	sched_add(&piezo_delay);
}

bool piezo_stop_cb(void * p) {
	PIEZO_DIS();
	return false;
}

void piezo_beep_pattern(char *pattern) {
}
