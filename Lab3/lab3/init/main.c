#include "printk.h"
#include "sbi.h"
#include "proc.h"

extern void test();

int start_kernel() {
    printk("2021");
    printk(" Hello RISC-V\n");
    dummy();
    test(); // DO NOT DELETE !!!

	return 0;
}
