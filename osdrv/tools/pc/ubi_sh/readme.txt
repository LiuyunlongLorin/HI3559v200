The mkubiimg.sh tool is specialized for making UBIFS image.

Usage: mkubiimg.sh  Chip Pagesize  Blocksize  Dir  Size  Tool_Path  Res
  Chip            Chip. hi35xx
  Pagesize        NAND page size. 2k/4k/8k.
  Blocksize       NAND block size. 128k/256k/1M
  Dir             The directory you want to make ubifs
  Size            This ubifs partition size in NAND. 48M, ... 50M
  Tool_Path       The path of mkfs.ubifs and ubinize
  Res             Reserve ubiimg and ubifs both (1:Yes 0:No(default))

Example:
  mkubiimg.sh hi35xx 2k 128k osdrv/pub/rootfs 50M osdrv/pub/bin/pc 0

Here 3 kinds of components would be made by mkubiimg.sh.
  The middleware reserve by Res:
	.ubiimg: Which can not burn to the SPI Nand Flash immediatly, by can be
			update to MTD partition by ubiupdatevol tool.
	.ubicfg: The configuration file for .ubiimg into .ubifs.
  The target image:
	.ubifs : The UBIFS image burns to Nand Flash immediatly, this file is
			convert from .ubiimg by ubinize tool. 

Notes:
	While making UBIFS image, you need to specify it's LEB, LEB=blocksize-pagesize*2
 if nand flash has not subpage; LEB=blocksize-pagesize if the nand flash has subpage.
 The catch: All nand flashs in "Flash Compatibility List" have not subpage. 
