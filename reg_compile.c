#include "AST.h"
#include "reg_code.h"

#define VAR_ARG 0
#define VAR_LOCAL 1
#define VAR_GLOBAL 2

#define MAX_ENV 1000

typedef struct env {
	Symbol *var;
	int var_kind;
	int pos;
} Environment;

int envp = 0;
Environment Env[MAX_ENV];

int label_counter = 0;
int local_var_pos;
int tmp_counter = 0;

void compileStoreVar(Symbol *var, int r)
{
	for (int i=envp-1; i>=0; i--) {
		if (Env[i].var == var) {
			switch (Env[i].var_kind) {
			case VAR_ARG:
				genCode2(STOREA, r, Env[i].pos);
				return;
			case VAR_LOCAL:
				genCode2(STOREL, r, Env[i].pos);
				return;
			case VAR_GLOBAL:
				genCode2(STOREL, r, Env[i].pos);//FIXME
				return;
			}
		}
	}
	error("undefined variable '%s'\n", var->name);
}

void compileLoadVar(int target, Symbol *var)
{
	for (int i=envp-1; i>=0; i--) {
		if (Env[i].var == var) {
			switch (Env[i].var_kind) {
			case VAR_ARG:
				genCode2(LOADA, target, Env[i].pos);
				return;
			case VAR_LOCAL:
				genCode2(LOADL, target, Env[i].pos);
				return;
			case VAR_GLOBAL:
				genCode2(LOADS, target, Env[i].pos);//FIXME
				return;
			}
		}
	}
	error("undefined variable '%s'\n", var->name);
}

void compileStatement(AST *p);
void defineFunction(Symbol *fsym, AST *params, AST *body)
{
	initGenCode();
//	envp = 0;
	int param_pos = 0;
	local_var_pos = 0;
	for ( ; params != NULL; params = getNext(params)) {
		Env[envp].var = getSymbol(getFirst(params));
		Env[envp].var_kind = VAR_ARG;
		Env[envp].pos = param_pos++;
		envp++;
	}
	compileStatement(body);
	genFuncCode(fsym->name, local_var_pos);
//	envp = 0;  /* reset */
}

void compileBlock(AST *local_vars, AST *statements)
{
//	AST *ast = local_vars;
	int envp_save = envp;
	for ( ; local_vars != NULL; local_vars = getNext(local_vars)) {
		AST *a = getFirst(local_vars);
		if (a->op != SYM) {	// int a = 10;
			compileStatement(a);
			continue;
		}

		Env[envp].var = getSymbol(getFirst(local_vars));
		Env[envp].var_kind = VAR_LOCAL;
		Env[envp].pos = local_var_pos++;
		envp++;
	}
	for ( ; statements != NULL; statements = getNext(statements)) {
		compileStatement(getFirst(statements));
	}
/*	for ( ; ast != NULL; ast = getNext(ast)) {
		AST *a = getFirst(ast);
		if (a->op == SYM) continue;
		compileStatement(getFirst(ast));
	}*/
	envp = envp_save;
}

int compileArgs(AST *args, int i);
void compileCallFunc(int target, Symbol *f, AST *args)
{
	int narg = compileArgs(args, 0);
	genCodeS(CALL, target, narg, f->name);
}

/*void compileExpr(int target, AST *p);
void printFunc(AST *args)
{
	int l = genString(getNth(args, 0)->str);
	int r = tmp_counter++;
	compileExpr(r, getNth(args, 1));
	genCode2(PRINTLN, r, l);
}*/

void compileExpr(int target, AST *p)
{
	if (!p) return;

	int r1, r2;
	switch (p->op) {
	case NUM:
		genCode2(LOADI, target, p->val);
		return;
	case STR:
		r1 = genString(p->str);
		genCode2(LOADS, target, r1/*.LC[0-9]*/);
		return;
	case SYM:
		compileLoadVar(target, getSymbol(p));
		return;
	case EX_EQ:
		if (target != -1) {
			error("assign has no value\n");
		}
		r1 = tmp_counter++;
		compileExpr(r1, p->right);
		compileStoreVar(getSymbol(p->left), r1);
		return;

	case PLUS_OP:
		r1 = tmp_counter++;
		r2 = tmp_counter++;
		compileExpr(r1, p->left);
		compileExpr(r2, p->right);
		genCode3(ADD, target, r1, r2);
		return;
	case MINUS_OP:
		r1 = tmp_counter++;
		r2 = tmp_counter++;
		compileExpr(r1, p->left);
		compileExpr(r2, p->right);
		genCode3(SUB, target, r1, r2);
		return;
	case MUL_OP:
		r1 = tmp_counter++;
		r2 = tmp_counter++;
		compileExpr(r1, p->left);
		compileExpr(r2, p->right);
		genCode3(MUL, target, r1, r2);
		return;
/*	case DIV_OP:
		r1 = tmp_counter++;
		r2 = tmp_counter++;
		compileExpr(r1, p->left);
		compileExpr(r2, p->right);
		genCode3(DIV, target, r1, r2);
		return;*/
	case LT_OP:
		r1 = tmp_counter++;
		r2 = tmp_counter++;
		compileExpr(r1, p->left);
		compileExpr(r2, p->right);
		genCode3(LT, target, r1, r2);
		return;
	case GT_OP:
		r1 = tmp_counter++;
		r2 = tmp_counter++;
		compileExpr(r1, p->left);
		compileExpr(r2, p->right);
		genCode3(GT, target, r1, r2);
		return;
	case CALL_OP:
		compileCallFunc(target, getSymbol(p->left), p->right);
		return;

/*	case PRINTLN_OP:
		if (target != -1) {
			error("println has no value\n");
		}
		printFunc(p->left);
		return;*/

	case GET_ARRAY_OP:
	/* not implemented */
	case SET_ARRAY_OP:
	/* not implemented */

	default:
		error("unknown operater/statement %d\n", p->op);
	}
}

void compileReturn(AST *expr)
{
	int r;
	if (expr != NULL) {
		r = tmp_counter++;
		compileExpr(r, expr);
	} else {
		r = -1;
	}
	genCode1(RET, r);
}

int compileArgs(AST *args, int i)
{
	int r, n;

	if (args != NULL) {
		n = compileArgs(getNext(args), i+1);
		r = tmp_counter++;
		compileExpr(r, getFirst(args));
		genCode2(ARG, r, i);
	} else {
		return 0;
	}
	return n+1;
}

void compileIf(AST *cond, AST *then_part, AST *else_part)
{
	int l1, l2;
	int r;

	r = tmp_counter++;
	compileExpr(r, cond);
	l1 = label_counter++;
	genCode2(BEQ0, r, l1);
	compileStatement(then_part);
	if (else_part != NULL) {
		l2 = label_counter++;
		genCode1(JUMP, l2);
		genCode1(LABEL, l1);
		compileStatement(else_part);
		genCode1(LABEL, l2);
	} else {
		genCode1(LABEL, l1);
	}
}

void compileWhile(AST *cond,AST *body)
{
	int l1,l2;
	int r;

	l1 = label_counter++;
	l2 = label_counter++;
	r = tmp_counter++;

	genCode1(LABEL, l1);
	compileExpr(r, cond);
	genCode2(BEQ0, r, l2);
	compileStatement(body);
	genCode1(JUMP, l1);
	genCode1(LABEL, l2);
}

void compileFor(AST *init,AST *cond,AST *iter,AST *body)
{
	/* not implemented */
}

void compileStatement(AST *p)
{
	if (p == NULL) return;

	switch (p->op) {
	case BLOCK_STATEMENT:
		compileBlock(p->left, p->right);
		break;
	case RETURN_STATEMENT:
		compileReturn(p->left);
		break;
	case IF_STATEMENT:
		compileIf(p->left, getNth(p->right, 0), getNth(p->right, 1));
		break;
	case WHILE_STATEMENT:
		compileWhile(p->left, p->right);
		break;
	case FOR_STATEMENT:
		compileFor(getNth(p->left, 0), getNth(p->left, 1), getNth(p->left, 2), p->right);
		break;
	default:
		compileExpr(-1, p);
	}
}

// global variable
void declareVariable(Symbol *vsym, AST *init_value)
{
	int a = init_value ? init_value->val : 0;
//	printf("declareVariable %s = %d\n", vsym->name, a);
	int l = genStatic(a);

	Env[envp].var = vsym;
	Env[envp].var_kind = VAR_GLOBAL;
	Env[envp].pos = l/*init_value*/;
	envp++;
}

// Array
void declareArray(Symbol *a, AST *size)
{
	/* not implemented */
	printf("declareArray is not implemented.\n");
}

#if 0
void compileExpression(int target, Object *p)
{
	Object *q;
	Symbol *op;
	int r1,r2;
	extern int tmp_counter;

	switch (p->kind) {
	case NUM:
		genCode2(LOADI,target,p->val);
		return;
	case SYM:
		compileLoadVar(target,getSymbol(p));
		return;
	case LIST:
		q = getFirst(p);
		if (q->kind != SYM) {
			compile_error("bad function or operator\n");
		}
		op = q->sym;
		if (op == plusSym) {
			r1 = tmp_counter++;
			r2 = tmp_counter++;
			compileExpression(r1,getNth(p,1));
			compileExpression(r2,getNth(p,2));
			genCode3(ADD,target,r1,r2);
			return;
		}
		if (op == minusSym) {
			r1 = tmp_counter++;
			r2 = tmp_counter++;
			compileExpression(r1,getNth(p,1));
			compileExpression(r2,getNth(p,2));
			genCode3(SUB,target,r1,r2);
			return;
		}
		if (op == mulSym) {
			r1 = tmp_counter++;
			r2 = tmp_counter++;
			compileExpression(r1,getNth(p,1));
			compileExpression(r2,getNth(p,2));
			genCode3(MUL,target,r1,r2);
			return;
		}
		if (op == lessSym) {
			r1 = tmp_counter++;
			r2 = tmp_counter++;
			compileExpression(r1,getNth(p,1));
			compileExpression(r2,getNth(p,2));
			genCode3(LT,target,r1,r2);
			return;
		}
		if (op == greaterSym) {
			r1 = tmp_counter++;
			r2 = tmp_counter++;
			compileExpression(r1,getNth(p,1));
			compileExpression(r2,getNth(p,2));
			genCode3(GT,target,r1,r2);
			return;
		}
		if (op == eqSym) {
			compile_error("assign operator must be statement\n");
			return;
		}
		compileCallFunc(target,op,getNext(p));
	}
}
#endif
