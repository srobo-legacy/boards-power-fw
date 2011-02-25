ARCH = msp430x167
CC := msp430-gcc
UIF_TTY = /dev/ttyUSB0

CFLAGS := -g -mmcu=${ARCH} -Wall -O3 -std=gnu99
CFLAGS += -include `pwd`/config.h
LDFLAGS := -Wl,-Map=power.map

O_FILES = main.o power.o piezo.o monitor.o input.o post.o
SUBDIRS = drivers libsric flash430

LDFLAGS += -Ldrivers -ldrivers
LDFLAGS += -Llibsric -lsric
LDFLAGS += -Lflash430 -lflash430

all: power-bottom power-top

include depend

power-bottom: ${O_FILES} ${SUBDIRS}
	${CC} -o $@ ${O_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,flash430/lkr/${ARCH}-bottom.x

power-top: ${O_FILES} ${SUBDIRS}
	${CC} -o $@ ${O_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,flash430/lkr/${ARCH}-top.x

drivers flash430:
	$(MAKE) -C $@ CC=${CC} ARCH=${ARCH} CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}"

libsric:
	$(MAKE) -C $@ CC=${CC} ARCH=${ARCH} CFLAGS="${CFLAGS} -I`pwd`" LDFLAGS="${LDFLAGS}"

depend: *.c
	rm -f depend
	for file in $^; do \
		${CC} ${CFLAGS} -MM $$file -o - >> $@ ; \
	done ;

.PHONY: clean ${SUBDIRS} flash

flash: power-bottom
	mspdebug uif -j -d ${UIF_TTY} -n "prog $<"

clean:
	-rm -f power-{bottom-top} power.map depend *.o
	for d in ${SUBDIRS} ; do\
		${MAKE} -C $$d clean ; \
	done ;

