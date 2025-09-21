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

