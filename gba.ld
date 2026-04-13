OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY {
    rom   (rx)  : ORIGIN = 0x08000000, LENGTH = 32M
    iwram (wx)  : ORIGIN = 0x03000000, LENGTH = 32K
    ewram (wx)  : ORIGIN = 0x02000000, LENGTH = 256K
}

SECTIONS {
    /* Code and read-only data in ROM */
    .text : {
        *(.text.crt0)
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
        __data_load = .;     /* where .data is stored in ROM */
    } > rom

    /* Initialized data: stored in ROM, runs in IWRAM */
    .data : AT(__data_load) {
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > iwram

    /* BSS in IWRAM — zero-initialized, no ROM storage */
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > iwram
}
