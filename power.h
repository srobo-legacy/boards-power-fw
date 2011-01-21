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

#ifndef __POWER_H
#define __POWER_H
/* Functions for controlling various power rails */

#include <stdbool.h>

void power_init(void);

/* Turn on/off LCD backlight SMPS */
void power_bl_enable(void);
void power_bl_disable(void);

/* Turn on/off the 'motor rail' */
void power_motor_enable(void);
void power_motor_disable(void);

/* Turn on/off the BeagleBoard (+ USB) 5V rail */
void power_bb_enable(void);
void power_bb_disable(void);

/* Find out if the BeagleBoard is on (true) or off (false) */
bool power_bb_status(void);

#endif /* __POWER_H */
