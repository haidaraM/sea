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
}
