
.extern start_kernel

    .section .text.init
    .globl _start
_start:
    # YOUR CODE HERE

    # ------------------

        # set stvec = _traps
        la x10, _traps
        add x10, x10, x10
        add x10, x10, x10
        csrrw x0, stvec, x10

    # ------------------

        # set sie[STIE] = 1
        # csrrsi x0, sie, 32
        addi x10, x0, 32
        csrrs x0, sie, x10
        # 32 : 100000

    # ------------------

        # set first time interrupt
        call clock_set_next_event
    # ------------------

        # set sstatus[SIE] = 1
        csrrsi x0, sstatus, 2

    # ------------------

    la  sp, boot_stack_top
    jal x0, start_kernel

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space  4096 # <-- change to your stack size
    # assign 4KB stack
    .globl boot_stack_top
boot_stack_top: