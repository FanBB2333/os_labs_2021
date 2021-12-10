#include "printk.h"
#include "sbi.h"
#include "proc.h"

extern void test();

int start_kernel() {
    printk("2021");
    printk(" Hello RISC-V\n");
    schedule(); // add 4.5 in Lab5
    test(); // DO NOT DELETE !!!

	return 0;
}
