#include <io.h>
#include "leds.h"
#include "piezo.h"
#include "power.h"

void init(void) {
	leds_init();
	piezo_init();
	power_init();
}

int main(void) {
	/* Disable watchdog timer */
	WDTCTL = WDTHOLD | WDTPW;

	init();
	led_set(0, 1);

	while(1);
}
