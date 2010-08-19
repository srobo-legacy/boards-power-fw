ARCH = msp430x169
CC := msp430-gcc

CFLAGS := -g -mmcu=${ARCH} -Wall -O3
LDFLAGS :=

O_FILES = main.o power.o piezo.o monitor.o pinint.o input.o
SUBDIRS = drivers

LDFLAGS += -Ldrivers -ldrivers

all: power

include depend

power: ${O_FILES} ${SUBDIRS}
	${CC} -o $@ ${O_FILES} ${CFLAGS} ${LDFLAGS}

${SUBDIRS}:
	$(MAKE) -C $@ CC=${CC} ARCH=${ARCH} CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}"

depend: *.c
	rm -f depend
	for file in $^; do \
		${CC} ${CFLAGS} -MM $$file -o - >> $@ ; \
	done ;

.PHONY: clean ${SUBDIRS}

clean:
	-rm -f power depend *.o
	for d in ${SUBDIRS} ; do\
		${MAKE} -C $$d clean ; \
	done ;

