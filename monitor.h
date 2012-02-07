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

#ifndef __MONITOR_H
#define __MONITOR_H
/* Functions for monitoring power rails and battery current */

void monitor_init(void);

/* Return the instantaneous battery voltage.
 * Multiply by 0.0036621 to get the value in volts. */
uint16_t monitor_get_voltage(void);

/* Return the instantaneous current being drawn from the battery.
 * Multiply by 0.012201 to get the value in amps. */
uint16_t monitor_get_current(void);

#endif /* __MONITOR_H */
