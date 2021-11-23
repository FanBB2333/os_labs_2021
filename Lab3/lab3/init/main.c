#include "printk.h"
#include "sbi.h"
#include "proc.h"

extern void test();

int start_kernel() {
    printk("2021");
    printk(" Hello RISC-V\n");
    printk("unsigned long : %d\n", sizeof(unsigned long));
    test(); // DO NOT DELETE !!!

	return 0;
}
