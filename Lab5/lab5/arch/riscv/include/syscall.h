#ifndef _SYSCALL_H
#define _SYSCALL_H

#define SYS_WRITE   64
#define SYS_GETPID  172

#include "types.h"

uint64 sys_write(unsigned int fd, const char* buf, uint64 count);
uint64 sys_getpid();

uint64 syscall(struct pt_regs *regs, uint64 call_id);




#endif