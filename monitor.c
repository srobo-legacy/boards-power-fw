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
/* Used to calculate a moving average over 4 samples */
uint16_t batt_voltage_ma_sum = 13107; /* 12V * 4 */

#define BATT_CURRENT_OFFSET 683
#define CHARGER_PRESENT_VOLTAGE 3741 /* 13.7V */
#define BATTERY_NORMAL_VOLTAGE 3031 /* 11.1V */
#define BATTERY_FLAT_VOLTAGE 2785 /* 10.2V */
#define BATTERY_VFLAT_VOLTAGE 2730 /* 10V */

#define BATT_OK 0xff

bool monitor_check(void* ud);

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

	sched_add(&check_task);
}

const piezo_note_t batt_flat_tune[] = {{.f=1000, .d=800, .v=5}};

bool monitor_check(void *ud) {
	/* --- VOLTAGE RAIL MONITORING --- */
	uint16_t batt_voltage_ma = batt_voltage_ma_sum/4;
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

	/* Flag used to indicate if the battery is flat. Once set the board
	 * will beep until powered off. This is also used as a counting
	 * variable so limit the beep rate while still flashing the LED */
	static uint8_t batt_flat = BATT_OK;

	if (batt_voltage_ma < BATTERY_FLAT_VOLTAGE && batt_flat == BATT_OK) {
		/* The battery is pretty much flat, turn off the motor rail.
		 * However keep the BeagleBoard and LCD powered. */
		power_motor_disable(POWER_MOTOR_CHARGER);
		batt_flat = 0;
	}

	if (batt_flat != BATT_OK) {
		chrg_toggle();
		if (batt_flat++ == 3) {
			piezo_play(batt_flat_tune, 1, false);
			batt_flat=0;
		}
	}

	return true;
}
