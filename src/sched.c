#include "sched.h"
#include "kheap.h"

struct pcb_s* current_process;
struct pcb_s kmain_process;

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

	__asm("cps #31");
	__asm("mov %0, lr":"=r"(current_process->lr_user));

	__asm("cps #19");

	// Charger le nouveau contexte dans la pile à la place des anciens registres
	for (int i = 0; i < 13; ++i)
	{
		*(adressePile +i) = nouveauContexte->registre[i];
	}
	*(adressePile+13) = nouveauContexte->lr;

	// On est obligé de faire ça avant de modifier la valeur de lr.
	// on ne peut pas taper directement dans la variable locale nouveauContexte quand 
	// on passe en mode système.
	current_process = nouveauContexte;	

	__asm("cps #31");
	__asm("mov lr, %0": :"r"(current_process->lr_user));
	__asm("cps #19");  
	
}

struct pcb_s* create_process(func_t* entry)
{
	struct pcb_s * process = (struct pcb_s *) kAlloc(sizeof(struct pcb_s));
	process->lr = (uint32_t) entry;
	process->lr_user = (uint32_t) entry;

	return process;
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
	kheap_init();
}