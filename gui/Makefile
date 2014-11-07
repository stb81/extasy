.PHONY: all clean

OBJS=text.o widget.o mainwindow.o window.o button.o listbox.o lineedit.o textedit.o spinbox.o hpaned.o knob.o curveview.o colorlabelchooser.o filechooser.o filechooser.form.o

CFLAGS=-std=c++11 -g -O2 -march=core2 -ffast-math $(shell sdl-config --cflags) $(shell pkg-config sigc++-2.0 --cflags) $(shell pkg-config glew --cflags)

all: libgui.a

clean:
	rm -rf libgui.a $(OBJS)

libgui.a: $(OBJS)
	ar crs libgui.a $(OBJS)

$(OBJS): %.o: %.cc %.h
	g++ $(CFLAGS) -c -o $@ $<

%.form.cc %.form.h: %.form ../formc/formc
	../formc/formc $<

# cancel conflicting implicit rules
%: %.o
%: %.cc
