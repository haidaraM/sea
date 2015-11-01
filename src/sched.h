#ifndef SCHED_H
#define SCHED_H
typedef enum { false, true } bool;

#include "stdint.h"
struct pcb_s
{
	// process id
	//uint32_t pid;

	// Registres du contexte
	uint32_t registre[13];

	// lr du svc
	uint32_t lr;

	// lr user
	uint32_t lr_user;

	// pointeur vers la pile d'éxécution du processus
	uint32_t * sp;

	// registre d'état du processus
	uint32_t cpsr;
	
	// pointeur sur le processus suivant dans la liste
	struct pcb_s* next;
	
	//pointer sur le proces precedent dans la liste (on en a besoin pour facilement effaces un element de la liste)
	struct pcb_s* previous;

	bool terminated;
};


typedef int (func_t)(void);

void sys_yieldto(struct pcb_s* dest);

void do_sys_yieldto(uint32_t* adressePile);

void sys_yield();

void do_sys_yield(uint32_t* adressePile);

void sys_exit();

void do_sys_exit();

void sched_init();

int get_amount_running();

struct pcb_s* create_process(func_t* entry);



#endif // SCHED_H
