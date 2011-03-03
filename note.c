#include <io.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>

#include "drivers/sched.h"

#include "libsric/sric.h"
#include "libsric/sric-gw.h"

#include "note.h"

bool note_trigger_send(void *ud);

static volatile uint16_t inp_flags;
static volatile uint16_t inp_edges;
static volatile bool note_send_trigger;
static volatile bool send_piezo_note;
static volatile uint8_t spare_piezo_notes;
static bool note_send_enable = false;

void
note_enable(uint8_t enable)
{

	note_send_enable = enable;
	return;
}

void
note_recv_input(uint16_t flags, uint16_t edge)
{

	inp_flags = flags;
	inp_edges = edge;
	note_send_trigger = true;

	return;
}

void
piezo_send_piezo_note(uint8_t free_space)
{
	spare_piezo_notes = free_space;
	send_piezo_note = true;
	return;
}

void
note_poll()
{

	dint();
	if (note_send_enable && note_send_trigger) {
		uint16_t sampled_flags = inp_flags;
		uint16_t sampled_edges = inp_edges;
		inp_flags = 0;
		note_send_trigger = false;
		eint();

		if (sampled_flags == 0)
			return;

		/* We have some pressed flags and current edges to send */
		gw_sric_if.tx_lock();
		gw_sric_if.txbuf[SRIC_DEST] = 1;/* Director */
		gw_sric_if.txbuf[SRIC_SRC] = sric_addr;
		gw_sric_if.txbuf[SRIC_LEN] = 5; /* NoteID, flags and edges */
		gw_sric_if.txbuf[SRIC_DATA] = 0;/* Note 0 */
		gw_sric_if.txbuf[SRIC_DATA+1] = sampled_flags & 0xFF;
		gw_sric_if.txbuf[SRIC_DATA+2] = sampled_flags >> 8;
		gw_sric_if.txbuf[SRIC_DATA+3] = sampled_edges & 0xFF;
		gw_sric_if.txbuf[SRIC_DATA+4] = sampled_edges >> 8;

		gw_sric_if.tx_cmd_start(SRIC_OVERHEAD + 4, true);
	} else {
		eint();
	}

	if (send_piezo_note) {
		gw_sric_if.tx_lock();

		gw_sric_if.txbuf[SRIC_DEST] = 1;
		gw_sric_if.txbuf[SRIC_SRC] = sric_addr;
		gw_sric_if.txbuf[SRIC_LEN] = 2;

		gw_sric_if.txbuf[SRIC_DATA] = 1;
		gw_sric_if.txbuf[SRIC_DATA+1] = spare_piezo_notes;

		gw_sric_if.tx_cmd_start(6, true);
	}

	return;
}
