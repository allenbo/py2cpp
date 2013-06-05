OBJ=main.o acceler.o bitset.o  graminit.o  listnode.o  node.o parser.o parsetok.o tokenizer.o grammar1.o ast.o Python-ast.o type.o symtab.o context.o
HEADER=errcode.h graminit.h node.h opcode.h parser.h parsetok.h token.h tokenizer.h grammar.h Python-ast.h symtab.h context.h

OBJ += code.o util.o
HEADER += code.h util.h
CFLAGS=
main:$(OBJ)
	gcc -o main $(OBJ) -g $(CFLAGS)
.c.o:
	gcc -c $< -g $(CFLAGS)

clean:
	rm -rf *.o main
