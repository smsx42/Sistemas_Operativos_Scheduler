#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>


#define MAX_PRIORITY 10
#define MIN_PRIORITY 1
#define DEFAULT_PRIORITY 5


struct Env *next_curenv;
size_t curenv_index = 0;

int cant_syscalls = 0;
int corridas_totales[NENV];

void sched_halt(void);

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	cant_syscalls++;
#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here - Round robin


	// agarro el índice actual pasa saber cual es el siguiente a correr
	// ver env.h la explicación
	if (curenv != NULL) {
		curenv_index = ENVX(curenv->env_id);
	}


	int offset;
	// reccorer la lista 1 vez
	for (int i = 0; i < NENV; i++) {
		offset = i + curenv_index;
		// vuelvo al comienzo
		if (offset >= NENV) {
			offset = 0;
		}
		next_curenv = &(envs[offset]);
		// corro el primero que este RUNNABLE
		if (next_curenv->env_status == ENV_RUNNABLE) {
			corridas_totales[offset]++;
			env_run(next_curenv);
		}
	}

	// no hay otros env RUNNABLE, correr otra vez el actual
	if (curenv && curenv->env_status == ENV_RUNNING) {
		corridas_totales[curenv_index]++;
		env_run(curenv);
	}

	// no hay nada que correr, seguir esperando
	sched_halt();
#endif

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// resetear_prioridad();

	// alterar_prioridad();

	// Your code here - Priorities
	struct Env *next_curenv = NULL;
	int selected_index = -1;
	int max_priority_found = -1;

	// Busco el proceso RUNNABLE con mayor prioridad
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE &&
		    envs[i].priority > max_priority_found) {
			max_priority_found = envs[i].priority;
			next_curenv = &envs[i];
			selected_index = i;
		}
	}

	// Incremento la prioridad de los procesos no seleccionados
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE && i != selected_index) {
			envs[i].priority++;
		}
	}

	// Ejecuto el proceso seleccionado
	if (next_curenv) {
		if (next_curenv->priority > MIN_PRIORITY) {
			next_curenv->priority = MIN_PRIORITY;
		}
		corridas_totales[selected_index]++;
		env_run(next_curenv);
	}

	// Si ningun proceso es RUNNABLE, intento correr el RUNNING
	if (curenv && curenv->env_status == ENV_RUNNING) {
		corridas_totales[selected_index]++;
		env_run(curenv);
	}

	// Si no hay nada que correr, idle
	sched_halt();
#endif

	// Without scheduler, keep runing the last environment while it exists
	if (curenv) {
		env_run(curenv);
	}

	// sched_halt never returns
	sched_halt();
}


// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		cprintf("---------ESTADISTICAS---------- \n");
		cprintf("La cantidad de procesos totales es de: %d \n", NENV);

		for (int i = 0; i < NENV; i++) {
			cprintf("Numero de proceso: %d, corridas totales : "
			        "%d\n",
			        i,
			        corridas_totales[i]);
		}

		cprintf("Cantidad de syscalls totales:: %d \n", cant_syscalls);

		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
