#ifndef __PIEZO_H
#define __PIEZO_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	/* Frequency in Hertz */
	uint16_t f;
	/* Duration in 100s of milliseconds (a value of '2' is a duration
	 * of 0.2s */
	uint8_t d;
} piezo_note_t;

/* An instance named 'piezo_conf' should be linked in */
typedef struct {
	/* Callback to get next note to be played, returns false when there's
	 * no more notes to be played. */
	bool (*gen_note) (piezo_note_t *n);
} piezo_config_t;

void piezo_init(void);

/* Begins playback of a tune.
 * Notes to be played will be obtained through the 'gen_note' callback */
void piezo_play(void);

#endif /* __PIEZO_H */
