PROJECT  =sockgames
VERSION  =1.0.0
PREFIX   =/usr/local
PROGS    =snake$(EXE)
CC       = cc -Wall
SOURCES  = main.c
SOURCES += sg-snake.c
SOURCES += sg-game.c sg-table.c sg-move.c sg-clock.c sg-points.c sg-symbols.c


all: $(PROGS)
clean:
	rm -f snake$(EXE)
install:
	mkdir -p $(PREFIX)/games
	cp $(PROGS) $(PREFIX)/games

snake$(EXE): $(SOURCES)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(SOURCES)

## -- BLOCK:c --
clean: clean-c
clean-c:
	rm -f *.o
## -- BLOCK:c --
