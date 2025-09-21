# catc 😺 - Tiny C Compiler with Lex & Yacc

🚀 **catc** is a lightweight C compiler built using Lex and Yacc!
Perfect for learning and experimenting with the basics of C compilation. ✨

## 📖 Overview

This project is a small C compiler that transforms C code into assembly using Lex and Yacc.
It’s simple, supports basic C syntax, and is ideal for those curious about how compilers work! 👨‍💻
👉 Great for learning or testing a minimal C compiler.

## 🛠️ Getting Started

Get up and running in just a few steps! 🎉

### 1. Install Dependencies
```bash
$ dnf install byacc flex
```

### 2. Build the Compiler
```bash
$ make 🔨
```

### 3. Compile and Run
- Convert C code to assembly:
```bash
$ ./catc -S ./test/hello.c
	.section	.rodata
.LC0:	.string	"Hello! tiny C world!!\n"
	.text
	.align	16
	.globl	main
	.type	main,@function
main:
	pushq	%rbp
	movq	%rsp,%rbp
	subq	$64,%rsp
	movq	%rbx,-8(%rbp)
	leaq	.LC0(%rip),%rax
	movq	%rax,%rdi
	call	printf
	movq	$0,%rax
	jmp	.L1
.L1:	movq	-8(%rbp),%rbx
	leave
	ret
```

- Assemble the output:
```bash
$ ./catc -S ./test/hello.c > ./test/hello.asm
$ as ./test/hello.asm -o ./test/hello.o
```

```bash
$ ./catc32 -S ./test/hello.c
	.section	.rodata
.LC0:	.string	"Hello! tiny C world!!\n"
	.text
	.align	4
	.global	main
	.type	main,@function
main:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$24,%esp
	movl	%ebx,-4(%ebp)
	lea	.LC0,%eax
	pushl	%eax
	call	printf
	movl	$0,%eax
	jmp	.L1
.L1:	movl	-4(%ebp),%ebx
	leave
	ret

$ ./catc32 -S ./test/hello.c > ./test/hello.asm
$ as --32 ./test/hello.asm -o ./test/hello.o
```

```bash
$ ./catc_arm -S ./test/hello.c
	.section	.rodata
.LC0:
	.asciz	"Hello! tiny C world!!\n"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
	push	{fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #24
	str	r4, [fp, #-8]
	ldr	r0, =.LC0
	mov	r0, r0
	bl	printf
	mov	r0, #0
	b	.L0
.L0:
	ldr	r4, [fp, #-8]
	sub	sp, fp, #4
	pop	{fp, pc}
```

```bash
$ ./catc_wasm -S ./test/hello.c
(func $main (export "main") (param i64) (result i64)
  (local i64)
  (local i64)
  (local $pc i32)
  i32.const 2147483646
  local.set $pc
  loop $big_loop
    local.get $pc
    i32.const 2147483646
    i32.eq
    if
      i32.const 0
      i64.extend_i32_u
      local.set 1
      local.get 1
      call $printf
      i64.const 0
      local.set 2
      local.get 2
      return
    end
    br $big_loop
  end
  i64.const 0
  return
)
```

- Link and run the program:
```bash
$ gcc ./test/hello.o -o ./test/hello
$ ./test/hello
Hello! tiny c world!!
```

- Or run directly:
```bash
$ ./catc ./test/test01.c
$ ./test/test01
Hello world. 100+23=123
```

## 📚 Project Structure
- `AST.c`, `AST.h`: Abstract Syntax Tree definitions and handling 🌳
- `clex.l`, `cparser.y`: Lex and Yacc source files 📝
- `x86_code_gen.c`: x86 assembly code generation ⚡️
- `test/`: Sample C code files 📂
- `Makefile`: Build script 🔧

## 🔗 Resources
- [TakeWiki](http://www.pwv.co.jp/~take/TakeWiki/index.php?FrontPage) 📖
- [C Parser](http://www.syuhitu.org/other/cparse/cparse.html) 📚
- [Tiny C Note](http://www.hpcs.cs.tsukuba.ac.jp/~msato/lecture-note/comp-lecture/tiny-c-note1.html) 📘

## 🌟 Project Stats
- ⭐ **90 Stars**
- 👀 **5 Watching**
- 🍴 **20 Forks**

## 🤝 Contributing
Contributions are welcome! 🐾
Feel free to report bugs or suggest features via [GitHub Issues](https://github.com/yui0/catc/issues) or submit a Pull Request.

## 📜 License
This project is open source. Check the repository for license details. 📄

😺 Dive into the world of compilers with **catc**!
Happy Coding! 💻
