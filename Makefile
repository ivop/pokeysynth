#CXXFLAGS=-O3 -Wall -Wextra -Wno-unused-parameter
CXXFLAGS=-O0 -g3 -Wall -Wextra -Wno-unused-parameter
SRC=src/PokeySynth.cpp \
	src/PokeyInstrument.cpp \
	src/Tuning.cpp \
	src/mzpokey.cpp \
	src/remez.cpp \
	src/uris.cpp \
	src/LoadSaveInstruments.cpp
SRCUI=src/PokeySynthUi.cpp \
	  src/InstrumentEditor.cpp \
	  src/uris.cpp \
	  src/LoadSaveInstruments.cpp \
	  src/platform.cpp
OBJ=$(SRC:.cpp=.o)
OBJUI=$(SRCUI:.cpp=.o)
LV2DIR=pokeysynth.lv2
POKEYSYNTHSO=$(LV2DIR)/pokeysynth.$(DLLEXT)
POKEYSYNTHUISO=$(LV2DIR)/pokeysynth_ui.$(DLLEXT)
POKEYSYNTHTTL=$(LV2DIR)/pokeysynth.ttl
MANIFESTTTL=$(LV2DIR)/manifest.ttl
LIBS=-lm
LIBSUI=-lm $(shell fltk-config --ldstaticflags)
DLLEXT=so
UITYPE=X11UI
XLINK=

ifeq ($(OS),Windows_NT)
	DLLEXT=dll
	UITYPE=WindowsUI
	XLINK=-static
endif

#include warnings.mk

all: $(POKEYSYNTHSO) $(POKEYSYNTHUISO) $(POKEYSYNTHTTL) $(MANIFESTTTL)

$(POKEYSYNTHSO): $(OBJ)
	$(CXX) -shared $(XLINK) -fPIC -s -o $@ $^ $(LIBS)

$(POKEYSYNTHUISO): $(OBJUI)
	$(CXX) -shared $(XLINK) -fPIC -s -o $@ $^ $(LIBSUI)

%.o: %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) -fPIC $<

$(POKEYSYNTHTTL): src/pokeysynth.ttl.in
	sed s/@DLLEXT@/$(DLLEXT)/ $< > $@
	sed -i s/@UITYPE@/$(UITYPE)/ $@

$(MANIFESTTTL): src/manifest.ttl.in
	sed s/@DLLEXT@/$(DLLEXT)/ $< > $@
	sed -i s/@UITYPE@/$(UITYPE)/ $@

test: $(POKEYSYNTHSO) $(POKEYSYNTHUISO) $(POKEYSYNTHTTL) $(MANIFESTTTL)
	cp -a $(LV2DIR) ~/.lv2

clean:
	rm -f *.o *.a *~ */*~ $(POKEYSYNTHSO) $(POKEYSYNTHUISO) $(OBJ) $(OBJUI) \
		$(POKEYSYNTHTTL) $(MANIFESTTTL)

depend:
	rm -f .depend
	+make .depend

.depend:
	$(CC) -MM $(SRC) $(SRCUI) | sed 's|[a-zA-Z0-9_-]*\.o|src/&|' > .depend

include .depend
