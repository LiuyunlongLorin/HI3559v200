Usage:
./fmc_nand_product      inputfile       outputfile      pagetype        ecctype oobsize yaffs   randomizer      pagenum/block   save_pin:
For example:             |               |               |               |       |       |       |               |               |
./fmc_nand_product      in_2k4b.yaffs   out_2k4b.yaffs   0               1       64      1       0               64              0
Page type Page size:
Input file:
Output file:
Pagetype:
0        2KB
1        4KB
ECC type ECC size:
1        4bit/512B
2        16bit/1K
3        24bit/1K
Chip OOB size:
yaffs2 image format:
0        NO
1        YES
Randomizer:
0        randomizer_disabled
1        randomizer_enabled
Pages_per_block:
64       64pages/block
128      128pages/block
Save Pin Mode:
0        disable
1        enable
