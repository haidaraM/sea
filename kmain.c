/*
void
dummy()
{
		return;
}

int
div(int dividend, int divisor)
{
	
	int result=0;
	int remainder = dividend;
	
	while(remainder >= divisor) {
		result++;
		remainder -=divisor;
	}
	
	return result;
}

int
compute_volume(int rad)
{
	int rad3 = rad*rad*rad;
	
	return div(4*355*rad3, 3*113);
}


int 
kmain()
{
	__asm("mrs r0, SPSR");
	
	//on stocke le mode du programme dans r0
	__asm("mrs r0, CPSR");

	//on passe le programme en mode USER
	__asm("cps 0b10000");
	
	//on rafraichit r0
	__asm("mrs r0, CPSR");
	
	//on passe le programme en mode SVC
	__asm("cps 0b10011");
	
	//on rafraichit r0
	__asm("mrs r0, CPSR");
	//l'appel de function avec b ne stock pas le PC dans lr (Link register)
	//__asm( "mov lr, pc" );
	//__asm( "b dummy" );
	// or, equivalently:
	//__asm("bl dummy");
	int radius =5;
	//"r3" sert a proteger le registre
	//__asm("mov r2, %0" : : "r"(radius) : "r3", "r2");
	
	//__asm("mov %0, r3" : "=r"(radius): : "r3", "r2");
	
	
	int volume;
	
	dummy();
	
	volume=compute_volume(radius);
	
	return volume;
}*/

#include "util.h"
#include "syscall.h"
#include "sched.h"

struct pcb_s pcb1, pcb2;
struct pcb_s *p1, *p2;
void user_process_1()
{
	int v1=5;
	while(1)
	{
		v1++;
		sys_yieldto(p2);
	}
}

void user_process_2()
{
	int v2=-12;
	while(1)
	{
		v2-=2;
		sys_yieldto(p1);
		}
}

void kmain( void )
{
	sched_init();
	p1=&pcb1;
	p2=&pcb2;
	// initialize p1 and p2
	// [your code goes yere]
	p1->lr = (uint32_t) &user_process_1;
	p2->lr = (uint32_t) &user_process_2;
	__asm("cps 0x10"); // switch CPU to USER mode
	// **********************************************************************
	p1->lr_user = (uint32_t) &user_process_1;
	p2->lr_user = (uint32_t) &user_process_2;
	sys_yieldto(p1);

	// this is now unreachable
	PANIC();
}