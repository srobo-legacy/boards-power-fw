#ifndef __NOTE_H
#define __NOTE_H
#include <stdbool.h>
#include <stdint.h>

void note_recv_input(uint16_t flags, uint16_t edge);
void note_enable(uint8_t enable);
void note_poll();

#endif	/* __NOTE_H */
