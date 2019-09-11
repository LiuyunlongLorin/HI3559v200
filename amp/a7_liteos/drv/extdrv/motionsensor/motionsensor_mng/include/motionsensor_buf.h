#ifndef __MOTIONSENSOR_BUF_H__
#define __MOTIONSENSOR_BUF_H__

#include "motionsensor_ext.h"
#include "hi_osal.h"

#define HIALIGN(x, a) ((a) * (((x) + (a) - 1) / (a)))
//#define BUF_BLOCK_NUM 6
#define BUF_BLOCK_NUM 6
#define MAX_USER_NUM 10
#define WR_GAP 100 //minmotionsensorm gap between reader pointer and write pointer, to prevent new datas overlap with reading/processing datas

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILURE
#define FAILURE -1
#endif

typedef struct
{
    HI_VOID * pStartAddr;      //start address
    HI_VOID * pWritePointer;   //write pointer
}MSENSOR_BUF_INFO_S;

typedef enum
{
    DATA_TYPE_X,
    DATA_TYPE_Y,
    DATA_TYPE_Z,
    DATA_TYPE_TEMP,
    DATA_TYPE_PTS,
    DATA_TYPE_BUTT
}MOTIONSENSOR_BUF_DATA_TYPE_E;

typedef struct
{
    HI_VOID * pReadPointer[MSENSOR_DATA_BUTT][DATA_TYPE_BUTT];
    HI_S32 s32Reverd3[4];
    //osal_spinlock_t read_lock;
}MSENSOR_BUF_USER_CONTEXT_S;

#define RESERVE_NUM 100

typedef struct
{
    osal_spinlock_t read_lock[MAX_USER_NUM];
}MSENSOR_USER_SYNC_S;

typedef struct
{
    HI_U32 u32UserCnt;    
    osal_spinlock_t              msensormng_lock;
    MSENSOR_USER_SYNC_S          stUserSync;
    MSENSOR_BUF_USER_CONTEXT_S * pstUserContext[MAX_USER_NUM];
    
    osal_mutex_t buf_mng_mutex;
}MSENSOR_BUF_USER_MNG_S;

HI_S32 MOTIONSENSOR_BUF_LockInit(HI_VOID);

HI_VOID MOTIONSENSOR_BUF_LockDeInit(HI_VOID);

HI_S32 MOTIONSENSOR_BUF_Init(MSENSOR_BUF_ATTR_S* pstBufAttr, HI_U32 u32GyroFreq, HI_U32 u32AccelFreq, HI_U32 u32MagFreq);

HI_S32 MOTIONSENSOR_BUF_Deinit(void);

HI_S32 MOTIONSENSOR_BUF_WriteData(MSENSOR_DATA_TYPE_E enDataType, HI_S32 x, HI_S32 y, HI_S32 z, HI_S32 temp, HI_U64 pts);

//int MOTIONSENSOR_BUF_WriteUsrData(MOTIONSENSOR_USRDATA_S* pstUsrData);
HI_S32 MOTIONSENSOR_BUF_GetData(HI_VOID* pstMotionsensorData);

HI_S32 MOTIONSENSOR_BUF_ReleaseData(HI_VOID* pstMotionsensorDataInfo);

HI_S32 MOTIONSENSOR_BUF_AddUser(HI_S32* ps32Id);

HI_S32 MOTIONSENSOR_BUF_DeleteUser(HI_S32* ps32Id);

HI_S32 MOTIONSENSOR_BUF_SyncInit(HI_VOID);

HI_S32 MOTIONSENSOR_BUF_SyncDeInit(HI_VOID);

#endif

