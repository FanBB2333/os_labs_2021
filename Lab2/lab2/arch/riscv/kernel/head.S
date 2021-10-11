
.extern start_kernel

    .section .text.init
    .globl _start
    .align 4
_start:
    # YOUR CODE HERE
        la  sp, boot_stack_top

    # ------------------

        # set stvec = _traps

        la x10, _traps
        csrw stvec, x10
        # csrrw x0, mtvec, x10

    # ------------------

        # set sie[STIE] = 1
        # csrrsi x0, sie, 32
        # addi x10, x0, 32


        li   x10, (1 << 5) | (1 << 1) | (1 << 9)
        csrw sie, x10
        # 32 : 100000





    # ------------------

        # set first time interrupt
        call clock_set_next_event
    # ------------------

        # set sstatus[SIE] = 1
        # csrrsi x0, sstatus, 2
        li   x10, (1 << 8) | (1 << 5) | (1 << 1)
        csrw sstatus, x10

    # ------------------

    jal x0, start_kernel

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space  4096 # <-- change to your stack size
    # assign 4KB stack
    .globl boot_stack_top
boot_stack_top: