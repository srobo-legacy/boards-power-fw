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
#include "flash430/sric-flash.h"

uint8_t sric_enable_input_notes( const sric_if_t *iface );
uint8_t sric_play_piezo( const sric_if_t *iface );
uint8_t sric_set_leds( const sric_if_t *iface );

const sric_cmd_t sric_commands[] = {
	{sric_flashr_fw_ver},
	{sric_flashw_fw_chunk},
	{sric_flashr_fw_next},
	{sric_flashr_crc},
	{sric_flashw_confirm},
	{sric_enable_input_notes},
	{sric_play_piezo},
	{sric_set_leds},
};

const uint8_t sric_cmd_num = sizeof(sric_commands) / sizeof(const sric_cmd_t);

uint8_t
sric_enable_input_notes( const sric_if_t *iface )
{

	if (iface->rxbuf[SRIC_LEN] != 2)
		return 0;

	note_enable(iface->rxbuf[SRIC_DATA+1]);
	return 0;
}

uint8_t
sric_play_piezo( const sric_if_t *iface )
{
	uint8_t piezo_input_ctl;

	if (iface->rxbuf[SRIC_LEN] < 2)
		return 0;

	piezo_input_ctl = iface->rxbuf[SRIC_DATA+1];
	if (iface->rxbuf[SRIC_LEN] < 2 + (piezo_input_ctl & 0xF))
		/* Insufficient data for buffer */
		return 0;

	piezo_play((piezo_note_t *)&iface->rxbuf[SRIC_DATA+2],
				piezo_input_ctl & 0xF,
				(piezo_input_ctl & 0x80) ? true : false);
	return 0;
}

uint8_t
sric_set_leds( const sric_if_t *iface )
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
