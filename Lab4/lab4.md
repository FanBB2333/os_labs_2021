# Lab 4: RV64 虚拟内存管理

**学号: ** 3190105838

**姓名: ** 范钊瑀

## 4.1 准备工程
在`defs.h`中需要添加以下的宏定义，由于该头文件将会被被`vm.c`, `mm.c`等头文件引用，我们需要预先定义好诸如OFFSET, VM的始末地址, OPENSBI_SIZE等宏。

```c
#define OPENSBI_SIZE (0x200000)

#define VM_START (0xffffffe000000000)
#define VM_END   (0xffffffff00000000)
#define VM_SIZE  (VM_END - VM_START)

#define PA2VA_OFFSET (VM_START - PHY_START)
```


## 4.2 开启虚拟内存映射

### 4.2.1 setup_vm 的实现

### 4.2.1.1 `setup_vm`函数
`setup_vm`中，我们需要开启两段地址映射，分别是一次等值映射和一次从低地址到高地址的映射，也即对根页表的不同位置的条目进行赋值，在`setup_vm`函数中，我们需要先利用虚拟地址计算出不同虚拟地址`VPN[2]`的值，再对对应的入口进行赋值。

```c
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
    early_pgtbl[getvpn(0x80000000, 2)] = pte; // PA == VA
    early_pgtbl[getvpn(0xffffffe000000000, 2)] = pte; // PA + PV2VA_OFFSET == VA
}
```

为了便于我们从39位的虚拟地址中迅速获得想要的域的内容，我们可以定义一个函数，只需传入虚拟地址和需要获取的部分`VPN`的编号`i`，即可获得对应的`VPN[i]`。
简单地说，函数就是将传入的虚拟地址右移对应的位数，并将其低9位mask出来并返回。
```c
//  38        30 29        21 20        12 11                           0
//  ---------------------------------------------------------------------
// |   VPN[2]   |   VPN[1]   |   VPN[0]   |          page offset         |
//  ---------------------------------------------------------------------
//                         Sv39 virtual address
int getvpn(unsigned long va, int idx){
    return ((va >> (12 + idx * 9)) & 0x1FF);
}
```

类似地，从物理地址中获取对应`PPN[i]`域的内容的任务，我们也可以用一个函数来完成，和虚拟地址中有所不同的是，物理地址中，`PPN[2]`为26位，而`PPN[1]`和`PPN[0]`为9位，所以在取出时需要对不同的index设置不同长度的mask。
```c
//  55                30 29        21 20        12 11                           0
//  -----------------------------------------------------------------------------
// |       PPN[2]       |   PPN[1]   |   PPN[0]   |          page offset         |
//  -----------------------------------------------------------------------------
//                             Sv39 physical address
int getppn(unsigned long pa, int idx){
    if(idx == 0)
        return (pa >> 12) & 0x1FF;
    if(idx == 1)
        return (pa >> 21) & 0x1FF;
    if(idx == 2)
        return (pa >> 30) & 0x3FFFFFF;
}

```
### 4.2.1.2 `relocate`函数
`relocate`函数位于汇编文件`head.S`中，在`relocate`函数中，我们既要对`ra`, `sp`进行设置，以适应开启虚拟地址后的映射，也要对`satp`寄存器进行设置，以指明根页表的基地址。
由于`satp`寄存器的结构如下图所示，我们需要进行赋值的域为`MODE`和`PPN`。
由于我们本次使用了`Sv39`模式的虚拟地址，即对应的虚拟地址有39位，同时需要将`MODE`域设为8
```
#  63      60 59                  44 43                                0
#  ---------------------------------------------------------------------
# |   MODE   |         ASID         |                PPN                |
#  ---------------------------------------------------------------------
```
所以我们用`t0`寄存器存储左移后的`MODE`域的值。
```assembly  
    li   t0, 8
    slli t0, t0, 60 
```
同理，我们用`t1`寄存器存储`PPN`域的值。由于`PA >> 12 == PPN`, 我们先获得`early_pgtbl`的地址，之后右移12位得到`PPN`域的值。
```assembly
    la   t1, early_pgtbl
    srli t1, t1, 12
```
最后，我们将`t0`和`t1`的值进行或运算，得到`satp`寄存器的值，并用`csrrw`指令将其赋值给`satp`寄存器。


```assembly
relocate:
    # set ra = ra + PA2VA_OFFSET
    # set sp = sp + PA2VA_OFFSET (If you have set the sp before)
    li  a0, 0xffffffe000000000 - 0x80000000
    add ra, ra, a0
    add sp, sp, a0

    # set satp with early_pgtbl
    add t0, x0, x0
    add t1, x0, x0
    // mode field: 8, PPN field: early_pgtbl >> 12
    li   t0, 8
    slli t0, t0, 60 
    // t0 = (8 << 60) 
    la  t1, early_pgtbl
    // Shift Right Logical Immediate
    srli t1, t1, 12
    // t1 = early_pgtbl >> 12
    or    t0, t0, t1
    csrrw x0, satp, t0
    # flush tlb
    sfence.vma zero, zero
    ret
```


### 4.2.2 setup_vm_final 的实现

```c
void setup_vm_final(void) {
    memset(swapper_pg_dir, 0x0, PGSIZE);
    // initialize to all mem 0x0

    // No OpenSBI mapping required
    int perm = 0;

    // mapping kernel text X|-|R|V
    perm = (1 << 0) | (1 << 1) | (1 << 3);
    create_mapping(swapper_pg_dir, (uint64)_stext, VA2PA((uint64)_stext), (uint64)_etext - (uint64)_stext, perm);

    // mapping kernel rodata -|-|R|V
    perm = (1 << 0) | (1 << 1);
    create_mapping(swapper_pg_dir, (uint64)_srodata, VA2PA((uint64)_srodata), (uint64)_erodata - (uint64)_srodata, perm);

    // mapping other memory -|W|R|V
    perm = (1 << 0) | (1 << 1) | (1 << 2);
    create_mapping(swapper_pg_dir, (uint64)_sdata, VA2PA((uint64)_sdata), (uint64)_ebss - (uint64)_sdata, perm);

    perm = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
    create_mapping(swapper_pg_dir, PGROUNDUP((uint64)_ekernel), VA2PA(PGROUNDUP((uint64)_ekernel)), (VM_START + PHY_SIZE) - PGROUNDUP((uint64)_ekernel), perm);

    // set satp with swapper_pg_dir
    // YOUR CODE HERE
    uint64 _satp = (8L << 60) | (VA2PA((uint64)swapper_pg_dir) >> 12);
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
```

### 4.2.3 create_mapping 的实现

```c
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

    int page_num = sz / PGSIZE;
    page_num = (sz % PGSIZE != 0) ? (page_num + 1) : page_num;
    // printk("ready to allocate page: %d\n", page_num);
    
    uint64 *pgd = pgtbl;
    uint64 *pmd = NULL;
    uint64 *pte = NULL;

    for(int i = 0; i < page_num; i++){
        // printk("pte: %x, getvpn(va, 2,1,0): %d  %d  %d\n", pte, getvpn(va, 2), getvpn(va, 1), getvpn(va, 0));

        // layer 2
        // if 1st entry doesn't exist, create a new one
        if( !page_exist(pgtbl[getvpn(va, 2)]) ){
            pmd = (uint64 *)kalloc(); // 64-bit PPN in PDG
            memset(pmd, 0x0, PGSIZE);
            pgtbl[getvpn(va, 2)] = (( VA2PA((uint64)pmd) >> 12) << 10) | 0x1;
        }
        else{
            pmd = (uint64 *)PA2VA( ( pgtbl[getvpn(va, 2)] >> 10 ) << 12 );
        }
        
        // layer 1
        if( !page_exist(pmd[getvpn(va, 1)]) ){
            pte = (uint64 *)kalloc(); // 64-bit PPN in PMD
            memset(pte, 0x0, PGSIZE);
            pmd[getvpn(va, 1)] = (( VA2PA((uint64)pte) >> 12) << 10) | 0x1;
        }
        else{
            pte = (uint64 *)PA2VA( ( pmd[getvpn(va, 1)] >> 10 ) << 12 );
        }
        // layer 0
        if( !page_exist(pte[getvpn(va, 0)]) ){
            pte[getvpn(va, 0)] = (((uint64)pa >> 12) << 10) | (uint64)perm;
        }
        else{
            printk("i: %d, EXISTING PAGE!!!\n", i);
        }
        pa = pa + PGSIZE;
        va = va + PGSIZE;
    }
}
```
### 4.2.4 mm.c 中的更改

由于在进行`mm_init`，即初始化可供`kalloc`分配的内存时，系统已经处于虚拟内存模式，因此需要修改`mm_init`函数接收的起始结束地址，将其均改为虚拟地址。需要注意的是，在进入`mm_init`函数时，由于我们已经设置过`satp`寄存器的值，开启了虚拟地址，符号表`System.map`和此处所读取到的`_ekernel`的值均为虚拟地址，这样符合我们的预期。
如下所示，可供分配的内存是紧挨在内核空间之后的，因此初始化时的起始地址可以用`_ekernel`，即内核空间的结束地址来表示，此外，由于所有物理内存的大小为128M(PHY_SIZE)，因此我们可以将`mm_init`函数的结束地址设置为虚拟地址开始处`VM_START`偏移`PHY_SIZE`之后的结果。

```c
void mm_init(void) {
    kfreerange((uint64)_ekernel , (VM_START + PHY_SIZE));
    printk("...mm_init done!\n");
}
```

### 4.2.5 head.S 中的更改
在head.S中，我们需要对刚刚完成的几个函数进行调用，调用时需要注意顺序关系，在最开头需要将系统栈`sp`指针初始化到正确的位置，之后便可以通过调用`setup_vm`函数来开启虚拟内存，并利用`relocate`函数来将`ra`, `sp`赋值到正确的位置，并对存有根页表基地址的`satp`寄存器进行赋值。
这之后，系统便运行在了虚拟内存的状态下，由于在`setup_vm_final`中需要调用`create_mapping`函数，继而可能会调用到`kalloc`函数来进行内存分配的操作，因此在这之前需要将内存初始化完毕，也即需要将`mm_init`函数调用完毕，因此需要将`mm_init`函数调用的位置放在`setup_vm_final`之前。

此外，我们将中断信号的开启以及相关寄存器的赋值放到了`setup_vm_final`后，这是因为在设置好第一次中断信号后，一秒后就会进入中断，我们并不能保证在所有设备上内存的初始化都能够在一秒内完成，否则在内存初始化的过程中产生了中断信号则会造成较为严重的后果。
```assembly
_start:
    la  sp, boot_stack_top
    call setup_vm
    call relocate

    call mm_init
    call setup_vm_final

    ......

```

### 4.3 编译及测试

### 4.4 思考与总结



## 思考题

- 1. 验证 .text, .rodata 段的属性是否成功设置，给出截图。


- 2. 为什么我们在 setup_vm 中需要做等值映射?


- 3. 在 Linux 中，是不需要做等值映射的。请探索一下不在 setup_vm 中做等值映射的方法。