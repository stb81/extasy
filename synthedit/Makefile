.PHONY: all clean

TARGET=libsynthedit.o
FORMS=$(wildcard *.form)
OBJS=synthedit.o formantedit.o $(addsuffix .o,$(FORMS))

CFLAGS=-std=c++11 -O2 -march=core2 -ffast-math -I.. $(shell sdl-config --cflags) $(shell pkg-config sigc++-2.0 --cflags) $(shell pkg-config glew --cflags)

FORMC=../formc/formc

all: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS) $(addsuffix .cc,$(FORMS)) $(addsuffix .h,$(FORMS))

$(TARGET): $(OBJS)
	ld -Ur -o $(TARGET) $(OBJS)

$(OBJS): %.o: %.cc $(addsuffix .h,$(FORMS))
	g++ $(CFLAGS) -c -o $@ $<

%.form.cc: %.form.h ;

%.form.h: %.form
	$(FORMC) $<

%.form:
