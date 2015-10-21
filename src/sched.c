#include "sched.h"
#include "kheap.h"

struct pcb_s* current_process;
struct pcb_s kmain_process;
struct pcb_s *last_process;

void do_sys_yieldto(uint32_t* adressePile)
{
	struct pcb_s* nouveauContexte;
	nouveauContexte = (struct pcb_s*)*( adressePile +2);

	// Sauvegarde de l'ancien contexte => mettre nos registres dans current_process
	for (int i = 0; i < 13; ++i)
	{
		current_process->registre[i] = *(adressePile + i);
	}
	current_process->lr = *(adressePile +13);

	__asm("cps #31"); // mode système
	__asm("mov %0, lr":"=r"(current_process->lr_user));
	__asm("mov %0, sp": "=r"(current_process->sp));
	__asm("cps #19"); // mode svc

	// Charger le nouveau contexte dans la pile à la place des anciens registres
	for (int i = 0; i < 13; ++i)
	{
		*(adressePile +i) = nouveauContexte->registre[i];
	}
	*(adressePile+13) = nouveauContexte->lr;

	// sauvegarde du spsr du mode svc dans currrent_process
	__asm("mrs %0, spsr" : "=r"(current_process->cpsr));

	// On est obligé de faire ça avant de modifier la valeur de lr.
	// on ne peut pas taper directement dans la variable locale nouveauContexte quand 
	// on passe en mode système.
	current_process = nouveauContexte;	

	__asm("cps #31"); // mode système
	__asm("mov lr, %0": :"r"(current_process->lr_user));
	__asm("mov sp, %0": :"r"(current_process->sp));
	__asm("cps #19");  // mode svc

	// restauration dans spsr du cpsr sauvegardé
	__asm("msr spsr, %0" : : "r"(current_process->cpsr));
	
}


void elect()
{
	current_process = current_process->next;
}

void do_sys_yield(uint32_t* adressePile)
{

	// Sauvegarde de l'ancien contexte => mettre nos registres dans current_process
	for (int i = 0; i < 13; ++i)
	{
		current_process->registre[i] = *(adressePile + i);
	}
	current_process->lr = *(adressePile +13);

	__asm("cps #31"); // mode système
	__asm("mov %0, lr":"=r"(current_process->lr_user));
	__asm("mov %0, sp": "=r"(current_process->sp));
	__asm("cps #19"); // mode svc
	
	// sauvegarde du spsr du mode svc dans currrent_process
	__asm("mrs %0, spsr" : "=r"(current_process->cpsr));
	
	elect();
	// Charger le nouveau contexte dans la pile à la place des anciens registres
	for (int i = 0; i < 13; ++i)
	{
		*(adressePile +i) = current_process->registre[i];
	}
	*(adressePile+13) = current_process->lr;

	
	__asm("cps #31"); // mode système
	__asm("mov lr, %0": :"r"(current_process->lr_user));
	__asm("mov sp, %0": :"r"(current_process->sp));
	__asm("cps #19");  // mode svc

	// restauration dans spsr du cpsr sauvegardé
	__asm("msr spsr, %0" : : "r"(current_process->cpsr));
}



/*
 Cree un nouveau process
*/
struct pcb_s* create_process(func_t* entry)
{
	struct pcb_s * process = (struct pcb_s *) kAlloc(sizeof(struct pcb_s));
	process->lr = (uint32_t) entry;
	process->lr_user = (uint32_t) entry;

	__asm("mrs %0, cpsr":"=r"(process->cpsr));

	// allocation de la pile d'éxécution
	uint32_t * stack = (uint32_t *) kAlloc(10000);

	// Il faut décaler sp vers la fin de la zone mémoire car la pile croit dans le sens 
	// décroissant des adresses.
	process->sp = stack + 10000;
	
	// round robin
	process->next = last_process->next;
	last_process->next=process;
	last_process=process;
	
	return process;
}

void sys_yield()
{
	__asm("mov r0, %0" : : "r"(6));
	
	__asm("SWI #0");
}

void sys_yieldto(struct pcb_s* dest)
{
	__asm("mov r0, %0" : : "r"(5));
	__asm("mov r2, %0" : : "r"(dest):"r0"); 
	
	__asm("SWI #0");
}

void sched_init()
{
	current_process = &kmain_process;
	last_process = current_process;
	last_process->next = last_process;
	current_process->next = last_process;
	kheap_init();
}
