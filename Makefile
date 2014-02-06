# NOTE: C implies LADSPA, C++ implies VST
#       it's hackish but it'll do for now

DISTNAME = crap
VERSION = git
FULLNAME = ${DISTNAME}-${VERSION}

BIN = ./bin
CRAP = ./crap
INCLUDE = ./include
TEMPLATE = ./template
UTIL = ./util

BOTH = eq eq_const noise tube
LADSPA_ONLY = 
VST_ONLY = delay_test
LADSPA = ${BOTH:=-ladspa} ${LADSPA_ONLY:=-ladspa}
VST = ${BOTH:=-vst} ${VST_ONLY:=-vst}
PLUGINS = ${LADSPA} ${VST}

EXE = design
_ = util util_def param
HEADERS = ${_:%=$(INCLUDE)/%.h}
SHOBJ = ${PLUGINS:%=$(BIN)/$(DISTNAME)_%.so}
OBJ = ${PLUGINS:%=$(BIN)/$(DISTNAME)_%.o} $(BIN)/vstsdk.o

# only for dist target right now
SRC = ${BOTH:%=$(CRAP)/%.h}
SRC += ${LADSPA_ONLY:%=$(CRAP)/%.h} ${VST_ONLY:%=$(CRAP)/%.h}
SRC += ${EXE:%=$(UTIL)/%.c}

BENCH = $(BIN)/bench.o
AGAINST = $(BIN)/crap_eq_const-ladspa.so

VST_SDK_DIR ?= .
VST_CPP = audioeffect.cpp audioeffectx.cpp vstplugmain.cpp
VST_SUB_DIR = public.sdk/source/vst2.x
VST_SRC = ${VST_CPP:%=${VST_SDK_DIR}/${VST_SUB_DIR}/%}
# temp objects before combining to vstsdk.o
VST_OBJ = ${VST_CPP:%.cpp=$(BIN)/%.o}
VST_DEF = ${VST_SDK_DIR}/public.sdk/samples/vst2.x/win/vstplug.def

ALL_CFLAGS = -Wall -Wno-unused-function ${CFLAGS} -std=gnu99 -I $(INCLUDE)
ALL_CXXFLAGS = -Wno-write-strings ${CXXFLAGS} -I ${VST_SDK_DIR}
ALL_CXXFLAGS += -I $(INCLUDE) -DBUILDING_DLL=1
ALL_LDFLAGS = -lm ${LDFLAGS}

PREFIX ?= /usr/local
EXEC_PREFIX ?= ${PREFIX}
LIBDIR ?= ${EXEC_PREFIX}/lib
LADSPADIR ?= ${LIBDIR}/ladspa

LADSPADEST = ${DESTDIR}${LADSPADIR}

ALL = ${SHOBJ} ${OBJ} ${EXE:%=$(BIN)/%}
MISC_CLEAN = bench ${BENCH} ${VST_OBJ}
MISC_DIST = LICENSE README.md Makefile
MISC_DIST += $(UTIL)/benchtime $(UTIL)/${BENCH:.o=.c}
MISC_DIST += $(UTIL)/generate
MISC_DIST += $(TEMPLATE)/vst.cpp $(TEMPLATE)/ladspa.c $(INCLUDE)/ladspa.h

all: ladspa vst ${EXE:%=$(BIN)/%}

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

ladspa: ${LADSPA:%=$(BIN)/$(DISTNAME)_%.so}

vst: ${VST:%=$(BIN)/$(DISTNAME)_%.so}

$(BIN)/bench: ${BENCH}
	@echo '    CC  '$@
	@${CC} ${ALL_CFLAGS} ${BENCH} -o $@ ${ALL_LDFLAGS} -rdynamic -ldl

.PHONY: benchmark
benchmark: $(BIN)/bench ${AGAINST}
	$(UTIL)/benchtime $(BIN)/bench ${AGAINST}

$(VST_OBJ): ${VST_SRC}
	@echo '    CXX '$@
	@${CXX} -c ${ALL_CXXFLAGS} ${CPPFLAGS} ${VST_SDK_DIR}/${VST_SUB_DIR}/$(notdir ${@:.o=.cpp}) -o $@

$(BIN)/vstsdk.o: ${VST_OBJ}
	@echo '    LD  '$@
	@${LD} -r ${VST_OBJ} -o $@

$(BIN)/%-ladspa.so: $(BIN)/%-ladspa.o
	@echo '    LD  '$@
	@${CC} ${ALL_CFLAGS} -shared $^ -o $@ ${ALL_LDFLAGS}

$(BIN)/%-vst.so: $(BIN)/%-vst.o $(BIN)/vstsdk.o
	@echo '    LD  '$@
	@${CXX} ${ALL_CXXFLAGS} -shared $^ -o $@ ${ALL_LDFLAGS}

$(BIN)/$(DISTNAME)_%-ladspa.o: $(CRAP)/%-ladspa.c ${HEADERS} $(INCLUDE)/ladspa.h
	@echo '    CC  '$@
	@${CC} -c ${ALL_CFLAGS} ${CPPFLAGS} $< -o $@

$(BIN)/$(DISTNAME)_%-vst.o: $(CRAP)/%-vst.cpp ${HEADERS}
	@echo '    CXX '$@
	@${CXX} -c ${ALL_CXXFLAGS} ${CPPFLAGS} $< -o $@

$(CRAP)/%-ladspa.c: $(CRAP)/%.h $(TEMPLATE)/ladspa.c $(UTIL)/generate
	@echo '    gen '$@
	@$(UTIL)/generate $(notdir $<) $@ $(TEMPLATE)/ladspa.c

$(CRAP)/%-vst.cpp: $(CRAP)/%.h $(TEMPLATE)/vst.cpp $(UTIL)/generate
	@$(UTIL)/generate $(notdir $<) $@ $(TEMPLATE)/vst.cpp

.SUFFIXES:

$(BIN)/%: $(BIN)/%.o
	@echo '    CC  '$@
	@${CC} ${ALL_CFLAGS} $< -o $@ ${ALL_LDFLAGS}

$(BIN)/%.o: $(UTIL)/%.c
	@echo '    CC  '$@
	@${CC} -c ${ALL_CFLAGS} ${CPPFLAGS} $< -o $@

install: all
	mkdir -p ${LADSPADEST}
	install -d ${LADSPADEST}
	install -m 644 $(BIN)/${LADSPA:=.so} ${LADSPADEST}

.PHONY: clean
clean:
	-rm -f ${ALL} ${MISC_CLEAN}

.PHONY: dist
dist:
	@echo "    dist target is broken for now, sorry"
	@false
	-rm -f ${FULLNAME}.tar.gz
	mkdir ${FULLNAME}
	cp ${MISC_DIST} ${FULLNAME}
	cp ${HEADERS} ${FULLNAME}
	cp ${SRC} ${FULLNAME}
	tar -cf ${FULLNAME}.tar ${FULLNAME}
	gzip ${FULLNAME}.tar
	rm -r ${FULLNAME}
