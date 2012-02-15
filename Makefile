ARCH = msp430x167
CC := msp430-gcc
NM := msp430-nm
SIZE := msp430-size
UIF_TTY = /dev/ttyUSB0

FW_VER = 3

CFLAGS := -g -mmcu=${ARCH} -Wall -Werror -O3 -std=gnu99 -fshort-enums
CFLAGS += -include `pwd`/config.h
LDFLAGS := -Wl,-Map=power.map

O_FILES = main.o power.o piezo.o monitor.o input.o post.o cmds.o note.o
SUBDIRS = drivers libsric flash430

LDFLAGS += -Ldrivers -ldrivers
LDFLAGS += -Llibsric -lsric
LDFLAGS += -Lflash430 -lflash430

all: power-bottom power-top ram-usage.txt size

include depend

power-bottom: ${O_FILES} ${SUBDIRS}
	${CC} -o $@ ${O_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,flash430/lkr/${ARCH}-bottom.x

power-top: ${O_FILES} ${SUBDIRS}
	${CC} -o $@ ${O_FILES} ${CFLAGS} ${LDFLAGS} -Wl,-T,flash430/lkr/${ARCH}-top.x

drivers flash430:
	$(MAKE) -C $@ CC=${CC} ARCH=${ARCH} CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}" FW_VER="${FW_VER}"

libsric:
	$(MAKE) -C $@ CC=${CC} ARCH=${ARCH} CFLAGS="${CFLAGS} -I`pwd`" LDFLAGS="${LDFLAGS}"

depend: *.c
	rm -f depend
	for file in $^; do \
		${CC} ${CFLAGS} -MM $$file -o - >> $@ ; \
	done ;

ram-usage.txt: power-bottom
	${NM} -nS $^ | grep " [dDbB] " > $@

.PHONY: clean ${SUBDIRS} flash size

size: power-bottom
	${SIZE} $^

flash: power-bottom
	mspdebug uif -j -d ${UIF_TTY} -n "prog $<"

clean:
	-rm -f power-{bottom,top} power.map depend *.o
	for d in ${SUBDIRS} ; do\
		${MAKE} -C $$d clean ; \
	done ;

