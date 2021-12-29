# Lab 5: RV64 用户模式

**学号: ** 3190105838

**姓名: ** 范钊瑀

## 4.1 准备工程
在基于Lab4的代码中，我们需要给`vmlinux.lds.S`文件中加入用户态程序的存储地址，在data段中我们可以加入以下代码来加载uapp程序：
```
        uapp_start = .;
        *(.uapp .uapp*)
        uapp_end = .;
```

同时需要添加有关用户态地址的相关宏定义，以便做地址映射

```c
#define USER_START (0x0000000000000000) // user space start virtual address
#define USER_END   (0x0000004000000000) // user space end virtual address
```

## 4.2 创建用户态进程

### 进程结构体定义
创建用户态进程的过程在`proc.c`中完成，由于进程的上下文恢复过程中需要用到sepc，sstatus，sscratch等寄存器，因此本次实验中将线程相关的结构体定义如下。
在`thread_struct`中加入`sepc, sstatus, sscratch`寄存器的存储段，在`task_struct`中加入每个线程自己的页表`pgd`。
```c
struct thread_struct {
    uint64_t ra;
    uint64_t sp;                     
    uint64_t s[12];

    uint64_t sepc, sstatus, sscratch; 
};

struct task_struct {
    struct thread_info* thread_info;
    uint64_t state;
    uint64_t counter;
    uint64_t priority;
    uint64_t pid;
    struct thread_struct thread;
    pagetable_t pgd;
};
```

### task_init初始化过程修改

对于0号线程，它应当处于内核态，因此初始化阶段时应将其sscratch的值赋为0。
```c
    idle->thread.sscratch = 0;
```

对于剩余的线程，我们需要逐个进行初始化，与上个实验不同的是，我们需要对新增加的域进行赋值，其中对于sstatus需要将其`SPP`位置为0(使得 sret 返回至 U-Mode)，将其`SPIE`(sret 之后开启中断)和`SUM`(S-Mode 可以访问 User 页面)位置为1，将其`sepc`位设为`USER_START`，即用户段程序的开始地址，使得线程切换之后能够进入到用户态程序继续执行。
对于用户态的线程，需要对每个线程都分配一个U-Mode的栈空间，在这里可以使用kalloc()完成，并且将其地址保存下来，在下方做地址映射是需要将这段栈空间对应的物理地址映射到进程自己的虚拟地址空间顶部。

在分配完栈空间后，需要对每个线程分配自己的页表，创建时我们同样可以同样地kalloc()一个页表，并将内核态页表中的所有页表项都赋到程序的页表中。

需要注意的是，在sstatus寄存器的设置中，SPP位需要设置为0，SPIE和SUM位需要设置为1。
此外，在下方的映射中，我们还需要将用户态的程序段映射到虚拟地址中的起始位置`USER_START`，以便在线程切换结束sret之后能找到正常的地址。
```c
for(int i = 1; i < NR_TASKS; i++){
    task[i] = (struct task_struct *)kalloc();
    task[i]->state = TASK_RUNNING;
    task[i]->counter = 0;
    task[i]->priority = rand();
    task[i]->pid = i; 
    task[i]->thread.ra = (uint64)&__dummy;
    printk("i: %d, pri: %d, ra: %lx\n", i, task[i]->priority, task[i]->thread.ra);

    task[i]->thread.sp = (char *)((uint64)task[i] + (uint64)PGSIZE);
    // 8:SPP 5:SPIE 18:SUM
    task[i]->thread.sstatus = csr_read(sstatus);
    task[i]->thread.sstatus &= (1L << 8) ;
    task[i]->thread.sstatus |= ((1L << 5) | (1L << 18));
    task[i]->thread.sepc = (USER_START);

    // assign a U-Mode Stack
    uint64 U_SP_VM = ((uint64)kalloc()); 
    // U-Mode sp: USER_END
    task[i]->thread.sscratch = (uint64)USER_END;

    // Copy swapper_pg_dir to the user pagetable
    task[i]->pgd = kalloc();
    for (int j = 0; j < 512; j++){
        task[i]->pgd[j] = swapper_pg_dir[j];
    }
    //D|A|G|U|X|W|R|V|
    //7|6|5|4|3|2|1|0|

    int perm = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
    create_mapping(task[i]->pgd, (USER_START), VA2PA((uint64)uapp_start), (uint64)uapp_end - (uint64)uapp_start, perm);
    create_mapping(task[i]->pgd, (uint64)USER_END - (uint64)PGSIZE, VA2PA(U_SP_VM), (uint64)PGSIZE, perm);
}
```
### __switch_to中保存/恢复相应寄存器与页表
由于`thread_struct`结构体定义的修改，在__switch_to中需要额外进行sepc, sstatus, sscratch以及satp寄存器的保存，由于在`task_struct`中`pgd`变量我们约定好用于存储用户态页表在内核态下的虚拟地址(`ask[i]->pgd = kalloc()`), 因此在__switch_to中需要进行少许的地址转换，即需要将取出的satp寄存器的值先转换得到真实的物理地址，再根据偏移量PA2VA_OFFSET得到对应的内核态虚拟地址并存储到`task[i]->thread.satp`对应的内存地址中。
```assembly
    // save sepc, sstatus, sscratch
    csrrs t0, sepc, x0 # csrr t0, sepc
    sd t0, 152(x10)
    csrrs t0, sstatus, x0 # csrr t0, sstatus
    sd t0, 160(x10)
    csrrs t0, sscratch, x0 # csrr t0, sscratch
    sd t0, 168(x10)

    # store new page table
    csrrs t0, satp, x0 # csrr t0, satp, t0 = satp

    li t1, 0xfffffffffff
    and t0, t0, t1 # t0 = satp & 0xfffffffffff, lowest 44 bits(PPN, but is physical address)
    slli t0, t0, 12 # t0 = t0 << 12, PA = PPN << 12 ( physical address PA)

    li t1, 0xffffffe000000000 - 0x80000000
    add t0, t0, t1 # t0 = PA + PA2VA_OFFSET
    sd t0, 176(x10)
```
在__switch_to中也需要处理satp寄存器，根据从结构体中取到的地址，将其转换成对应的satp寄存器的值，将satp设置为新的页表的对应地址并flush快表。之后便可`ret`返回。如果在此处忘记掉刷新快表数据，会导致不同线程之间的地址映射出现混乱，从而导致不可预测的错误结果。
```assembly
    ld t0, 152(x11)
    csrrw x0, sepc, t0
    ld t0, 160(x11)
    csrrw x0, sstatus, t0
    ld t0, 168(x11)
    csrrw x0, sscratch, t0

    # load new page table
    ld t0, 176(x11) # t0 = VA (is a virtual address)
    li t1, 0xffffffe000000000 - 0x80000000
    sub t0, t0, t1 # t0 = pgd - PA2VA_OFFSET, t0 = PA( physical address )

    srli t0, t0, 12 # t0 = t0 >> 12, PPN = PA >> 12
    li t1, 8 << 60
    or t0, t0, t1
    csrrw x0, satp, t0

    sfence.vma zero, zero  # flush TLB
```


## 4.3 修改中断入口/返回逻辑 ( _trap ) 以及中断处理函数 （ trap_handler ）

### 栈切换
#### __dummy
4.2 中我们初始化时， thread_struct.sp保存了S-Mode sp，thread_struct.sscratch保存了U-Mode sp，因此在 S-Mode 切换到 U->Mode 的时候，我们只需要交换对应的寄存器的值即可。
切换过程如下所示，在切换时由于我们需要用到t0和t1这两个临时的寄存器，所以需要提前将它们的值存下来，并分别利用csrrs和add指令读出sscratch和sp寄存器中的值，将值交换之后重新写入到两个寄存器中，在写入完成后需要恢复t0和t1的值并调用sret以进入用户态程序。
```assembly
__dummy:
    # exchange the stack pointers
    # S-Mode -> U->Mode
    sd t0, -8(sp)
    sd t1, -16(sp)
    csrrs t0, sscratch, x0 # t0 = sscratch
    add t1, sp, x0 # t1 = sp
    add sp, t0, x0
    csrrw x0, sscratch, t1
    ld t0, -8(t1)
    ld t1, -16(t1)

    sret
```
#### _trap
_trap是进入中断之后的开始执行语句，对于ecall指令引起的中断，由于是U-Mode在切换到S-Mode过程中引发的，因此我们需要在handle其之前将sp赋为相应的S-Mode的地址，也就是说在_trap的首尾我们都需要做sp和sscratch切换的操作。
注意如果是内核线程(没有U-Mode Stack)触发了异常，则不需要进行切换。（内核线程的sp永远指向的S-Mode Stack，sscratch为0）这一点我们在对内核线程初始化时已经赋好值了。
下面是在_traps中的处理片段，在进入_traps之后需要先检测sscratch的值，如果为0，则不需要交换寄存器，否则需要交换sp和sscratch的值，然后进入到正常的中断处理流程。
```assembly
_traps:
    # exchange the stack pointers
    # U-Mode -> S->Mode
    sd t0, -8(sp)
    sd t1, -16(sp)
    csrrs t0, sscratch, x0 # t0 = sscratch
    add t1, sp, x0 # t1 = sp
    beqz t0, _exchange_sp_sscratch_end# if sscratch == 0, kernel-thread, end
    add sp, t0, x0
    csrrw x0, sscratch, t1
_exchange_sp_sscratch_end:
    ld t0, -8(t1)
    ld t1, -16(t1)
    ......
```
此外，由于`trap_handler`函数的参数增加了一个`pt_regs`，我们需要在调用之前将参数放到正确的位置。由于在保存参数时，我们是按顺序将x1-x31压栈的，因此这里只需要将sp的值赋给trap_handler的第三个参数即可。
```assembly
    csrr x10, scause
    csrr x11, sepc
    add  x12, sp, x0

    call trap_handler
```
### `struct pt_regs`的定义
`struct pt_regs`被用于在进入中断处理程序之前将所有寄存器的值传递到处理函数中，以便函数在需要的时候方便调用。因此，需要注意结构体的变量定义顺序需要和中断处理过程中的压栈顺序相符合。

```c
struct pt_regs {
    uint64 x[32];
    uint64 sepc;
    uint64 sstatus;
};
```

### 重新设计`trap_handler`
在本次实验的`trap_handler`中，我们需要处理的是新引入的异常，因此需要判断最高位为0的时候的scause值。对于U-Mode中的ecall指令，`scause`值为8，此时我们需要将从`regs`中取得的寄存器的值传入`syscall`函数用于处理对应类型的系统调用。(`regs`的赋值过程已经在上一节中介绍)

```c
void trap_handler(uint64_t scause, uint64_t sepc, struct pt_regs *regs) {
    // 通过 `scause` 判断trap类型
    // 如果是interrupt 判断是否是timer interrupt
    // 如果是timer interrupt 则打印输出相关信息, 并通过 `clock_set_next_event()` 设置下一次时钟终端
    // `clock_set_next_event()` 见 4.5 节
    // 其他interrupt / exception 可以直接忽略

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
        unsigned long exception_code = scause ; 
        //Environment call from U-mode
        if (exception_code == 8) {
            // call system call with the id saved in a7
            // a7 is x17
            syscall(regs, regs->x[17]);
        }

        
    }
}
```


## 4.4 添加系统调用

### 添加`syscall.c`文件
`syscall.c`文件中用于处理内核态有关系统调用相关的函数。由`syscall`函数和两个系统调用的实现组成。
#### `syscall`函数
`syscall`函数会在`trap_handler`中被调用，随之传入的还有所有寄存器的值与对应的系统调用ID
```c
uint64 syscall(struct pt_regs *regs, uint64 call_id)
{
    uint64 _ret;
    switch (call_id) {
        case SYS_WRITE:
            // arguments: fd, buf, count
            // arguments: a0, a1, a2
            _ret = sys_write(regs->x[10], regs->x[11], regs->x[12]);
            break;
        case SYS_GETPID:
            _ret = sys_getpid();
            break;

        default:
            break;
    }
    regs->x[10] = _ret;
    return _ret;
}
```

需要注意的是，在调用完syscall，并从`trap_handler`中返回之后，需要将恢复出的sepc进行自增操作，否则会一直卡在产生异常的指令无法继续执行。这些逻辑在_traps中执行。
```assembly
    ld x10, 256(sp)
    
    addi x10, x10, 4
    csrrw x0, sepc, x10 # restore sepc
```

#### 系统调用：SYS_WRITE
`sys_write`函数用于将`buf`中的数据写入到指定的输出流中，对于我们本次实验，fd为标准输出（1），即我们可以直接调用内核态的`printk`函数来将一个个字符按序输出到屏幕上，为了防止在buf中的某个字符是ASCII为0的结束字符，循环结束的判断条件加入了`buf[i] != 0`。最终函数将返回实际打印出的字符数。 

```c
uint64 sys_write(unsigned int fd, const char* buf, uint64 count){
    int total_out = 0;
    for(int i = 0; i < count && buf[i]; i++){
        printk("%c", buf[i]);
        total_out++;
    }
    return total_out;
}
```


#### 系统调用：SYS_GETPID
`sys_getpid()`函数用于获取当前线程的pid，由于我们在`syscall.c`中已经声明了当前线程的`current`指针，因此可以直接通过`current->pid`的写法获取pid。

```c
uint64 sys_getpid(){
    return current->pid;
}
```


## 4.5 修改 head.S 以及 start_kernel





