
.extern start_kernel

    .section .text.init
    .globl _start
    # .align 4
_start:
        call setup_vm
        call relocate
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
        csrw sie, x10
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
        csrw sstatus, x10

        ld x10, 0(sp)
        addi sp, sp, 8
    # ------------------
    call mm_init
    call task_init

    jal x0, start_kernel

    .section .bss.stack
    .globl boot_stack


    .global relocate
relocate:
    # set ra = ra + PA2VA_OFFSET
    # set sp = sp + PA2VA_OFFSET (If you have set the sp before)

    ###################### 
    #   YOUR CODE HERE   #
    ######################

    # TODO: test whether there exists a negative mark before PA2VA_OFFSET
    add ra, ra, a0
    add sp, sp, a0

    # set satp with early_pgtbl
    #  63      60 59                  44 43                                0
    #  ---------------------------------------------------------------------
    # |   MODE   |         ASID         |                PPN                |
    #  ---------------------------------------------------------------------
    ###################### 
    #   YOUR CODE HERE   #
    ######################
    add t0, x0, x0
    // mode field: 8, PPN field: 0x80000000 >> 12
    li  t0, (8 << 60) | (0x80000000 >> 12)




    # flush tlb
    sfence.vma zero, zero

    ret

    .section .bss.stack
    .globl boot_stack


boot_stack:
    .space  4096 # <-- change to your stack size
    # assign 4KB stack
    .globl boot_stack_top
boot_stack_top: