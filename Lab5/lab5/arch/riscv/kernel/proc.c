//arch/riscv/kernel/proc.c
#include "proc.h"
#include "printk.h"
#include "defs.h"
#include "mm.h"
#include "rand.h"
#include "vm.h"


extern void __dummy();
extern void __switch_to(struct task_struct* prev, struct task_struct* next);


struct task_struct* idle;           // idle process
struct task_struct* current;        // 指向当前运行线程的 `task_struct`
struct task_struct* task[NR_TASKS]; // 线程数组，所有的线程都保存在此
extern unsigned long  swapper_pg_dir[512];

void task_init() {
    // 1. 调用 kalloc() 为 idle 分配一个物理页
    // 2. 设置 state 为 TASK_RUNNING;
    // 3. 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
    // 4. 设置 idle 的 pid 为 0
    // 5. 将 current 和 task[0] 指向 idle
    /* YOUR CODE HERE */
    idle = (struct task_struct *)kalloc();
    idle->state = TASK_RUNNING;
    idle->counter = 0;
    idle->priority = 0;
    idle->pid = 0;    
    current = idle;
    task[0] = idle;
    // assign U-Mode Stack


    // 1. 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
    // 2. 其中每个线程的 state 为 TASK_RUNNING, counter 为 0, priority 使用 rand() 来设置, pid 为该线程在线程数组中的下标。
    // 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`,
    // 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址， `sp` 设置为 该线程申请的物理页的高地址

    /* YOUR CODE HERE */
    for(int i = 1; i < NR_TASKS; i++){
        task[i] = (struct task_struct *)kalloc();
        task[i]->state = TASK_RUNNING;
        task[i]->counter = 0;
        task[i]->priority = rand();
        task[i]->pid = i; 
        task[i]->thread.ra = (uint64)&__dummy;
        printk("i: %d, pri: %d, ra: %lx\n", i, task[i]->priority, task[i]->thread.ra);

        task[i]->thread.sp = (char *)((uint64)task[i] + (uint64)PGSIZE);
        // SPP SPIE SUM
        task[i]->thread.sstatus = (1L << 8) | (1L << 5) | (1L << 18);
        task[i]->thread.sepc = (uint64_t)USER_START;
        task[i]->thread.sscratch = (uint64_t)USER_END;


        // assign a U-Mode Stack
        task[i]->thread_info->user_sp = kalloc();


        // Copy swapper_pg_dir to the user pagetable

        int a=1;
        task[i]->pgd = kalloc();
        // memcpy(task[i]->pgd, swapper_pg_dir, sizeof(uint64) * 512);
        for (int j = 0; j < 512; j++){
            task[i]->pgd[j] = swapper_pg_dir[j];
        }
        
        //X|W|R|V
        int perm = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
        create_mapping(task[i]->pgd, PA2VA(USER_START), VA2PA((uint64)uapp_start), (uint64)uapp_end - (uint64)uapp_start, perm);

    }


    printk("...proc_init done!\n");
}

// arch/riscv/kernel/proc.c

void dummy() {
    uint64 MOD = 1000000007;
    uint64 auto_inc_local_var = 0;
    int last_counter = -1;
    printk("dummy start!\n");
    while(1) {
        if (last_counter == -1 || current->counter != last_counter) {
            last_counter = current->counter;
            auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
            printk("[PID = %d] is running. auto_inc_local_var = %d\n", current->pid, auto_inc_local_var);
        }
    }
}

void switch_to(struct task_struct* next) {
    /* YOUR CODE HERE */
    printk("switch from [PID = %d COUNTER = %d] to [PID = %d COUNTER = %d]\n", current->pid, current->counter, next->pid, next->counter);
    if(current->pid != next->pid) {
        struct task_struct * _tmp = current;
        current = next;
        __switch_to(_tmp, next);
    }
    else{
        // do nothing
    }
}

void do_timer(void) {
    /* 1. 如果当前线程是 idle 线程 直接进行调度 */
    /* 2. 如果当前线程不是 idle 对当前线程的运行剩余时间减 1 
          若剩余时间任然大于0 则直接返回 否则进行调度 */

    /* YOUR CODE HERE */
    if(current->pid == idle->pid){
        printk("current == idle\n");
        schedule();
    }
    else{
        (current->counter)--;
        if(current->counter > 0){
            return;
        }
        else{
            schedule();
        }

    }
}

int find_min_time(){
    int _min_time = -1;
    int _min_id = -1;
    for(int i = 0; i < NR_TASKS && task[i]->counter > 0; i++){
        if(task[i]->counter < _min_time){
            _min_time = task[i]->counter;
            _min_id = i;
        }
    }
    return _min_id;
}

// Implement SJF
#ifdef SJF
void schedule(void) {
    /* YOUR CODE HERE */

    int all_zeros = 1;
    int min_index = find_min_time();
    int min_time = (min_index == -1) ? -1 : task[min_index]->counter;

    printk("min_idx : %d, min_time : %d\n", min_index, min_time);
    printk("schedule\n");
    for(int i = 1; i < NR_TASKS; i++){
        if(task[i]->state == TASK_RUNNING){
            if(all_zeros && task[i]->counter > 0){
                all_zeros = 0;
            }
            if(task[i]->counter < min_time && task[i]->counter > 0){
                min_time = task[i]->counter;
                min_index = i;
            }

        }
    }
    printk("all_zeros: %d, min_time: %d, min_index: %d\n", all_zeros, min_time, min_index);
    if(all_zeros){
        for(int i = 1; i < NR_TASKS; i++){
            if(task[i]->state == TASK_RUNNING){
                task[i]->counter = rand();
                printk("SET [PID = %d COUNTER = %d]\n", task[i]->pid, task[i]->counter);

            }
        }
        // schedule();
        min_index = find_min_time();

        min_time = (min_index == -1) ? -1 : task[min_index]->counter;
        for(int i = 1; i < NR_TASKS; i++){
            if(task[i]->state == TASK_RUNNING){
                if(task[i]->counter < min_time){
                    min_time = task[i]->counter;
                    min_index = i;
                }

            }
        }
    }
    // schedule ith process
    printk("switch_to %d\n", min_index);
    switch_to(task[min_index]);
    
}
#endif

#ifdef PRIORITY
void schedule(void){
    /* YOUR CODE HERE */
    int all_zeros = 1;
    int max_index = -1;
    int max_priority = 0;
    printk("schedule\n");
    for(int i = 1; i < NR_TASKS; i++){
        // printk("i: %d, pri: %d\n", i, task[i]->priority);
        if(task[i]->state == TASK_RUNNING){
            if(all_zeros && task[i]->counter > 0){
                all_zeros = 0;
            }
            if(task[i]->priority > max_priority && task[i]->counter > 0){
                max_priority = task[i]->priority;
                max_index = i;
            }

        }
    }
    printk("all_zeros: %d, max_priority: %d, max_index: %d\n", all_zeros, max_priority, max_index);
    if(all_zeros){
        for(int i = 1; i < NR_TASKS; i++){
            if(task[i]->state == TASK_RUNNING){
                task[i]->counter = rand();
                printk("SET [PID = %d COUNTER = %d]\n", task[i]->pid, task[i]->counter);

            }
        }
        // schedule();
        max_index = -1;
        max_priority = 0;
        for(int i = 1; i < NR_TASKS; i++){
            if(task[i]->state == TASK_RUNNING && task[i]->counter > 0){
                printk("pri: %d, max_pri: %d, %d\n",task[i]->priority, max_priority, max_priority < (int)task[i]->priority );
                if(task[i]->priority > max_priority){
                    // printk("i: %d, pri: %d\n", i, task[i]->priority);
                    max_priority = task[i]->priority;
                    max_index = i;
                }

            }
        }
    }
    // schedule ith process
    printk("switch_to %d\n", max_index);
    switch_to(task[max_index]);
    
}

#endif
