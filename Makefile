##### global settings #####

.PHONY: clean

CC := gcc
LD := ld
BIN := test
TESTFILE := hehe.decaf
CFLAGS := -m32 -O2 -Wall -c -Iinclude
ifdef DEBUG
CFLAGS +=-g -DYYDEBUG -DDEBUG
endif
ifdef SYMDEBUG
CFLAGS += -DSYMDEBUG
endif
ifdef IRDEBUG
CFLAGS += -DIRDEBUG
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
	gcc -m32 -o $(BIN) $(OBJS)

clean: 
	rm -rf *.h *.c *.o *.output *.dot $(BIN)

