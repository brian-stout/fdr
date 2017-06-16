.intel_syntax noprefix

Decimal:
	.asciz "%d\n"

.globl test
.type test, @function

test:

	#Stack alignment
	push rbp

	mov rsi, rdi
	mov rdi, OFFSET Decimal

	call printf

    mov rax, 1

	#Removing all the stack space set aside for a clean close
	pop rbp

    ret	
