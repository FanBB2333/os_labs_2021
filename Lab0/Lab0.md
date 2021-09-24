# Lab 0: GDB + QEMU 调试 64 位 RISC-V LINUX

**学号: 3190105838**

**姓名: 范钊瑀**

## 4.1 搭建Docker环境

使用`cat oslab.tar | docker import - oslab:2021`命令来导入打包好的image, 导入后会返回镜像的sha256值，可以使用`docker images`来查看本地存在的镜像

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210916104502188.png" alt="image-20210916104502188" style="zoom:33%;" />

再利用`docker run`来创建一个叫做`oslab0`的实例

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210916172040389.png" alt="image-20210916172040389" style="zoom:33%;" />

在exec状态中使用exit会直接关闭容器，此时我们可以使用docker start重新开启它，并可以用docker ps来观察正在运行中的容器

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210916172411376.png" alt="image-20210916172411376" style="zoom:33%;" />

可以使用docker exec 的it模式来指定终端进入交互界面，这样便可以进入容器内部进行交互

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210916172530043.png" alt="image-20210916172530043" style="zoom: 33%;" />

而在运行一个容器时，可以指定共享文件的目录，之后便可以在容器内部访问到本机存储的文件，如下图所示，容器内的have-fun-debugging 目录被映射到了本机个人目录下的某个文件夹，可以用ls命令来查看文件夹中的内容。

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210917191118765.png" alt="image-20210917191118765" style="zoom:25%;" />



## 4.2 获取 Linux 源码和已经编译好的文件系统

在容器内部本次实验的文件夹中下载最新的Linux源码，并clone实验手册中所给出的镜像文件

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210917191459284.png" alt="image-20210917191459284" style="zoom:33%;" />

如下图所示，我们已经准备好了Linux源码和需要clone的镜像

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210917192427439.png" alt="image-20210917192427439" style="zoom:33%;" />

## 4.3 编译 linux 内核

在Linux内核的目录下执行make命令

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210917192752143.png" alt="image-20210917192752143" style="zoom:25%;" />

出现如下图这样的标志便代表着可以内核编译完成

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210917200133930.png" alt="image-20210917200133930" style="zoom: 33%;" />



再用qemu加载编译好的内核文件以启动系统，出现OpenSBI界面即代表启动成功

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210917201701385.png" alt="image-20210917201701385" style="zoom: 25%;" />



再启动gdb来调试内核，为了方便观察，在这里使用了`tmux`同时开启左右两个终端窗口方便观察，这里注意要加上`-s -S`参数，否则内核会直接进行载入。可以看到在输入qemu的启动命令后，内核并没有像之前那样直接启动，而是在等待`gdb`

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210917205632503.png" alt="image-20210917205632503" style="zoom:20%;" />





## 4.4 使用gdb对内核进行调试

可以使用b(即break)命令来在某个函数处添加断点，如下图所示，由于此时还未运行，故左侧屏幕中看不到内核启动的输出

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210919110630273.png" alt="image-20210919110630273" style="zoom:20%;" />



可以使用c(continue)命令来继续执行程序，在`start_kernel`函数处我们遇到了第一个断点

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210919111352863.png" alt="image-20210919111352863" style="zoom:20%;" />

继续执行，直到遇到`boot_cpu_init()`函数，再使用`backtrace`查看函数调用情况，不难发现是`start_kernel()`函数作为caller，它call了`boot_cpu_init()`函数

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210919113043078.png" alt="image-20210919113043078" style="zoom:33%;" />



`frame`表示栈帧，可以和`info`函数一起调用作为查询程序当前的堆栈帧信息，其中包含栈帧指针地址(`frame`)，当前`pc`地址，参数、局部变量的起始地址，caller的地址等等。如下图为执行`info frame`(简写为`i frame`)的输出

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210919131448929.png" alt="image-20210919131448929" style="zoom:33%;" />



使用`step instruction`指令可以将指令单步执行，如下图所示，我们能看到每条指令的地址。

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210919150355563.png" alt="image-20210919150355563" style="zoom: 33%;" />



也可以使用`finish`语句来结束当前函数

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210919150648541.png" alt="image-20210919150648541" style="zoom:33%;" />



`layout`用于查看不同的界面，例如可以使用`layout asm`查看执行到的汇编语句，当然也可以查看`regs`(寄存器), `src`(源码)等等。

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210919151724494.png" alt="image-20210919151724494" style="zoom: 33%;" />

由于编译时没有加`-g`参数，这里无法显示局部变量的值

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210919154731021.png" alt="image-20210919154731021" style="zoom:33%;" />

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210919154743980.png" alt="image-20210919154743980" style="zoom:33%;" />

当然，为了方便调试，我们可以将断点保存下来，下次可以用`source`来读取

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210919150208465.png" alt="image-20210919150208465" style="zoom: 33%;" />



## 思考题

1. 使用 `riscv64-unknown-elf-gcc` 编译单个 `.c` 文件
使用`riscv64-unknown-elf-gcc -o q1.out q1.c`命令来得到可执行的elf文件，测试编译用代码如下：

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210920120004736.png" alt="image-20210920120004736" style="zoom: 25%;" />

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210920115420524.png" alt="image-20210920115420524" style="zoom: 33%;" />

执行得到结果

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210920120252023.png" alt="image-20210920120252023" style="zoom:33%;" />

2. 使用 `riscv64-unknown-elf-objdump` 反汇编 1 中得到的编译产物，将过长的结果保存到文件中以便于后续查看，使用`riscv64-unknown-elf-objdump -d q1.out > asm.txt`命令反汇编，并找到main函数，我们能够发现压栈弹栈，分配变量空间，以及调用输出的过程。

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210920120730431.png" alt="image-20210920120730431" style="zoom:33%;" />





3. 调试 Linux 时:
   1. 在 GDB 中查看汇编代码
   2. 在 0x80000000 处下断点
   3. 查看所有已下的断点
   4. 在 d 处下断点
   5. 清除 0x80000000 处的断点
   6. 继续运行直到触发 0x80200000 处的断点
   7. 单步调试一次
   8. 退出 QEMU

我们可以用`layout asm`来查看汇编代码，要在指定位置添加断点则可以利用`break`指令，后接`*`和地址，可以利用`info`指令查看已经存在的断点，`delete`指令用于删除已经存在的断点，单步调试需要用到`next`命令，而在最后退出gdb的时候可以使用`quit`命令

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210920121951614.png" alt="image-20210920121951614" style="zoom:22%;" />





由于是第一次使用gdb，前面的实验中没有意识到之前编译的内核中没有开启调试信息，因此`vmlinux`文件中不存在`symbol table`信息，上网查阅得到原因后，重新利用`make menuconfig`图形化界面（如下图）加入了调试信息，并重新编译，才正确得到了`symbol table`（由于害怕是内核的问题，这里也重新下载了一遍内核s）

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210920160642601.png" alt="image-20210920160642601" style="zoom:15%;" />



此时添加断点的时候会输出`file`信息：

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210920163225170.png" alt="image-20210920163225170" style="zoom:20%;" />

再重新执行上面的操作：

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210920171920569.png" alt="image-20210920171920569" style="zoom:15%;" />

执行到达位于 0x80200000 的断点：

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210920172049299.png" alt="image-20210920172049299" style="zoom: 15%;" />

单步调试可以使用`next`，即`n`命令

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210920172613714.png" alt="image-20210920172613714" style="zoom:15%;" />

而退出gdb可以使用`quit`命令。

4. 使用 `make` 工具清除 Linux 的构建产物

在执行`make`的相同目录下执行`make clean`命令即可，注意这样会清除掉编译好的`vmlinux`等文件

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210920160756971.png" alt="image-20210920160756971" style="zoom:20%;" />

5. `vmlinux` 和 `Image` 的关系和区别是什么？

为了得到编译时的输出，我重新编译了一次linux内核，得到最后一屏的输出如下

Image是编译linux内核得到的镜像文件，而vmlinux作调试用，其中会包含`symbol table`等虽然与启动项无关，但是和调试有关的数据，从下图中我们也能发现，是先生成vmlinux，再通过`OBJCOPY`生成`Image`，经过查询发现`OBJCOPY`并不会改变文件的内容，而只是进行了格式的转换，也就是说`Image`是在`vmlinux`的基础上进行格式转换得到的，是不包含调试信息（诸如`symbol table`等）的。

<img src="/Users/l1ght/Library/Application Support/typora-user-images/image-20210920162308447.png" alt="image-20210920162308447" style="zoom:20%;" />

