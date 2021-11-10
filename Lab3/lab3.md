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
在这里我们需要修改`proc.c`文件的`task_init()`函数以初始化各个进程


### 4.3.2 __dummy 与 dummy 介绍

### 4.3.3 实现线程切换


### 4.3.4 实现调度入口函数

### 4.3.5 实现线程调度

#### 4.3.5.1 短作业优先调度算法

#### 4.3.5.2 优先级调度算法

## 4.4 编译及测试
