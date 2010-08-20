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

#ifndef __SCHED_H
#define __SCHED_H
#include <stdbool.h>
#include <stdint.h>

typedef struct {
	/* Timout in milliseconds */
	uint16_t t;
	/* Callback to be called after timeout.
	 * Return true to execute task again */
	bool (*cb) (void *udata);
	/* Pointer to user data, is passed to the callback */
	void *udata;
} sched_task_t;

void sched_init(void);

/* Add a task to the schedule queue */
void sched_add(sched_task_t *task);

/* Remove a task from the schedule queue */
void sched_rem(sched_task_t *task);

#endif /* __SCHED_H */
