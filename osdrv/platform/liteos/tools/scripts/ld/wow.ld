.wow_rodata ALIGN(0x4) : {
}

.wow_text ALIGN(0x4) : {
}

.wow_data ALIGN(0x4) : {
INCLUDE system_config.ld
}

__wow_bss_start = ABSOLUTE(.);

.wow_bss ALIGN(0x4) : {
}

__wow_bss_end = ABSOLUTE(.);
