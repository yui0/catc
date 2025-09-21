# ©2016,2025 YUICHIRO NAKADA

# Compilers
CC_X86 = clang
#CC_ARM = arm-none-eabi-gcc
CC_ARM = clang
LEX = flex
YACC = yacc -dv
#YACC = bison -d -v -o y.tab.c

# Common flags
CFLAGS = -Wall -Os

# Architecture-specific settings
#CFLAGS_X86_32 = $(CFLAGS) -m32
#CFLAGS_X86_64 = $(CFLAGS) -m64
#CFLAGS_ARM = $(CFLAGS) -march=armv7-a
CFLAGS_X86_32 = $(CFLAGS)
CFLAGS_X86_64 = $(CFLAGS)
CFLAGS_ARM = $(CFLAGS)
CFLAGS_WASM = $(CFLAGS)

# Object files
COMMON_OBJS = y.tab.o AST.o reg_compile.o
OBJS_X86_32 = $(COMMON_OBJS) code_gen_x86.o
OBJS_X86_64 = $(COMMON_OBJS) code_gen_x86_64.o
OBJS_ARM = $(COMMON_OBJS) code_gen_arm.o
OBJS_WASM = $(COMMON_OBJS) code_gen_wasm.o

# Executable names
PROGRAM_X86_32 = catc32
PROGRAM_X86_64 = catc
PROGRAM_ARM = catc_arm
PROGRAM_WASM = catc_wasm

# Default target: build all executables
all: $(PROGRAM_X86_32) $(PROGRAM_X86_64) $(PROGRAM_ARM) $(PROGRAM_WASM)

# Rules for each executable
$(PROGRAM_X86_32): $(OBJS_X86_32)
	$(CC_X86) $(CFLAGS_X86_32) -o $@ $^

$(PROGRAM_X86_64): $(OBJS_X86_64)
	$(CC_X86) $(CFLAGS_X86_64) -o $@ $^

$(PROGRAM_ARM): $(OBJS_ARM)
	$(CC_ARM) $(CFLAGS_ARM) -o $@ $^

$(PROGRAM_WASM): $(OBJS_WASM)
	$(CC_X86) $(CFLAGS_WASM) -o $@ $^

# Compile source files to object files
.c.o:
	$(CC_X86) $(CFLAGS) -c $< -o $@

# ARM-specific object compilation
arm_code_gen.o: arm_code_gen.c
	$(CC_ARM) $(CFLAGS_ARM) -c $< -o $@

# Lexer and parser
lex.yy.c: clex.l
	$(LEX) clex.l

y.tab.c: cparser.y
	$(YACC) cparser.y

y.tab.o: cparser.y lex.yy.c

# Clean rule
.PHONY: clean
clean:
	$(RM) $(PROGRAM_X86_32) $(PROGRAM_X86_64) $(PROGRAM_ARM) $(PROGRAM_WASM) $(COMMON_OBJS) $(OBJS_X86_32) $(OBJS_X86_64) $(OBJS_ARM) $(OBJS_WASM) lex.yy.c y.tab.[ch] y.output *.o *.s
