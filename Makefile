CXXFLAGS=-O3 -Wall -Wextra -Wno-unused-parameter
#CXXFLAGS=-O0 -g3 -Wall -Wextra -Wno-unused-parameter
LIBS=-lm
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
POKEYSYNTHSO=$(LV2DIR)/pokeysynth.so
POKEYSYNTHUISO=$(LV2DIR)/pokeysynth_ui.so

#include warnings.mk

all: $(POKEYSYNTHSO) $(POKEYSYNTHUISO)

$(POKEYSYNTHSO): $(OBJ)
	$(CXX) -shared -fPIC -s -o $@ $^

$(POKEYSYNTHUISO): $(OBJUI)
	$(CXX) -shared -fPIC -s -o $@ $^ -lfltk

%.o: %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) -fPIC $<

test: $(POKEYSYNTHSO) $(POKEYSYNTHUISO)
	cp -a $(LV2DIR) ~/.lv2

clean:
	rm -f *.o *.a *~ */*~ $(POKEYSYNTHSO) $(POKEYSYNTHUISO) $(OBJ) $(OBJUI)

depend:
	rm -f .depend
	+make .depend

.depend:
	$(CC) -MM $(SRC) $(SRCUI) > .depend

include .depend
