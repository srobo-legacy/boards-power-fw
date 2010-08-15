#include <io.h>
#include "power.h"

#define MOTOR_EN_PIN (1<<0)
#define BL_EN_PIN    (1<<1)
/* BB 5V rail enable line is active low */
#define BB_EN_PIN    (1<<2)

void power_init(void) {
	P5OUT &= ~(MOTOR_EN_PIN | BL_EN_PIN);
	P5OUT |= BB_EN_PIN;
	P5DIR |= MOTOR_EN_PIN | BL_EN_PIN | BB_EN_PIN;
}

void power_bl_enable(void) {
	P5OUT |= BL_EN_PIN;
}
void power_bl_disable(void) {
	P5OUT &= ~BL_EN_PIN;
}

void power_motor_enable(void) {
	P5OUT |= MOTOR_EN_PIN;
}
void power_motor_disable(void) {
	P5OUT &= ~MOTOR_EN_PIN;
}

void power_bb_enable(void) {
	P5OUT &= ~BB_EN_PIN;
}
void power_bb_disable(void) {
	P5OUT |= BB_EN_PIN;
}
