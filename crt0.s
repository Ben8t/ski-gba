@ GBA CRT0 — minimal startup
@ Sets up stack, zeroes BSS, calls main.
@ Must be in ARM mode (not Thumb) for the ROM header vector.

    .section .text.crt0
    .arm
    .global _start

_start:
    b       rom_header_end      @ jump past the 192-byte ROM header area

    @ ── GBA ROM Header (fixed offsets) ──────────────────────────────────
    @ Nintendo logo (0x04 .. 0x9F): must contain exact Nintendo logo data.
    @ We zero-fill here; for real cartridge use we would embed it.
    @ For emulator (mGBA) this is not strictly checked.
    .fill   156, 1, 0           @ Nintendo logo placeholder
    .byte   0, 0, 0, 0          @ game title (12 chars, then zeros)
    .byte   0, 0, 0, 0
    .byte   0, 0, 0, 0
    .ascii  "SKIF"              @ game code (4 chars)
    .ascii  "BN"                @ maker code
    .byte   0x96                @ fixed value
    .byte   0x00                @ main unit code
    .byte   0x00                @ device type
    .fill   7, 1, 0             @ reserved
    .byte   0x00                @ software version
    .byte   0x00                @ complement check (filled by fixrom or ignored)
    .byte   0, 0                @ reserved

rom_header_end:
    @ ── System init ──────────────────────────────────────────────────────
    @ Set IRQ stack
    mov     r0, #0x12           @ IRQ mode
    msr     CPSR_c, r0
    ldr     sp, =0x03007FA0

    @ Set system/user stack
    mov     r0, #0x1F           @ system mode
    msr     CPSR_c, r0
    ldr     sp, =0x03008000

    @ Copy .data from ROM to IWRAM
    ldr     r0, =__data_start
    ldr     r1, =__data_end
    ldr     r2, =__data_load
1:  cmp     r0, r1
    ldrlt   r3, [r2], #4
    strlt   r3, [r0], #4
    blt     1b

    @ Zero .bss
    ldr     r0, =__bss_start
    ldr     r1, =__bss_end
    mov     r2, #0
2:  cmp     r0, r1
    strlt   r2, [r0], #4
    blt     2b

    @ Jump to C main
    bl      main

    @ Hang if main returns
2:  b       2b
