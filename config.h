#ifndef __CONFIG_H
#define __CONFIG_H

/* Which timer module to use for the schedule module
 * 0 = TIMER_A
 * 1 = TIMER_B */
#define SCHED_TIMER_B 1

/* Amount to divide SMCLK by to give a 1kHz timer */
#define SCHED_DIV 8000

/* Size of the queue for schedule tasks */
#define SCHED_QUEUE_SIZE 2

/* Length of the piezo tune buffer, each note is 6 bytes */
#define PIEZO_BUFFER_LEN 64

#endif /* __CONFIG_H */
