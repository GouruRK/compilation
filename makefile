CC=gcc
CFLAGS=-Wall -g -Iinclude -Iobj -Isrc
PARSER=parser
LEXER=lexer
EXEC=tpcc

INCLUDE_DIR=include
SRC_DIR=src
BUILD_DIR=obj
BIN_DIR=bin

SOURCES=$(wildcard $(SRC_DIR)/*.c)
SRC_OBJS=$(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

$(BIN_DIR)/$(EXEC): obj/$(LEXER).o obj/$(PARSER).o $(SRC_OBJS)
	@mkdir bin --parent
	$(CC) -o $@ $^

$(BUILD_DIR)/$(PARSER).o: obj/$(PARSER).c $(INCLUDE_DIR)/tree.h $(INCLUDE_DIR)/args.h
$(BUILD_DIR)/$(LEXER).o: obj/$(LEXER).c obj/$(PARSER).h

$(BUILD_DIR)/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(BUILD_DIR)/$(LEXER).c: src/$(LEXER).lex obj/$(PARSER).h
	flex -o $@ $<

$(BUILD_DIR)/$(PARSER).c $(BUILD_DIR)/$(PARSER).h: $(SRC_DIR)/$(PARSER).y
	@mkdir obj --parent
	bison -d -o $(BUILD_DIR)/$(PARSER).c $<

clean:
	rm -f obj/*

mrproper: clean
	rm -f bin/*

test: $(BIN_DIR)/$(EXEC)
	@chmod u+x test
	./runtests.sh
