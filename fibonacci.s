.intel_syntax noprefix
.globl asm_fib_calc
.type asm_fib_calc, @function

asm_fib_calc:

	push rbp

	#Setting r15 to 0
	xor r15, r15

	#Grabbing the number returned from strtol so jumps and cmps don't modify it
	mov r15, rdi

	mov rcx, rdi #Setting Counter
	inc ecx	#Avoids off by one error since the fib sequence starts at 0

	xor r8, r8 #Temporarily hold values for r12
	xor r9, r9 #Temporarily hold values for r13
	xor r10, r10 #Temporarily hold values for r14
	xor r11, r11 #Temporarily hold values for r15
	xor r12, r12 #Will be the over flow for r13
	xor r13, r13 #Will be the over flow for r14
	xor r14, r14 #Will be the over flow for r15
	xor r15, r15 #The initial register 

	inc r15

	#Fibonacci Loop
1:
	#Flip the values to save them
    xchg r11, r15

	#Add the results together to get the next one
    add r11, r15

	xchg r10, r14
	adc r10, r14 #Will add overflow into another register

	#Repeat for additional overflows
	xchg r9, r13
	adc r9, r13

	xchg r8, r12
	adc r8, r12

	#Decrement by one each run through
	sub ecx, 1
	#If counter register 0 end loop
	jnz 1b

	mov eax, 1

	pop rbp
	ret

