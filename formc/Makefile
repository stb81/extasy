.PHONY: clean

OBJS=formc.o scanner.o parser.o

formc: $(OBJS)
	g++ -o formc $(OBJS) -lfl
	
clean:
	rm -f formc $(OBJS) parser.cc parser.hh scanner.cc
	
$(OBJS): %.o: %.cc parser.hh
	g++ -O2 -c -o $@ $<
	
scanner.cc: scanner.l
	flex -o scanner.cc scanner.l
	
parser.cc parser.hh: parser.y
	bison -d -o parser.cc parser.y

