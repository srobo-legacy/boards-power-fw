#include <io.h>
#include "leds.h"
#include "power.h"

int main(void) {
	/* Disable watchdog timer */
	WDTCTL = WDTHOLD | WDTPW;

	leds_init();
	power_init();
	led_set(0, 1);

	while(1);
}
