DISTNAME = crap
VERSION = git

EXE = design
SHOBJ = crap_eq.so crap_eq_const.so
MID =
HEADERS = crap_util.h crap_util_def.h ladspa.h

OBJ = ${SHOBJ:.so=.o} ${EXE:=.o} ${MID}
SRC = ${OBJ:.o=.c}

WARNING_FLAGS = -Wall -Wno-unused-function
ALL_CFLAGS = -std=gnu99 -fpic ${WARNING_FLAGS} ${CFLAGS}
ALL_LDFLAGS = -lm ${LDFLAGS}

PREFIX ?= /usr/local
EXEC_PREFIX ?= ${PREFIX}
LIBDIR ?= ${EXEC_PREFIX}/lib
LADSPADIR ?= ${LIBDIR}/ladspa

FULLNAME = ${DISTNAME}-${VERSION}
ALL = ${OBJ} ${SHOBJ} ${EXE}
LADSPADEST = ${DESTDIR}${LADSPADIR}

all: options ${ALL}

.PHONY: options
options:
	@echo "ALL_CFLAGS     = ${ALL_CFLAGS}"
	@echo "CPPFLAGS       = ${CPPFLAGS}"
	@echo "ALL_LDFLAGS    = ${ALL_LDFLAGS}"
	@echo "CC             = ${CC}"

%.so: %.o ${MID}
	@echo LD $< ${MID} -o $@
	@${CC} ${ALL_CFLAGS} -shared $< ${MID} -o $@ ${ALL_LDFLAGS}

%: %.o ${MID}
	@echo LD $< ${MID} -o $@
	@${CC} ${ALL_CFLAGS} $< ${MID} -o $@ ${ALL_LDFLAGS}

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

