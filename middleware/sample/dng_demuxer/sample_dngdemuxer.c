#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tiffio.h>
#include <stdlib.h>

#include "hi_dng.h"
#include "hi_dng_err.h"

static char* in_filename = NULL;

#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    if (argc < 2)
    {
        printf("usage: %s filename  \n", argv[0]);
        return -1;
    }

    in_filename  = argv[1];
    char cmd[64];

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8* pu8ThmBuffer = NULL;
    HI_U8* pu8ScreenBuffer = NULL;
    HI_MW_PTR handle = 0;
    HI_DNG_CONFIG_S stMuxerCfg;
    HI_DNG_DEMUXER_IMAGE_INFO_S stThmInfo;
    HI_DNG_DEMUXER_IMAGE_INFO_S  stScreenInfo;
    memset(&stMuxerCfg, 0x00, sizeof(HI_DNG_CONFIG_S));
    memset(&stThmInfo, 0x00, sizeof(HI_DNG_DEMUXER_IMAGE_INFO_S));
    memset(&stScreenInfo, 0x00, sizeof(HI_DNG_DEMUXER_IMAGE_INFO_S));
    //open dng
    snprintf(stMuxerCfg.aszFileName, DNG_MAX_FILE_NAME, "%s", in_filename);
    stMuxerCfg.enConfigType = HI_DNG_CONFIG_TYPE_DEMUXER;
    s32Ret = HI_DNG_Create(&handle, &stMuxerCfg);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_DNG_Create failed\n");
        goto ENV_DEL;
    }

    // get thm len and data buffer
    s32Ret = HI_DNG_GetImageInfo(handle, 0, &stThmInfo);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_DNG_GetImageInfo HI_DNG_DEMUXER_IMAGE_IFD0 failed\n");
        goto DEMUXER_DESTROY;
    }

    printf("thm datalen %d width %d height %d \n", stThmInfo.u32DataLen, stThmInfo.u32Width, stThmInfo.u32Height);
    pu8ThmBuffer = (HI_U8*)malloc(stThmInfo.u32DataLen);
    if (NULL == pu8ThmBuffer)
    {
        printf("pu8ThmBuffer malloc failed\n");
        goto DEMUXER_DESTROY;
    }
    memset(pu8ThmBuffer, 0x00, stThmInfo.u32DataLen);

    s32Ret = HI_DNG_GetImageData(handle, pu8ThmBuffer, stThmInfo.u32DataLen);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_DNG_GetImageData failed\n");
        goto THM_REL;
    }
    printf("already get thm data \n");

    // get screennail len and data buffer
    s32Ret = HI_DNG_GetImageInfo(handle, 2, &stScreenInfo);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_DNG_GetImageInfo HI_DNG_DEMUXER_IMAGE_SUBSCREEN failed\n");
        goto THM_REL;
    }
    printf("screennail datalen %d width %d height %d \n", stScreenInfo.u32DataLen, stScreenInfo.u32Width, stScreenInfo.u32Height);


    pu8ScreenBuffer = (HI_U8*)malloc(stScreenInfo.u32DataLen);
    if (NULL == pu8ScreenBuffer)
    {
        printf("pu8ScreenBuffer malloc failed\n");
        goto THM_REL;
    }
    memset(pu8ScreenBuffer, 0x00, stScreenInfo.u32DataLen);

    s32Ret = HI_DNG_GetImageData(handle, pu8ScreenBuffer, stScreenInfo.u32DataLen);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_DNG_GetImageData HI_DNG_DEMUXER_IMAGE_SUBSCREEN failed\n");
        goto SCREEN_REL;
    }


    //close
    printf("please 'quit' the case\n");

    while (fgets(cmd, 10, stdin))
    {
        if (strncmp(cmd, "quit", 4) == 0 )
        {
            break;
        }
    }

    //save screen and free screen buffer
    FILE* screenFile = fopen("screennail.jpg", "wb");
    fwrite(pu8ScreenBuffer, stScreenInfo.u32DataLen, 1, screenFile);

    fclose(screenFile);
SCREEN_REL:
    if (pu8ScreenBuffer)
    {
        free(pu8ScreenBuffer);
        pu8ScreenBuffer = NULL;
    }

    //save thm and free thm buffer
    FILE* thumbFile = fopen("thumb.jpg", "wb");
    fwrite(pu8ThmBuffer, stThmInfo.u32DataLen, 1, thumbFile);
    fclose(thumbFile);

THM_REL:
    if (pu8ThmBuffer)
    {
        free(pu8ThmBuffer);
        pu8ThmBuffer = NULL;
    }

DEMUXER_DESTROY:
    HI_DNG_Destroy(handle);

ENV_DEL:
    printf("dng demuxer sample case finish\n");
    return 0;
}
