BUILD_DIR=build

OBJ=main.o \
		acceler.o \
		bitset.o \
		graminit.o  \
		listnode.o \
		node.o \
		parser.o \
		parsetok.o \
		tokenizer.o \
		grammar1.o \
		ast.o \
		Python-ast.o \
		type.o \
		symtab.o \
		context.o \
		exception.o \
		operator.o \
		primary.o

OBJ += code.o util.o

BUILD_OBJ=$(addprefix $(BUILD_DIR)/, $(OBJ))

HEADER=errcode.h \
		graminit.h \
		node.h \
		opcode.h \
		parser.h \
		parsetok.h \
		token.h \
		tokenizer.h \
		grammar.h \
		Python-ast.h \
		symtab.h \
		context.h \
		exception.h \
		operator.h \
		primary.h

HEADER += code.h util.h
CFLAGS=

TARGET=converter


.PHONY:all make_dirs
all:make_dirs $(TARGET)

make_dirs:$(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $@


$(TARGET):$(BUILD_OBJ)
	gcc -o $(TARGET) $(BUILD_OBJ) -g $(CFLAGS)

$(BUILD_DIR)/%.o: %.c
	gcc -c $< -g $(CFLAGS) -o $@

clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)
