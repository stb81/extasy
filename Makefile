LIBS=synthedit/libsynthedit.o tracker/tracker.o synth/libsynth.a gui/libgui.a

.PHONY: all clean run $(LIBS) formc/formc

CFLAGS=-std=c++11 -g -O2 -march=core2 -ffast-math -fopenmp $(shell sdl-config --cflags) $(shell pkg-config sigc++-2.0 --cflags) $(shell pkg-config glew --cflags)
LDFLAGS=-fopenmp $(shell sdl-config --libs) $(shell pkg-config sigc++-2.0 --libs) $(shell pkg-config glew --libs) -lfftw3 -lasound -lGL

all: extasy

run: extasy
	./extasy

clean:
	$(MAKE) -C synth clean
	$(MAKE) -C gui clean
	$(MAKE) -C synthedit clean
	$(MAKE) -C formc clean
	$(MAKE) -C tracker clean
	$(MAKE) -C fontedit clean
	rm -f extasy

extasy: $(LIBS)
	g++ $(LDFLAGS) -o extasy $(OBJS) $(LIBS) ../rtmidi-2.0.0/librtmidi.a

tracker/tracker.o: formc/formc synthedit/libsynthedit.o
	$(MAKE) -C tracker tracker.o

synth/libsynth.a:
	$(MAKE) -C synth libsynth.a

gui/libgui.a: formc/formc
	$(MAKE) -C gui libgui.a

synthedit/libsynthedit.o: formc/formc
	$(MAKE) -C synthedit libsynthedit.o

formc/formc:
	$(MAKE) -C formc formc

