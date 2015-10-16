#ifndef SCHED_H
#define SCHED_H

#include "stdint.h"
struct pcb_s
{
	// process id
	//uint32_t pid;

	//
	uint32_t registre[13];

	// lr
	uint32_t lr;

	//
	uint32_t lr_user;

};


void sys_yieldto(struct pcb_s* dest);

void do_sys_yieldto(uint32_t* adressePile);
void sched_init();

#endif // SCHED_H
