CC=gcc
CFLAGS=-Wall -g -Iinclude -Iobj -Isrc
PARSER=parser
LEXER=lexer
EXEC=tpcc

INCLUDE_DIR=include
SRC_DIR=src
BUILD_DIR=obj
BIN_DIR=bin
BUILTIN_DIR=builtin

ARCHIVE_OUTPUT=ProjetCompilationL3_ALVES_KIES

SOURCES=$(wildcard $(SRC_DIR)/*.c)
SRC_OBJS=$(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

TARGET=$(wildcard $(BUILD_DIR)/*.asm)
TARGET_OBJ=$(patsubst $(BUILD_DIR)/%.asm, $(BUILD_DIR)/%.o, $(TARGET))
TARGET_NAME=$(basename $(patsubst $(BUILD_DIR)/%.asm, $(BIN_DIR)/%.o, $(TARGET)))

$(BIN_DIR)/$(EXEC): obj/$(LEXER).o obj/$(PARSER).o $(SRC_OBJS)
	@mkdir $(BIN_DIR) --parent
	$(CC) -o $@ $^

$(BUILD_DIR)/$(PARSER).o: obj/$(PARSER).c $(INCLUDE_DIR)/tree.h $(INCLUDE_DIR)/args.h
$(BUILD_DIR)/$(LEXER).o: obj/$(LEXER).c obj/$(PARSER).h

$(BUILD_DIR)/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(BUILD_DIR)/$(LEXER).c: src/$(LEXER).lex obj/$(PARSER).h
	flex -o $@ $<

$(BUILD_DIR)/$(PARSER).c $(BUILD_DIR)/$(PARSER).h: $(SRC_DIR)/$(PARSER).y
	@mkdir $(BUILD_DIR) --parent
	bison -d -o $(BUILD_DIR)/$(PARSER).c $<

asm: $(TARGET_OBJ) $(BUILTIN_OBJ)
	$(CC) -o $(TARGET_NAME) $^ -nostartfiles -no-pie

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.asm
	nasm -f elf64 -o $@ $< 

clean:
	rm -f obj/*

mrproper: clean
	rm -f bin/*

zip: mrproper
	rm -f $(ARCHIVE_OUTPUT).zip
	zip -r $(ARCHIVE_OUTPUT).zip include/ src/ builtin/ makefile

tar: mrproper
	tar -czf $(ARCHIVE_OUTPUT).tar.gz --transform 's,^,$(ARCHIVE_OUTPUT)/,' builtin/ include/ rep/ src/ test/ makefile runtests.sh 

test: $(BIN_DIR)/$(EXEC)
	@chmod u+x runtests.sh
	./runtests.sh
