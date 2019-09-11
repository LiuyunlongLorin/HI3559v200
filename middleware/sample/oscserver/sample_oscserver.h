#include "hi_type.h"
#include "hi_mw_type.h"
#include "hi_osc_server.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#define OSCSVR_MANUFACTYRER_MAX_LEN        (32)
#define OSCSVR_MODEL_MAX_LEN               (32)
#define OSCSVR_SERIAL_NUMBER_MAX_LEN       (32)
#define OSCSVR_FIRMWARE_VERSION_MAX_LEN    (16)
#define OSCSVR_SUPPORT_URL_VERSION_MAX_LEN (128)
#define SAMPLE_OSC_STREAM_FRAME_LEN        (400*1024)
/*list file file name*/
#define OSCSAMPLE_FILES_NAME_MAX_LEN          (32)
#define OSCSAMPLE_FILES_COMMAND_MAX_LEN       (64)
/*Command id len*/
#define OSCSAMPLE_COMMAND_ID_MAX_LEN       (12)
/*live preview stream buff len*/
#define OSCSAMPLE_STREAM_BUFF_MAX_LEN      (4*1000*1000)

#define OSCSAMPLE_LISTFILES_NUMBER         (20)

typedef enum hi_OSC_STATE_E
{
    HIOSC_STATE_SERVICES_READY = 1,
    HIOSC_STATE_SERVICES_UNABLE = 2,
} HI_OSC_STATE_E;

typedef struct hiOSCSAMPLE_LIST_FILE_S
{
    /*input*/
    HI_CHAR aszFileType[HI_OSC_LISTFILE_FILETYPE_LEN];
    HI_CHAR aszFileName[HI_OSC_LISTFILE_FILE_NAME];               /*the name of image*/
    HI_CHAR aszFileURL[HI_OSC_LISTFILE_FILE_URL];                 /*the url of the image*/
    HI_CHAR aszDownURL[HI_OSC_LISTFILE_FILE_URL];                 /*the url of the image*/
    HI_S32 s32FileSize;                                          /*the size of the image in bytes*/
    HI_CHAR aszDateTimeZone[HI_OSC_LISTFILE_DATE_TIME];            /*time for image, use 24 hour format*/
    HI_DOUBLE dLat;                                               /*Latitude of the image capture location, optional*/
    HI_DOUBLE dLng;                                               /*Longitude of the image capture location, optional*/
    HI_S32 s32Width;                                              /*width of the image*/
    HI_S32 s32Height;                                             /*height of the image*/
    HI_CHAR aszThumbnail[HI_OSC_LISTFILE_THUMB_ENCODE];            /*Base64 encoded string for thumbnail image (when maxThumbSize != NULL)*/
    /*added in list file*/
    HI_BOOL bIsProcessed;
    HI_CHAR aszPreviewUrl[HI_OSC_LISTFILE_PREVIEW_URL];
    HI_BOOL bDownLoad;
} HI_OSCSAMPLE_FILE_S;

#define OSCSAM_UNLOCK(mutex)  \
    do \
    { \
        (HI_VOID)pthread_mutex_unlock(&mutex); \
    }while(0)

#define OSCSAM_LOCK(mutex) \
    do \
    { \
        (HI_VOID)pthread_mutex_lock(&mutex);  \
    }while(0)

HI_S32 SAMPLE_OSCSVR_Start();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
