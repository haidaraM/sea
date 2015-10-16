#ifndef SCHED_H
#define SCHED_H

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

};


typedef int (func_t)(void);

void sys_yieldto(struct pcb_s* dest);

void do_sys_yieldto(uint32_t* adressePile);

void sched_init();

struct pcb_s* create_process(func_t* entry);



#endif // SCHED_H
