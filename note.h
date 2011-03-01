#include <stdbool.h>
#include <stdint.h>

/* How many times to send input notes a second */
#define NOTE_FREQ		20
/* Actual number of ms to wait between notes */
#define NOTE_PERIOD		1000/NOTE_FREQ

void note_recv_input(uint16_t flags, uint16_t edge);
void note_enable(uint8_t enable);
void note_poll();
