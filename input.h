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

#ifndef __INPUT_H
#define __INPUT_H

#include <stdint.h>

/* Push buttons */
#define INPUT_B0    (1<<0)
#define INPUT_B1    (1<<1)
#define INPUT_B2    (1<<2)

/* Rotary encoder buttons */
#define INPUT_R0B   (1<<3)
#define INPUT_R1B   (1<<4)

/* Rotary encoder rotations */
#define INPUT_R0CW  (1<<5)
#define INPUT_R0CCW (1<<6)
#define INPUT_R1CW  (1<<7)
#define INPUT_R1CCW (1<<8)

typedef struct {
	/* Callback for button presses/rotary encoder turns
	 * flags indicates what has been pressed */
	void (*inp_cb) (uint16_t flags);
} input_conf_t;

void input_init(void);

/* Get the state of the inputs */
uint16_t input_get(void);

#endif /* __INPUT_H */
