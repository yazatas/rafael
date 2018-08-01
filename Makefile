CC = gcc
CFLAGS = -Wall -Wextra -O0 -g -Wno-unused-parameter -Wno-unused-variable
CFLAGS += -Wno-unused-function -Wno-return-type -Wno-format -std=c99
SOURCES=$(wildcard src/*.c)
OBJECTS=$(addprefix obj/,$(notdir $(SOURCES:.c=.o)))

all: main

main: $(OBJECTS)
	$(CC) -o $@ $+

obj/%.o: src/%.c | obj disk
	$(CC) $(CFLAGS) -c -o $@ $<

obj:
	@mkdir -p $@

disk:
	dd if=/dev/zero of=disk bs=65536000 count=1 &> /dev/null

clean:
	rm -rf obj main disk
