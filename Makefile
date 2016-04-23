##### global settings #####

.PHONY: clean

CC := gcc
LD := ld
BIN := test
TESTFILE := hehe.decaf
CFLAGS := -O2 -Wall -c -g -DYYDEBUG -Iinclude
LEXICAL := decaf.l
GRAMMAR := decaf.y
LEXICAL_C := lexical.c
GRAMMAR_C := decaf.c
OBJS += lexical.o
OBJS += decaf.o



all: bin 

test: bin
	./$(BIN) $(TESTFILE)

src : $(LEXICAL) $(GRAMMAR)
	flex -F -o $(LEXICAL_C) $(LEXICAL)
	bison -v -d -o $(GRAMMAR_C) $(GRAMMAR)

$(OBJS): src
	$(CC) $(CFLAGS) $(LEXICAL_C) $(GRAMMAR_C)

bin: $(OBJS)
	gcc -o $(BIN) $(OBJS)

clean: 
	rm -rf *.h *.c *.o *.output $(BIN)

