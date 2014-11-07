.PHONY: all clean

OBJS=trackerapp.o patternedit.o arrangementedit.o

CFLAGS=-std=c++11 -g -O2 -march=core2 -ffast-math $(shell sdl-config --cflags) $(shell pkg-config sigc++-2.0 --cflags) $(shell pkg-config glew --cflags) -I. -I.. -I../../rtmidi-2.0.0

all: tracker.o

clean:
	rm -f tracker.o $(OBJS)

tracker.o: $(OBJS)
	ld -Ur -o tracker.o $(OBJS)

$(OBJS): %.o: %.cc
	g++ $(CFLAGS) -c -o $@ $<
