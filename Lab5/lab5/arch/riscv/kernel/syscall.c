#include "syscall.h"
#include "defs.h"
#include "types.h"

#include "printf.c"


uint64 syscall(struct pt_regs *regs, uint64 call_id)
{
    uint64 _ret;
    switch (call_id) {
        case SYS_WRITE:
            // _ret = sys_write(regs[0], (char*)regs[1], regs[2]);
            break;
        case SYS_GETPID:
            _ret = sys_getpid();
            break;

        default:
            break;
    }

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
    
    return getpid();
}

