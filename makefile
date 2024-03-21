CC=gcc
CFLAGS=-Wall -g -Iobj -Isrc
PARSER=parser
LEXER=lexer
EXEC=tpcc

bin/tpcas: obj/$(LEXER).o obj/$(PARSER).o obj/tree.o obj/table.o obj/args.o obj/main.o
	@mkdir bin --parent
	$(CC) -o $@ $^

obj/tree.o: src/tree.c src/tree.h
obj/args.o: src/args.c src/args.h src/tree.h

obj/$(PARSER).o: obj/$(PARSER).c src/tree.h src/args.h
obj/$(LEXER).o: obj/$(LEXER).c obj/$(PARSER).h

obj/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

obj/$(LEXER).c: src/$(LEXER).lex obj/$(PARSER).h
	flex -o $@ $<

obj/$(PARSER).c obj/$(PARSER).h: src/$(PARSER).y
	@mkdir obj --parent
	bison -d -o obj/$(PARSER).c $<

clean:
	rm -f obj/*

mrproper: clean
	rm -f bin/*