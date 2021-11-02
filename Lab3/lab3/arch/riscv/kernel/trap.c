// trap.c 
#include "clock.h"
#include "printk.h"

int dec2bit(unsigned long num, int index) {
    return (num>>(index-1)) & 1;
}

void trap_handler(unsigned long scause, unsigned long sepc) {
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
            // printk("call do_timer\n");
            do_timer();
            clock_set_next_event();
        }
    }
}