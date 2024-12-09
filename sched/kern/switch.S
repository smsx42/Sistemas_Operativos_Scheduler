/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

.globl context_switch;
/*
context_switch:
	pop %eax 						 # eax = *esp.   esp = esp + 4
	mov (%esp), %esp 			# esp = *esp

	popal 								 # restore edi, esi, ebp, ebx, edx, ecx, eax
	
	add $4, %esp 					# esp = esp + 4
	pop %es 						  # es = *esp.   esp = esp + 4
	add $4, %esp 					# esp = esp + 4
	pop %ds 						  # ds = *esp.   esp = esp + 4

	iret
*/
context_switch:
	add $4, %esp				   # esp = esp + 4
	mov (%esp), %esp		   # esp = *esp

    popal								# restore edi, esi, ebp, ebx, edx, ecx, eax
    pop %es							 # es = *(esp).   esp = esp + 4
    pop %ds							 # ds = *(esp).   esp = esp + 4

    add $8, %esp				  # esp = esp + 8

    iret

spin:
	jmp spin
