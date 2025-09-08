# ©2016,2025 YUICHIRO NAKADA

CC = clang
CFLAGS = -Wall -Os
LEX = flex
YACC = yacc -dv

# Default to 64-bit if ARCH is not specified
ARCH ?= 64

# Set executable name and architecture-specific flags
ifeq ($(ARCH),32)
    PROGRAM = catc32
    CFLAGS += -m32
    OBJS = y.tab.o AST.o reg_compile.o x86_code_gen.o
else
    PROGRAM = catc
    CFLAGS += -m64
    OBJS = y.tab.o AST.o reg_compile.o x86_64_code_gen.o
endif

#OBJS = y.tab.o AST.o reg_compile.o x86_code_gen.o

.SUFFIXES: .c .o

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROGRAM) $^

.c.o:
	$(CC) $(CFLAGS) -c $<

lex.yy.c: clex.l
	$(LEX) clex.l
y.tab.c: cparser.y
	$(YACC) cparser.y
y.tab.o: cparser.y lex.yy.c

.PHONY: clean
clean:
	$(RM) catc catc32 $(OBJS) lex.yy.c y.tab.[ch] y.output *.o *.s
