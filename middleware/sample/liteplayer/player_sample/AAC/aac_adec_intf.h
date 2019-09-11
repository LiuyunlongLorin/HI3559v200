#ifndef __AAC_ADEC_INTF_H__
#define __AAC_ADEC_INTF_H__

#include "hi_type.h"
#include "aacdec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


typedef struct _AAC_FRAME_INFO_S
{
    HI_S32 s32Samplerate;   /* sample rate*/
    HI_S32 s32BitRate;                  /* bitrate */
    HI_S32 s32ChnCnt;                  /* chn cnt*/
    HI_S32 s32Profile;                  /* profile*/
    HI_S32 s32Bitwith;                  /* bitwith*/
} HI_AAC_FRAME_INFO_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __AAC_ADEC_INTF_H__ */
