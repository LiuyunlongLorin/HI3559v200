#define ENTRY(name) \
  .globl name; \
  ALIGN; \
  name:

#define END(name) \
  .size name, .-name

#define ALIGN .align 0
#define ALIGN_STR ".align 0"

#define ENDPROC(name) \
  .type name, %function; \
  END(name)

#define put_byte_1  lsl #8
#define put_byte_0      lsl #0
#define get_byte_2  lsr #16
#define get_byte_1  lsr #8
#define get_byte_0      lsl #0
#define push            lsl
#define pull            lsr
#define ASM_MACRO_START   .macro
#define ASM_MACRO_END       .endm
regR1 .req    r1
regR2 .req    r2
regR3 .req    r3
regR4 .req    r4
regR5 .req    r5


     .text

     ASM_MACRO_START  loadreg4l, reg1, reg2, reg3, reg4
    ldmia   r0!, {\reg1, \reg2, \reg3, \reg4}   @ load addr in r0 to reg1 to reg4
    ASM_MACRO_END

    ASM_MACRO_START  load_regs
    ldmfd   sp!, {r1, r4 - r8, pc}  @ load stack to r1 r4 - r8 and pc
    ASM_MACRO_END

    ASM_MACRO_START  save_regs
    stmfd   sp!, {r1, r4 - r8, lr}  @ save r1 r4-r8 and lr to stack
    ASM_MACRO_END

    ASM_MACRO_START  loadreg1l, reg1
    ldr \reg1, [r0], #4
    ASM_MACRO_END

    ASM_MACRO_START  loadreg2l, reg1, reg2
    ldr \reg1, [r0], #4
    ldr \reg2, [r0], #4
    ASM_MACRO_END

    ASM_MACRO_START  loadreg1b, reg1
    ldrb    \reg1, [r0], #1 @ load 1 + address in r0 to reg1
    ASM_MACRO_END

    ASM_MACRO_START  loadreg2b, reg1, reg2
    ldrb    \reg1, [r0], #1
    ldrb    \reg2, [r0], #1
   ASM_MACRO_END


#define FN_ENTRY    ENTRY(csum_partial_copy_nocheck)
#define FN_EXIT     ENDPROC(csum_partial_copy_nocheck)

.Lclean0:     mov r0, r3
        load_regs


.Ldest_unaligned:
        tst regR1, #1
        beq .Ldest_16bit

        loadreg1b  ip
        sub regR2, regR2, #1
        adcs    regR3, regR3, ip, put_byte_1
        strb    ip, [regR1], #1
        tst regR1, #2
        moveq   pc, lr

.Ldest_16bit:    loadreg2b  r8, ip
        sub regR2, regR2, #2
        adcs    regR3, regR3, r8, put_byte_0
        strb    r8, [regR1], #1
        adcs    regR3, regR3, ip, put_byte_1
        strb    ip, [regR1], #1
        mov pc, lr


.Lless_than_8:
        teq regR2, #0
        beq .Lclean0

        tst regR1, #1
        beq .Lless_than_8_aligned

        loadreg1b  ip
        sub regR2, regR2, #1
        adcs    regR3, regR3, ip, put_byte_1
        strb    ip, [regR1], #1
        tst regR2, #6
        beq .Lless_than_8_byte_only

1:      loadreg2b  r8, ip
        sub regR2, regR2, #2
        adcs    regR3, regR3, r8, put_byte_0
        strb    r8, [regR1], #1
        adcs    regR3, regR3, ip, put_byte_1
        strb    ip, [regR1], #1

.Lless_than_8_aligned:
        tst regR2, #6
        bne 1b
.Lless_than_8_byte_only:
        tst regR2, #1
        beq .Lfinished
        loadreg1b  r8
        adcs    regR3, regR3, r8, put_byte_0
        strb    r8, [regR1], #1
        b   .Lfinished


FN_ENTRY
        save_regs

        cmp regR2, #8
        blo .Lless_than_8

        adds    regR3, regR3, #0
        tst regR1, #3
        blne    .Ldest_unaligned

        tst r0, #3                 @test source if it is not aligned
        bne .Lsource_not_aligned

        /* if source and dest is aligned, go on */
        bics    ip, regR2, #15
        beq 12f

11:     loadreg4l  r4, r5, r6, r7
        stmia   regR1!, {r4, r5, r6, r7}
        adcs    regR3, regR3, regR4
        adcs    regR3, regR3, regR5
        adcs    regR3, regR3, r6
        adcs    regR3, regR3, r7
        sub ip, ip, #16
        teq ip, #0
        bne 11b

12:     ands    ip, regR2, #12
        beq 14f
        tst ip, #8
        beq 13f
        loadreg2l  regR4, regR5
        stmia   regR1!, {regR4, regR5}
        adcs    regR3, regR3, regR4
        adcs    regR3, regR3, regR5
        tst ip, #4
        beq 14f

13:     loadreg1l  regR4
        str regR4, [regR1], #4
        adcs    regR3, regR3, regR4

14:     ands    regR2, regR2, #3
        beq .Lfinished
        loadreg1l  regR4
        tst regR2, #2
        mov regR5, regR4, get_byte_0
        beq .Lquit
        adcs regR3, regR3, regR4, push #16  @update checksum
        strb regR5, [regR1], #1
        mov regR5, regR4, get_byte_1
        strb regR5, [regR1], #1
        mov regR5, regR4, get_byte_2

        /* all branches will go to Lquit to exit this program */
.Lquit:
        tst regR2, #1
        strneb  regR5, [regR1], #1
        andne   regR5, regR5, #255
        adcnes  regR3, regR3, regR5, put_byte_0


        /* we have to rotate the checksum if the dest pointer was not 16 bit aligned */
.Lfinished:
        adc r0, regR3, #0
        ldr regR3, [sp, #0]       @ dest
        tst regR3, #1
        movne   r0, r0, ror #8
        load_regs

        /* routines for source not aligned */
.Lsource_not_aligned:
        adc regR3, regR3, #0
        and ip, r0, #3
        bic r0, r0, #3
        loadreg1l  regR5
        cmp ip, #2
        beq .Lsource2_aligned    @go to source2_aligned if ip equal to 2
        bhi .Lsource3_aligned    @go to source3_aligned if ip larger than 2
        mov regR4, regR5, pull #8
        bics    ip, regR2, #15
        beq 22f                  @serch label 22 forward
21:     loadreg4l  r5, r6, r7, r8
        orr regR4, regR4, regR5, push #24
        mov regR5, regR5, pull #8
        orr regR5, regR5, r6, push #24
        mov r6, r6, pull #8
        orr r6, r6, r7, push #24
        mov r7, r7, pull #8
        orr r7, r7, r8, push #24
        stmia   regR1!, {r4, r5, r6, r7}
        adcs    regR3, regR3, regR4    @update checksum
        adcs    regR3, regR3, regR5
        adcs    regR3, regR3, r6
        adcs    regR3, regR3, r7
        mov regR4, r8, pull #8
        sub ip, ip, #16
        teq ip, #0
        bne 21b                 @serch label 21 forward
22:     ands    ip, regR2, #12
        beq 24f
        tst ip, #8
        beq 23f
        loadreg2l  r5, r6
        orr regR4, regR4, regR5, push #24
        mov regR5, regR5, pull #8
        orr regR5, regR5, r6, push #24
        stmia   regR1!, {regR4, regR5}
        adcs    regR3, regR3, regR4
        adcs    regR3, regR3, regR5
        mov regR4, r6, pull #8
        tst ip, #4
        beq 24f
23:     loadreg1l  regR5
        orr regR4, regR4, regR5, push #24
        str regR4, [regR1], #4
        adcs    regR3, regR3, regR4
        mov regR4, regR5, pull #8
24:     ands    regR2, regR2, #3
        beq .Lfinished
        mov regR5, regR4, get_byte_0
        tst regR2, #2
        beq .Lquit
        adcs    regR3, regR3, regR4, push #16
        strb    regR5, [regR1], #1
        mov regR5, regR4, get_byte_1
        strb    regR5, [regR1], #1
        mov regR5, regR4, get_byte_2
        b   .Lquit

        /* routine for source larger than 16 bit aligned */
.Lsource3_aligned:
        mov regR4, regR5, pull #24
        adds    regR3, regR3, #0
        bics    ip, regR2, #15
        beq 32f                     @serch label 32 foward
31:     loadreg4l  r5, r6, r7, r8
        orr regR4, regR4, regR5, push #8
        mov regR5, regR5, pull #24
        orr regR5, regR5, r6, push #8
        mov r6, r6, pull #24
        orr r6, r6, r7, push #8
        mov r7, r7, pull #24
        orr r7, r7, r8, push #8
        stmia   regR1!, {r4, r5, r6, r7}
        adcs    regR3, regR3, regR4        @update checksum
        adcs    regR3, regR3, regR5
        adcs    regR3, regR3, r6
        adcs    regR3, regR3, r7
        mov regR4, r8, pull #24
        sub ip, ip, #16
        teq ip, #0
        bne 31b                     @serch label 31 behind
32:     ands    ip, regR2, #12
        beq 34f
        tst ip, #8
        beq 33f
        loadreg2l  r5, r6
        orr regR4, regR4, regR5, push #8
        mov regR5, regR5, pull #24
        orr regR5, regR5, r6, push #8
        stmia   regR1!, {regR4, regR5}
        adcs    regR3, regR3, regR4
        adcs    regR3, regR3, regR5
        mov regR4, r6, pull #24
        tst ip, #4
        beq 34f
33:     loadreg1l  regR5
        orr regR4, regR4, regR5, push #8
        str regR4, [regR1], #4
        adcs    regR3, regR3, regR4
        mov regR4, regR5, pull #24
34:     ands    regR2, regR2, #3
        beq .Lfinished
        mov regR5, regR4, get_byte_0
        tst regR2, #2
        beq .Lquit
        strb    regR5, [regR1], #1
        adcs    regR3, regR3, regR4
        loadreg1l  regR4
        mov regR5, regR4, get_byte_0
        strb    regR5, [regR1], #1
        adcs    regR3, regR3, regR4, push #24
        mov regR5, regR4, get_byte_1
        b   .Lquit

        /* routine for source 16 bit aligned */
.Lsource2_aligned:
        mov regR4, regR5, pull #16
        adds    regR3, regR3, #0
        bics    ip, regR2, #15
        beq 42f                     @serch label 42 foward
41:     loadreg4l  r5, r6, r7, r8
        orr regR4, regR4, regR5, push #16
        mov regR5, regR5, pull #16
        orr regR5, regR5, r6, push #16
        mov r6, r6, pull #16
        orr r6, r6, r7, push #16
        mov r7, r7, pull #16
        orr r7, r7, r8, push #16
        stmia   regR1!, {r4, r5, r6, r7}
        adcs    regR3, regR3, regR4        @update checksum
        adcs    regR3, regR3, regR5
        adcs    regR3, regR3, r6
        adcs    regR3, regR3, r7
        mov regR4, r8, pull #16
        sub ip, ip, #16
        teq ip, #0
        bne 41b                     @serch label 41 behind
42:     ands    ip, regR2, #12
        beq 44f
        tst ip, #8
        beq 43f
        loadreg2l  r5, r6
        orr regR4, regR4, regR5, push #16
        mov regR5, regR5, pull #16
        orr regR5, regR5, r6, push #16
        stmia   regR1!, {regR4, regR5}
        adcs    regR3, regR3, regR4
        adcs    regR3, regR3, regR5
        mov regR4, r6, pull #16
        tst ip, #4
        beq 44f
43:     loadreg1l  r5
        orr regR4, regR4, regR5, push #16
        str regR4, [regR1], #4
        adcs    regR3, regR3, regR4
        mov regR4, regR5, pull #16
44:     ands    regR2, regR2, #3
        beq .Lfinished
        mov regR5, regR4, get_byte_0
        tst regR2, #2
        beq .Lquit
        adcs    regR3, regR3, regR4
        strb    regR5, [regR1], #1
        mov regR5, regR4, get_byte_1
        strb    regR5, [regR1], #1
        tst regR2, #1
        beq .Lfinished
        loadreg1b  r5
        b   .Lquit


FN_EXIT

