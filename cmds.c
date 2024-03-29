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

#include "cmds.h"
#include "leds.h"
#include "note.h"
#include "piezo.h"
#include "power.h"
#include "monitor.h"
#include "flash430/sric-flash.h"
#include "drivers/stack.h"

uint8_t sric_enable_input_notes(const sric_if_t *iface);
uint8_t sric_play_piezo(const sric_if_t *iface);
uint8_t sric_set_leds(const sric_if_t *iface);
uint8_t sric_motor_rail(const sric_if_t *iface);
uint8_t sric_get_leds(const sric_if_t *iface);
uint8_t sric_get_vi(const sric_if_t *iface);
uint8_t sric_get_stack_usage(const sric_if_t *iface);

const sric_cmd_t sric_commands[] = {
	{sric_flashr_fw_ver},
	{sric_flashw_fw_chunk},
	{sric_flashr_fw_next},
	{sric_flashr_crc},
	{sric_flashw_confirm},
	{sric_enable_input_notes},
	{sric_play_piezo},
	{sric_set_leds},
	{sric_motor_rail},
	{sric_get_leds},
	{sric_get_vi},
	{sric_get_stack_usage},
};

const uint8_t sric_cmd_num = sizeof(sric_commands) / sizeof(const sric_cmd_t);

/* Receives a single byte of data: if nonzero then enable input notifications.
 * if zero, disable. */
uint8_t
sric_enable_input_notes(const sric_if_t *iface)
{

	if (iface->rxbuf[SRIC_LEN] != 2)
		return 0;

	note_enable(iface->rxbuf[SRIC_DATA+1]);
	return 0;
}

/* Receives a control byte and up to 12 consecutive piezo_note_t's. Control
 * byte lower nibble describes the number of piezo_note_t's that are in this
 * frame, top nibble is for flags. Only flag currently used is the topmost,
 * which indicates whether a notification should be sent when the piezo
 * buffer gets low */
uint8_t
sric_play_piezo(const sric_if_t *iface)
{
	uint8_t piezo_input_ctl, num_notes, i;

	if (iface->rxbuf[SRIC_LEN] < 2)
		return 0;

	piezo_input_ctl = iface->rxbuf[SRIC_DATA+1];
	num_notes = piezo_input_ctl & 0xF;

	if (iface->rxbuf[SRIC_LEN] < 2 + num_notes * 5)
		/* Insufficient data for buffer */
		return 0;

	for( i=0; i<num_notes; i++ ) {
		piezo_note_t p;
		uint8_t *note = iface->rxbuf + SRIC_DATA + 2 + (i*5);

		p.f = (((uint16_t)note[0]) << 8) | note[1];
		p.d = (((uint16_t)note[2]) << 8) | note[3];
		p.v = note[4];

		piezo_play( &p, 1, (piezo_input_ctl & 0x80) ? true : false);
	}

	return 0;
}

/* Receives a single byte: bits 0, 1 and 2 of that byte indicate whether the
 * corresponding user LED should be set on or off */
uint8_t
sric_set_leds(const sric_if_t *iface)
{
	uint8_t leds;

	if (iface->rxbuf[SRIC_LEN] != 2)
		return 0;

	leds = iface->rxbuf[SRIC_DATA+1];
	uled_set(0, leds & 1);
	uled_set(1, leds & 2);
	uled_set(2, leds & 4);
	return 0;
}

/* Receives a single byte: bit 0 controls the motor rail */
uint8_t sric_motor_rail(const sric_if_t *iface)
{
	if (iface->rxbuf[SRIC_LEN] != 2)
		return 0;

	if (iface->rxbuf[SRIC_DATA+1])
		power_motor_enable();
	else
		power_motor_disable();

	return 0;
}

/* Takes no input, responds with one byte detailing the current user led
 * configuration */
uint8_t
sric_get_leds(const sric_if_t *iface)
{
	uint8_t resp;

	resp = 0;
	if (uled_get(0) != 0)
		resp |= 1;
	if (uled_get(1) != 0)
		resp |= 2;
	if (uled_get(2) != 0)
		resp |= 4;

	iface->txbuf[SRIC_DATA] = resp;
	return 1;
}

/* Takes no input, responds with 4 bytes as follows:
 *   voltage_lsb - Battery voltage, least significant byte
 *   voltage_msb - Battery voltage, most significant byte
 *   current_lsb - Battery current, least significant byte
 *   current_msb - Battery current, most significant byte */
uint8_t
sric_get_vi(const sric_if_t *iface)
{
	uint16_t v, i;
	v = monitor_get_voltage();
	i = monitor_get_current();

	iface->txbuf[SRIC_DATA+0] = v & 0xff;
	iface->txbuf[SRIC_DATA+1] = (v >> 8);
	iface->txbuf[SRIC_DATA+2] = i & 0xff;
	iface->txbuf[SRIC_DATA+3] = (i >> 8);
	return 4;
}

/* Takes no input, responds with 2 uint16_t with the max stack size and
 * max stack usage */
uint8_t
sric_get_stack_usage(const sric_if_t *iface)
{
	uint16_t s_usage = stack_max_usage();
	uint16_t s_size = stack_size;
	iface->txbuf[SRIC_DATA+0] = s_size & 0xff;
	iface->txbuf[SRIC_DATA+1] = s_size >> 8;
	iface->txbuf[SRIC_DATA+2] = s_usage & 0xff;
	iface->txbuf[SRIC_DATA+3] = s_usage >> 8;
	return 4;
}
