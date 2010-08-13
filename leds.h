#ifndef __LEDS_H
#define __LEDS_H

/* Debugging and SRIC activity LED above SRIC sockets */
#define LED_DBG_OUT P5OUT
#define LED_DBG (1<<7)

/* LED labelled 'CHARGE' next to power LEDs */
#define LED_CHRG_OUT P4OUT
#define LED_CHRG (1<<1)

/* System LEDs (next to buttons) numbered clockwise from the top-left */
#define LED0_OUT P3OUT
#define LED1_OUT P5OUT
#define LED2_OUT P5OUT

#define LED0 (1<<3)
#define LED1 (1<<6)
#define LED2 (1<<5)

/* User LEDs (in a row under the LCD) from the left */
#define ULED0_OUT P5OUT
#define ULED1_OUT P4OUT
#define ULED2_OUT P3OUT

#define ULED0 (1<<4)
#define ULED1 (1<<0)
#define ULED2 (1<<2)

#define LEDSP3 (LED0 | ULED2)
#define LEDSP4 (LED_CHRG | ULED1)
#define LEDSP5 (LED_DBG | LED1 | LED2 | ULED0)


#define leds_init() do { P3OUT &= ~LEDSP3; P3DIR |= LEDSP3; \
                         P4OUT &= ~LEDSP4; P4DIR |= LEDSP4; \
                         P5OUT &= ~LEDSP5; P5DIR |= LEDSP5; } while(0)

#define __led_set(led_out, led, x) do { if (x) led_out |= led; \
                                        else   led_out &= ~led; } while(0)

#define __led_toggle(led_out, led) do { led_out ^= led; } while(0)

#define led_set(led, x)  __led_set(LED##led##_OUT, LED##led, x)
#define uled_set(led, x) __led_set(ULED##led##_OUT, ULED##led, x)
#define dbg_set(x)       __led_set(LED_DBG_OUT, LED_DBG, x)
#define chrg_set(x)      __led_set(LED_CHRG_OUT, LED_CHRG, x)

#define led_toggle(led)  __led_toggle(LED##led##_OUT, LED##led)
#define uled_toggle(led) __led_toggle(ULED##led##_OUT, ULED##led)
#define dbg_toggle()     __led_toggle(LED_DBG_OUT, LED_DBG)
#define chrg_toggle()    __led_toggle(LED_CHRG_OUT, LED_CHRG)

#endif /* __LEDS_H */
