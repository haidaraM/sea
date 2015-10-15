#ifndef SYSCALL
#define SYSCALL

#include "hw.h"
#include "stdint.h"

void sys_reboot();
void sys_nop();
void sys_settime(uint64_t date_ms);
uint64_t sys_gettime();


#endif
