/*
 * register machine intermediate opcode
 */

#define LOADI 	1	// LOADI r,i	load int
#define LOADA 	2	// LOADA r,n	load arg
#define LOADL	3	// LOADL r,n	load local var
#define STOREA	4	// STOREA r,n	store arg
#define STOREL	5	// STOREL r,n	store local var
#define ADD 	6	// ADD t,r1,r2
#define SUB 	7	// SUB t,r1,r2
#define MUL 	8	// MUL t,r1,r2
//#define DIV 	8	// DIV t,r1,r2
#define GT	9 	// GT  t,r1,r2
#define LT	10	// LT  r,r1,r2
#define BEQ0	11	// BQ  r,L	branch if eq 0
#define JUMP	12	// JUMP L
#define ARG     13	// ARG r,n	set Argument
#define CALL	14	// CALL r,func
#define RET	15	// RET r	return
#define PRINTLN	16	// PRINTLN r,L	println function
#define LABEL 	17	// LABEL L	label
#define LOADS   18	// load string label

#define LOADADDR 19
#define LOAD    20
#define STORE	21

char *code_name(int code);
int get_code(char *name);

void initGenCode();
void genCode1(int opcode, int operand1);
void genCode2(int opcode, int operand1, int operand2);
void genCode3(int opcode, int operand1, int operand2, int operand3);
void genCodeS(int opcode, int operand1, int operand2, char *s);

void genFuncCode(char *entry_name, int n_local);
int genString(char *s);
int genStatic(int a);
