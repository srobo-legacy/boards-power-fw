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

#include <stdbool.h>
#include <io.h>
#include <signal.h>
#include "monitor.h"
#include "piezo.h"
#include "drivers/pinint.h"
#include "drivers/sched.h"

#define ISENSE (1<<0)
#define VSENSE (1<<1)
#define VPUMP  (1<<2)
#define VMOTOR (1<<3)

#define field_set(x, val, mask) do { x &= ~mask; x |= val; } while(0)
#define CDETECT (1<<7)
#define SET_CDETECT_EDGE(e) field_set(P2IES, e ? CDETECT:0, CDETECT)

uint16_t batt_voltage=0;
uint16_t batt_current=0;
uint16_t motor_voltage=0;
uint16_t pump_voltage=0;
bool charger_present = false;

void monitor_cdetect_cb(uint16_t flags);
bool monitor_cdetect_task_cb(void* ud);
bool monitor_charger_check(void* ud);

static const pinint_conf_t cdetect_int = {
	.mask = CDETECT << 8,
	.int_cb = monitor_cdetect_cb,
};

interrupt (ADC12_VECTOR) adc_isr(void) {
	uint8_t adc12v_l = ADC12IV;

	if (adc12v_l == 0x08) {
		batt_current = ADC12MEM0;
		batt_voltage = ADC12MEM1;
		pump_voltage = ADC12MEM2;
		motor_voltage = ADC12MEM3;
		ADC12IFG &= ~0x02;
	} else {
		ADC12IFG = 0;
	}
}

/* Check to see if the charger is present and charging the battery
 * every 5 seconds */
static sched_task_t charger_check_task = {.cb=monitor_charger_check, .t=5000};

void monitor_init(void) {
	/* Init ADC stuff */
	P6DIR &= ~(ISENSE|VSENSE|VPUMP|VMOTOR);
	P6SEL |=  (ISENSE|VSENSE|VPUMP|VMOTOR); /* Disable digital inputs*/

	ADC12CTL0 = ADC12ON;
	ADC12CTL0 |= SHT0_0 | SHT1_0 /* 4 ADC12CLK cycles sample-and-hold */
	           | MSC             /* Multi sample */
	           | REF2_5V         /* 2.5V reference voltage */
	           | REFON           /* Turn reference generator on */
	           | ADC12ON;        /* Turn ADC12 module on */

	ADC12CTL1 = CSTARTADD_0      /* Start conversion from channel 0 */
	          | SHS_ADC12SC      /* Sample-and-hold from ADC12SC */
	          | SHP
	          | ADC12DIV_7       /* No division of ADC12 clock */
	          | ADC12SSEL_0      /* Clock sourced from ADC12OSC */
	          | CONSEQ_REPEAT_SEQUENCE; /* Repeat conversion of channels */

	ADC12MCTL0 = SREF_1   /* V+ = Internal Ref; V- = GND */
	           | INCH_0;  /* Channel A0, battery current */

	ADC12MCTL1 = SREF_1   /* V+ = Internal Ref; V- = GND */
	           | INCH_1;  /* Channel A1, battery voltage */

	ADC12MCTL2 = SREF_1   /* V+ = Internal Ref; V- = GND */
	           | INCH_2;  /* Channel A2, charge-pump voltage */

	ADC12MCTL3 = EOS      /* End of Sequence */
	           | SREF_1   /* V+ = Internal Ref; V- = GND */
	           | INCH_3;  /* Channel A3, motor rail voltage */

	ADC12IE = 0x2;      /* Interrupt on last conversion */

	ADC12CTL0 |= ENC;
	ADC12CTL0 |= ADC12SC;

	/* Init charger detection stuff */
	pinint_add( &cdetect_int );
	P2DIR &= ~CDETECT;
	P2SEL &= ~CDETECT;
	SET_CDETECT_EDGE(IO_IESPIN_FALLING);
	P2IFG &= ~CDETECT;
	P2IE  |=  CDETECT;

	sched_add(&charger_check_task);
}

piezo_note_t ch_in[]  = {{.f=600, .d=200, .v=3}, {.f=800, .d=200, .v=3}};
piezo_note_t ch_out[] = {{.f=800, .d=200, .v=3}, {.f=600, .d=200, .v=3}};

static sched_task_t cdetect_task = {.cb=monitor_cdetect_task_cb, .t=200};
static bool cdetect_waiting = false;

void monitor_cdetect_cb(uint16_t flags) {
	if (!cdetect_waiting) {
		cdetect_waiting = true;
		sched_add(&cdetect_task);
	}
}

bool monitor_cdetect_task_cb(void *ud) {
	if (P2IN & CDETECT) {
		SET_CDETECT_EDGE(IO_IESPIN_FALLING);
	} else {
		SET_CDETECT_EDGE(IO_IESPIN_RISING);
	}
	monitor_charger_check(NULL);
	cdetect_waiting = true;
	return false;
}

bool monitor_charger_check(void *ud) {
	/* Check to see if the thing is charging */
	return true;
}
