#include "syscall.h"
#include "defs.h"
#include "types.h"
#include "proc.h"
#include "printf.c"

extern struct task_struct* current;

uint64 syscall(struct pt_regs *regs, uint64 call_id)
{
    uint64 _ret;
    switch (call_id) {
        case SYS_WRITE:
            // arguments: fd, buf, count
            // arguments: a0, a1, a2
            _ret = sys_write(regs->x[10], (char*)regs->x[11], regs->x[12]);
            break;
        case SYS_GETPID:
            _ret = sys_getpid();
            // x[10] is a0
            regs->x[10] = _ret;
            break;

        default:
            break;
    }

    __asm__ volatile (
        "csrr t0, sepc\n"
        "addi t0, t0, 4\n"
        "csrw sepc, t0\n"
        :
        :
        :"memory"
	);
    regs->x[10] = _ret;

    return _ret;
}

uint64 sys_write(unsigned int fd, const char* buf, uint64 count){
    int i;
    for(i = 0; i < count; i++){
        putc(buf[i]);
    }
    return count;
}

uint64 sys_getpid(){

    return current->pid;
}

