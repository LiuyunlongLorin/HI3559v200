
.text

regBUF .req    r0
regLEN .req    r1
regSUM .req    r2
regTD0 .req    r3
regTD1 .req    r4  @ save before use
regTD2 .req    r5  @ save before use
regTD3 .req    lr
regSP   .req    sp
regIP   .req     ip

.Lable0:
        mov r0, regSUM
        add regSP, regSP, #4
        ldr pc, [regSP], #4

@lable less 8
.Lable_L8:
        teq regLEN, #0
        beq .Lable0

        tst regBUF, #1         @ odd addr?
        movne   regSUM, regSUM, ror #8
        ldrneb  regTD0, [regBUF], #1
        subne   regLEN, regLEN, #1
        adcnes  regSUM, regSUM, regTD0, lsl #8

@lable less 4
.Lable_L4:
        tst regLEN, #6
        beq .Lable_L8Byte

.Lable_L8_word_loop:
        ldrh    regTD0, [regBUF], #2
        sub regLEN, regLEN, #2
        adcs    regSUM, regSUM, regTD0
        tst regLEN, #6
        bne .Lable_L8_word_loop

 @less 8 byte
.Lable_L8Byte:
        tst regLEN, #1
        ldrneb  regTD0, [regBUF], #1
        adcnes  regSUM, regSUM, regTD0, lsl #0
.Lable_done:
        adc r0, regSUM, #0
        ldr regTD0, [regSP], #4
        tst regTD0, #1
        movne   r0, r0, ror #8
        ldr pc, [regSP], #4        @ return

.Lable_not_align:
        tst regBUF, #1
        ldrneb  regTD0, [regBUF], #1
        subne   regLEN, regLEN, #1
        adcnes  regSUM, regSUM, regTD0, lsl #8

        tst regBUF, #2
        ldrneh  regTD0, [regBUF], #2
        subne   regLEN, regLEN, #2
        adcnes  regSUM, regSUM, regTD0
        mov pc, lr

.globl csum_partial;
.type csum_partial, function;
.align 4,0x90;
csum_partial:
        stmfd   regSP!, {regBUF, lr}
        cmp regLEN, #8
        blo .Lable_L8           @ lable less 8

        tst regBUF, #1
        movne   regSUM, regSUM, ror #8

        adds    regSUM, regSUM, #0
        tst regBUF, #3
        blne    .Lable_not_align

1:      bics    regIP, regLEN, #31
        beq 3f

        stmfd   regSP!, {r4 - r5}
2:      ldmia   regBUF!, {regTD0, regTD1, regTD2, regTD3}
        adcs    regSUM, regSUM, regTD0
        adcs    regSUM, regSUM, regTD1
        adcs    regSUM, regSUM, regTD2
        adcs    regSUM, regSUM, regTD3
        ldmia   regBUF!, {regTD0, regTD1, regTD2, regTD3}
        adcs    regSUM, regSUM, regTD0
        adcs    regSUM, regSUM, regTD1
        adcs    regSUM, regSUM, regTD2
        adcs    regSUM, regSUM, regTD3
        sub regIP, regIP, #32
        teq regIP, #0
        bne 2b
        ldmfd   regSP!, {r4 - r5}

3:      tst regLEN, #0x1c      @ should not change C
        beq .Lable_L4  @lable less 4

4:      ldr regTD0, [regBUF], #4
        sub regLEN, regLEN, #4
        adcs    regSUM, regSUM, regTD0
        tst regLEN, #0x1c
        bne 4b
        b   .Lable_L4 @lable less 4
.size csum_partial, .-csum_partial
