
.extern start_kernel

    .section .text.init
    .globl _start
    # .align 4
_start:
        la  sp, boot_stack_top
        
        call setup_vm
        call relocate


        call mm_init
        call setup_vm_final
    # YOUR CODE HERE
        addi sp, sp, -8
        sd x10, 0(sp)
    # ------------------

        # set stvec = _traps

        la x10, _traps
        csrw stvec, x10
        # csrrw x0, mtvec, x10

    # ------------------

        # set sie[STIE] = 1

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

        # comment on lab5-4.5
        # li   x10, (1 << 1)
        # csrw sstatus, x10

        ld x10, 0(sp)
        addi sp, sp, 8
    # ------------------

    call task_init

    jal x0, start_kernel


    .global relocate
relocate:
    # set ra = ra + PA2VA_OFFSET
    # set sp = sp + PA2VA_OFFSET (If you have set the sp before)
    li  a0, 0xffffffe000000000 - 0x80000000
    add ra, ra, a0
    add sp, sp, a0

    # set satp with early_pgtbl
    #  63      60 59                  44 43                                0
    #  ---------------------------------------------------------------------
    # |   MODE   |         ASID         |                PPN                |
    #  ---------------------------------------------------------------------

    add t0, x0, x0
    add t1, x0, x0
    // mode field: 8, PPN field: early_pgtbl >> 12
    li   t0, 8
    slli t0, t0, 60 
    // t0 = (8 << 60) 
    la  t1, early_pgtbl
    // Shift Right Logical Immediate
    srli t1, t1, 12
    // t1 = early_pgtbl >> 12
    or    t0, t0, t1
    csrrw x0, satp, t0
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