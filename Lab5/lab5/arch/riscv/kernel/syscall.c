#include "syscall.h"
#include "defs.h"
#include "types.h"

#include "printf.c"


uint64 syscall(uint64 a0, uint64 a1, uint64 a2, uint64 a3, uint64 a4, uint64 a5, uint64 a6, uint64 a7, 
            uint64 call_id)
{
    uint64 _ret;
    switch (call_id) {
        case SYS_WRITE:
            _ret = sys_write(a0, (char*)a1, a2);
            break;
        case SYS_GETPID:
            _ret = sys_getpid();
            break;

        default:
            break;
    }

    return _ret;
}

uint64 sys_write(unsigned int fd, const char* buf, size_t count){
    int i;
    for(i = 0; i < count; i++){
        putc(buf[i]);
    }
    return count;
}

uint64 sys_getpid(){
    
    return getpid();
}

