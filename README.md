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
.LC0:
	.string	"Hello! tiny c world!!\n"
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
```

- Assemble the output:
```bash
$ ./catc -S ./test/hello.c > ./test/hello.asm
$ as ./test/hello.asm -o ./test/hello.o
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
