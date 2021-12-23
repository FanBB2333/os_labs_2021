#include "syscall.h"
#include "stdio.h"


static inline long getpid() {
    long ret;
    // printf("getpid start with pid: %d\n", SYS_GETPID);
    asm volatile ("li a7, %1\n"
                  "ecall\n"
                  "mv %0, a0\n"
                : "+r" (ret) 
                : "i" (SYS_GETPID));
    return ret;
}

void aaa() {
    printf("aaa\n");    
}

void bbb() {
    printf("bbb\n");
}

int main() {
    register unsigned long current_sp __asm__("sp");
    while (1) {
        printf("This is a sample output from user space.\n");
        printf("[U-MODE] pid: %ld\n", getpid());
        // printf("[U-MODE] pid: %ld, sp is %lx\n", getpid(), current_sp);

        for (unsigned int i = 0; i < 0x4FFFFFFF; i++);

    }

    return 0;
}
