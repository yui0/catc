# ©2016 YUICHIRO NAKADA

CC = clang
CFLAGS = -Wall -Os
LEX = flex
YACC = yacc -dv

PROGRAM = catc
OBJS = y.tab.o AST.o reg_compile.o x86_code_gen.o

.SUFFIXES: .c .o

$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $^

.c.o:
	$(CC) $(CFLAGS) -c $<

lex.yy.c: c.l
	$(LEX) c.l
y.tab.c: cparser.y
	$(YACC) cparser.y
y.tab.o: cparser.y
#y.tab.o: cparser.y lex.yy.c

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS) lex.yy.c y.tab.[ch] y.output
