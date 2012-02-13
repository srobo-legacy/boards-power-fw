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
#include "leds.h"
#include "power.h"

#define ISENSE (1<<0)
#define VSENSE (1<<1)
#define VPUMP  (1<<2)
#define VMOTOR (1<<3)

#define CDETECT (1<<7)
#define SET_CDETECT_EDGE(e) do { if (e) P2IES |= CDETECT; \
                                 else P2IES &= ~CDETECT; } while(0)

uint16_t batt_voltage=0;
int16_t batt_current=0;
uint16_t motor_voltage=0;
uint16_t pump_voltage=0;
bool charger_present = false;
bool batt_flat = false;
/* Used to calculate a moving average over 4 samples */
uint16_t batt_voltage_ma_sum = 13107; /* 12V * 4 */

#define BATT_CURRENT_OFFSET 683
#define CHARGER_PRESENT_VOLTAGE 3741 /* 13.7V */
#define BATTERY_NORMAL_VOLTAGE 3031 /* 11.1V */
#define BATTERY_FLAT_VOLTAGE 2785 /* 10.2V */
#define BATTERY_VFLAT_VOLTAGE 2730 /* 10V */

void monitor_cdetect_cb(uint16_t flags);
bool monitor_cdetect_task_cb(void* ud);
bool monitor_check(void* ud);

static const pinint_conf_t cdetect_int = {
	.mask = CDETECT << 8,
	.int_cb = monitor_cdetect_cb,
};

interrupt (ADC12_VECTOR) adc_isr(void) {
	uint8_t adc12v_l = ADC12IV;

	if (adc12v_l == 0x08) {
		batt_current = ADC12MEM0 - BATT_CURRENT_OFFSET;
		batt_voltage = ADC12MEM1;
		pump_voltage = ADC12MEM2;
		motor_voltage = ADC12MEM3;
		ADC12IFG &= ~0x02;
	} else {
		ADC12IFG = 0;
	}
}

/* Check the state of the voltage rails and charger ever second */
static const sched_task_t check_task = {.cb=monitor_check, .t=1000};

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

	sched_add(&check_task);
	/* Simulate a charger plug/unplug event to get the code
	 * into the correct state */
	monitor_cdetect_cb(0);
}

piezo_note_t ch_in[]  = {{.f=600, .d=200, .v=3}, {.f=800, .d=200, .v=3}};
piezo_note_t ch_out[] = {{.f=800, .d=200, .v=3}, {.f=600, .d=200, .v=3}};
piezo_note_t batt_flat_tune[] = {{.f=1000, .d=800, .v=5}};

static const sched_task_t cdetect_task = {.cb=monitor_cdetect_task_cb, .t=200};
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
		power_motor_enable(POWER_MOTOR_CHARGER);
	} else {
		SET_CDETECT_EDGE(IO_IESPIN_RISING);
		power_motor_disable(POWER_MOTOR_CHARGER);
	}
	monitor_check(NULL);
	cdetect_waiting = false;
	return false;
}

bool monitor_check(void *ud) {
	/* --- CHARGER DETECTION --- */

	bool charger_present_tmp;

	if (P2IN & CDETECT) {
		/* No charger plugged in, cannot be charging no matter what
		 * other measurement say */
		charger_present_tmp = false;
	} else if (batt_current < 0 ||
	    (batt_current < 20 && batt_voltage > CHARGER_PRESENT_VOLTAGE)) {
		/* Definitely charging as the current is negative or the
		 * current isn't negative but the voltage indicates
		 * that the charge is still connected and switched on */
		charger_present_tmp = true;
	} else {
		/* Current is positive and the voltage is low, running
		 * on the battery and not charging */
		charger_present_tmp = false;
	}

	if (charger_present == true && charger_present_tmp == false) {
		/* Charger removed */
		/*piezo_play(ch_out, 2, false);*/
		chrg_set(0);
	} else if (charger_present == false && charger_present_tmp == true) {
		/* Charger plugged in */
		/*piezo_play(ch_in, 2, false);*/
		chrg_set(1);
	}

	charger_present = charger_present_tmp;


	/* --- VOLTAGE RAIL MONITORING --- */
	uint16_t batt_voltage_ma = batt_voltage_ma_sum/4;
	/* Only update the moving average when the charger isn't plugged in */
	if (!charger_present)
		batt_voltage_ma_sum = batt_voltage_ma_sum
		                      + batt_voltage - batt_voltage_ma;

	if (batt_voltage < BATTERY_VFLAT_VOLTAGE) {
		/* Perform emergency shutdown */
		power_motor_disable(POWER_MOTOR_CHARGER);
		power_bl_disable();
		power_bb_disable();
		piezo_play(batt_flat_tune, 1, false);
		dint();
		while(1) {
			/* Prod WDT */
			WDTCTL = WDTCNTCL | WDTPW;
		}
	}

	if (batt_voltage_ma < BATTERY_FLAT_VOLTAGE) {
		/* The battery is pretty much flat, turn off the motor rail.
		 * However keep the BeagleBoard and LCD powered. */
		power_motor_disable(POWER_MOTOR_CHARGER);
		batt_flat = true;
	}

	if (batt_flat && !charger_present) {
		static uint8_t i = 3;
		chrg_toggle();
		if (i++ == 3) {
			piezo_play(batt_flat_tune, 1, false);
			i=0;
		}
		/* The battery has been recharged to a suitable level */
		if (batt_voltage_ma > BATTERY_NORMAL_VOLTAGE) {
			power_motor_enable(POWER_MOTOR_CHARGER);
			batt_flat = false;
			chrg_set(0);
		}
	}

	return true;
}
