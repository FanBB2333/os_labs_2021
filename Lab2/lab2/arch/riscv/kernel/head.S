
.extern start_kernel

    .section .text.init
    .globl _start
    # .align 4
_start:
    # YOUR CODE HERE
        la  sp, boot_stack_top
        addi sp, sp, -8
        sd x10, 0(sp)
    # ------------------

        # set stvec = _traps

        la x10, _traps
        csrw stvec, x10
        # csrrw x0, mtvec, x10

    # ------------------

        # set sie[STIE] = 1
        # csrrsi x0, sie, 32
        # addi x10, x0, 32



        # li   x10, (1 << 5) 
        # csrw sip, x10

        li   x10, (1 << 5)
        csrs sie, x10
        # 32 : 100000




        ld x10, 0(sp)
        addi sp, sp, 8
    # ------------------

        # set first time interrupt
        call clock_set_next_event
    # ------------------
        addi sp, sp, -8
        sd x10, 0(sp)
        # set sstatus[SIE] = 1
        # csrrsi x0, sstatus, 2
        li   x10, (1 << 1)
        csrs sstatus, x10

        ld x10, 0(sp)
        addi sp, sp, 8
    # ------------------

    jal x0, start_kernel

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space  4096 # <-- change to your stack size
    # assign 4KB stack
    .globl boot_stack_top
boot_stack_top: