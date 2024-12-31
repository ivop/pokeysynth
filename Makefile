CFLAGS=-O3 -Wall -Wextra -Wno-unused-parameter
#CFLAGS=-O0 -g3 -Wall -Wextra -Wno-unused-parameter
LIBS=-lm
SRC_FILES=pokeysynth.c mzpokey.c remez.c
LV2DIR=pokeysynth.lv2
POKEYSYNTHSO=$(LV2DIR)/pokeysynth.so

all: $(POKEYSYNTHSO)

$(POKEYSYNTHSO): pokeysynth.c mzpokey.c remez.c
	$(CC) -shared -fPIC -s -o $@ $^

test: $(POKEYSYNTHSO)
	cp -a $(LV2DIR) ~/.lv2

clean:
	rm -f *.o *.a *~ */*~ $(POKEYSYNTHSO)

depend:
	rm -f .depend
	+make .depend

.depend:
	$(CC) -MM $(SRC_FILES) > .depend

include .depend
