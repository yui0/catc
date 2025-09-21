// ©2025 Yuichiro Nakada
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdbool.h>
#include "reg_code.h"

void error(char *fmt, ...);

#define MAX_CODE 100
#define MAX_DATA_ENTRIES 100 // Define maximum number of data entries

extern FILE *yyout;

struct _code {
    int opcode;
    int operand1, operand2, operand3;
    char *s_operand;
} Codes[MAX_CODE];

struct DataEntry {
    int l;
    char *s;
    int type; // 0: string, 1: quad
    int value; // for quad
};

struct DataEntry data_entries[MAX_DATA_ENTRIES];
int data_entry_count = 0; // Track number of data entries

int offset_map[10000];

extern int label_counter;

int n_code;
void initGenCode()
{
    n_code = 0;
    data_entry_count = 0; // Initialize data entry count
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
 * Code generator for WASM
 */

int genString(char *s)
{
    if (data_entry_count >= MAX_DATA_ENTRIES) {
        error("Data entry limit exceeded");
        return -1;
    }
    int l = label_counter++;
    struct DataEntry *e = &data_entries[data_entry_count++];
    e->l = l;
    e->s = strdup(s);
    e->type = 0;
    return l;
}

int genStatic(int a)
{
    if (data_entry_count >= MAX_DATA_ENTRIES) {
        error("Data entry limit exceeded");
        return -1;
    }
    int l = label_counter++;
    struct DataEntry *e = &data_entries[data_entry_count++];
    e->l = l;
    e->type = 1;
    e->value = a;
    return l;
}

void genWasmHeader()
{
    fprintf(yyout, "(module\n");
    fprintf(yyout, "  (import \"env\" \"puts\" (func $puts (param i32) (result i32)))\n");
    fprintf(yyout, "  (memory 1)\n");
    fprintf(yyout, "  (export \"memory\" (memory 0))\n");
}

void genWasmData()
{
    int offset = 0;
    for (int i = 0; i < data_entry_count; i++) {
        struct DataEntry *e = &data_entries[i];
        offset_map[e->l] = offset;
        fprintf(yyout, "  (data (i32.const %d) \"", offset);
        if (e->type == 0) {
            for (char *p = e->s; *p; p++) {
                if (*p == '\"' || *p == '\\') {
                    fprintf(yyout, "\\%c", *p);
                } else if (isprint(*p)) {
                    fprintf(yyout, "%c", *p);
                } else {
                    fprintf(yyout, "\\%02x", (unsigned char)*p);
                }
            }
            fprintf(yyout, "\\00\")\n");
            offset += strlen(e->s) + 1;
        } else {
            long long v = e->value;
            for (int j = 0; j < 8; j++) {
                fprintf(yyout, "\\%02x", (int)(v & 0xff));
                v >>= 8;
            }
            fprintf(yyout, "\")\n");
            offset += 8;
        }
    }
}

void genWasmFooter()
{
    fprintf(yyout, ")\n");
}

void genFuncCode(char *entry_name, int n_local)
{
    int n_param = 0;
    int max_temp = -1;
    int min_label = INT_MAX;
    int max_label = -1;
    for (int i = 0; i < n_code; i++) {
        int op1 = Codes[i].operand1;
        int op2 = Codes[i].operand2;
        int op3 = Codes[i].operand3;
        if (op1 > max_temp) max_temp = op1;
        if (op2 > max_temp) max_temp = op2;
        if (op3 > max_temp) max_temp = op3;
        if (Codes[i].opcode == LOADA || Codes[i].opcode == STOREA) {
            if (op2 > n_param) n_param = op2;
        }
        if (Codes[i].opcode == LABEL || Codes[i].opcode == JUMP) {
            if (op1 < min_label) min_label = op1;
            if (op1 > max_label) max_label = op1;
        }
        if (Codes[i].opcode == BEQ0) {
            if (op2 < min_label) min_label = op2;
            if (op2 > max_label) max_label = op2;
        }
    }
    n_param += 1;
    max_temp += 1; // 0 to max_temp inclusive

    int temp_base = n_param + n_local;
    int entry_pc = min_label - 1;

    fprintf(yyout, "(func $%s ", entry_name);
    fprintf(yyout, "(export \"%s\") ", entry_name);
    for (int i = 0; i < n_param; i++) {
        fprintf(yyout, "(param i64) ");
    }
    fprintf(yyout, "(result i64)\n");
    for (int i = 0; i < n_local; i++) {
        fprintf(yyout, "  (local i64)\n");
    }
    for (int i = 0; i < max_temp; i++) {
        fprintf(yyout, "  (local i64)\n");
    }
    fprintf(yyout, "  (local $pc i32)\n");

    // Set initial pc
    int initial_pc = (Codes[0].opcode == LABEL) ? Codes[0].operand1 : entry_pc;
    fprintf(yyout, "  i32.const %d\n", initial_pc);
    fprintf(yyout, "  local.set $pc\n");
    fprintf(yyout, "  loop $big_loop\n");

    int i = 0;
    bool has_entry_block = (Codes[0].opcode != LABEL);
    if (has_entry_block) {
        // Entry block
        fprintf(yyout, "    local.get $pc\n    i32.const %d\n    i32.eq\n    if\n", entry_pc);
        bool ended_with_unconditional = false;
        while (i < n_code && Codes[i].opcode != LABEL) {
            int opcode = Codes[i].opcode;
            int op1 = Codes[i].operand1;
            int op2 = Codes[i].operand2;
            int op3 = Codes[i].operand3;
            char *opds = Codes[i].s_operand;
            switch (opcode) {
            case LOADI:
                if (op1 < 0) break;
                fprintf(yyout, "      i64.const %d\n", op2);
                fprintf(yyout, "      local.set %d\n", temp_base + op1);
                break;
            case LOADS:
                if (op1 < 0) break;
                fprintf(yyout, "      i32.const %d\n", offset_map[op2]);
                fprintf(yyout, "      i64.extend_i32_u\n");
                fprintf(yyout, "      local.set %d\n", temp_base + op1);
                break;
            case LOADA:
                if (op1 < 0) break;
                fprintf(yyout, "      local.get %d\n", op2);
                fprintf(yyout, "      local.set %d\n", temp_base + op1);
                break;
            case LOADL:
                if (op1 < 0) break;
                fprintf(yyout, "      local.get %d\n", n_param + op2);
                fprintf(yyout, "      local.set %d\n", temp_base + op1);
                break;
            case STOREA:
                fprintf(yyout, "      local.get %d\n", temp_base + op1);
                fprintf(yyout, "      local.set %d\n", op2);
                break;
            case STOREL:
                fprintf(yyout, "      local.get %d\n", temp_base + op1);
                fprintf(yyout, "      local.set %d\n", n_param + op2);
                break;
            case BEQ0:
                fprintf(yyout, "      local.get %d\n", temp_base + op1);
                fprintf(yyout, "      i64.eqz\n");
                fprintf(yyout, "      if\n");
                fprintf(yyout, "        i32.const %d\n", op2);
                fprintf(yyout, "        local.set $pc\n");
                fprintf(yyout, "        br $big_loop\n");
                fprintf(yyout, "      end\n");
                break;
            case JUMP:
                fprintf(yyout, "      i32.const %d\n", op1);
                fprintf(yyout, "      local.set $pc\n");
                fprintf(yyout, "      br $big_loop\n");
                ended_with_unconditional = true;
                break;
            case CALL:
                if (strcmp(opds, "puts") == 0) {
                    fprintf(yyout, "      local.get %d\n", temp_base + op1);
                    fprintf(yyout, "      i32.wrap_i64\n");
                    fprintf(yyout, "      call $puts\n");
                    fprintf(yyout, "      drop\n");
                } else {
                    fprintf(yyout, "      call $%s\n", opds);
                    if (op1 >= 0) {
                        fprintf(yyout, "      local.set %d\n", temp_base + op1);
                    }
                }
                break;
            case ARG:
                fprintf(yyout, "      local.get %d\n", temp_base + op1);
                break;
            case RET:
                fprintf(yyout, "      local.get %d\n", temp_base + op1);
                fprintf(yyout, "      return\n");
                ended_with_unconditional = true;
                break;
            case ADD:
                fprintf(yyout, "      local.get %d\n", temp_base + op2);
                fprintf(yyout, "      local.get %d\n", temp_base + op3);
                fprintf(yyout, "      i64.add\n");
                if (op1 >= 0) {
                    fprintf(yyout, "      local.set %d\n", temp_base + op1);
                }
                break;
            case SUB:
                fprintf(yyout, "      local.get %d\n", temp_base + op2);
                fprintf(yyout, "      local.get %d\n", temp_base + op3);
                fprintf(yyout, "      i64.sub\n");
                if (op1 >= 0) {
                    fprintf(yyout, "      local.set %d\n", temp_base + op1);
                }
                break;
            case MUL:
                fprintf(yyout, "      local.get %d\n", temp_base + op2);
                fprintf(yyout, "      local.get %d\n", temp_base + op3);
                fprintf(yyout, "      i64.mul\n");
                if (op1 >= 0) {
                    fprintf(yyout, "      local.set %d\n", temp_base + op1);
                }
                break;
            case LT:
                fprintf(yyout, "      local.get %d\n", temp_base + op2);
                fprintf(yyout, "      local.get %d\n", temp_base + op3);
                fprintf(yyout, "      i64.lt_s\n");
                if (op1 >= 0) {
                    fprintf(yyout, "      local.set %d\n", temp_base + op1);
                }
                break;
            case GT:
                fprintf(yyout, "      local.get %d\n", temp_base + op2);
                fprintf(yyout, "      local.get %d\n", temp_base + op3);
                fprintf(yyout, "      i64.gt_s\n");
                if (op1 >= 0) {
                    fprintf(yyout, "      local.set %d\n", temp_base + op1);
                }
                break;
            }
            i++;
        }
        if (!ended_with_unconditional) {
            if (i < n_code && Codes[i].opcode == LABEL) {
                fprintf(yyout, "      i32.const %d\n", Codes[i].operand1);
                fprintf(yyout, "      local.set $pc\n");
                fprintf(yyout, "      br $big_loop\n");
            } else {
                fprintf(yyout, "      unreachable\n");
            }
        }
        fprintf(yyout, "    end\n");
    }

    // Process labeled blocks
    while (i < n_code) {
        if (Codes[i].opcode != LABEL) {
            error("Expected label");
        }
        int block_label = Codes[i].operand1;
        i++;
        fprintf(yyout, "    local.get $pc\n    i32.const %d\n    i32.eq\n    if\n", block_label);
        bool ended_with_unconditional = false;
        while (i < n_code && Codes[i].opcode != LABEL) {
            int opcode = Codes[i].opcode;
            int op1 = Codes[i].operand1;
            int op2 = Codes[i].operand2;
            int op3 = Codes[i].operand3;
            char *opds = Codes[i].s_operand;
            switch (opcode) {
            case LOADI:
                if (op1 < 0) break;
                fprintf(yyout, "      i64.const %d\n", op2);
                fprintf(yyout, "      local.set %d\n", temp_base + op1);
                break;
            case LOADS:
                if (op1 < 0) break;
                fprintf(yyout, "      i32.const %d\n", offset_map[op2]);
                fprintf(yyout, "      i64.extend_i32_u\n");
                fprintf(yyout, "      local.set %d\n", temp_base + op1);
                break;
            case LOADA:
                if (op1 < 0) break;
                fprintf(yyout, "      local.get %d\n", op2);
                fprintf(yyout, "      local.set %d\n", temp_base + op1);
                break;
            case LOADL:
                if (op1 < 0) break;
                fprintf(yyout, "      local.get %d\n", n_param + op2);
                fprintf(yyout, "      local.set %d\n", temp_base + op1);
                break;
            case STOREA:
                fprintf(yyout, "      local.get %d\n", temp_base + op1);
                fprintf(yyout, "      local.set %d\n", op2);
                break;
            case STOREL:
                fprintf(yyout, "      local.get %d\n", temp_base + op1);
                fprintf(yyout, "      local.set %d\n", n_param + op2);
                break;
            case BEQ0:
                fprintf(yyout, "      local.get %d\n", temp_base + op1);
                fprintf(yyout, "      i64.eqz\n");
                fprintf(yyout, "      if\n");
                fprintf(yyout, "        i32.const %d\n", op2);
                fprintf(yyout, "        local.set $pc\n");
                fprintf(yyout, "        br $big_loop\n");
                fprintf(yyout, "      end\n");
                break;
            case JUMP:
                fprintf(yyout, "      i32.const %d\n", op1);
                fprintf(yyout, "      local.set $pc\n");
                fprintf(yyout, "      br $big_loop\n");
                ended_with_unconditional = true;
                break;
            case CALL:
                if (strcmp(opds, "puts") == 0) {
                    fprintf(yyout, "      local.get %d\n", temp_base + op1);
                    fprintf(yyout, "      i32.wrap_i64\n");
                    fprintf(yyout, "      call $puts\n");
                    fprintf(yyout, "      drop\n");
                } else {
                    fprintf(yyout, "      call $%s\n", opds);
                    if (op1 >= 0) {
                        fprintf(yyout, "      local.set %d\n", temp_base + op1);
                    }
                }
                break;
            case ARG:
                fprintf(yyout, "      local.get %d\n", temp_base + op1);
                break;
            case RET:
                fprintf(yyout, "      local.get %d\n", temp_base + op1);
                fprintf(yyout, "      return\n");
                ended_with_unconditional = true;
                break;
            case ADD:
                fprintf(yyout, "      local.get %d\n", temp_base + op2);
                fprintf(yyout, "      local.get %d\n", temp_base + op3);
                fprintf(yyout, "      i64.add\n");
                if (op1 >= 0) {
                    fprintf(yyout, "      local.set %d\n", temp_base + op1);
                }
                break;
            case SUB:
                fprintf(yyout, "      local.get %d\n", temp_base + op2);
                fprintf(yyout, "      local.get %d\n", temp_base + op3);
                fprintf(yyout, "      i64.sub\n");
                if (op1 >= 0) {
                    fprintf(yyout, "      local.set %d\n", temp_base + op1);
                }
                break;
            case MUL:
                fprintf(yyout, "      local.get %d\n", temp_base + op2);
                fprintf(yyout, "      local.get %d\n", temp_base + op3);
                fprintf(yyout, "      i64.mul\n");
                if (op1 >= 0) {
                    fprintf(yyout, "      local.set %d\n", temp_base + op1);
                }
                break;
            case LT:
                fprintf(yyout, "      local.get %d\n", temp_base + op2);
                fprintf(yyout, "      local.get %d\n", temp_base + op3);
                fprintf(yyout, "      i64.lt_s\n");
                if (op1 >= 0) {
                    fprintf(yyout, "      local.set %d\n", temp_base + op1);
                }
                break;
            case GT:
                fprintf(yyout, "      local.get %d\n", temp_base + op2);
                fprintf(yyout, "      local.get %d\n", temp_base + op3);
                fprintf(yyout, "      i64.gt_s\n");
                if (op1 >= 0) {
                    fprintf(yyout, "      local.set %d\n", temp_base + op1);
                }
                break;
            }
            i++;
        }
        if (!ended_with_unconditional) {
            if (i < n_code && Codes[i].opcode == LABEL) {
                fprintf(yyout, "      i32.const %d\n", Codes[i].operand1);
                fprintf(yyout, "      local.set $pc\n");
                fprintf(yyout, "      br $big_loop\n");
            } else {
                fprintf(yyout, "      unreachable\n");
            }
        }
        fprintf(yyout, "    end\n");
    }

    fprintf(yyout, "    br $big_loop\n");
    fprintf(yyout, "  end\n");
    fprintf(yyout, "  i64.const 0\n");
    fprintf(yyout, "  return\n");
    fprintf(yyout, ")\n");
}
