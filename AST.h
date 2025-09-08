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
	FOR_STATEMENT,

	GENERIC_OP,
	GENERIC_ASSOC,
	DEFAULT_ASSOC,
	ARRAY_ACCESS,
	DOT_OP,
	PTR_OP,
	INC_OP,
	DEC_OP,
	COMPOUND_LITERAL,
	UNARY_OP,
	SIZEOF_OP,
	ALIGNOF_OP,
	ADDR_OP,
	DEREF_OP,
	BIT_NOT_OP,
	NOT_OP,
	CAST_OP,
	DIV_OP,
	MOD_OP,
	LEFT_OP,
	RIGHT_OP,
	LE_OP,
	GE_OP,
	EQ_OP,
	NE_OP,
	BIT_AND_OP,
	BIT_XOR_OP,
	BIT_OR_OP,
	AND_OP,
	OR_OP,
	COND_OP,
	ASSIGN_OP,
	STORAGE_SPEC,
	TYPE_SPEC,
	TYPE_QUAL,
	FUNC_SPEC,
	ALIGN_SPEC,
	INIT_DECL,
	VOID_TYPE,
	CHAR_TYPE,
	SHORT_TYPE,
	INT_TYPE,
	LONG_TYPE,
	FLOAT_TYPE,
	DOUBLE_TYPE,
	SIGNED_TYPE,
	UNSIGNED_TYPE,
	BOOL_TYPE,
	COMPLEX_TYPE,
	IMAGINARY_TYPE,
	ATOMIC_TYPE,
	STRUCT_TYPE,
	ENUM_TYPE,
	TYPEDEF_TYPE,
	STRUCT_SPEC,
	STRUCT_DECL,
	BITFIELD,
	ENUM_SPEC,
	ENUM_CONST,
	ATOMIC_SPEC,
	CONST_QUAL,
	RESTRICT_QUAL,
	VOLATILE_QUAL,
	ATOMIC_QUAL,
	ALIGNAS_SPEC,
	ARRAY_DECL,
	STAR,
	FUNC_DECL,
	POINTER,
	ELLIPSIS,
	PARAM_DECL,
	TYPE_NAME,
	ABS_DECL,
	ARRAY_ABS,
	FUNC_ABS,
	DESIGN_INIT,
	STATIC_ASSERT,
	LABEL_STAT,
	CASE_STAT,
	DEFAULT_STAT,
	SWITCH_STATEMENT,
	DO_WHILE_STATEMENT,
	GOTO_STATEMENT,
	CONTINUE_STATEMENT,
	BREAK_STATEMENT
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

void printAST(AST *p);

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
