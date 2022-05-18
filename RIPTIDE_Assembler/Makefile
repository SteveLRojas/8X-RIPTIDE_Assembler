CFLAGS=-std=c99 -pedantic -Wall -Wextra -Wconversion -D_XOPEN_SOURCE -march=native -O3
LDLIBS=-lm

.PHONY: all

all: 8xasm.exe

8xasm.exe: RIPTIDE_Assembler.o

%.exe:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

lib%.a: %.o
	$(AR) -rs $@ $^

.PHONY: clean
clean:
	$(RM) -- *.exe *.a *.o
