/*   Copyright (C) 2010 Richard Barlow, Robert Spanton

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
#include "drivers/xt2.h"
#include "drivers/sched.h"
#include "drivers/pinint.h"
#include "leds.h"
#include "piezo.h"
#include "power.h"
#include "monitor.h"
#include "input.h"
#include "post.h"
#include "drivers/usart.h"
#include "libsric/sric.h"
#include "libsric/hostser.h"
#include "libsric/sric-gw.h"
#include "libsric/token-dir.h"
#include "libsric/sric-client.h"

piezo_config_t piezo_config = {
	.buf_low = NULL,
};

input_conf_t input_conf = {
	/* This should not be set to the SRIC function
	 * until after POST */
	.inp_cb = NULL,
};

const usart_t usart_config[2] = {
        {
                .tx_gen_byte = sric_tx_cb,
                .rx_byte = sric_rx_cb,

		/* 115200 baud -- values from http://mspgcc.sourceforge.net/baudrate.html */
                .br0 = 0x45,
                .br1 = 0x00,
		.mctl = 0xAA,

                .sel_rx = &P3SEL,
                .sel_tx = &P3SEL,
                .sel_rx_num = 5,
                .sel_tx_num = 4,
        },

        {
                .tx_gen_byte = hostser_tx_cb,
                .rx_byte = hostser_rx_cb,

                /* 115200 baud -- values from http://mspgcc.sourceforge.net/baudrate.html */
                .br0 = 0x45,
                .br1 = 0x00,
        	.mctl = 0xAA,

                .sel_rx = &P3SEL,
                .sel_tx = &P3SEL,
                .sel_rx_num = 7,
                .sel_tx_num = 6,
        }
};

const hostser_conf_t hostser_conf = {
        .usart_tx_start = usart_tx_start,
        .usart_tx_start_n = 1,

        /* Send received frames to the gateway */
        .rx_cb = sric_gw_hostser_rx,
        /* Notify the gateway when transmission is complete */
        .tx_done_cb = sric_gw_hostser_tx_done,
};

const sric_conf_t sric_conf = {
        .usart_tx_start = usart_tx_start,
        .usart_rx_gate = usart_rx_gate,
        .usart_n = 0,

	.txen_dir = &P3DIR,
	.txen_port = &P3OUT,
	.txen_mask = (1<<0),

        /* Send received frames to the gateway */
        .rx_cmd = sric_gw_sric_rxcmd,
        .rx_resp = sric_gw_sric_rxresp,
	.error = sric_gw_sric_err,
	.token_drv = &token_dir_drv,
};

const token_dir_conf_t token_dir_conf = {
	.haz_token = sric_haz_token,

	.to_port = &P3OUT,
	.to_dir = &P3DIR,
	.to_mask = (1<<1),

	.ti_port = &P1IN,
	.ti_dir = &P1DIR,
	.ti_mask = (1<<0),
};

const sric_client_conf_t sric_client_conf = {
	.devclass = SRIC_CLASS_POWER,
};

void init_board(void) {
	leds_init();

	/* Start crystal osc. and source MCLK from it.
	 * LED stuck on indicates fault */
	dbg_set(1);
	xt2_start();
	dbg_set(0);

	/* Source SMCLK from XT2 */
	BCSCTL2 |= SELS;

	pinint_init();
	sched_init();
	piezo_init();
	power_init();
	monitor_init();
	input_init();
}

void init_sric(void) {
	/* Bring the SRIC related stuff up: */
	usart_init();
	hostser_init();
	sric_init();
	sric_gw_init();
	token_dir_init();

	/* Eventually this will be a function to send notes
	 * to the BeagleBoard */
	input_conf.inp_cb = NULL;

	eint();

	while (1)
		;
}

int main(void) {
	/* Disable watchdog timer */
	WDTCTL = WDTHOLD | WDTPW;

	init_board();

	/* Run POST. Requires interrupts to be enabled but they
	 * should be disabled while init'ing SRIC */
	eint();
	post();
	dint();

	init_sric();

	led_set(0, 1);
	piezo_beep();
	power_motor_enable();
	//power_bb_enable();
	//power_bl_enable();


	while(1);
}
