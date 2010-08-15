#ifndef __POWER_H
#define __POWER_H
/* Functions for controlling various power rails */

void power_init(void);

/* Turn on/off LCD backlight SMPS */
void power_bl_enable(void);
void power_bl_disable(void);

/* Turn on/off the 'motor rail' */
void power_motor_enable(void);
void power_motor_disable(void);

/* Turn on/off the BeagleBoard (+ USB) 5V rail */
void power_bb_enable(void);
void power_bb_disable(void);

#endif /* __POWER_H */
