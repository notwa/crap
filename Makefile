DISTNAME = crap
VERSION = git

EXE = design
SHOBJ = crap_eq.so crap_eq_const.so
MID = crap_util.o
HEADERS = crap_util.h ladspa.h

OBJ = ${SHOBJ:.so=.o} ${EXE:=.o} ${MID}
SRC = ${OBJ:.o=.c}

PLACEBO_FLAGS = -fomit-frame-pointer -fstrength-reduce -funroll-loops -ffast-math
ALL_CFLAGS = -O3 ${PLACEBO_FLAGS} -std=c99 -fPIC -Wall ${CFLAGS}
ALL_LDFLAGS = -lm ${LDFLAGS}
SHARED_LDFLAGS = -shared

PREFIX ?= /usr/local
EXEC_PREFIX ?= ${PREFIX}
LIBDIR ?= ${EXEC_PREFIX}/lib
LADSPADIR ?= ${LIBDIR}/ladspa

# should CFLAGS really be used in linking too? seems odd

FULLNAME = ${DISTNAME}-${VERSION}
ALL = ${OBJ} ${SHOBJ} ${EXE}
LADSPADEST = ${DESTDIR}${LADSPADIR}

all: options ${ALL}

.PHONY: options
options:
	@echo "ALL_CFLAGS     = ${ALL_CFLAGS}"
	@echo "CPPFLAGS       = ${CPPFLAGS}"
	@echo "ALL_LDFLAGS    = ${ALL_LDFLAGS}"
	@echo "SHARED_LDFLAGS = ${SHARED_LDFLAGS}"
	@echo "CC             = ${CC}"

%.so: %.o ${MID}
	@echo LD $< ${MID} -o $@
	@${CC} ${SHARED_LDFLAGS} $< ${MID} -o $@ ${ALL_LDFLAGS}

%: %.o ${MID}
	@echo LD $< ${MID} -o $@
	@${CC} $< ${MID} -o $@ ${ALL_LDFLAGS}

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

