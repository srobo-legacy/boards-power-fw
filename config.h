#ifndef __CONFIG_H
#define __CONFIG_H

/* Which timer module to use for the schedule module
 * 0 = TIMER_A
 * 1 = TIMER_B */
#define SCHED_TIMER_B 1

/* Amount to divide SMCLK by to give a 1kHz timer */
#define SCHED_DIV 8000

/* Size of the queue for schedule tasks */
#define SCHED_QUEUE_SIZE 4

/* Whether the PIEZO should be enabled */
#define PIEZO_ENABLED 1

/* Length of the piezo tune buffer, each note is 6 bytes */
#define PIEZO_BUFFER_LEN 16

/* Whether we're the bus director
   If 1, this causes the device to take address 1, generate and manage
   the token. */
#define SRIC_DIRECTOR 1

/* Number of pin interrupt table entries */
#define PININT_NCONF 4

#endif /* __CONFIG_H */
