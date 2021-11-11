# Lab 3: RV64 内核线程调度

**学号: ** 3190105838

**姓名: **范钊瑀

## 4.1 准备工程
首先我们将Lab2中用于终端处理，系统初始化等部分的代码复用到本次实验中，并在定义头文件`defs.h`中加入有关的宏定义。相关的宏定义中包含

```c++
#define PHY_START 0x0000000080000000
#define PHY_SIZE  128 * 1024 * 1024 // 128MB， QEMU 默认内存大小
#define PHY_END   (PHY_START + PHY_SIZE)

#define PGSIZE 0x1000 // 4KB
#define PGROUNDUP(addr) ((addr + PGSIZE - 1) & (~(PGSIZE - 1)))
#define PGROUNDDOWN(addr) (addr & (~(PGSIZE - 1)))
```


## 4.2 proc.h 数据结构定义

添加`proc.h`文件用于存储基本数据类型的定义，`thread_struct `结构体用于存储保存下来的ra、sp、s0、s1等寄存器的值，`task_struct `用于保存线程的状态、运行时间等数据。

## 4.3 线程调度功能实现

### 4.3.1 线程初始化
在这里我们需要修改`proc.c`文件的`task_init()`函数以初始化各个线程，首先我们需要设置os最初的线程的`task_struct`，它本身是一个`idle`线程，在初始化的时候需要我们将`idle`指针指向它，同时由于它随着os的启动而运行，我们也要将`current`指针指向它。
利用`kalloc()`函数可以分配一个`task_struct`的连续地址，我们将这个地址作为`idle`线程的`task_struct`的首地址，因此将`idle`指针指向它。同理我们也需要对其状态`state`，`counter`，`priority`，`time_slice`，`pid`，进行初始化，其中`state`为`TASK_RUNNABLE`，代表线程在运行，可被调度，`counter`为0，因为我们想要立即开始其它线程的调度，`priority`和`pid`也可设为0。

```c
idle = (struct task_struct *)kalloc();
idle->state = TASK_RUNNING;
idle->counter = 0;
idle->priority = 0;
idle->pid = 0;    
current = idle;
task[0] = idle;
```
类似的，我们也应当对其他线程的`task_struct`进行初始化操作，在这里的循环中，每个`task`线程数组里的个体都被分配了一块地址空间，并且将其状态`state`，`counter`，`priority`，`pid`进行类似的初始化操作。此外我们也需要设置每个线程的`ra`和`sp`位，`ra`位被设置成函数`__dummy`的首地址，`__dummy`函数是用于线程第一次调度的一个特殊函数，它设置了`sepc`的值，并从中断中返回到`dummy()`函数的位置，由于创建完的一个新线程在第一次调度时是没有上下文需要恢复的，因此在第一次调度的时候我们需要将线程的返回地址“引导”到我们的循环`dummy()`函数中，做这件事的就是`__dummy`函数。而`sp`被设置为该线程申请的物理页的高地址。在这里需要注意的是，我们最好将`task[i]`和`PGSIZE`统一转化成uint64类型之后再求和，否则可能会产生加上了`PGSIZE`倍数的偏移量的后果。


```c
for(int i = 1; i < NR_TASKS; i++){
    task[i] = (struct task_struct *)kalloc();
    task[i]->state = TASK_RUNNING;
    task[i]->counter = 0;
    task[i]->priority = rand();
    task[i]->pid = i; 
    task[i]->thread.ra = (uint64)&__dummy;
    task[i]->thread.sp = ((uint64)task[i] + (uint64)PGSIZE);

}
```

### 4.3.2 __dummy 与 dummy 介绍

### 4.3.3 实现线程切换


### 4.3.4 实现调度入口函数

### 4.3.5 实现线程调度

#### 4.3.5.1 短作业优先调度算法

#### 4.3.5.2 优先级调度算法

## 4.4 编译及测试
