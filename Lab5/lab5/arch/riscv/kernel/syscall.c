#include "syscall.h"
#include "defs.h"
#include "types.h"
#include "proc.h"
#include "printk.h"

extern struct task_struct* current;

uint64 syscall(struct pt_regs *regs, uint64 call_id)
{
    uint64 _ret;
    switch (call_id) {
        case SYS_WRITE:
            // arguments: fd, buf, count
            // arguments: a0, a1, a2
            _ret = sys_write(regs->x[10], regs->x[11], regs->x[12]);
            regs->x[10] = _ret;
            break;
        case SYS_GETPID:
            _ret = sys_getpid();
            printk("getpid: %ld\n", _ret);
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
    int total_out = 0;
    // printk("start write\n");
    for(int i = 0; i < count && buf[i]; i++){
        printk("%c", buf[i]);
        total_out++;
    }
    // printk("end write\n");
    return total_out;
}

uint64 sys_getpid(){

    return current->pid;
}

