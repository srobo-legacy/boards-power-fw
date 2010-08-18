ARCH = msp430x169
CC := msp430-gcc

CFLAGS := -g -mmcu=${ARCH} -Wall -O3
LDFLAGS :=

H_FILES = leds.h power.h piezo.h monitor.h pinint.h input.h
C_FILES = main.c power.c piezo.c monitor.c pinint.c input.c

power: ${H_FILES} ${C_FILES}
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS}

.PHONY: clean

clean:
	-rm -f power

