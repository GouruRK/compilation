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

ZIP_TARGET=ProjetCompilationL3_ALVES_KIES.zip

SOURCES=$(wildcard $(SRC_DIR)/*.c)
SRC_OBJS=$(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

BUILTIN=$(wildcard $(BUILTIN_DIR)/*.asm)
BUILTIN_OBJ=$(patsubst $(BUILTIN_DIR)/%.asm, $(BUILD_DIR)/%.o, $(BUILTIN))

TARGET=$(wildcard $(BUILD_DIR)/*.asm)
TARGET_OBJ=$(patsubst $(BUILD_DIR)/%.asm, $(BUILD_DIR)/%.o, $(TARGET))
TARGET_NAME=$(patsubst $(BUILD_DIR)/%.asm, $(BIN_DIR)/%, $(TARGET))

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

$(BUILD_DIR)/%.o: $(BUILTIN_DIR)/%.asm
	nasm -f elf64 -o $@ $< 

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.asm
	nasm -f elf64 -o $@ $< 

clean:
	rm -f obj/*

mrproper: clean
	rm -f bin/*

zip: mrproper
	rm -f $(ZIP_TARGET)
	zip -r $(ZIP_TARGET) include/ src/ makefile

test: $(BIN_DIR)/$(EXEC)
	@chmod u+x runtests.sh
	./runtests.sh
