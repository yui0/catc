#include <stdio.h>
#include <sys/utsname.h>
#include "reg_code.h"

#define MAX_CODE 100

extern FILE *yyout;

struct _code {
	int opcode;
	int operand1, operand2, operand3;
	char *s_operand;
} Codes[MAX_CODE];

int isDarwin = 0;
void checkSystem()
{
	struct utsname uts;
	uname(&uts);
	if (strcmp(uts.sysname, "Darwin") == 0) {
		isDarwin = 1;
	}
	/* else Linux */
}

int n_code;
void initGenCode()
{
	n_code = 0;
	checkSystem();
}

void genCode1(int opcode, int operand1)
{
	Codes[n_code].operand1 = operand1;
	Codes[n_code++].opcode = opcode;
}

void genCode2(int opcode, int operand1, int operand2)
{
	Codes[n_code].operand1 = operand1;
	Codes[n_code].operand2 = operand2;
	Codes[n_code++].opcode = opcode;
}

void genCode3(int opcode, int operand1, int operand2, int operand3)
{
	Codes[n_code].operand1 = operand1;
	Codes[n_code].operand2 = operand2;
	Codes[n_code].operand3 = operand3;
	Codes[n_code++].opcode = opcode;
}

void genCodeS(int opcode, int operand1, int operand2, char *s)
{
	Codes[n_code].operand1 = operand1;
	Codes[n_code].operand2 = operand2;
	Codes[n_code].s_operand = s;
	Codes[n_code++].opcode = opcode;
}

/*
 *  code generator for x86
 */

#define N_REG 4
#define N_SAVE 4

#define TMP_OFF(i) 		-((i+1)+1)*4
#define LOCAL_VAR_OFF(i)	-(N_SAVE+1+(i+1))*4
#define ARG_OFF(i)		((i)+2)*4

#define REG_AX 0
#define REG_BX 1
#define REG_CX 2
#define REG_DX 3

#define MAX_ARGS 6 /* for Darwin */

char *tmpRegName[N_REG] = { "%eax", "%ebx", "%ecx", "%edx" };
int tmpRegState[N_REG];
int tmpRegSave[N_SAVE];

/*
 * sample register allocation
 */
void initTmpReg()
{
	int i;
	for (i = 0; i < N_REG; i++) {
		tmpRegState[i] = -1;
	}
	for (i = 0; i < N_SAVE; i++) {
		tmpRegSave[i] = -1;
	}
}

/* getReg: get free register */
int getReg(int r)
{
	for (int i=0; i < N_REG; i++) {
		if (tmpRegState[i] < 0) {
			tmpRegState[i] = r;
			return i;
		}
	}
	error("no temp reg\n");
}

void saveReg(int reg)
{
	if (tmpRegState[reg] < 0) return;

	for (int i=0; i < N_SAVE; i++) {
		if (tmpRegSave[i] < 0) {
			fprintf(yyout, "\tmovl\t%s,%d(%%ebp)\n",tmpRegName[reg],TMP_OFF(reg));
			tmpRegSave[i] = tmpRegState[reg];
			tmpRegState[reg] = -1;
			return;
		}
	}
	error("no temp save\n");
}

void saveAllRegs()
{
	for (int i=0; i < N_REG; i++) {
		saveReg(i);
	}
}

/* assign r to reg */
void assignReg(int r, int reg)
{
	if (tmpRegState[reg] == r) return;

	saveReg(reg);
	tmpRegState[reg] = r;
}

/* load r into reg */
int useReg(int r)
{
	int i, rr;

	for (i = 0; i < N_REG; i++) {
		if (tmpRegState[i] == r) {
			return i;
		}
	}
	/* not found in register, then restore from save area. */
	for (i = 0; i < N_SAVE; i++) {
		if (tmpRegSave[i] == r) {
			rr = getReg(r);
			tmpRegSave[i] = -1;
			/* load into regsiter */
			fprintf(yyout, "\tmovl\t%d(%%ebp),%s\n",TMP_OFF(i),tmpRegName[rr]);
			return rr;
		}
	}
	error("reg is not found\n");
}

void freeReg(int reg)
{
	tmpRegState[reg] = -1;
}

/*
 * Code generation
 */
extern int label_counter;

void genFuncCode(char *entry_name, int n_local)
{
	int i;
	int opd1,opd2,opd3;
	int r,r1,r2;
	char *opds;
	int ret_lab,l1,l2;
	int frame_size;

	// function header
	fprintf(yyout, "\t.text\n");
	fprintf(yyout, "\t.align\t4\n");
	if (isDarwin) {
		fprintf(yyout, "\t.globl\t_%s\n", entry_name);    /* .globl <name> */
		fprintf(yyout, "_%s:\n", entry_name);             /* <name>:              */
	} else {
		fprintf(yyout, "\t.globl\t%s\n", entry_name);    /* .globl <name> */
		fprintf(yyout, "\t.type\t%s,@function\n", entry_name);/* .type <name>,@function */
		fprintf(yyout, "%s:\n", entry_name);             /* <name>:              */
	}
	fprintf(yyout, "\tpushl\t%%ebp\n");
	fprintf(yyout, "\tmovl\t%%esp,%%ebp\n");

	frame_size = -LOCAL_VAR_OFF(n_local);
	ret_lab = label_counter++;

	if (isDarwin) {
		frame_size = ((frame_size+16*MAX_ARGS+15)&(~15)) - 8;
	}

	fprintf(yyout, "\tsubl\t$%d,%%esp\n",frame_size);
	fprintf(yyout, "\tmovl\t%%ebx,-4(%%ebp)\n");

	initTmpReg();

	for (i = 0; i < n_code; i++) {
		/*debug*//* fprintf(yyout, "%s %d %d %d\n",code_name(Codes[i].opcode),
		       Codes[i].operand1,Codes[i].operand2,Codes[i].operand3); */
		opd1 = Codes[i].operand1;
		opd2 = Codes[i].operand2;
		opd3 = Codes[i].operand3;
		opds = Codes[i].s_operand;

		switch (Codes[i].opcode) {
		case LOADI:	// load int
			if (opd1 < 0) break;
			r = getReg(opd1);
			fprintf(yyout, "\tmovl\t$%d,%s\n", opd2, tmpRegName[r]);
			break;
		case LOADS:	// load string label
			if (opd1 < 0) break;
			r = getReg(opd1);
			fprintf(yyout, "\tlea\t.LC%d,%s\n", opd2, tmpRegName[r]);
			break;
		case LOADA:	/* load arg */
			if (opd1 < 0) {
				break;
			}
			r = getReg(opd1);
			fprintf(yyout, "\tmovl\t%d(%%ebp),%s\n",ARG_OFF(opd2),tmpRegName[r]);
			break;
		case LOADL:	/* load local */
			if (opd1 < 0) {
				break;
			}
			r = getReg(opd1);
			fprintf(yyout, "\tmovl\t%d(%%ebp),%s\n",LOCAL_VAR_OFF(opd2),tmpRegName[r]);
			break;
		case STOREA:	/* store arg */
			r = useReg(opd1);
			freeReg(r);
			fprintf(yyout, "\tmovl\t%s,%d(%%ebp)\n",tmpRegName[r],ARG_OFF(opd2));
			break;
		case STOREL:	/* store local */
			r = useReg(opd1);
			freeReg(r);
			fprintf(yyout, "\tmovl\t%s,%d(%%ebp)\n",tmpRegName[r],LOCAL_VAR_OFF(opd2));
			break;
		case BEQ0:	/* conditional branch */
			r = useReg(opd1);
			freeReg(r);
			fprintf(yyout, "\tcmpl\t$0,%s\n",tmpRegName[r]);
			fprintf(yyout, "\tje\t.L%d\n",opd2);
			break;
		case LABEL:
			fprintf(yyout, ".L%d:\n",Codes[i].operand1);
			break;
		case JUMP:
			fprintf(yyout, "\tjmp\t.L%d\n",Codes[i].operand1);
			break;

		case CALL:
			saveAllRegs();
			if (isDarwin) {
				fprintf(yyout, "\tcall\t_%s\n",opds);
			} else {
				fprintf(yyout, "\tcall\t%s\n",opds);
			}
			if (opd1 < 0) {
				break;
			}
			assignReg(opd1,REG_AX);
			if (!isDarwin) {
				fprintf(yyout, "\tadd $%d,%%esp\n",opd2*4);
			}
			break;
		case ARG:
			r = useReg(opd1);
			freeReg(r);
			if (isDarwin) {
				fprintf(yyout, "\tmovl\t%s,%d(%%esp)\n",tmpRegName[r],opd2*4);
			} else {
				fprintf(yyout, "\tpushl\t%s\n",tmpRegName[r]);
			}
			break;
		case RET:
			r = useReg(opd1);
			freeReg(r);
			if (r != REG_AX) {
				fprintf(yyout, "\tmovl\t%s,%%eax\n",tmpRegName[r]);
			}
			fprintf(yyout, "\tjmp .L%d\n",ret_lab);
			break;

		case ADD:
			r1 = useReg(opd2);
			r2 = useReg(opd3);
			freeReg(r1);
			freeReg(r2);
			if (opd1 < 0) {
				break;
			}
			assignReg(opd1,r1);
			fprintf(yyout, "\taddl\t%s,%s\n",tmpRegName[r2],tmpRegName[r1]);
			break;
		case SUB:
			r1 = useReg(opd2);
			r2 = useReg(opd3);
			freeReg(r1);
			freeReg(r2);
			if (opd1 < 0) {
				break;
			}
			assignReg(opd1,r1);
			fprintf(yyout, "\tsubl\t%s,%s\n",tmpRegName[r2],tmpRegName[r1]);
			break;
		case MUL:
			r1 = useReg(opd2);
			r2 = useReg(opd3);
			freeReg(r1);
			freeReg(r2);
			if (opd1 < 0) {
				break;
			}
			assignReg(opd1,REG_AX);
			saveReg(REG_DX);
			if (r1 != REG_AX) {
				fprintf(yyout, "\tmovl %s,%s\n",tmpRegName[r1],tmpRegName[REG_AX]);
			}
			fprintf(yyout, "\timull\t%s,%s\n",tmpRegName[r2],tmpRegName[REG_AX]);
			break;
		case LT:
			r1 = useReg(opd2);
			r2 = useReg(opd3);
			freeReg(r1);
			freeReg(r2);
			if (opd1 < 0) {
				break;
			}
			r = getReg(opd1);
			l1 = label_counter++;
			l2 = label_counter++;
			fprintf(yyout, "\tcmpl\t%s,%s\n",tmpRegName[r2],tmpRegName[r1]);
			fprintf(yyout, "\tjl .L%d\n",l1);
			fprintf(yyout, "\tmovl\t$0,%s\n",tmpRegName[r]);
			fprintf(yyout, "\tjmp .L%d\n",l2);
			fprintf(yyout, ".L%d:\tmovl\t$1,%s\n",l1,tmpRegName[r]);
			fprintf(yyout, ".L%d:",l2);
			break;
		case GT:
			r1 = useReg(opd2);
			r2 = useReg(opd3);
			freeReg(r1);
			freeReg(r2);
			if (opd1 < 0) {
				break;
			}
			r = getReg(opd1);
			l1 = label_counter++;
			l2 = label_counter++;
			fprintf(yyout, "\tcmpl\t%s,%s\n",tmpRegName[r2],tmpRegName[r1]);
			fprintf(yyout, "\tjg .L%d\n",l1);
			fprintf(yyout, "\tmovl\t$0,%s\n",tmpRegName[r]);
			fprintf(yyout, "\tjmp .L%d\n",l2);
			fprintf(yyout, ".L%d:\tmovl\t$1,%s\n",l1,tmpRegName[r]);
			fprintf(yyout, ".L%d:",l2);
			break;

		case PRINTLN:
			r = useReg(opd1);
			freeReg(r);
			if (isDarwin) {
				fprintf(yyout, "\tmovl\t%s,4(%%esp)\n",tmpRegName[r]);
				fprintf(yyout, "\tlea\t.LC%d,%s\n",opd2, tmpRegName[r]);
				fprintf(yyout, "\tmovl\t%s,0(%%esp)\n",tmpRegName[r]);
				saveAllRegs();
				fprintf(yyout, "\tcall\t_println\n");
			} else {
				fprintf(yyout, "\tpushl\t%s\n",tmpRegName[r]);
				fprintf(yyout, "\tlea\t.LC%d,%s\n",opd2, tmpRegName[r]);
				fprintf(yyout, "\tpushl\t%s\n",tmpRegName[r]);
				saveAllRegs();
				fprintf(yyout, "\tcall\tprintln\n");
				fprintf(yyout, "\taddl\t$8,%%esp\n");
			}
			break;
		}
	}

	/* return sequence */
	fprintf(yyout, ".L%d:\tmovl\t-4(%%ebp), %%ebx\n",ret_lab);
	fprintf(yyout, "\tleave\n");
	fprintf(yyout, "\tret\n");
}

int genString(char *s)
{
	int l;
	l = label_counter++;
	if (isDarwin) {
		fprintf(yyout, "\t.cstring\n");
		fprintf(yyout, ".LC%d:\n", l);
		fprintf(yyout, "\t.asciz \"%s\"\n", s);
	} else {
		fprintf(yyout, "\t.section\t.rodata\n");
		fprintf(yyout, ".LC%d:\n", l);
//		fprintf(yyout, "\t.string \"%s\"\n", s);
		fprintf(yyout, "\t.string\t%s\n", s);
	}
	return l;
}
