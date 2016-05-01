# catc

This is a simple tiny C Compiler using Lex & Yacc.

## Usage

	$ dnf install byacc flex
	$ make
	$ ./catc -S _hello.c
		.section	.rodata
	.LC0:
		.string	"Hello! tiny c world!!\n"
		.text
		.align	4
		.globl	main
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
	$ ./catc -S _hello.c > _hello.asm
	$ as _hello.asm -o _hello.o
	$ gcc _hello.o -o _hello
	$ ./_hello
	Hello! tiny c world!!
	$ ./catc _sample.c
	$ ./_sample
	Hello world.
	100+23=123

## Refrence
- http://www.pwv.co.jp/~take/TakeWiki/index.php?FrontPage
- http://www.syuhitu.org/other/cparse/cparse.html
- http://www.hpcs.cs.tsukuba.ac.jp/~msato/lecture-note/comp-lecture/tiny-c-note1.html
