CC = gcc

all: test

test: %: %.o putchar.o getchar.o putint.o getint.o
	$(CC) -o $@ $^ -Wall

fct_nasm: %: %.o putchar.o getchar.o putint.o getint.o
	$(CC) -o $@ $^ -nostartfiles -no-pie

%.o: %.asm
	nasm -f elf64 -o $@ $< 

clean:
	rm -rf *.o
	rm -f test
