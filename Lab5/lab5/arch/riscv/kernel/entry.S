    .section .text.entry
    .align 4

    .global __dummy
__dummy:
    # la x10, dummy
    # csrrw x0, sepc, x10 # restore sepc
    
    # exchange the stack pointers
    # S-Mode -> U->Mode
    csrrs x10, sscratch, x0 # x10 = sscratch
    add x11, sp, x0 # x11 = sp
    add sp, x10, x0
    csrrw x0, sscratch, x11


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
    
    // save sepc, sstatus, sscratch
    csrrs t0, sepc, x0 # csrr t0, sepc
    sd t0, 152(x10)
    csrrs t0, sstatus, x0 # csrr t0, sstatus
    sd t0, 160(x10)
    csrrs t0, sscratch, x0 # csrr t0, sscratch
    sd t0, 168(x10)

    # store new page table
    csrrs t0, satp, x0 # csrr t0, satp, t0 = satp

    li t1, 0xfffffffffff
    and t0, t0, t1 # t0 = satp & 0xfffffffffff, lowest 44 bits(PPN, but is physical address)
    slli t0, t0, 12 # t0 = t0 << 12, PA = PPN << 12 ( physical address PA)

    li t1, 0xffffffe000000000 - 0x80000000
    add t0, t0, t1 # t0 = PA + PA2VA_OFFSET
    sd t0, 176(x10)



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

    ld t0, 152(x11)
    csrrw x0, sepc, t0
    ld t0, 160(x11)
    csrrw x0, sstatus, t0
    ld t0, 168(x11)
    csrrw x0, sscratch, t0

    # load new page table
    ld t0, 176(x11) # t0 = PA (is a virtual address)
    li t1, 0xffffffe000000000 - 0x80000000
    sub t0, t0, t1 # t0 = pgd - PA2VA_OFFSET, t0 = PA( physical address )

    srli t0, t0, 12 # t0 = t0 >> 12, PPN = PA >> 12
    li t1, 8 << 60
    or t0, t0, t1
    csrrw x0, satp, t0

    ret


    .globl _traps 
_traps:
    # 1. save 32 registers and sepc to stack

    # addi sp, sp, -8
    # sd x2,  0(sp)

    # TODO: test the sequence
    addi sp, sp, -264
    addi sp, sp, -8

    sd x0,  0(sp)
    sd x1,  8(sp)
    sd x2,  16(sp)
    sd x3,  24(sp)
    sd x4,  32(sp)
    sd x5,  40(sp)
    sd x6,  48(sp)
    sd x7,  56(sp)
    sd x8,  64(sp)
    sd x9,  72(sp)
    sd x10, 80(sp)
    sd x11, 88(sp)
    sd x12, 96(sp)
    sd x13, 104(sp)
    sd x14, 112(sp)
    sd x15, 120(sp)
    sd x16, 128(sp)
    sd x17, 136(sp)
    sd x18, 144(sp)
    sd x19, 152(sp)
    sd x20, 160(sp)
    sd x21, 168(sp)
    sd x22, 176(sp)
    sd x23, 184(sp)
    sd x24, 192(sp)
    sd x25, 200(sp)
    sd x26, 208(sp)
    sd x27, 216(sp)
    sd x28, 224(sp)
    sd x29, 232(sp)
    sd x30, 240(sp)
    sd x31, 248(sp)

    csrrs x10, sepc, x0 # get sepc to x10
    sd x10, 256(sp)
    csrrs x10, sstatus, x0 # get sstatus to x10
    sd x10, 264(sp)

    add x12, sp, x0 # x12 = sp, temp save sp
    # -----------
    jal ra, _exchange_sp_sscratch

    # 2. call trap_handler
    # need to pass the paras
    csrr x10, scause
    csrr x11, sepc

    call trap_handler
    # -----------

    jal ra, _exchange_sp_sscratch

    # 3. restore sepc and 32 registers (x2(sp) should be restore last) from stack
    ld x10, 256(sp)
    csrrw x0, sepc, x10 # restore sepc
    ld x10, 264(sp)
    csrrw x0, sstatus, x10 # restore sstatus

    sd x0,  0(sp)
    sd x1,  8(sp)
    # sd x2,  16(sp)
    sd x3,  24(sp)
    sd x4,  32(sp)
    sd x5,  40(sp)
    sd x6,  48(sp)
    sd x7,  56(sp)
    sd x8,  64(sp)
    sd x9,  72(sp)
    sd x10, 80(sp)
    sd x11, 88(sp)
    sd x12, 96(sp)
    sd x13, 104(sp)
    sd x14, 112(sp)
    sd x15, 120(sp)
    sd x16, 128(sp)
    sd x17, 136(sp)
    sd x18, 144(sp)
    sd x19, 152(sp)
    sd x20, 160(sp)
    sd x21, 168(sp)
    sd x22, 176(sp)
    sd x23, 184(sp)
    sd x24, 192(sp)
    sd x25, 200(sp)
    sd x26, 208(sp)
    sd x27, 216(sp)
    sd x28, 224(sp)
    sd x29, 232(sp)
    sd x30, 240(sp)
    sd x31, 248(sp)

    addi sp, sp, 264
    addi sp, sp, 8
    # -----------
    # 4. return from trap
    sret
    # -----------

_exchange_sp_sscratch:
    // TODO: add the save procedure of t0 and t1 regs
    # exchange the stack pointers
    # x-Mode -> x->Mode
    csrrs t0, sscratch, x0 # t0 = sscratch
    beq t0, x0, _exchange_sp_sscratch_end# if sscratch == 0, kernel-thread, end
    add t1, sp, x0 # t1 = sp
    add sp, t0, x0
    csrrw x0, sscratch, t1
_exchange_sp_sscratch_end:
    ret
