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

#include <io.h>
#include "input.h"
#include "pinint.h"
#include "leds.h"

#define BUT0 (1<<1)
#define BUT1 (1<<3)
#define BUT2 (1<<4)

#define RBUT0 (1<<(2+8)) /* On P2 so shift by 8 for flags */
#define RBUT1 (1<<7)

#define R0A (1<<(0+8))
#define R0B (1<<(1+8))
#define R1A (1<<5)
#define R1B (1<<6)

#define P1SIGNALS (BUT0 | BUT1 | BUT2 | RBUT1 | R1A | R1B)
#define P2SIGNALS ((RBUT0 | R0A | R0B) >> 8)

#define TEST_R_A(n) (n ? P1IN & R1A : P2IN & (R0A>>8))
#define TEST_R_B(n) (n ? P1IN & R1B : P2IN & (R0B>>8))

#define EDGE_RISING 0
#define EDGE_FALLING 1
/* n = rotary encoder (0 or 1)
 * inp = input pin (A or B)
 * e = edge (0 = rising, 1 = falling) */
#define SET_R_INT_EDGE(n, inp, e) do { \
                                  if (!n) { \
                                      if (e) \
                                          P2IES |=  (R0##inp >> 8); \
                                      else \
                                          P2IES &= ~(R0##inp >> 8); \
                                  } else { \
                                      if (e) \
                                          P1IES |=  R1##inp; \
                                      else \
                                          P1IES &= ~R1##inp; \
                                  } } while(0)

#define EN_RB_INT(n) do { \
                     if (!n) P2IE |= (R0B >> 8); \
                     else    P1IE |= R1B; \
                     } while(0)
#define DIS_RB_INT(n) do { \
                      if (!n) P2IE &= ~(R0B >> 8); \
                      else    P1IE &  ~R1B; \
                      } while(0)


typedef enum {
	EV_A_RISE, /* A rising  */
	EV_A_FALL, /* A falling */
	EV_B_RISE, /* B rising  */
	EV_B_FALL, /* B falling */
} input_rot_event_t;

typedef enum {
	S_AH,    /* A high, idle state */
	S_AL_BH, /* A low, B high, anti-clockwise when entered from S_AL_BL */
	S_AL_BL, /* Alow, B low, clockwise when entered from S_AL_BH */
} input_rot_state_t;

static input_rot_state_t input_rot_state[2] = {S_AH, S_AH};

void but_isr(uint16_t flags);
static void input_rot_fsm(uint8_t n, input_rot_event_t ev);

void input_init(void) {
	pinint_conf[PININT_BUTTON].mask =
	        (BUT0 | BUT1 | BUT2 | RBUT0 | RBUT1 | R0A | R0B | R1A | R1B);
	pinint_conf[PININT_BUTTON].int_cb = but_isr;

	P1DIR &= ~P1SIGNALS; /* Set to inputs */
	P2DIR &= ~P2SIGNALS;

	P1SEL &= ~P1SIGNALS; /* GPIO function */
	P2SEL &= ~P2SIGNALS;

	P1IES |=  P1SIGNALS; /* Int on High-Low transition */
	P2IES |=  P2SIGNALS;

	P1IFG &= ~P1SIGNALS; /* Clear interrupt flags */
	P2IFG &= ~P2SIGNALS;

	P1IE  |=  P1SIGNALS; /* Enable interrupts on those pins */
	P2IE  |=  P2SIGNALS;
}

void but_isr(uint16_t flags) {
	if (flags & BUT0) {
		led_toggle(0);
	}
	if (flags & BUT1) {
		led_toggle(1);
	}
	if (flags & BUT2) {
		led_toggle(2);
	}
	if (flags & RBUT0) {
		uled_toggle(0);
	}
	if (flags & RBUT1) {
		uled_toggle(1);
	}

	if (flags & R0A) {
		if (TEST_R_A(0))
			input_rot_fsm(0, EV_A_RISE);
		else
			input_rot_fsm(0, EV_A_FALL);
	} else if (flags & R0B) {
		if (TEST_R_B(0))
			input_rot_fsm(0, EV_B_RISE);
		else
			input_rot_fsm(0, EV_B_FALL);
	}
}

static void input_rot_fsm(uint8_t n, input_rot_event_t ev) {
	switch (input_rot_state[n]) {
	case S_AH:
		if (ev == EV_A_FALL && !TEST_R_B(n)) {
			SET_R_INT_EDGE(n, A, EDGE_RISING);
			SET_R_INT_EDGE(n, B, EDGE_RISING);
			EN_RB_INT(n);
			input_rot_state[n] = S_AL_BL;
		} else if (ev == EV_A_FALL && TEST_R_B(n)) {
			SET_R_INT_EDGE(n, A, EDGE_RISING);
			SET_R_INT_EDGE(n, B, EDGE_FALLING);
			EN_RB_INT(n);
			input_rot_state[n] = S_AL_BH;
		}
		break;
	case S_AL_BH:
		if (ev == EV_B_FALL) {
			SET_R_INT_EDGE(n, B, EDGE_RISING);
			uled_toggle(0);
			input_rot_state[n] = S_AL_BL;
		} else if (ev == EV_A_RISE && TEST_R_B(n)) {
			SET_R_INT_EDGE(n, A, EDGE_FALLING);
			DIS_RB_INT(n);
			input_rot_state[n] = S_AH;
		} else if (ev == EV_A_RISE && !TEST_R_B(n)) {
			/* Something strange happened, just return to idle */
			SET_R_INT_EDGE(n, A, EDGE_FALLING);
			DIS_RB_INT(n);
			input_rot_state[n] = S_AH;
		}
		break;
	case S_AL_BL:
		if (ev == EV_B_RISE) {
			SET_R_INT_EDGE(n, B, EDGE_FALLING);
			uled_toggle(1);
			input_rot_state[n] = S_AL_BH;
		} else if (ev == EV_A_RISE && !TEST_R_B(n)) {
			SET_R_INT_EDGE(n, A, EDGE_FALLING);
			DIS_RB_INT(n);
			input_rot_state[n] = S_AH;
		} else if (ev == EV_A_RISE && TEST_R_B(n)) {
			/* Something strange happened, just return to idle */
			SET_R_INT_EDGE(n, A, EDGE_FALLING);
			DIS_RB_INT(n);
			input_rot_state[n] = S_AH;
		}
		break;
	}
}
