CXXFLAGS=-O3 -Wall -Wextra -Wno-unused-parameter
#CXXFLAGS=-O0 -g3 -Wall -Wextra -Wno-unused-parameter
LIBS=-lm
SRC=PokeySynth.cpp PokeyInstrument.cpp Tuning.cpp mzpokey.cpp remez.cpp
OBJ=$(SRC:.cpp=.o)
LV2DIR=pokeysynth.lv2
POKEYSYNTHSO=$(LV2DIR)/pokeysynth.so

all: $(POKEYSYNTHSO)

$(POKEYSYNTHSO): $(OBJ)
	$(CXX) -shared -fPIC -s -o $@ $^

%.o: %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) -fPIC $<

test: $(POKEYSYNTHSO)
	cp -a $(LV2DIR) ~/.lv2

clean:
	rm -f *.o *.a *~ */*~ $(POKEYSYNTHSO)

depend:
	rm -f .depend
	+make .depend

.depend:
	$(CC) -MM $(SRC) > .depend

include .depend
