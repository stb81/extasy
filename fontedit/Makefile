.PHONY: clean

OBJS=fontedit.o fontedit.form.o

CFLAGS=-std=c++11 -g -O2 -march=core2 -ffast-math -fopenmp $(shell sdl-config --cflags) $(shell pkg-config sigc++-2.0 --cflags) $(shell pkg-config glew --cflags) -I. -I..
LDFLAGS=-s $(shell sdl-config --libs) $(shell pkg-config sigc++-2.0 --libs) $(shell pkg-config glew --libs) -lGL -L../gui

fontedit: $(OBJS)
	g++ $(LDFLAGS) -o fontedit $(OBJS) -lgui

clean:
	rm -f fontedit $(OBJS) fontedit.form.cc fontedit.form.h
	
$(OBJS): %.o: %.cc fontedit.form.h
	g++ $(CFLAGS) -c -o $@ $<

%.form.cc %.form.h: %.form
	../formc/formc $<

%.form:
