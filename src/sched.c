#include "sched.h"
#include "kheap.h"

struct pcb_s* current_process;
struct pcb_s kmain_process;
struct pcb_s *last_process;

// au lieu de do_sys_yield ce fonction s'en fou de gerer les registres du mode SVC.
// (Ca peut poser des ennuis, parce qu'on perde le SPSR du svc.)
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
	__asm("cps #19"); // mode svc (supervisor)

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
	__asm("cps #19");  // mode svc (supervisor)

	// restauration dans spsr du cpsr sauvegardé
	__asm("msr spsr, %0" : : "r"(current_process->cpsr));
	
}


void elect()
{
	current_process = current_process->next;
}

void do_sys_yield(uint32_t* adressePile)
{

	// Sauvegarde de l'ancien contexte => mettre nos registres (qui sont deja
	// sur la pile, grace a syscall.c) dans la struct current_process
	for (int i = 0; i < 13; ++i)
	{
		current_process->registre[i] = *(adressePile + i);
	}
	current_process->lr = *(adressePile +13);

	// Les registres physiques 13, 14 sont pas les memes, quand on est en mode SVC,
	// du coup avant de sauvgarder leur contenu on doit passer en mode systeme
	// (qui utilise les memes regustres physiques que mode utilisateur).
	__asm("cps #31"); // passer en mode système
	__asm("mov %0, lr":"=r"(current_process->lr_user));
	__asm("mov %0, sp": "=r"(current_process->sp));
	__asm("cps #19"); // passer en mode svc (supervisor)
	
	// sauvegarde du spsr du mode svc dans currrent_process
	__asm("mrs %0, spsr" : "=r"(current_process->cpsr));
	
	//avancer au context suivant
	elect();
	
	// Charger le nouveau contexte a la pile, c'est a dire a la place des anciens
	// registres (niveau pile) - elles seront apres charge dans
	// les registres physiques par syscall.c
	for (int i = 0; i < 13; ++i)
	{
		*(adressePile +i) = current_process->registre[i];
	}
	*(adressePile+13) = current_process->lr;
	
	__asm("cps #31"); // passer en mode système
	__asm("mov lr, %0": :"r"(current_process->lr_user));
	__asm("mov sp, %0": :"r"(current_process->sp));
	__asm("cps #19");  // passer en mode svc (supervisor)

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
		
	// initialiser le champ qui indique que le precessus n'est pas erncore termine
	process->terminated=false;
	
	// round robin
	// gestion des references (direction normale)
	process->next = last_process->next;
	last_process->next=process;
	// gestion des references (direction inverse)
	process->next->previous = process;
	process->previous = last_process;
	
	// mettre a jour last_process
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
	// initialiser double-chained liste
	last_process->next = last_process;
	last_process->previous = last_process;
	current_process->next = last_process;
	kheap_init();
}

void do_sys_exit()
{
	// chercher le pcb avant et changer son pointer vers le pcb prochain
	current_process->previous->next = current_process->next;
	current_process->next->previous = current_process->previous;
	
	// indiquer que le processus est termine
	current_process->terminated=true;
	
	// librere les ressources dont on avait besoin pour la gestion de ce processus
	// ToDo [...]
}

void sys_exit()
{
	//declencher do_sys_init
	__asm("mov r0, %0" : : "r"(7));	
	__asm("SWI #0");
}

// ce function doit combien des processus sont en train de s'executer
// lorsque il ne reste que une seul (kmain) on peut beint terminer le noyeau.
int get_amount_running()
{
	//ce boucle compte combien des processus dans la liste ne osnt pas encore marque comme termine
	int amountRunning = 0;
	struct pcb_s * looper = current_process->next;
	while(looper != current_process)
	{
		if(looper->terminated==false)
		{
			amountRunning++;
		}
		looper = looper->next;
	}
	return amountRunning;
}
