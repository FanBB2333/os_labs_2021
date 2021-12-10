// trap.c 
#include "clock.h"
#include "printk.h"
#include "proc.h"
#include "syscall.h"



int dec2bit(unsigned long num, int index) {
    return (num>>(index-1)) & 1;
}
void trap_handler(uint64_t scause, uint64_t sepc, struct pt_regs *regs) {
    // 通过 `scause` 判断trap类型
    // 如果是interrupt 判断是否是timer interrupt
    // 如果是timer interrupt 则打印输出相关信息, 并通过 `clock_set_next_event()` 设置下一次时钟终端
    // `clock_set_next_event()` 见 4.5 节
    // 其他interrupt / exception 可以直接忽略

    // # YOUR CODE HERE
    // scause 最高位1 代表是interrupt
    int interrupt = dec2bit(scause, 64);
    if(interrupt == 1){
        unsigned long exception_code = scause - (1UL << 63); 
        if(exception_code == 5){
            clock_set_next_event();
            do_timer();
        }
    }

    else if(interrupt == 0){
        printk("exception: %d\n", scause);
        unsigned long exception_code = scause - (1UL << 63); 
        //Environment call from U-mode
        if (exception_code == 8) {
            printk("Environment call from U-mode\n");
            // call system cal
            syscall(regs, exception_code);
        }

        
    }
}
