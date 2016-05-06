#include <stdio.h>
#include <ctype.h>
#include <string.h>

//#define DEBUG

#define FALSE 0
#define TRUE 1

enum code {
	LIST,
	NUM,
	STR,
	SYM,
	EX_EQ,
	PLUS_OP,
	MINUS_OP,
	MUL_OP,
	LT_OP,
	GT_OP,
	GET_ARRAY_OP,
	SET_ARRAY_OP,
	CALL_OP,
//	PRINTLN_OP,
	IF_STATEMENT,
	BLOCK_STATEMENT,
	RETURN_STATEMENT,
	WHILE_STATEMENT,
	FOR_STATEMENT
};

typedef struct abstract_syntax_tree {
	enum code op;
	int val;
	struct symbol *sym;
	struct abstract_syntax_tree *left, *right;
	char *str;
} AST;

typedef struct symbol {
	char *name;
	int type, pointer;
//	int val;
//	int *addr;
//	AST *func_params;
//	AST *func_body;
} Symbol;

#define MAX_SYMBOLS 10000

extern Symbol SymbolTable[];
extern int n_symbols;

AST *makeSymbol(char *name);
Symbol *lookupSymbol(char *name);
Symbol *getSymbol(AST *p);

AST *makeNum(int val);
AST *makeStr(char *s);
AST *makeAST(enum code op, AST *right, AST *left);

AST *getNth(AST *p, int nth);
AST *getNext(AST *p);
AST *addLast(AST *l, AST *p);

#define getFirst(p) getNth(p, 0)
#define makeList1(x1) makeAST(LIST, x1, NULL)
#define makeList2(x1, x2) makeAST(LIST, x1, makeAST(LIST, x2, NULL))
#define makeList3(x1, x2, x3) makeAST(LIST, x1, makeAST(LIST, x2, makeAST(LIST, x3, NULL)))
#define addList(x1) ((x1->op!=LIST) ? makeList1(x1) : x1)

/* prototype for interface from parser to interpreter/compiler */
void defineFunction(Symbol *fsym, AST *params,AST *body);
void declareVariable(Symbol *vsym, AST *init_value);
void declareArray(Symbol *asym, AST *size);

void error(char *fmt, ...);
