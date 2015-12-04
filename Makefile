DISTNAME = crap
VERSION = git
FULLNAME = $(DISTNAME)-$(VERSION)

BIN ?= ./bin
VST_SDK_DIR ?= .

BOTH = eq eq_const eq_const_T420 eq_const_T420_svf mugi4 noise tube level
LADSPA = $(BOTH)
VST = $(BOTH) delay_test

UTILS = design bench
INCLUDES = util Crap biquad svf Param os6iir os2piir

BENCH_AGAINST = eq_const

###

PROGRAM = ${UTILS:%=$(BIN)/%}
HEADERS = ${INCLUDES:%=include/%.hpp}

LADSPA_SHOBJ = ${LADSPA:%=$(BIN)/ladspa/$(DISTNAME)_%.so}
VST_SHOBJ = ${VST:%=$(BIN)/vst/$(DISTNAME)_%.so}
SHOBJ = $(LADSPA_SHOBJ) $(VST_SHOBJ)
OBJ = ${SHOBJ:.so=.o}
AGAINST = $(BIN)/ladspa/$(DISTNAME)_$(BENCH_AGAINST).so
#EXE = ${PROGRAM:=.exe}
EXE = $(BIN)/design.exe
DLL = ${SHOBJ:.so=.dll}

VST_CPP = audioeffect.cpp audioeffectx.cpp vstplugmain.cpp
VST_CPP_DIR = $(VST_SDK_DIR)/public.sdk/source/vst2.x
VST_SRC = ${VST_CPP:%=$(VST_CPP_DIR)/%}
VST_OBJ = ${VST_CPP:%.cpp=$(BIN)/%.o}
VST_DEF = $(VST_SDK_DIR)/public.sdk/samples/vst2.x/win/vstplug.def

GENERAL_FLAGS = -Wall -Winline -Wno-unused-function -Wno-sign-compare -I . -I include
ALL_CXXFLAGS = $(GENERAL_FLAGS) -std=gnu++11 $(CXXFLAGS)
ALL_LDFLAGS = -lm $(LDFLAGS)

LADSPA_FLAGS = 
VST_FLAGS = -Wno-write-strings -Wno-narrowing
VST_FLAGS += -I $(VST_SDK_DIR) -DBUILDING_DLL=1

OPT_FLAGS = -Ofast -march=core2 -mfpmath=sse

OBJCOPY ?= objcopy

# any possibly produced files besides intermediates
ALL = $(SHOBJ) $(PROGRAM) $(BIN)/vstsdk.o $(EXE) $(DLL)

.SUFFIXES:

# TODO: force options before clean before everything else

.PHONY: all options clean dist pretest ladspa vst $(UTILS)
.PHONY: benchmark windows linux
all: pretest ladspa vst

exe: $(EXE)

dll: $(DLL)

windows: ALL_CXXFLAGS += $(OPT_FLAGS)
windows: dll

linux: ALL_CXXFLAGS += $(OPT_FLAGS) -fpic
linux: VST_FLAGS += -D__cdecl=
linux: all

linux-arm: ALL_CXXFLAGS += -Ofast -march=native -DFORCE_SINGLE -fpic
linux-arm: VST_FLAGS += -D__cdecl=
linux-arm: all

options:
	@echo "CPPFLAGS       = $(CPPFLAGS)"
	@echo "ALL_CXXFLAGS   = $(ALL_CXXFLAGS)"
	@echo "ALL_LDFLAGS    = $(ALL_LDFLAGS)"
	@echo "CXX            = $(CXX)"
	@echo "LD             = $(LD)"
	@echo

ladspa: $(LADSPA_SHOBJ)

vst: $(VST_SHOBJ)

pretest: util/denorm_test.hpp
	@$(CXX) -E $(ALL_CXXFLAGS) $^ -o /dev/null

benchmark: $(BIN)/bench $(AGAINST)
	util/benchtime $(BIN)/bench $(AGAINST)

$(UTILS): %: $(BIN)/%

$(BIN)/%.exe: $(BIN)/%
	@echo '  OBJCOPY '$@
	@$(OBJCOPY) -S $< $@

$(BIN)/%.dll: $(BIN)/%.so
	@echo '  OBJCOPY '$@
	@$(OBJCOPY) -S $< $@

$(BIN)/ladspa/%.so: $(BIN)/ladspa/%.o
	@echo '  CXXLD   '$@
	@$(CXX) $(ALL_CXXFLAGS) $(LADSPA_FLAGS) -shared $^ -o $@ $(ALL_LDFLAGS)

$(BIN)/vst/%.so: $(BIN)/vst/%.o $(BIN)/vstsdk.o
	@echo '  CXXLD   '$@
	@$(CXX) $(ALL_CXXFLAGS) $(VST_FLAGS) -shared $^ -o $@ $(ALL_LDFLAGS)

$(BIN)/ladspa/$(DISTNAME)_%.o: crap/ladspa/%.cpp $(HEADERS) include/ladspa.hpp
	@echo '  CXX     '$@
	@$(CXX) -c $(ALL_CXXFLAGS) $(LADSPA_FLAGS) $(CPPFLAGS) $< -o $@

$(BIN)/vst/$(DISTNAME)_%.o: crap/vst/%.cpp $(HEADERS)
	@echo '  CXX     '$@
	@$(CXX) -c $(ALL_CXXFLAGS) $(VST_FLAGS) $(CPPFLAGS) $< -o $@

crap/ladspa/%.cpp: crap/%.hpp template/ladspa.cpp util/generate
	@echo '  GEN     '$@
	@util/generate crap/$(notdir $<) $@ template/ladspa.cpp

crap/vst/%.cpp: crap/%.hpp template/vst.cpp util/generate
	@echo '  GEN     '$@
	@util/generate crap/$(notdir $<) $@ template/vst.cpp

$(BIN)/vstsdk.o: $(VST_OBJ)
	@echo '  LD      '$@
	@$(LD) -r $^ -o $@

.INTERMEDIATE: $(VST_OBJ)
$(VST_OBJ): $(BIN)/%.o: $(VST_CPP_DIR)/%.cpp
	@echo '  CXX     '$@
	@$(CXX) -c $(ALL_CXXFLAGS) $(VST_FLAGS) $(CPPFLAGS) $< -o $@

$(BIN)/bench: util/bench.cpp
	@echo '  CXXLD   '$@
	@$(CXX) $(ALL_CXXFLAGS) $(LADSPA_FLAGS) $< -o $@ $(ALL_LDFLAGS) -rdynamic -ldl

$(BIN)/design: util/design.cpp
	@echo '  CXXLD   '$@
	@$(CXX) $(ALL_CXXFLAGS) $< -o $@ $(ALL_LDFLAGS)

clean:
	rm -f $(ALL)

dist:
	@echo "# dist target is unimplemented, trying git instead"
	git archive --prefix=$(FULLNAME)/ HEAD -o $(FULLNAME).tar.gz
