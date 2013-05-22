DISTNAME = crap
VERSION = git

SHOBJ = crap_eq.so
MID = crap_util.o
OBJ = ${SHOBJ:.so=.o} ${MID}
SRC = ${OBJ:.o=.c}
HEADERS = crap_util.h ladspa.h

PLACEBO_FLAGS = -fomit-frame-pointer -fstrength-reduce -funroll-loops -ffast-math
ALL_CFLAGS = -O3 ${PLACEBO_FLAGS} -std=c99 -fPIC -Wall ${CFLAGS}
ALL_LDFLAGS = -nostartfiles -shared ${LDFLAGS}

PREFIX ?= /usr/local
EXEC_PREFIX ?= ${PREFIX}
LIBDIR ?= ${EXEC_PREFIX}/lib
LADSPADIR ?= ${LIBDIR}/ladspa

# should CFLAGS really be used in linking too? seems odd

FULLNAME = ${DISTNAME}-${VERSION}
ALL = ${OBJ} ${SHOBJ}
LADSPADEST = ${DESTDIR}${LADSPADIR}

all: options ${ALL}

.PHONY: options
options:
	@echo "ALL_CFLAGS  = ${ALL_CFLAGS}"
	@echo "CPPFLAGS    = ${CPPFLAGS}"
	@echo "ALL_LDFLAGS = ${ALL_LDFLAGS}"
	@echo "CC          = ${CC}"

%.so: %.o
	@echo LD $< ${MID} -o $@
	@${CC} ${ALL_LDFLAGS} $< ${MID} -o $@

%.o: %.c ${HEADERS}
	@echo CC $< -o $@
	@${CC} -c ${ALL_CFLAGS} ${CPPFLAGS} $< -o $@

install: all
	mkdir -p ${LADSPADEST}
	install -d ${LADSPADEST}
	install -m 644 ${SHOBJ} ${LADSPADEST}

clean:
	-rm -f ${ALL}

dist:
	-rm -f ${FULLNAME}.tar.gz
	mkdir -p ${FULLNAME}
	cp LICENSE README.md Makefile ${HEADERS} ${SRC} ${FULLNAME}
	tar -cf ${FULLNAME}.tar ${FULLNAME}
	gzip ${FULLNAME}.tar
	rm -rf ${FULLNAME}

