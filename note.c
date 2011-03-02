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
static volatile uint16_t last_edges;
static volatile bool note_send_trigger;
static bool note_send_enable = false;

static const sched_task_t note_trigger = {
	.t = NOTE_PERIOD,
	.cb = note_trigger_send
};

void
note_enable(uint8_t enable)
{
	bool old;

	old = note_send_enable;

	if (enable == 0 && old) {
		note_send_enable = false;
		sched_rem(&note_trigger);
	} else if (enable != 0 && !old) {
		note_send_enable = true;
		sched_add(&note_trigger);
	}

	return;
}

void
note_recv_input(uint16_t flags, uint16_t edge)
{
        /* See input.h: flags register button presses, edge determines their
	 * current states. We store all presses to be sent to the BB, and
	 * the most recent pressed state. These are sent raw to the BB at
	 * intervals: there's no point decoding them here */
        inp_flags |= flags;
        last_edges = edge;
        return;
}

bool
note_trigger_send(void *ud)
{

	note_send_trigger = true;
	return true;
}

void
note_poll()
{

	dint();
	if (note_send_enable && note_send_trigger) {
		uint16_t sampled_flags = inp_flags;
		inp_flags = 0;
		note_send_trigger = false;
		eint();

		/* We have some pressed flags and current edges to send */
		gw_sric_if.tx_lock();
		gw_sric_if.txbuf[SRIC_DEST] = 1;/* Director */
		gw_sric_if.txbuf[SRIC_SRC] = sric_addr;
		gw_sric_if.txbuf[SRIC_LEN] = 5; /* NoteID, flags and edges */
		gw_sric_if.txbuf[SRIC_DATA] = 0;/* Note 0 */
		gw_sric_if.txbuf[SRIC_DATA+1] = sampled_flags & 0xFF;
		gw_sric_if.txbuf[SRIC_DATA+2] = sampled_flags >> 8;
		gw_sric_if.txbuf[SRIC_DATA+3] = last_edges & 0xFF;
		gw_sric_if.txbuf[SRIC_DATA+4] = last_edges >> 8;

		gw_sric_if.tx_cmd_start(SRIC_OVERHEAD + 4, true);
	} else {
		eint();
	}

	return;
}
