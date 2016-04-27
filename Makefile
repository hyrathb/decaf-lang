##### global settings #####

.PHONY: clean

CC := gcc
LD := ld
BIN := test
TESTFILE := hehe.decaf
CFLAGS := -O2 -Wall -c -Iinclude
ifdef DEBUG
CFLAGS +=-g -DYYDEBUG -DDEBUG
endif
LEXICAL := decaf.l
GRAMMAR := decaf.y
LEXICAL_C := lexical.c
GRAMMAR_C := decaf.c
OBJS += lexical.o
OBJS += decaf.o
OBJS += symtable.o
SOURCE += source/symtable.c


all: bin 

test: bin
	./$(BIN) $(TESTFILE)

src : $(LEXICAL) $(GRAMMAR)
	flex -F -o $(LEXICAL_C) $(LEXICAL)
	bison -v -g -d -o $(GRAMMAR_C) $(GRAMMAR)

$(OBJS): src $(SOURCE)
	$(CC) $(CFLAGS) $(LEXICAL_C) $(GRAMMAR_C) $(SOURCE)

bin: $(OBJS)
	gcc -o $(BIN) $(OBJS)

clean: 
	rm -rf *.h *.c *.o *.output *.dot $(BIN)

