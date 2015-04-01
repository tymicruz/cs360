#---------- ts.s file ----------------------
        .global get_ebp, get_esp
get_ebp:
         movl %ebp, %eax
         ret
get_esp:
	movl %esp, % eax
	ret
