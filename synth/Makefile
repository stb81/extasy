.PHONY: all clean

OBJS=module.o sequencer.o serialization.o padsynth.o fmsynth.o additivesynth.o pwmsynth.o pitchless.o kickdrum.o hypersaw.o vocalpad.o customsynth.o customscanner.o customparser.o

CFLAGS=-std=c++11 -O2 -march=core2 -ffast-math -fopenmp

all: libsynth.a

clean:
	rm -rf libsynth.a $(OBJS) customscanner.cc customparser.cc customparser.hh

libsynth.a: $(OBJS)
	ar crs libsynth.a $(OBJS)

$(OBJS): %.o: %.cc
	g++ $(CFLAGS) -c -o $@ $<
	
customsynth.cc: customparser.hh

customscanner.cc: customscanner.l
	flex -o customscanner.cc customscanner.l

customparser.cc customparser.hh: customparser.y
	bison -d -o customparser.cc customparser.y
