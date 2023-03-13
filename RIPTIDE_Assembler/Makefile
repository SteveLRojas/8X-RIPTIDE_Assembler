CFLAGS=-std=c99 -pedantic -Wall -Wextra -Wconversion -D_XOPEN_SOURCE -march=native -O3
LDFLAGS=-lm
TARGET = ./bin/8xasm

.PHONY: all
all: $(TARGET)


TARGET_OBJS = RIPTIDE_Assembler.o

$(TARGET): $(TARGET_OBJS)
	mkdir -p $(dir $@)
	$(CC) $(TARGET_OBJS) $(CFLAGS) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) -f $(TARGET) *.o
