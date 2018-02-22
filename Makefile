CC = gcc
CFLAGS = -Wall -Wextra -O0 -g -Wno-unused-parameter -Wno-unused-variable
CFLAGS += -Wno-unused-function -Wno-return-type -std=c99
SOURCES=$(wildcard src/*.c)
OBJECTS=$(addprefix obj/,$(notdir $(SOURCES:.c=.o)))

all: main

main: $(OBJECTS)
	dd if=/dev/zero of=hdd_file bs=65536000 count=1 &> /dev/null
	$(CC) -o $@ $+

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c -o $@ $<

obj:
	@mkdir -p $@

clean:
	rm -rf obj main hdd_file
