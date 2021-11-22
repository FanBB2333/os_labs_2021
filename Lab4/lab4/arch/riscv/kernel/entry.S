    .section .text.entry
    .align 4

    .global __dummy
__dummy:
    # YOUR CODE HERE
    la x10, dummy
    csrrw x0, sepc, x10 # restore sepc

    sret


    .global __switch_to

__switch_to:
    # x10: prev task_struct
    # save state to prev process
    # YOUR CODE HERE
    sd ra, 40(x10)
    sd sp, 48(x10)
    sd s0, 56(x10)
    sd s1, 64(x10)
    sd s2, 72(x10)
    sd s3, 80(x10)
    sd s4, 88(x10)
    sd s5, 96(x10)
    sd s6, 104(x10)
    sd s7, 112(x10)
    sd s8, 120(x10)
    sd s9, 128(x10)
    sd s10, 136(x10)
    sd s11, 144(x10)



    # x11: next task_struct

    # restore state from next process
    # YOUR CODE HERE
    ld ra, 40(x11)
    ld sp, 48(x11)
    ld s0, 56(x11)
    ld s1, 64(x11)
    ld s2, 72(x11)
    ld s3, 80(x11)
    ld s4, 88(x11)
    ld s5, 96(x11)
    ld s6, 104(x11)
    ld s7, 112(x11)
    ld s8, 120(x11)
    ld s9, 128(x11)
    ld s10, 136(x11)
    ld s11, 144(x11)

    ret


    .globl _traps 
_traps:
    # YOUR CODE HERE
    # -----------

        # 1. save 32 registers and sepc to stack

    # addi sp, sp, -8
    # sd x2,  0(sp)

    addi sp, sp, -248
    addi sp, sp, -8

    sd x0,  248(sp)
    sd x1,  240(sp)
    sd x3,  232(sp)
    sd x4,  224(sp)
    sd x5,  216(sp)
    sd x6,  208(sp)
    sd x7,  200(sp)
    sd x8,  192(sp)
    sd x9,  184(sp)
    sd x10, 176(sp)
    sd x11, 168(sp)
    sd x12, 160(sp)
    sd x13, 152(sp)
    sd x14, 144(sp)
    sd x15, 136(sp)
    sd x16, 128(sp)
    sd x17, 120(sp)
    sd x18, 112(sp)
    sd x19, 104(sp)
    sd x20, 96(sp)
    sd x21, 88(sp)
    sd x22, 80(sp)
    sd x23, 72(sp)
    sd x24, 64(sp)
    sd x25, 56(sp)
    sd x26, 48(sp)
    sd x27, 40(sp)
    sd x28, 32(sp)
    sd x29, 24(sp)
    sd x30, 16(sp)
    sd x31, 8(sp)

    csrrs x10, sepc, x0 # get sepc to x10
    sd x10, 0(sp)
    # -----------

        # 2. call trap_handler
    # need to pass the paras
    csrr x10, scause
    csrr x11, sepc
    call trap_handler
    # -----------

        # 3. restore sepc and 32 registers (x2(sp) should be restore last) from stack
    ld x10, 0(sp)
    csrrw x0, sepc, x10 # restore sepc

    ld x0,  248(sp)
    ld x1,  240(sp)
    ld x3,  232(sp)
    ld x4,  224(sp)
    ld x5,  216(sp)
    ld x6,  208(sp)
    ld x7,  200(sp)
    ld x8,  192(sp)
    ld x9,  184(sp)
    ld x10, 176(sp)
    ld x11, 168(sp)
    ld x12, 160(sp)
    ld x13, 152(sp)
    ld x14, 144(sp)
    ld x15, 136(sp)
    ld x16, 128(sp)
    ld x17, 120(sp)
    ld x18, 112(sp)
    ld x19, 104(sp)
    ld x20, 96(sp)
    ld x21, 88(sp)
    ld x22, 80(sp)
    ld x23, 72(sp)
    ld x24, 64(sp)
    ld x25, 56(sp)
    ld x26, 48(sp)
    ld x27, 40(sp)
    ld x28, 32(sp)
    ld x29, 24(sp)
    ld x30, 16(sp)
    ld x31, 8(sp)

    addi sp, sp, 248
    addi sp, sp, 8
    # -----------

        # 4. return from trap
    # mret
    sret
    # -----------