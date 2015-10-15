#include "syscall.h"
#include "util.h"

uint64_t* adressePile;

void do_sys_reboot()
{
	__asm("mov pc, #0");
}

void do_sys_nop()
{
	
}

void do_sys_gettime(){

	uint64_t systime = get_date_ms();

	uint32_t timeLb = systime & 0xFFFFFFFF;
	uint32_t timeHb = systime >> 32; 


	*adressePile = timeLb;
	*((unsigned int*)adressePile+1) = timeHb;

}

//dummy function to ommit warning for unused varibales
void fonctionBidon(int truth, uint64_t date)
{
		
}

void do_sys_settime()
{
	int halftheTruth = 21;
	int theTruth = 2*halftheTruth;
	

	uint32_t maDate_lb = *((unsigned int*) adressePile+7);
	uint32_t maDate_hb = *((unsigned int*) adressePile+8);
//	uint32_t maDate_lb = *(adressePile + 8);
//	uint32_t maDate_hb = *(adressePile + 9);
	
	
	uint64_t date = ( (uint64_t)maDate_hb << 32) + maDate_lb;

	fonctionBidon(theTruth, date);
	
	set_date_ms(date);

}

void __attribute__((naked )) swi_handler(void)
{
	//on sauve le contexte dans la pile
	__asm("stmfd sp!, {r0-r12, lr}");
//	__asm("stmfd sp, {r0-r12, lr}");
	
	//sp a ete mis a jour par la commande precedente tout seul (indique par le "!" apres sp)
	__asm("mov %0, sp" : "=r"(adressePile));
	
	int choix;
	
	__asm("mov %0, r0" : "=r"(choix));
	
	switch (choix) {
		case 1 :
			do_sys_reboot();
			break;
		case 2 :
			do_sys_nop();
			break;
		case 3 :			
			do_sys_settime();
			break;
		case 4:
			do_sys_gettime();
			break;
		default :
			PANIC();
			
	}
	//on recupere le contexte dans la pile
	__asm("ldmfd sp!, {r0-r12, pc}^");
	
}

void sys_reboot()
{
	
	__asm("mov r0, %0" : : "r"(1));
	
	__asm("SWI #0");
}

void sys_nop()
{
	__asm("mov r0, %0" : : "r"(2));
	
	__asm("SWI #0");
}

void sys_settime(uint64_t date_ms)
{
	//first calculate, then manually modify registers!
	uint32_t date_lowbits = date_ms & 0xFFFFFFFF;
	uint32_t date_highbits = date_ms >> 32;    
	
	__asm("mov r7, %0" : : "r"(date_lowbits));
	__asm("mov r8, %0" : : "r"(date_highbits)); 
	__asm("mov r0, %0" : : "r"(3));	

	
	__asm("SWI #0");
}




uint64_t sys_gettime()
{
	__asm("mov r0, %0" : : "r"(4));


	__asm("SWI #0");


	uint32_t ourTimeLb;
	uint32_t ourTimeHb;

	__asm("mov %0, r0" : "=r"(ourTimeLb));
	__asm("mov %0, r1" : "=r"(ourTimeHb));

	uint64_t realTime = ( (uint64_t) ourTimeHb << 32) + ourTimeLb;

	return realTime;

}