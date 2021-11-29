#include "defs.h"
#include "mm.h"
#include "string.h"
#include "printk.h"

// arch/riscv/kernel/vm.c

extern void relocate(long OFFSET);
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm);
/* early_pgtbl: 用于 setup_vm 进行 1GB 的 映射。 */
unsigned long  early_pgtbl[512] __attribute__((__aligned__(0x1000)));

int getvpn(unsigned long va, int idx){
    return ((va >> (12 + idx * 9)) & 0x1FF);
}

int getppn(unsigned long pa, int idx){
    if(idx == 0)
        return (pa >> 12) & 0x1FF;
    if(idx == 1)
        return (pa >> 21) & 0x1FF;
    if(idx == 2)
        return (pa >> 30) & 0x3FFFFFF;
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
    unsigned long pte;
    memset(early_pgtbl, 0x0, PGSIZE);

    pte = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (getppn(0x80000000, 2) << 28);

    // printk("pte: %x\n", pte);
    // printk("getvpn(0x80000000, 2): %d\n", getvpn(0x80000000, 2));
    // printk("getppn(0xffffffe000000000, 2): %d\n", getvpn(0xffffffe000000000, 2));
    early_pgtbl[getvpn(0x80000000, 2)] = pte; // PA == VA
    early_pgtbl[getvpn(0xffffffe000000000, 2)] = pte; // PA + PV2VA_OFFSET == VA
    // relocate((long)PA2VA_OFFSET);
    
}

// arch/riscv/kernel/vm.c 

/* swapper_pg_dir: kernel pagetable 根目录， 在 setup_vm_final 进行映射。 */
unsigned long  swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));

extern char _stext[];
extern char _etext[];

extern char _srodata[];
extern char _erodata[];

extern char _sdata[];
extern char _edata[];

extern char _sbss[];
extern char _ebss[];

extern char _ekernel[];

uint64 PA2VA(uint64 pa){
    uint64 _offset = 0xffffffe000000000 - 0x80000000;
    return (_offset + pa);
}

uint64 VA2PA(uint64 va){
    uint64 _offset = 0xffffffe000000000 - 0x80000000;
    return (va - _offset);
}

void setup_vm_final(void) {
    memset(swapper_pg_dir, 0x0, PGSIZE);
    // initialize to all mem 0x0

    // No OpenSBI mapping required
    int perm = 0;

    // mapping kernel text X|-|R|V
    perm = (1 << 0) | (1 << 1) | (1 << 3);
    create_mapping(swapper_pg_dir, (uint64)&_stext, VA2PA((uint64)&_stext), (uint64)&_etext - (uint64)&_stext, perm);

    // mapping kernel rodata -|-|R|V
    perm = (1 << 0) | (1 << 1);
    create_mapping(swapper_pg_dir, (uint64)&_srodata, VA2PA((uint64)&_srodata), (uint64)&_erodata - (uint64)&_srodata, perm);

    // mapping other memory -|W|R|V
    perm = (1 << 0) | (1 << 1) | (1 << 2);
    create_mapping(swapper_pg_dir, (uint64)&_sdata, VA2PA((uint64)&_sdata), (uint64)&_ebss - (uint64)&_sdata, perm);

    // set satp with swapper_pg_dir

    // YOUR CODE HERE
    uint64 _satp = (8 << 60) | ((uint64)swapper_pg_dir >> 12);
    __asm__ volatile (
        "csrrw x0, satp, %[_satp]\n"
        :
        :[_satp] "r" (_satp)
        :"memory"
	);

    // flush TLB
    asm volatile("sfence.vma zero, zero");
    return;
}

int page_exist(uint64 pte){
    return (pte & 0x1);
}

/* 创建多级页表映射关系 */
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm) {
    /*
    pgtbl 为根页表的基地址
    va, pa 为需要映射的虚拟地址、物理地址
    sz 为映射的大小
    perm 为映射的读写权限

    创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
    可以使用 V bit 来判断页表项是否存在
    */
    // 2   | (PUD) | 1   | 0   | OFFSET
    // PGD | (PUD) | PMD | PTE | OFFSET
    //第一层是页面目录(PDG)，第二层是中间目录(PMD)，页表(PTE)
    // PTE: 页表项（page table entry）
    // PGD(Page Global Directory)
    // PUD(Page Upper Directory)
    // PMD(Page Middle Directory)
    // PT(Page Table)

    // TODO : implement sz
    int page_num = sz / PGSIZE;
    
    uint64 *pgd = pgtbl;
    uint64 *pmd = NULL;
    uint64 *pte = NULL;

    for(int i = 0; i < page_num; i++){

        // layer 2
        // if 1st entry doesn't exist, create a new one
        if( !page_exist(pgtbl[getvpn(va, 2)]) ){
            pmd = (uint64 *)kalloc(); // 64-bit PPN in PDG
            pgtbl[getvpn(va, 2)] = (((uint64)pmd >> 12) << 10);
        }
        else{
            pmd = ( pgtbl[getvpn(va, 2)] >> 10 ) << 12;
        }
        
        // layer 1
        if( !page_exist(pmd[getvpn(va, 1)]) ){
            pte = (uint64 *)kalloc(); // 64-bit PPN in PMD
            pmd[getvpn(va, 1)] = (((uint64)pte >> 12) << 10);
        }
        else{
            pte = ( pmd[getvpn(va, 1)] >> 10 ) << 12;
        }

        // layer 0
        if( !page_exist(pte[getvpn(va, 0)]) ){
            pte[getvpn(va, 0)] = (((uint64)pa >> 12) << 10) | (uint64)perm;
        }
        else{
            printk("EXISTING PAGE!!!\n");
        }
        pa = pa + 1 * PGSIZE;
        va = va + 1 * PGSIZE;
    }

    
    
}