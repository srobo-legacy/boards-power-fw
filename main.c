#include <io.h>
#include "leds.h"

int main(void) {
	leds_init();
	led_set(0, 1);

	while(1);
}
