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
#include "flash430/sric-flash.h"

const sric_cmd_t sric_commands[] = {
	{sric_flashr_fw_ver},
	{sric_flashw_fw_chunk},
	{sric_flashr_fw_next},
	{sric_flashr_crc},
	{sric_flashw_confirm},
};

const uint8_t sric_cmd_num = sizeof(sric_commands) / sizeof(const sric_cmd_t);
