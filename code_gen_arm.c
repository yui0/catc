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
 * Code generator for ARM (AArch32)
 */

#define N_REG 4
#define N_SAVE 4

#define TMP_OFF(i)      (-((i+1)+1)*4)
#define LOCAL_VAR_OFF(i)    (-(N_SAVE+1+(i+1))*4)
#define ARG_OFF(i)      ((i)+2)*4

#define REG_R0 0
#define REG_R1 1
#define REG_R2 2
#define REG_R3 3

#define MAX_ARGS 4 /* AAPCS: first 4 args in r0-r3 */

char *tmpRegName[N_REG] = { "r0", "r1", "r2", "r3" };
int tmpRegState[N_REG];
int tmpRegSave[N_SAVE];

/*
 * Register allocation
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
            fprintf(yyout, "\tstr\t%s, [sp, #%d]\n", tmpRegName[reg], TMP_OFF(reg));
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
            fprintf(yyout, "\tldr\t%s, [sp, #%d]\n", tmpRegName[rr], TMP_OFF(i));
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
    fprintf(yyout, "\t.align\t2\n");
    if (isDarwin) {
        fprintf(yyout, "\t.global\t_%s\n", entry_name);
        fprintf(yyout, "_%s:\n", entry_name);
    } else {
        fprintf(yyout, "\t.global\t%s\n", entry_name);
        fprintf(yyout, "\t.type\t%s, %%function\n", entry_name);
        fprintf(yyout, "%s:\n", entry_name);
    }
    fprintf(yyout, "\tpush\t{fp, lr}\n"); // Save frame pointer and link register
    fprintf(yyout, "\tadd\tfp, sp, #4\n"); // Set up frame pointer

    frame_size = -LOCAL_VAR_OFF(n_local);
    frame_size = (frame_size + 7) & (~7); // 8-byte align stack
    fprintf(yyout, "\tsub\tsp, sp, #%d\n", frame_size);
    fprintf(yyout, "\tstr\tr4, [fp, #-8]\n"); // Save callee-saved register r4

    initTmpReg();

    for (i = 0; i < n_code; i++) {
        opd1 = Codes[i].operand1;
        opd2 = Codes[i].operand2;
        opd3 = Codes[i].operand3;
        opds = Codes[i].s_operand;

        switch (Codes[i].opcode) {
        case LOADI: // Load immediate
            if (opd1 < 0) break;
            r = getReg(opd1);
            fprintf(yyout, "\tmov\t%s, #%d\n", tmpRegName[r], opd2);
            break;
        case LOADS: // Load string label
            if (opd1 < 0) break;
            r = getReg(opd1);
            fprintf(yyout, "\tldr\t%s, =.LC%d\n", tmpRegName[r], opd2);
            break;
        case LOADA: // Load argument
            if (opd1 < 0) break;
            r = getReg(opd1);
            fprintf(yyout, "\tldr\t%s, [fp, #%d]\n", tmpRegName[r], ARG_OFF(opd2));
            break;
        case LOADL: // Load local
            if (opd1 < 0) break;
            r = getReg(opd1);
            fprintf(yyout, "\tldr\t%s, [fp, #%d]\n", tmpRegName[r], LOCAL_VAR_OFF(opd2));
            break;
        case STOREA: // Store argument
            r = useReg(opd1);
            freeReg(r);
            fprintf(yyout, "\tstr\t%s, [fp, #%d]\n", tmpRegName[r], ARG_OFF(opd2));
            break;
        case STOREL: // Store local
            r = useReg(opd1);
            freeReg(r);
            fprintf(yyout, "\tstr\t%s, [fp, #%d]\n", tmpRegName[r], LOCAL_VAR_OFF(opd2));
            break;
        case BEQ0: // Conditional branch
            r = useReg(opd1);
            freeReg(r);
            fprintf(yyout, "\tcmp\t%s, #0\n", tmpRegName[r]);
            fprintf(yyout, "\tbeq\t.L%d\n", opd2);
            break;
        case LABEL:
            fprintf(yyout, ".L%d:\n", Codes[i].operand1);
            break;
        case JUMP:
            fprintf(yyout, "\tb\t.L%d\n", Codes[i].operand1);
            break;
        case CALL:
            saveAllRegs();
            if (isDarwin) {
                fprintf(yyout, "\tbl\t_%s\n", opds);
            } else {
                fprintf(yyout, "\tbl\t%s\n", opds);
            }
            if (opd1 < 0) {
                break;
            }
            assignReg(opd1, REG_R0); // Return value in r0
            break;
        case ARG:
            r = useReg(opd1);
            freeReg(r);
            if (opd2 < MAX_ARGS) {
                fprintf(yyout, "\tmov\t%s, %s\n", tmpRegName[opd2], tmpRegName[r]);
            } else {
                fprintf(yyout, "\tstr\t%s, [sp, #%d]\n", tmpRegName[r], (opd2 - MAX_ARGS) * 4);
            }
            break;
        case RET:
            r = useReg(opd1);
            freeReg(r);
            if (r != REG_R0) {
                fprintf(yyout, "\tmov\tr0, %s\n", tmpRegName[r]);
            }
            fprintf(yyout, "\tb\t.L%d\n", ret_lab);
            break;
        case ADD:
            r1 = useReg(opd2);
            r2 = useReg(opd3);
            freeReg(r1);
            freeReg(r2);
            if (opd1 < 0) {
                break;
            }
            assignReg(opd1, r1);
            fprintf(yyout, "\tadd\t%s, %s, %s\n", tmpRegName[r1], tmpRegName[r1], tmpRegName[r2]);
            break;
        case SUB:
            r1 = useReg(opd2);
            r2 = useReg(opd3);
            freeReg(r1);
            freeReg(r2);
            if (opd1 < 0) {
                break;
            }
            assignReg(opd1, r1);
            fprintf(yyout, "\tsub\t%s, %s, %s\n", tmpRegName[r1], tmpRegName[r1], tmpRegName[r2]);
            break;
        case MUL:
            r1 = useReg(opd2);
            r2 = useReg(opd3);
            freeReg(r1);
            freeReg(r2);
            if (opd1 < 0) {
                break;
            }
            assignReg(opd1, REG_R0);
            fprintf(yyout, "\tmul\t%s, %s, %s\n", tmpRegName[REG_R0], tmpRegName[r1], tmpRegName[r2]);
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
            fprintf(yyout, "\tcmp\t%s, %s\n", tmpRegName[r1], tmpRegName[r2]);
            fprintf(yyout, "\tblt\t.L%d\n", l1);
            fprintf(yyout, "\tmov\t%s, #0\n", tmpRegName[r]);
            fprintf(yyout, "\tb\t.L%d\n", l2);
            fprintf(yyout, ".L%d:\n\tmov\t%s, #1\n", l1, tmpRegName[r]);
            fprintf(yyout, ".L%d:\n", l2);
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
            fprintf(yyout, "\tcmp\t%s, %s\n", tmpRegName[r1], tmpRegName[r2]);
            fprintf(yyout, "\tbgt\t.L%d\n", l1);
            fprintf(yyout, "\tmov\t%s, #0\n", tmpRegName[r]);
            fprintf(yyout, "\tb\t.L%d\n", l2);
            fprintf(yyout, ".L%d:\n\tmov\t%s, #1\n", l1, tmpRegName[r]);
            fprintf(yyout, ".L%d:\n", l2);
            break;
        }
    }

    /* Return sequence */
    fprintf(yyout, ".L%d:\n", ret_lab);
    fprintf(yyout, "\tldr\tr4, [fp, #-8]\n"); // Restore r4
    fprintf(yyout, "\tsub\tsp, fp, #4\n"); // Restore stack pointer
    fprintf(yyout, "\tpop\t{fp, pc}\n"); // Restore frame pointer and return
}

int genString(char *s)
{
    int l = label_counter++;
    if (isDarwin) {
        fprintf(yyout, "\t.section\t__TEXT,__cstring,cstring_literals\n");
        fprintf(yyout, ".LC%d:\n", l);
        fprintf(yyout, "\t.asciz\t%s\n", s);
    } else {
        fprintf(yyout, "\t.section\t.rodata\n");
        fprintf(yyout, ".LC%d:\n", l);
        fprintf(yyout, "\t.asciz\t%s\n", s);
    }
    return l;
}

int genStatic(int a)
{
    int l = label_counter++;
    fprintf(yyout, "\t.data\n");
    fprintf(yyout, ".LC%d:\n", l);
    fprintf(yyout, "\t.word\t%d\n", a);
    return l;
}
