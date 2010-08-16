#include <io.h>
#include "piezo.h"

#define PIEZO_PIN (1<<2)

void piezo_init(void) {
	P1OUT &= ~PIEZO_PIN;
	P1DIR |= PIEZO_PIN;
}

void piezo_play(void) {
}

void piezo_beep(void) {
}

void piezo_beep_pattern(char *pattern) {
}
