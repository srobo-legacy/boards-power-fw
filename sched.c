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

#include <stddef.h>
#include <io.h>
#include <signal.h>
#include "sched.h"
#include "config.h"
#include "leds.h"

typedef struct {
	typeof(TACTL)   *CTL;
	typeof(TAR)     *R;
	typeof(TACCTL0) *CCTL0;
	typeof(TACCR0)  *CCR0;
	typeof(TAIV)    *IV;
} sched_regs_t;

static const sched_regs_t regs = {
#if SCHED_TIMER == 1
	.CTL   = &TBCTL,
	.R     = &TBR,
	.CCTL0 = &TBCCTL0,
	.CCR0  = &TBCCR0,
	.IV    = &TBIV,
#else
	.CTL   = &TACTL,
	.R     = &TAR,
	.CCTL0 = &TACCTL0,
	.CCR0  = &TACCR0,
	.IV    = &TAIV,
#endif
};

typedef struct {
	sched_task_t *task;
	uint16_t time;
} sched_queuetask_t;

sched_queuetask_t sched_queue[SCHED_QUEUE_SIZE];

#define LOOP_Q for (uint8_t i = 0; i < SCHED_QUEUE_SIZE; i++)

volatile uint16_t sched_time;

#if SCHED_TIMER == 1
interrupt (TIMERB0_VECTOR) sched_timer_isr(void) {
#else
interrupt (TIMERA0_VECTOR) sched_timer_isr(void) {
#endif

	sched_time++;

	LOOP_Q {
		if (sched_queue[i].time == sched_time &&
		    sched_queue[i].task != NULL) {

			if (sched_queue[i].task->cb(
			             sched_queue[i].task->udata)) {

				sched_queue[i].time = sched_time +
				               sched_queue[i].task->t;
			} else {
				sched_rem(sched_queue[i].task);
			}
		}
	}
}

void sched_init(void) {
	*(regs.CTL) = TASSEL_SMCLK   /* Source clock from SMCLK */
	            | MC_UPTO_CCR0;  /* Count up to CCR0 */
	*(regs.CCTL0) = CCIE;        /* Enable interrupt on CCR0 */
	*(regs.CCR0) = SCHED_DIV;
}

void sched_add(sched_task_t *task) {
	bool added = false;
	LOOP_Q {
		if (sched_queue[i].task == NULL) {
			sched_queue[i].task = task;
			sched_queue[i].time = sched_time + task->t;
			added = true;
			break;
		}
	}
	if (!added)
		while(1); /* No more space for tasks! */
}

void sched_rem(sched_task_t *task) {
	LOOP_Q {
		if (sched_queue[i].task == task) {
			sched_queue[i].task = NULL;
		}
	}
}
