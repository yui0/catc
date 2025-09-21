// ©2025 Yuichiro Nakada
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include "reg_code.h"

void error(char *fmt, ...);

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
 * Code generator for x86-64
 */

#define N_REG 6
#define N_SAVE 6

#define TMP_OFF(i)        (-((i+1)+1)*8)
#define LOCAL_VAR_OFF(i)  (-(N_SAVE+1+(i+1))*8)
#define ARG_OFF(i)        ((i)+2)*8

#define REG_AX 0
#define REG_BX 1
#define REG_CX 2
#define REG_DX 3
#define REG_SI 4
#define REG_DI 5

#define MAX_ARGS 6 /* for Darwin and Linux x86-64 */

char *tmpRegName[N_REG] = { "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi" };
int tmpRegState[N_REG];
int tmpRegSave[N_SAVE];

/*
 * Sample register allocation
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
    for (int i = 0; i < N_REG; i++) {
        if (tmpRegState[i] < 0) {
            tmpRegState[i] = r;
            return i;
        }
    }
    error("no temp reg\n");
    return 0;
}

void saveReg(int reg)
{
    if (tmpRegState[reg] < 0) return;

    for (int i = 0; i < N_SAVE; i++) {
        if (tmpRegSave[i] < 0) {
            fprintf(yyout, "\tmovq\t%s,%d(%%rbp)\n", tmpRegName[reg], TMP_OFF(i));
            tmpRegSave[i] = tmpRegState[reg];
            tmpRegState[reg] = -1;
            return;
        }
    }
    error("no temp save\n");
}

void saveAllRegs()
{
    for (int i = 0; i < N_REG; i++) {
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
            /* load into register */
            fprintf(yyout, "\tmovq\t%d(%%rbp),%s\n", TMP_OFF(i), tmpRegName[rr]);
            return rr;
        }
    }
    error("reg is not found\n");
    return 0;
}

void freeReg(int reg)
{
    tmpRegState[reg] = -1;
}

// Code generation
extern int label_counter;
void genFuncCode(char *entry_name, int n_local)
{
    int i;
    int opd1, opd2, opd3;
    int r, r1, r2;
    char *opds;
    int ret_lab, l1, l2;
    int frame_size;

    // Function header
    fprintf(yyout, "\t.text\n");
    fprintf(yyout, "\t.align\t16\n");
    if (isDarwin) {
        fprintf(yyout, "\t.globl\t_%s\n", entry_name);
        fprintf(yyout, "_%s:\n", entry_name);
    } else {
        fprintf(yyout, "\t.globl\t%s\n", entry_name);
        fprintf(yyout, "\t.type\t%s,@function\n", entry_name);
        fprintf(yyout, "%s:\n", entry_name);
    }
    fprintf(yyout, "\tpushq\t%%rbp\n");
    fprintf(yyout, "\tmovq\t%%rsp,%%rbp\n");

    frame_size = -LOCAL_VAR_OFF(n_local);
    ret_lab = label_counter++;

    // Align stack to 16 bytes
    frame_size = ((frame_size + 15) & (~15));
    if (frame_size > 0) {
        fprintf(yyout, "\tsubq\t$%d,%%rsp\n", frame_size);
    }
    fprintf(yyout, "\tmovq\t%%rbx,-8(%%rbp)\n");

    initTmpReg();

    // x86-64 argument registers
    char *argRegName[MAX_ARGS] = { "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9" };

    for (i = 0; i < n_code; i++) {
        opd1 = Codes[i].operand1;
        opd2 = Codes[i].operand2;
        opd3 = Codes[i].operand3;
        opds = Codes[i].s_operand;

        switch (Codes[i].opcode) {
        case LOADI: // Load int
            if (opd1 < 0) break;
            r = getReg(opd1);
            fprintf(yyout, "\tmovq\t$%d,%s\n", opd2, tmpRegName[r]);
            break;
        case LOADS: // Load string label
            if (opd1 < 0) break;
            r = getReg(opd1);
            fprintf(yyout, "\tleaq\t.LC%d(%%rip),%s\n", opd2, tmpRegName[r]);
            break;
        case LOADA: // Load arg
            if (opd1 < 0) break;
            r = getReg(opd1);
            if (opd2 < MAX_ARGS) {
                fprintf(yyout, "\tmovq\t%s,%s\n", argRegName[opd2], tmpRegName[r]);
            } else {
                fprintf(yyout, "\tmovq\t%d(%%rbp),%s\n", ARG_OFF(opd2 - MAX_ARGS), tmpRegName[r]);
            }
            break;
        case LOADL: // Load local
            if (opd1 < 0) break;
            r = getReg(opd1);
            fprintf(yyout, "\tmovq\t%d(%%rbp),%s\n", LOCAL_VAR_OFF(opd2), tmpRegName[r]);
            break;
        case STOREA: // Store arg
            r = useReg(opd1);
            freeReg(r);
            if (opd2 < MAX_ARGS) {
                fprintf(yyout, "\tmovq\t%s,%s\n", tmpRegName[r], argRegName[opd2]);
            } else {
                fprintf(yyout, "\tmovq\t%s,%d(%%rbp)\n", tmpRegName[r], ARG_OFF(opd2 - MAX_ARGS));
            }
            break;
        case STOREL: // Store local
            r = useReg(opd1);
            freeReg(r);
            fprintf(yyout, "\tmovq\t%s,%d(%%rbp)\n", tmpRegName[r], LOCAL_VAR_OFF(opd2));
            break;
        case BEQ0: // Conditional branch
            r = useReg(opd1);
            freeReg(r);
            fprintf(yyout, "\tcmpq\t$0,%s\n", tmpRegName[r]);
            fprintf(yyout, "\tje\t.L%d\n", opd2);
            break;
        case LABEL:
            fprintf(yyout, ".L%d:\n", opd1);
            break;
        case JUMP:
            fprintf(yyout, "\tjmp\t.L%d\n", opd1);
            break;
        case CALL:
            saveAllRegs();
            if (isDarwin) {
                fprintf(yyout, "\tcall\t_%s\n", opds);
            } else {
                fprintf(yyout, "\tcall\t%s\n", opds);
            }
            if (opd1 >= 0) {
                assignReg(opd1, REG_AX);
            }
            break;
        case ARG:
            r = useReg(opd1);
            freeReg(r);
            if (opd2 < MAX_ARGS) {
                fprintf(yyout, "\tmovq\t%s,%s\n", tmpRegName[r], argRegName[opd2]);
            } else {
                fprintf(yyout, "\tpushq\t%s\n", tmpRegName[r]);
            }
            break;
        case RET:
            r = useReg(opd1);
            freeReg(r);
            if (r != REG_AX) {
                fprintf(yyout, "\tmovq\t%s,%%rax\n", tmpRegName[r]);
            }
            fprintf(yyout, "\tjmp\t.L%d\n", ret_lab);
            break;
        case ADD:
            r1 = useReg(opd2);
            r2 = useReg(opd3);
            freeReg(r1);
            freeReg(r2);
            if (opd1 < 0) break;
            assignReg(opd1, r1);
            fprintf(yyout, "\taddq\t%s,%s\n", tmpRegName[r2], tmpRegName[r1]);
            break;
        case SUB:
            r1 = useReg(opd2);
            r2 = useReg(opd3);
            freeReg(r1);
            freeReg(r2);
            if (opd1 < 0) break;
            assignReg(opd1, r1);
            fprintf(yyout, "\tsubq\t%s,%s\n", tmpRegName[r2], tmpRegName[r1]);
            break;
        case MUL:
            r1 = useReg(opd2);
            r2 = useReg(opd3);
            freeReg(r1);
            freeReg(r2);
            if (opd1 < 0) break;
            assignReg(opd1, REG_AX);
            saveReg(REG_DX);
            if (r1 != REG_AX) {
                fprintf(yyout, "\tmovq\t%s,%s\n", tmpRegName[r1], tmpRegName[REG_AX]);
            }
            fprintf(yyout, "\timulq\t%s,%s\n", tmpRegName[r2], tmpRegName[REG_AX]);
            break;
        case LT:
            r1 = useReg(opd2);
            r2 = useReg(opd3);
            freeReg(r1);
            freeReg(r2);
            if (opd1 < 0) break;
            r = getReg(opd1);
            l1 = label_counter++;
            l2 = label_counter++;
            fprintf(yyout, "\tcmpq\t%s,%s\n", tmpRegName[r2], tmpRegName[r1]);
            fprintf(yyout, "\tjl\t.L%d\n", l1);
            fprintf(yyout, "\tmovq\t$0,%s\n", tmpRegName[r]);
            fprintf(yyout, "\tjmp\t.L%d\n", l2);
            fprintf(yyout, ".L%d:\tmovq\t$1,%s\n", l1, tmpRegName[r]);
            fprintf(yyout, ".L%d:\n", l2);
            break;
        case GT:
            r1 = useReg(opd2);
            r2 = useReg(opd3);
            freeReg(r1);
            freeReg(r2);
            if (opd1 < 0) break;
            r = getReg(opd1);
            l1 = label_counter++;
            l2 = label_counter++;
            fprintf(yyout, "\tcmpq\t%s,%s\n", tmpRegName[r2], tmpRegName[r1]);
            fprintf(yyout, "\tjg\t.L%d\n", l1);
            fprintf(yyout, "\tmovq\t$0,%s\n", tmpRegName[r]);
            fprintf(yyout, "\tjmp\t.L%d\n", l2);
            fprintf(yyout, ".L%d:\tmovq\t$1,%s\n", l1, tmpRegName[r]);
            fprintf(yyout, ".L%d:\n", l2);
            break;
        }
    }

    /* Return sequence */
    fprintf(yyout, ".L%d:\tmovq\t-8(%%rbp),%%rbx\n", ret_lab);
    fprintf(yyout, "\tleave\n");
    fprintf(yyout, "\tret\n\n");
}

int genString(char *s)
{
	int l = label_counter++;
	if (isDarwin) {
		fprintf(yyout, "\t.cstring\n");
		fprintf(yyout, ".LC%d:\n", l);
		fprintf(yyout, "\t.asciz\t\"%s\"\n", s);
	} else {
		fprintf(yyout, "\t.section\t.rodata\n");
		fprintf(yyout, ".LC%d:", l);
		fprintf(yyout, "\t.string\t%s\n", s);
	}
	return l;
}
/*int genString(char *s)
{
    int l = label_counter++;
    if (isDarwin) {
        fprintf(yyout, "\t.section\t__TEXT,__cstring,cstring_literals\n");
        fprintf(yyout, ".LC%d:\n", l);
        fprintf(yyout, "\t.asciz\t\"%s\"\n", s);
    } else {
        fprintf(yyout, "\t.section\t.rodata\n");
        fprintf(yyout, ".LC%d:\n", l);
        fprintf(yyout, "\t.string\t\"%s\"\n", s);
    }
    return l;
}*/

int genStatic(int a)
{
    int l = label_counter++;
    fprintf(yyout, "\t.data\n");
    fprintf(yyout, ".LC%d:\n", l);
    fprintf(yyout, "\t.quad\t%d\n", a);
    return l;
}
