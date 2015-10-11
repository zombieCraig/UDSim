CC=g++
CFLAGS=-ggdb -std=gnu++11 -MMD -MP -I/usr/include/SDL2 -Iinclude 
LDFLAGS=-lSDL2 -lSDL2_image -lSDL2_ttf
OBJS=main.o gui.o logparser.o module.o gamedata.o can.o canframe.o
DEP         := $(OBJS:.o=.d)

all: udsim
-include $(DEP)

main.o:
	$(CC) $(CFLAGS) -c main.cc

gui.o:
	$(CC) $(CFLAGS) -c gui.cc

logparser.o:
	$(CC) $(CFLAGS) -c logparser.cc

module.o:
	$(CC) $(CFLAGS) -c module.cc

can.o:
	$(CC) $(CFLAGS) -c can.cc

canframe.o:
	$(CC) $(CFLAGS) -c canframe.cc

gamedata.o:
	$(CC) $(CFLAGS) -c gamedata.cc

udsim: $(OBJS)
	$(CC) -o udsim $(OBJS) $(LDFLAGS)

clean:
	rm -f udsim *.o
