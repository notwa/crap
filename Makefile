# NOTE: C implies LADSPA, C++ implies VST
#       it's hackish but it'll do for now

DISTNAME = crap
VERSION = git
FULLNAME = ${DISTNAME}-${VERSION}

BOTH = crap_eq_const
LADSPA_ONLY = crap_eq crap_noise
VST_ONLY =
LADSPA = ${BOTH:=-ladspa} ${LADSPA_ONLY:=-ladspa}
VST = ${BOTH:=-vst} ${VST_ONLY:=-vst}
PLUGINS = ${LADSPA} ${VST}

EXE = design
HEADERS = crap_util.h crap_util_def.h
SHOBJ = ${PLUGINS:=.so}
OBJ = ${PLUGINS:=.o} vstsdk.o

# only for dist target right now
SRC = ${BOTH:=.h} ${LADSPA_ONLY:=-ladspa.c} ${VST_ONLY:=-vst.cpp} ${EXE:=.c}

BENCH = bench.o
AGAINST = ./crap_eq_const-ladspa.so

VST_SDK_DIR ?= .
VST_CPP = audioeffect.cpp audioeffectx.cpp vstplugmain.cpp
VST_SRC = ${VST_CPP:%=${VST_SDK_DIR}/public.sdk/source/vst2.x/%}
# temp objects before combining to vstsdk.o
VST_OBJ = ${VST_CPP:.cpp=.o}
VST_DEF = ${VST_SDK_DIR}/public.sdk/samples/vst2.x/win/vstplug.def

ALL_CFLAGS = -Wall -Wno-unused-function ${CFLAGS} -std=gnu99
ALL_CXXFLAGS = -Wno-write-strings ${CXXFLAGS} -I ${VST_SDK_DIR} -DBUILDING_DLL=1
ALL_LDFLAGS = -lm ${LDFLAGS}

PREFIX ?= /usr/local
EXEC_PREFIX ?= ${PREFIX}
LIBDIR ?= ${EXEC_PREFIX}/lib
LADSPADIR ?= ${LIBDIR}/ladspa

LADSPADEST = ${DESTDIR}${LADSPADIR}

ALL = ${SHOBJ} ${OBJ} ${EXE}
MISC_CLEAN = bench ${BENCH}
MISC_DIST = LICENSE README.md Makefile
MISC_DIST += benchtime ${BENCH:.o=.c}
MISC_DIST += generate-ladspa generate-vst common.sh
MISC_DIST += template-vst.cpp template-ladspa.c ladspa.h

all: options ladspa vst ${EXE}

.PHONY: options
options:
	@echo "CPPFLAGS       = ${CPPFLAGS}"
	@echo "ALL_CFLAGS     = ${ALL_CFLAGS}"
	@echo "ALL_CXXFLAGS   = ${ALL_CXXFLAGS}"
	@echo "ALL_LDFLAGS    = ${ALL_LDFLAGS}"
	@echo "CC             = ${CC}"
	@echo "CXX            = ${CXX}"
	@echo "LD             = ${LD}"
	@echo

ladspa: ${LADSPA:=.so}

vst: ${VST:=.so}

bench: ${AGAINST} ${BENCH}
	@echo CC ${BENCH} -o $@
	@${CC} ${ALL_CFLAGS} ${BENCH} -o $@ ${ALL_LDFLAGS} -rdynamic -ldl

.PHONY: benchmark
benchmark: bench
	./benchtime ./bench ${AGAINST}

vstsdk.o: ${VST_SRC}
	@echo CXX -c $^
	@${CXX} -c ${ALL_CXXFLAGS} ${CPPFLAGS} $^
	@echo LD -r ${VST_OBJ} -o $@
	@${LD} -r ${VST_OBJ} -o $@
	rm ${VST_OBJ}

%-ladspa.so: %-ladspa.o
	@echo CC $^ -o $@
	@${CC} ${ALL_CFLAGS} -shared $^ -o $@ ${ALL_LDFLAGS}

%-vst.so: %-vst.o vstsdk.o
	@echo CXX $^ -o $@
	@${CXX} ${ALL_CXXFLAGS} -shared $^ -o $@ ${ALL_LDFLAGS}

%-ladspa.o: %-ladspa.c ${HEADERS} ladspa.h
	@echo CC -c $< -o $@
	@${CC} -c ${ALL_CFLAGS} ${CPPFLAGS} $< -o $@

%-vst.o: %-vst.cpp ${HEADERS}
	@echo CXX -c $< -o $@
	@${CXX} -c ${ALL_CXXFLAGS} ${CPPFLAGS} $< -o $@

%-ladspa.c: %.h template-ladspa.c generate-ladspa common.sh
	./generate-ladspa $< $@

%-vst.cpp: %.h template-vst.cpp generate-vst common.sh
	./generate-vst $< $@

.SUFFIXES:

%: %.o
	@echo CC $< -o $@
	@${CC} ${ALL_CFLAGS} $< -o $@ ${ALL_LDFLAGS}

%.o: %.c
	@echo CC -c $< -o $@
	@${CC} -c ${ALL_CFLAGS} ${CPPFLAGS} $< -o $@

install: all
	mkdir -p ${LADSPADEST}
	install -d ${LADSPADEST}
	install -m 644 ${LADSPA:=.so} ${LADSPADEST}

.PHONY: clean
clean:
	-rm -f ${ALL} ${MISC_CLEAN}

.PHONY: dist
dist:
	-rm -f ${FULLNAME}.tar.gz
	mkdir ${FULLNAME}
	cp ${MISC_DIST} ${HEADERS} ${SRC} ${FULLNAME}
	tar -cf ${FULLNAME}.tar ${FULLNAME}
	gzip ${FULLNAME}.tar
	rm -r ${FULLNAME}

