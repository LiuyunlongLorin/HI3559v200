#ifndef __N3_DRV_H__
#define __N3_DRV_H__

typedef enum VDEC_N3_FORMAT_RESOLUTION
{
    VDEC_N3_FMT_SH720,
    VDEC_N3_FMT_H960,

    VDEC_N3_FMT_SH720_27MHz,
    VDEC_N3_FMT_SH720_36MHz,

    VDEC_N3_FMT_720P,
    VDEC_N3_FMT_1080P,

    VDEC_N3_FMT_RES_AUTO,

} VDEC_N3_FORMAT_RESOLUTION;

typedef enum VDEC_N3_FORMAT_FPS
{
    VDEC_N3_FMT_NTSC,
    VDEC_N3_FMT_PAL,

    VDEC_N3_FMT_25P,
    VDEC_N3_FMT_30P,

    VDEC_N3_FMT_FPS_AUTO,

} VDEC_N3_FORMAT_FPS;

typedef enum VDEC_N3_CHIP
{
    VDEC_N3_CHIP_UNKNOWN = 0,

    VDEC_N3_CHIP_N1,
    VDEC_N3_CHIP_N2,
    VDEC_N3_CHIP_N3,

} VDEC_N3_CHIP;




typedef struct n3_init_param
{
    // app -> drv
    unsigned int devaddr;
    VDEC_N3_CHIP chip_def;
    VDEC_N3_FORMAT_RESOLUTION fmt_res;
    VDEC_N3_FORMAT_FPS fmt_fps;

} n3_init_param;

typedef struct n3_detect_param
{
    // app -> drv
    unsigned int devaddr;

    // app <- drv
    VDEC_N3_CHIP chip_def;
    unsigned int chip_id;

} n3_detect_param;

typedef struct n3_fmt_detect_param
{
    // app -> drv
    unsigned int devaddr;
    VDEC_N3_CHIP chip_def;

        // N2(6114)
    unsigned char b1_ec;
    unsigned char b1_ed;
    unsigned char b1_ee;
    unsigned char b1_ef;
    unsigned char b1_f0;

    // N3(6124B)
    int ch;
    unsigned char b0_d0;
    unsigned char b0_d1;
    unsigned char b0_d2;
    unsigned char b0_d3;
    unsigned char b0_b8;
    unsigned char b5to8_f0;

} n3_fmt_detect_param;

typedef struct n3_novideo_param
{
    // app -> drv
    unsigned int devaddr;
    VDEC_N3_CHIP chip_def;
    unsigned char ch;

    unsigned char b1_d8;

} n3_novideo_param;

typedef struct n3_960H_set_param
{
    // app -> drv
    unsigned int devaddr;
    VDEC_N3_CHIP chip_def;

    unsigned char channel_mode;

} n3_960H_set_param;

#define IOC_VDEC_INIT                   0x40
#define IOC_VDEC_DETECT_CHIP            0x41
#define IOC_VDEC_SELECT_CHANNEL         0x42

#define IOC_VDEC_READ_B1_ECtoF0         0x50
#define IOC_VDEC_READ_B1_D8             0x51

#define IOC_VDEC_NOVIDEO_READ_B0_B8     0x54

#endif
