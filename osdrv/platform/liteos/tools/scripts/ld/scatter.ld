.fast_rodata ALIGN(0x1000) : { __fast_rodata_start = ABSOLUTE(.); . = .;
}

.fast_ctors ALIGN(0x4) : {
. = ALIGN(4); __init_array_start__ = ABSOLUTE(.); KEEP(*(SORT(.init_array.*))) KEEP(*(SORT(.init_array))) __init_array_end__ = ABSOLUTE(.);
}

__fast_rodata_end = ABSOLUTE(.);

/* Exception handling  */
PROVIDE (__EH_FRAME_BEGIN__ = LOADADDR(.eh_frame));
.eh_frame ALIGN (0x8) : { KEEP (*(.eh_frame)) *(.eh_frame.*);  } > ram

.fast_text ALIGN(0x4) : { __fast_text_start = ABSOLUTE(.); . = .;
}

__fast_text_end = ABSOLUTE(.);

.fast_data ALIGN(0x4) : { __fast_data_start = ABSOLUTE(.); . = .;
}

__fast_data_end = ABSOLUTE(.);
