#ifndef _SYSCALL_H
#define _SYSCALL_H

#define SYS_WRITE   64
#define SYS_GETPID  172

#include "types.h"

uint64 sys_write(unsigned int fd, const char* buf, uint64 count);
uint64 sys_getpid();

uint64 syscall(uint64 a0, uint64 a1, uint64 a2, uint64 a3, uint64 a4, uint64 a5, uint64 a6, uint64 a7, 
            uint64 call_id);





#endif