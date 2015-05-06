DISTNAME = crap
VERSION = git
FULLNAME = $(DISTNAME)-$(VERSION)

BIN ?= ./bin
VST_SDK_DIR ?= .

BOTH = eq eq_const eq_const_T420 noise tube
LADSPA = $(BOTH)
VST = $(BOTH) delay_test

UTILS = design bench
INCLUDES = util util_def param os6iir os2piir

BENCH_AGAINST = eq_const

###

PROGRAM = ${UTILS:%=$(BIN)/%}
HEADERS = ${INCLUDES:%=include/%.h}

LADSPA_SHOBJ = ${LADSPA:%=$(BIN)/$(DISTNAME)_%-ladspa.so}
VST_SHOBJ = ${VST:%=$(BIN)/$(DISTNAME)_%-vst.so}
SHOBJ = $(LADSPA_SHOBJ) $(VST_SHOBJ)
OBJ = ${SHOBJ:.so=.o}
AGAINST = $(BIN)/$(DISTNAME)_$(BENCH_AGAINST)-ladspa.so
#EXE = ${PROGRAM:=.exe}
EXE = $(BIN)/design.exe
DLL = ${SHOBJ:.so=.dll}

VST_CPP = audioeffect.cpp audioeffectx.cpp vstplugmain.cpp
VST_CPP_DIR = $(VST_SDK_DIR)/public.sdk/source/vst2.x
VST_SRC = ${VST_CPP:%=$(VST_CPP_DIR)/%}
VST_OBJ = ${VST_CPP:%.cpp=$(BIN)/%.o}
VST_DEF = $(VST_SDK_DIR)/public.sdk/samples/vst2.x/win/vstplug.def

INLINE_FLAGS = -Winline
GENERAL_FLAGS = -Wall -Wno-unused-function -Wno-sign-compare -I include $(INLINE_FLAGS)
ALL_CFLAGS = $(GENERAL_FLAGS) -std=gnu11 $(CFLAGS)
ALL_CXXFLAGS = $(GENERAL_FLAGS) $(CXXFLAGS)
ALL_LDFLAGS = -lm $(LDFLAGS)

LADSPA_FLAGS = 
VST_FLAGS = -Wno-write-strings -Wno-narrowing
VST_FLAGS += -I $(VST_SDK_DIR) -DBUILDING_DLL=1

OPT_FLAGS = -Ofast -march=native -mfpmath=sse

# any possibly produced files besides intermediates
ALL = $(SHOBJ) $(PROGRAM) $(BIN)/vstsdk.o $(EXE) $(DLL)

.SUFFIXES:

# TODO: force options before clean before everything else

.PHONY: all options clean dist pretest ladspa vst $(UTILS)
.PHONY: benchmark windows linux
all: pretest ladspa vst

exe: $(EXE)

dll: $(DLL)

windows: ALL_CFLAGS += $(OPT_FLAGS)
windows: ALL_CXXFLAGS += $(OPT_FLAGS)
windows: dll

linux: ALL_CFLAGS += $(OPT_FLAGS) -fpic
linux: ALL_CXXFLAGS += $(OPT_FLAGS) -fpic
linux: VST_FLAGS += -D__cdecl=
linux: all

linux-arm: ALL_CFLAGS += -Ofast -march=native -DFORCE_SINGLE -fpic
linux-arm: ALL_CXXFLAGS += -Ofast -march=native -DFORCE_SINGLE -fpic
linux-arm: VST_FLAGS += -D__cdecl=
linux-arm: all

options:
	@echo "CPPFLAGS       = $(CPPFLAGS)"
	@echo "ALL_CFLAGS     = $(ALL_CFLAGS)"
	@echo "ALL_CXXFLAGS   = $(ALL_CXXFLAGS)"
	@echo "ALL_LDFLAGS    = $(ALL_LDFLAGS)"
	@echo "CC             = $(CC)"
	@echo "CXX            = $(CXX)"
	@echo "LD             = $(LD)"
	@echo

ladspa: $(LADSPA_SHOBJ)

vst: $(VST_SHOBJ)

pretest: util/denorm_test.h
	@$(CC) -E $(ALL_CFLAGS) $^ -o /dev/null

benchmark: $(BIN)/bench $(AGAINST)
	util/benchtime $(BIN)/bench $(AGAINST)

$(UTILS): %: $(BIN)/%

$(BIN)/%.exe: $(BIN)/%
	@echo '  OBJCOPY '$@
	@$(OBJCOPY) -S $< $@

$(BIN)/%.dll: $(BIN)/%.so
	@echo '  OBJCOPY '$@
	@$(OBJCOPY) -S $< $@

$(BIN)/%-ladspa.so: $(BIN)/%-ladspa.o
	@echo '  CCLD    '$@
	@$(CC) $(ALL_CFLAGS) $(LADSPA_FLAGS) -shared $^ -o $@ $(ALL_LDFLAGS)

$(BIN)/%-vst.so: $(BIN)/%-vst.o $(BIN)/vstsdk.o
	@echo '  CXXLD   '$@
	@$(CXX) $(ALL_CXXFLAGS) $(VST_FLAGS) -shared $^ -o $@ $(ALL_LDFLAGS)

$(BIN)/$(DISTNAME)_%-ladspa.o: crap/%-ladspa.c $(HEADERS) include/ladspa.h
	@echo '  CC      '$@
	@$(CC) -c $(ALL_CFLAGS) $(LADSPA_FLAG) $(CPPFLAGS) $< -o $@

$(BIN)/$(DISTNAME)_%-vst.o: crap/%-vst.cpp $(HEADERS)
	@echo '  CXX     '$@
	@$(CXX) -c $(ALL_CXXFLAGS) $(VST_FLAGS) $(CPPFLAGS) $< -o $@

crap/%-ladspa.c: crap/%.h template/ladspa.c util/generate
	@echo '  GEN     '$@
	@util/generate $(notdir $<) $@ template/ladspa.c

crap/%-vst.cpp: crap/%.h template/vst.cpp util/generate
	@echo '  GEN     '$@
	@util/generate $(notdir $<) $@ template/vst.cpp

$(BIN)/vstsdk.o: $(VST_OBJ)
	@echo '  LD      '$@
	@$(LD) -r $^ -o $@

.INTERMEDIATE: $(VST_OBJ)
$(VST_OBJ): $(BIN)/%.o: $(VST_CPP_DIR)/%.cpp
	@echo '  CXX     '$@
	@$(CXX) -c $(ALL_CXXFLAGS) $(VST_FLAGS) $(CPPFLAGS) $< -o $@

$(BIN)/bench: util/bench.c
	@echo '  CCLD    '$@
	@$(CC) $(ALL_CFLAGS) $(LADSPA_FLAGS) $< -o $@ $(ALL_LDFLAGS) -rdynamic -ldl

$(BIN)/design: util/design.c
	@echo '  CCLD    '$@
	@$(CC) $(ALL_CFLAGS) $< -o $@ $(ALL_LDFLAGS)

clean:
	rm -f $(ALL)

dist:
	@echo "# dist target is unimplemented, trying git instead"
	git archive --prefix=$(FULLNAME)/ HEAD -o $(FULLNAME).tar.gz
