#include "sched.h"


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

	__asm("cps #31");
	__asm("mov lr, %0": :"r"(nouveauContexte->lr_user));
	__asm("cps #19");  
	// on change le processus courant
	// 
	current_process = nouveauContexte;	

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
}