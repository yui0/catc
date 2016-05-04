#include <stdlib.h>
#include "AST.h"

Symbol SymbolTable[MAX_SYMBOLS];
int n_symbols = 0;

AST *makeNum(int val)
{
	AST *p = (AST *)malloc(sizeof(AST));
	p->op = NUM;
	p->val = val;
	return p;
}

AST *makeStr(char *s)
{
	AST *p = (AST *)malloc(sizeof(AST));
	p->op = STR;
	p->str = s;
	return p;
}

AST *makeAST(enum code op, AST *left, AST *right)
{
//	printf("[%d,%x,%x]\n",op,left,right);
	AST *p = (AST *)malloc(sizeof(AST));
	p->op = op;
	p->left = left;
	p->right = right;
	return p;
}

AST *getNth(AST *p, int nth)
{
	if (p->op != LIST) {
		fprintf(stderr, "bad access to list\n");
		exit(1);
	}
	if (nth > 0) {
		return (getNth(p->right, nth-1));
	} else {
		return p->left;
	}
}

AST *addLast(AST *l, AST *p)
{
	AST *q;

	if (l == NULL) {
		return makeAST(LIST, p, NULL);
	}
	q = l;
	while (q->right != NULL) {
		q = q->right;
	}
	q->right = makeAST(LIST, p, NULL);
	return l;
}

AST *getNext(AST *p)
{
	if (p->op != LIST) {
		fprintf(stderr, "bad access to list\n");
		exit(1);
	} else {
		return p->right;
	}
}

Symbol *lookupSymbol(char *name)
{
	int i;
	Symbol *sp = NULL;
	for (i = 0; i < n_symbols; i++) {
		if (strcmp(SymbolTable[i].name,name) == 0) {
			sp = &SymbolTable[i];
			break;
		}
	}
	if (sp == NULL) {
		sp = &SymbolTable[n_symbols++];
		sp->name = strdup(name);
	}
	return sp;
}

AST *makeSymbol(char *name)
{
	//printf("SYM:[%s]\n", name);
	AST *p = (AST *)malloc(sizeof(AST));
	p->op = SYM;
	p->sym = lookupSymbol(name);
	return p;
}

Symbol *getSymbol(AST *p)
{
	if (!p) fprintf(stderr, "AST is null at getSymbol!\n");
	if (p->op != SYM) {
		fprintf(stderr, "bad access to symbol\n");
		exit(1);
	} else {
		return p->sym;
	}
}

// for debug
char *code_name(enum code op)
{
	switch (op) {
	case LIST:
		return "LIST";
	case NUM:
		return "NUM";
	case STR:
		return "STR";
	case SYM:
		return "SYM";
	case EX_EQ:
		return "EX_EQ";
	case PLUS_OP:
		return "PLUS_OP";
	case MINUS_OP:
		return "MINUS_OP";
	case MUL_OP:
		return "MUL_OP";
	case LT_OP:
		return "LT_OP";
	case GT_OP:
		return "GT_OP";
	case GET_ARRAY_OP:
		return "GET_ARRAY_OP";
	case SET_ARRAY_OP:
		return "SET_ARRAY_OP";
	case CALL_OP:
		return "CALL_OP";
//	case PRINTLN_OP:
//		return "PRINTLN_OP";
	case IF_STATEMENT:
		return "IF_STATEMENT";
	case BLOCK_STATEMENT:
		return "BLOCK_STATEMENT";
	case RETURN_STATEMENT:
		return "RETURN_STATEMENT";
	case WHILE_STATEMENT:
		return "WHILE_STATEMENT";
	case FOR_STATEMENT:
		return "FOR_STATEMENT";
	default:
		return "???";
	}
}
void printAST(AST *p)
{
	if (p == NULL) {
		printf("()");
		return;
	}
	switch (p->op) {
	case NUM:
		printf("%d", p->val);
		break;
	case SYM:
		printf("'%s'", p->sym->name);
		break;
	case LIST:
		printf("(LIST ");
		while (p != NULL) {
			printAST(p->left);
			p = p->right;
			if (p != NULL) {
				printf(" ");
			}
		}
		printf(")");
		break;
	default:
		printf("(%s ", code_name(p->op));
		printAST(p->left);
		printf(" ");
		printAST(p->right);
		printf(")");
	}
	fflush(stdout);
}
