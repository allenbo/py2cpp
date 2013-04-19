OBJ=main.o acceler.o bitset.o  graminit.o  listnode.o  node.o parser.o parsetok.o tokenizer.o grammar1.o ast.o Python-ast.o
HEADER=errcode.h graminit.h node.h opcode.h parser.h parsetok.h token.h tokenizer.h grammar.h Python-ast.h

main:$(OBJ)
	gcc -o main $(OBJ) -g
.c.o:
	gcc -c $< -g

clean:
	rm -rf *.o main