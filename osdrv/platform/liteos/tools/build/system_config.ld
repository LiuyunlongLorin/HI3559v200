. = ALIGN(8); __block_systcb_start = .; __systcb_start = .; KEEP(*(.systcb)); __systcb_end = .; . += (__systcb_end - __systcb_start)*((64 + 1) - 1); __block_systcb_end = .;
. = ALIGN(8); __block_sysmemused_start = .; __sysmemused_start = .; KEEP(*(.sysmemused)); __sysmemused_end = .; . += (__sysmemused_end - __sysmemused_start)*((64 + 1) - 1); __block_sysmemused_end = .;
. = ALIGN(8); __block_sysqcb_start = .; __sysqcb_start = .; KEEP(*(.sysqcb)); __sysqcb_end = .; . += (__sysqcb_end - __sysqcb_start)*(1024 - 1); __block_sysqcb_end = .;
. = ALIGN(8); __block_sysmcb_start = .; __sysmcb_start = .; KEEP(*(.sysmcb)); __sysmcb_end = .; . += (__sysmcb_end - __sysmcb_start)*(1024 - 1); __block_sysmcb_end = .;
. = ALIGN(8); __block_sysscb_start = .; __sysscb_start = .; KEEP(*(.sysscb)); __sysscb_end = .; . += (__sysscb_end - __sysscb_start)*(1024 - 1); __block_sysscb_end = .;
. = ALIGN(8); __block_sysswtcb_start = .; __sysswtcb_start = .; KEEP(*(.sysswtcb)); __sysswtcb_end = .; . += (__sysswtcb_end - __sysswtcb_start)*(1024 - 1); __block_sysswtcb_end = .;
. = ALIGN(8); __block_pthdata_start = .; __pthdata_start = .; KEEP(*(.pthdata)); __pthdata_end = .; . += (__pthdata_end - __pthdata_start)*(64 - 1); __block_pthdata_end = .;
. = ALIGN(8); __block_mqcb_start = .; __mqcb_start = .; KEEP(*(.mqcb)); __mqcb_end = .; . += (__mqcb_end - __mqcb_start)*(1024 - 1); __block_mqcb_end = .;
. = ALIGN(8); __block_errno_start = .; __errno_start = .; KEEP(*(.errno)); __errno_end = .; . += (__errno_end - __errno_start)*((64 + 1) - 1); __block_errno_end = .;
