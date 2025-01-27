CXXFLAGS=-O3 -Wall -Wextra -Wno-unused-parameter -Wsuggest-override -Wzero-as-null-pointer-constant
#CXXFLAGS=-O0 -g3 -Wall -Wextra -Wno-unused-parameter -Wsuggest-override -Wzero-as-null-pointer-constant
LIBS=-lm
SRC=PokeySynth.cpp PokeyInstrument.cpp Tuning.cpp mzpokey.cpp remez.cpp \
	uris.cpp LoadSaveInstruments.cpp
SRCUI=PokeySynthUi.cpp InstrumentEditor.cpp uris.cpp LoadSaveInstruments.cpp \
	  platform.cpp
OBJ=$(SRC:.cpp=.o)
OBJUI=$(SRCUI:.cpp=.o)
LV2DIR=pokeysynth.lv2
POKEYSYNTHSO=$(LV2DIR)/pokeysynth.so
POKEYSYNTHUISO=$(LV2DIR)/pokeysynth_ui.so

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
	rm -f *.o *.a *~ */*~ $(POKEYSYNTHSO) $(POKEYSYNTHUISO)

depend:
	rm -f .depend
	+make .depend

.depend:
	$(CC) -MM $(SRC) $(SRCUI) > .depend

include .depend
