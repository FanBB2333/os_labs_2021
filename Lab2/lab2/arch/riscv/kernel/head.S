
.extern start_kernel

    .section .text.init
    .globl _start
_start:
    # YOUR CODE HERE

    # ------------------

        # set stvec = _traps

    # ------------------

        # set sie[STIE] = 1

    # ------------------

        # set first time interrupt

    # ------------------

        # set sstatus[SIE] = 1

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