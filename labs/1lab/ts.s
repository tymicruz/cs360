#--------------------- s.s file --------------------------
#NAME: TYLER CRUZ
			.global   main, mymain, myprintf, get_ebp, get_esp
main:
	pushl	%ebp
	movl	%esp, %ebp

	pushl one
	pushl $tyler
	call myprintf
	addl $8, %esp
# (1). Write ASSEMBLY code to call myprintf(FMT)
#      HELP: How does mysum() call printf() in the class notes.
	pushl $FMT
	call myprintf
	addl $4, %esp
    
# (2). Write ASSEMBLY code to call mymain(argc, argv, env)
#      HELP: When crt0.o calls main(int argc, char *argv[], char *env[]), 
#            it passes argc, argv, env to main(). 
#            Draw a diagram to see where are argc, argv, env?

	pushl 16(%ebp) #push env on stack
	pushl 12(%ebp) #push argv on stack
	pushl 8(%ebp) #push argc on stack
	call mymain
	addl $12, %esp #clean up stack
       
# (3). Write code to call myprintf(fmt,a,b)	
#      HELP: same as in (1) above
	pushl b
	pushl a
	pushl $fmt
	call myprintf
	addl $12, %esp

# (4). Return to caller
	movl  %ebp, %esp
	popl %ebp
	ret

#get stack pointer
get_esp:
	movl %esp, %eax
	ret

#get stack frame pointer
get_ebp:
	movl %ebp, %eax
	ret
#---------- DATA section of assembly code ---------------
	.data
FMT:	.asciz "main() in assembly call mymain() in C\n"
a:	.long 1234
b:	.long 5678
fmt:	.asciz "a=%d b=%d\n"
one: .long 1
tyler: .asciz "My name is Tyler Cruz and I'm #%d.\n"
#---------  end of s.s file ----------------------------
