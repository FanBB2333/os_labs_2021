// arch/riscv/kernel/vm.c

/* early_pgtbl: 用于 setup_vm 进行 1GB 的 映射。 */
unsigned long  early_pgtbl[512] __attribute__((__aligned__(0x1000)));

int getvpn(unsigned long va, int idx){
    return ((va >> (12 + idx * 9)) & 0x1FF);
}

void setup_vm(void) {
    /* 
    1. 由于是进行 1GB 的映射 这里不需要使用多级页表 
    2. 将 va 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
        high bit 可以忽略
        中间9 bit 作为 early_pgtbl 的 index
        低 30 bit 作为 页内偏移 这里注意到 30 = 9 + 9 + 12， 即我们只使用根页表， 根页表的每个 entry 都对应 1GB 的区域。 
    3. Page Table Entry 的权限 V | R | W | X 位设置为 1
    */
    unsigned long va;
    early_pgtbl[getvpn(0x80000000, 2)] = 0x80000000; // PA == VA
    early_pgtbl[getvpn(0xffffffe000000000, 2)] = 0x80000000; // PA + PV2VA_OFFSET == VA
    

    
}