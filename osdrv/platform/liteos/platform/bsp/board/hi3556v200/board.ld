OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
MEMORY
{
    ram : ORIGIN = 0x80200000, LENGTH = 0x10000000
    sram : ORIGIN = 0x40000000, LENGTH = 0x1000
}
SECTIONS
{
    /DISCARD/ : { *(.comment .note) }
    .ram_vectors (0x80200000 + 0x00000000) : {
        __ram_vectors_vma = .;
        KEEP (*(.vectors))
    } > ram
    __ram_vectors_lma = LOADADDR(.ram_vectors);
}
