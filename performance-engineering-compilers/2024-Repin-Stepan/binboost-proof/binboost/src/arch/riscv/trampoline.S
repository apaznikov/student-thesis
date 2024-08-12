.align 6
.global bb_jit_begin
bb_jit_begin:
    add     sp, sp, -16
    sd      ra, 0(sp)
    sd      s0, 8(sp)
    mv      a0, ra
    call    bb_jit_begin_impl
    ld      ra, 0(sp)
    ld      s0, 8(sp)
    add     sp, sp, 16
    jr      a0
