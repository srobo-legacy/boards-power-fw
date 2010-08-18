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
#include <signal.h>
#include "monitor.h"

#define ISENSE (1<<0)
#define VSENSE (1<<1)
#define VPUMP  (1<<2)
#define VMOTOR (1<<3)

uint16_t batt_voltage=0;
uint16_t batt_current=0;

interrupt (ADC12_VECTOR) adc_isr(void) {
	uint8_t adc12v_l = ADC12IV;

	if (adc12v_l == 0x08) {
		batt_current = ADC12MEM0;
		batt_voltage = ADC12MEM1;
		ADC12IFG &= ~0x02;
	} else {
		ADC12IFG = 0;
	}
}

void monitor_init(void) {
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

	ADC12MCTL1 = EOS      /* End of Sequence */
	         | SREF_1   /* V+ = Internal Ref; V- = GND */
	         | INCH_1;  /* Channel A1, battery voltage */

	ADC12IE = 0x2;      /* Interrupt on last conversion */

	ADC12CTL0 |= ENC;
	ADC12CTL0 |= ADC12SC;
}

