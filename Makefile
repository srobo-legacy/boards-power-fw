ARCH = msp430x247
CC := msp430-gcc

CFLAGS := -g -mmcu=${ARCH} -Wall -O3
LDFLAGS :=

H_FILES =
C_FILES = main.c

power: ${H_FILES} ${C_FILES}
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS}

.PHONY: clean

clean:
	-rm -f power

