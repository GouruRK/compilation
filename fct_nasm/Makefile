CC = gcc

all: start_assembleur test

test: %: %.o putchar.o getchar.o putint.o getint.o
	$(CC) -o $@ $^ -Wall

start_assembleur: %: %.o putchar.o getchar.o putint.o getint.o
	$(CC) -o $@ $^ -nostartfiles -no-pie

utils.o:
	nasm -f elf64 -o utils.o utils.asm

%.o: %.asm
	nasm -f elf64 -o $@ $<



clean:
	rm -rf *.o
	rm -f test
	rm -f start_assembleur

	
