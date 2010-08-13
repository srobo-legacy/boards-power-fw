#include <io.h>
#include "leds.h"

int main(void) {
	/* Disable watchdog timer */
	WDTCTL = WDTHOLD | WDTPW;

	leds_init();
	led_set(0, 1);

	while(1);
}
