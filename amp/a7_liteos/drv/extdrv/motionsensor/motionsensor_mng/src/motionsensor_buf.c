#include <linux/kernel.h>
#include "hi_debug.h"
#include "motionsensor_buf.h"
#include "motionsensor_proc.h"
#include "motionsensor_exe.h"
#include "mpi_sys.h"


static HI_BOOL g_bForward = HI_TRUE;
static HI_BOOL g_bBufInit = HI_FALSE;
MSENSOR_BUF_INFO_S g_astBufInfo[MSENSOR_DATA_BUTT][DATA_TYPE_BUTT];
static HI_S64 g_s64Offset;
HI_BOOL g_bAlreadyReleased[MAX_USER_NUM];
MSENSOR_BUF_USER_MNG_S g_stUserMng;
#define DATA_RESERVE_NUM 50

extern HI_S32 MOTIONSENSOR_BUF_WriteData2Buf(HI_VOID);

#define TEST_ON 0
__inline static HI_VOID* MOTIONSENSOR_Remap_Nocache(HI_U64 u64PhyAddr,HI_U32 u32Size)
{
#if TEST_ON
    return u64PhyAddr;
#else
    return osal_ioremap_nocache(u64PhyAddr, HIALIGN(u32Size, 4));

#endif
}

__inline static HI_VOID  MOTIONSENSOR_Unmap(HI_VOID* pVirAddr)
{
#if TEST_ON

#else
    osal_iounmap(pVirAddr);
#endif
}

HI_S32 MOTIONSENSOR_GetUserId(HI_S32* ps32Id)
{
    HI_S32 i;

    for (i = 0; i <MAX_USER_NUM; i++)
    {
        if (HI_NULL == g_stUserMng.pstUserContext[i])
        {
            *ps32Id = i;
            return HI_SUCCESS;
        }
    }

    HI_TRACE_MSENSOR(HI_DBG_ERR,"No free user for use.\n");

    return HI_FAILURE;
}



HI_S32 MOTIONSENSOR_BUF_AddUser(HI_S32* ps32Id)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_UL flags;
    //HI_UL mngflags;
    HI_S32 i = 0;
    HI_S32 j = 0;

    MSENSOR_BUF_USER_CONTEXT_S* pstUserContext;

    *ps32Id = -1;

    if (HI_FALSE == g_bBufInit)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"Buf not init,g_bBufInit:%d\n",g_bBufInit);
        return HI_FAILURE;
    }

    osal_mutex_lock(&g_stUserMng.buf_mng_mutex);

    //Step 1: Get available ID
    if (g_stUserMng.u32UserCnt > MAX_USER_NUM)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"Motionsensor ID has reached toplimit.\n");
        goto ERROR_0;
    }

    /*get user id for use*/
    s32Ret = MOTIONSENSOR_GetUserId(ps32Id);
    if (HI_SUCCESS != s32Ret)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"GetUserId failed.\n");
        goto ERROR_0;
    }

    if((*ps32Id < 0)||(*ps32Id >= MAX_USER_NUM))
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"*ps32Id(%d) out of range[0,%d].\n",*ps32Id,MAX_USER_NUM);
        goto ERROR_0;
    }

    //Step 2: Create context for new ID
    pstUserContext = (MSENSOR_BUF_USER_CONTEXT_S*)osal_vmalloc(sizeof(MSENSOR_BUF_USER_CONTEXT_S));

    if (HI_NULL == pstUserContext)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"vmalloc failed.\n");
        goto ERROR_0;
    }

    osal_memset(pstUserContext, 0, sizeof(MSENSOR_BUF_USER_CONTEXT_S));
    //osal_spin_lock_init(&pstUserContext->read_lock);

    osal_spin_lock_irqsave(&g_stUserMng.stUserSync.read_lock[*ps32Id], &flags);

    for (i = 0; i < DATA_TYPE_BUTT; i++)
    {

        if (DATA_TYPE_PTS == i)
        {
            for (j = 0; j < MSENSOR_DATA_BUTT; j++)
            {
                if (g_stMotionsensorProcInfo.au32BufSize[j] > DATA_RESERVE_NUM * BUF_BLOCK_NUM)
                {
                    ///osal_printk(" line=%d i:%d j:%d\n", __LINE__,i,j);
                    ///osal_printk("AccelBuff %%%%%% [%d]pStartAddr:%p pWritePointer:%p\n",i,g_astBufInfo[j][i].pWritePointer,g_astBufInfo[j][i].pStartAddr);

                    #if 1
                    pstUserContext->pReadPointer[j][i] = (unsigned long long int *)g_astBufInfo[j][i].pWritePointer;
                    #else
                    if ((unsigned long long*)g_astBufInfo[j][i].pWritePointer - (unsigned long long*)g_astBufInfo[j][i].pStartAddr>= DATA_RESERVE_NUM)
                    {
                        ///osal_printk("###line=%d i:%d j:%d Size:%d\n", __LINE__,i,j,g_stMotionsensorProcInfo.au32BufSize[j]);
                        pstUserContext->pReadPointer[j][i] = (unsigned long long*)g_astBufInfo[j][i].pWritePointer - DATA_RESERVE_NUM;
                    }
                    else
                    {
                        ///osal_printk("***line=%d i:%d j:%d Size:%d\n", __LINE__,i,j,g_stMotionsensorProcInfo.au32BufSize[j]);
                        osal_printk("***line=%d i:%d j:%d\n", __LINE__,i,j);
                        //osal_printk(" line=%d i:%d j:%d\n", __LINE__,i,j);
                        //osal_printk(" line=%d i:%d j:%d pStartAddr:%p au32BufSize:%d pWritePointer:%p pStartAddr:%p\n", __LINE__,i,j,
                        //    g_astBufInfo[j][i].pStartAddr,g_stMotionsensorProcInfo.au32BufSize[j],g_astBufInfo[j][i].pWritePointer,g_astBufInfo[j][i].pStartAddr);

                        //osal_printk(" line=%d Mid:%p\n", __LINE__,
                        //    g_astBufInfo[j][i].pStartAddr + g_stMotionsensorProcInfo.au32BufSize[j] * 2 / BUF_BLOCK_NUM);
                        ///pstUserContext->pReadPointer[j][i] = (unsigned long long*)g_astBufInfo[j][i].pStartAddr + g_stMotionsensorProcInfo.au32BufSize[j]  / BUF_BLOCK_NUM -
                        ///                                     (DATA_RESERVE_NUM - ((unsigned long long*)g_astBufInfo[j][i].pWritePointer - (unsigned long long*)g_astBufInfo[j][i].pStartAddr));

                        pstUserContext->pReadPointer[j][i] = (unsigned long long*)g_astBufInfo[j][i].pStartAddr + g_stMotionsensorProcInfo.au32BufSize[j] / BUF_BLOCK_NUM -
                                                            (DATA_RESERVE_NUM - ((unsigned long long*)g_astBufInfo[j][i].pWritePointer - (unsigned long long*)g_astBufInfo[j][i].pStartAddr));

                    }
                    #endif
                    //osal_printk("XXXXXX j=%d,read=%p,write=%p\n", j, pstUserContext->pReadPointer[j][i], g_astBufInfo[j][i].pWritePointer);
                }
                else
                {
                    /////osal_printk("line=%d\n", __LINE__);
                    pstUserContext->pReadPointer[j][i] = g_astBufInfo[j][i].pWritePointer;
                }
            }

            //osal_printk("------------j=%d,read=0x%x,write=0x%x\n",j,pstUserContext->pReadPointer[j][i],g_astBufInfo[j][i].pWritePointer);
        }
        else
        {
            for (j = 0; j < MSENSOR_DATA_BUTT; j++)
            {
                #if 1
                pstUserContext->pReadPointer[j][i] = (int *)g_astBufInfo[j][i].pWritePointer;
                #else
                if (g_stMotionsensorProcInfo.au32BufSize[j] > DATA_RESERVE_NUM * BUF_BLOCK_NUM)
                {
                    if ((int*)g_astBufInfo[j][i].pWritePointer - (int*)g_astBufInfo[j][i].pStartAddr >= DATA_RESERVE_NUM)
                    {
                        //osal_printk(" line=%d i:%d j:%d\n", __LINE__,i,j);
                        pstUserContext->pReadPointer[j][i] = (int*)g_astBufInfo[j][i].pWritePointer - DATA_RESERVE_NUM;
                    }
                    else
                    {
                        pstUserContext->pReadPointer[j][i] = (int*)g_astBufInfo[j][i].pStartAddr + g_stMotionsensorProcInfo.au32BufSize[j] / BUF_BLOCK_NUM -
                                                             (DATA_RESERVE_NUM - ((int*)g_astBufInfo[j][i].pWritePointer - (int*)g_astBufInfo[j][i].pStartAddr));

                        //pstUserContext->pReadPointer[j][i] = g_astBufInfo[j][i].pStartAddr + g_stMotionsensorProcInfo.au32BufSize[j] * 2 / BUF_BLOCK_NUM -
                        //                                    2*(DATA_RESERVE_NUM - ((unsigned long long*)g_astBufInfo[j][i].pWritePointer - (unsigned long long*)g_astBufInfo[j][i].pStartAddr));
                    }
                }
                else
                {
                    pstUserContext->pReadPointer[j][i] = g_astBufInfo[j][i].pWritePointer;
                }
                #endif

            }
        }

    }

    //osal_spin_lock_irqsave(&g_stUserMng.msensormng_lock, &mngflags);

    g_bForward = HI_TRUE;
    g_stUserMng.pstUserContext[*ps32Id] = pstUserContext;

    //osal_printk("###########fun:%s *ps32Id:%d pReadPointer[GYRO][PTS]:%p\n",__func__,*ps32Id,g_stUserMng.pstUserContext[*ps32Id]->pReadPointer[MSENSOR_DATA_GYRO][DATA_TYPE_PTS]);

    g_stUserMng.u32UserCnt++;

    //osal_spin_unlock_irqrestore(&g_stUserMng.msensormng_lock, &mngflags);

    osal_spin_unlock_irqrestore(&g_stUserMng.stUserSync.read_lock[*ps32Id], &flags);

    osal_mutex_unlock(&g_stUserMng.buf_mng_mutex);

    osal_msleep(200);

    return HI_SUCCESS;
ERROR_0:
    osal_mutex_unlock(&g_stUserMng.buf_mng_mutex);
    return HI_FAILURE;
}

HI_S32 MOTIONSENSOR_BUF_DeleteUser(HI_S32* ps32Id)
{
    MSENSOR_BUF_USER_CONTEXT_S* pstMSenserBufUserContexTemp = HI_NULL;
    unsigned long flags;

    if((*ps32Id < 0)||(*ps32Id >= MAX_USER_NUM))
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"*ps32Id(%d) out of range[0,%d].\n",*ps32Id,MAX_USER_NUM);
        return HI_FAILURE;
    }

    if (HI_FALSE == g_bBufInit)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"When Delete User,Motionsensor buffer hasn't been initialised.\n");
        return HI_FAILURE;
    }

    if (HI_NULL == g_stUserMng.pstUserContext[*ps32Id])
    {
        HI_TRACE_MSENSOR(HI_DBG_DEBUG,"u32Id is NULL.\n");
        return HI_SUCCESS;
    }

    osal_mutex_lock(&g_stUserMng.buf_mng_mutex);

    osal_spin_lock_irqsave(&g_stUserMng.msensormng_lock, &flags);

    //Release context
    pstMSenserBufUserContexTemp = g_stUserMng.pstUserContext[*ps32Id];

    g_stUserMng.pstUserContext[*ps32Id] = HI_NULL;
    g_stUserMng.u32UserCnt--;

    osal_spin_unlock_irqrestore(&g_stUserMng.msensormng_lock, &flags);
    osal_mutex_unlock(&g_stUserMng.buf_mng_mutex);

    if(HI_NULL != pstMSenserBufUserContexTemp)
    {
        osal_vfree(pstMSenserBufUserContexTemp);
    }

    return HI_SUCCESS;
}

HI_S32 MOTIONSENSOR_BUF_Init(MSENSOR_BUF_ATTR_S* pstBufAttr, HI_U32 u32GyroFreq, HI_U32 u32AccelFreq, HI_U32 u32MagFreq)
{
    HI_VOID* pVirAddr = HI_NULL;
    HI_U32 u32GyroBlockSize;
    HI_U32 u32AccelBlockSize;
    HI_U32 u32MagBlockSize;
    HI_S32 i;

    if (HI_TRUE == g_bBufInit)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"buf already inited\n");
        return HI_SUCCESS;
    }

    if (0 == pstBufAttr->u64PhyAddr || 0 == pstBufAttr->u32Buflen)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"buf addr can not be null and buf size must lager than 0\n");
        return HI_FAILURE;
    }

    if (0 == u32GyroFreq && 0 == u32AccelFreq && 0 == u32MagFreq)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"can't all frequency be 0\n");
        return HI_FAILURE;
    }

    pVirAddr = MOTIONSENSOR_Remap_Nocache((unsigned long long int)pstBufAttr->u64PhyAddr, pstBufAttr->u32Buflen);

    if (HI_NULL == pVirAddr)
    {
         HI_TRACE_MSENSOR(HI_DBG_ERR,"ioremap err\n");
        return HI_FAILURE;
    }

    //Debug for l00346266
    osal_memset(pVirAddr,0,pstBufAttr->u32Buflen);

    HI_TRACE_MSENSOR(HI_DBG_DEBUG,"u64PhyAddr:%llx pVirAddr:%p u32Buflen:%d\n",
        pstBufAttr->u64PhyAddr,pVirAddr,pstBufAttr->u32Buflen);

    g_s64Offset = pstBufAttr->u64PhyAddr - (HI_U64)(HI_UL)pVirAddr;
    u32GyroBlockSize = pstBufAttr->u32Buflen * u32GyroFreq / (u32GyroFreq + u32AccelFreq + u32MagFreq) / BUF_BLOCK_NUM / 32 * 32;
    u32AccelBlockSize = pstBufAttr->u32Buflen * u32AccelFreq / (u32GyroFreq + u32AccelFreq + u32MagFreq) / BUF_BLOCK_NUM / 32 * 32;
    u32MagBlockSize = (pstBufAttr->u32Buflen / BUF_BLOCK_NUM - u32GyroBlockSize - u32AccelBlockSize) / 32 * 32;

    //osal_printk("u32GyroFreq:%d u32AccelFreq:%d u32MagFreq:%d\n",u32GyroFreq, u32AccelFreq, u32MagFreq);
    //osal_printk("GyroBuff Total_u32Buflen:%d u32GyroBlockSize:%d,u32AccelBlockSize:%d,u32MagBlockSize:%d\n",pstBufAttr->u32Buflen,u32GyroBlockSize,u32AccelBlockSize,u32MagBlockSize);


    for (i = 0; i < DATA_TYPE_BUTT; i++)
    {
        g_astBufInfo[MSENSOR_DATA_GYRO][i].pStartAddr = (HI_U8 *)pVirAddr + u32GyroBlockSize * i;
        g_astBufInfo[MSENSOR_DATA_GYRO][i].pWritePointer = g_astBufInfo[MSENSOR_DATA_GYRO][i].pStartAddr;

        //osal_printk("GyroBuff u32GyroBlockSize:%d [%d]pStartAddr:%p\n",u32GyroBlockSize,i,g_astBufInfo[MOTIONSENSOR_DATA_GYRO][i].pStartAddr);

        g_astBufInfo[MSENSOR_DATA_ACC][i].pStartAddr = (HI_U8 *)pVirAddr + u32GyroBlockSize * BUF_BLOCK_NUM + u32AccelBlockSize * i;
        g_astBufInfo[MSENSOR_DATA_ACC][i].pWritePointer = g_astBufInfo[MSENSOR_DATA_ACC][i].pStartAddr;

        //osal_printk("AccelBuff ***** [%d]pStartAddr:%p pWritePointer:%p\n",i,g_astBufInfo[MOTIONSENSOR_DATA_ACCEL][i].pStartAddr,g_astBufInfo[MOTIONSENSOR_DATA_ACCEL][i].pWritePointer);
        //osal_printk("AccelBuff u32AccelBlockSize:%d [%d]pStartAddr:%p\n",u32AccelBlockSize,i,g_astBufInfo[MOTIONSENSOR_DATA_ACCEL][i].pStartAddr);
        g_astBufInfo[MSENSOR_DATA_MAGN][i].pStartAddr = (HI_U8 *)pVirAddr + u32GyroBlockSize * BUF_BLOCK_NUM + u32AccelBlockSize * BUF_BLOCK_NUM + u32MagBlockSize * i;
        g_astBufInfo[MSENSOR_DATA_MAGN][i].pWritePointer = g_astBufInfo[MSENSOR_DATA_MAGN][i].pStartAddr;
    }

    HI_TRACE_MSENSOR(HI_DBG_DEBUG,"**PTS-pWritePointer:%p\n",g_astBufInfo[MSENSOR_DATA_GYRO][DATA_TYPE_PTS].pWritePointer);

    g_stMotionsensorProcInfo.au64BufAddr[MSENSOR_DATA_GYRO] = (unsigned long long int)(HI_UL)g_astBufInfo[MSENSOR_DATA_GYRO][DATA_TYPE_X].pStartAddr;
    g_stMotionsensorProcInfo.au64BufAddr[MSENSOR_DATA_ACC] = (unsigned long long  int)(HI_UL)g_astBufInfo[MSENSOR_DATA_ACC][DATA_TYPE_X].pStartAddr;
    g_stMotionsensorProcInfo.au64BufAddr[MSENSOR_DATA_MAGN] = (unsigned long long  int)(HI_UL)g_astBufInfo[MSENSOR_DATA_MAGN][DATA_TYPE_X].pStartAddr;

    //osal_printk("au64BufAddr[MOTIONSENSOR_DATA_GYRO]:0x%llx au64BufAddr[MOTIONSENSOR_DATA_ACCEL]:0x%llx\n",g_stMotionsensorProcInfo.au64BufAddr[MOTIONSENSOR_DATA_GYRO],g_stMotionsensorProcInfo.au64BufAddr[MOTIONSENSOR_DATA_ACCEL]);
    g_stMotionsensorProcInfo.au32BufSize[MSENSOR_DATA_GYRO] = u32GyroBlockSize * BUF_BLOCK_NUM;
    g_stMotionsensorProcInfo.au32BufSize[MSENSOR_DATA_ACC] = u32AccelBlockSize * BUF_BLOCK_NUM;
    g_stMotionsensorProcInfo.au32BufSize[MSENSOR_DATA_MAGN] = u32MagBlockSize * BUF_BLOCK_NUM;


    HI_TRACE_MSENSOR(HI_DBG_DEBUG,"#####GYRO-au64BufAddr:%llu ACC-au64BufAddr:%llu MAGN-au64BufAddr:%llu\n",
        g_stMotionsensorProcInfo.au64BufAddr[MSENSOR_DATA_GYRO],
        g_stMotionsensorProcInfo.au64BufAddr[MSENSOR_DATA_ACC],
        g_stMotionsensorProcInfo.au64BufAddr[MSENSOR_DATA_MAGN]);

    //osal_memset(&g_stUserMng, 0, sizeof(MOTIONSENSOR_BUF_USER_MNG_S));
    //MOTIONSENSOR_BUF_SyncInit();
    osal_mutex_init(&g_stUserMng.buf_mng_mutex);

    for (i = 0; i < MAX_USER_NUM; i++)
    {
        g_bAlreadyReleased[i] = true;
    }

    g_bBufInit = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 MOTIONSENSOR_BUF_Deinit(HI_VOID)
{
    HI_S32 i = 0;

    if (HI_FALSE == g_bBufInit)
    {
         HI_TRACE_MSENSOR(HI_DBG_ERR,"buf already deinited\n");
        return HI_SUCCESS;
    }

    for (i = 0; i < MAX_USER_NUM; i++)
    {
        MOTIONSENSOR_BUF_DeleteUser(&i);
    }

    MOTIONSENSOR_Unmap(g_astBufInfo[MSENSOR_DATA_GYRO][DATA_TYPE_X].pStartAddr);
    
    //osal_printk("####fun:%s line:%d++\n",__func__,__LINE__);
    osal_memset(&g_astBufInfo, 0, sizeof(g_astBufInfo));
    g_bBufInit = HI_FALSE;
    //osal_mutex_destory(&g_stUserMng.buf_mng_mutex);
    //MOTIONSENSOR_BUF_SyncDeInit();

    return HI_SUCCESS;
}

static HI_U64 u64Times = 0;
static HI_U64 u64LastPTS = 0;

HI_S32 inline MOTIONSENSOR_BUF_WriteData(MSENSOR_DATA_TYPE_E enDataType, HI_S32 x, HI_S32 y, HI_S32 z, HI_S32 temp, HI_U64 pts)
{
    HI_S32 i = 0;
    HI_VOID* pNextWritePointer = HI_NULL;
    HI_VOID* pTempPointer = HI_NULL;
    //unsigned long flags;    

    if (HI_FALSE == g_bBufInit)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"when WriteData(pts:%lld),motionsensor buf not be inited yet\n",pts);
        return HI_FAILURE;
    }

    //HI_TRACE_MSENSOR(HI_DBG_ERR,"enDataType:%d  x:%d y:%d z:%d temp:%d pts:%llu intel:%lld\n",enDataType, x, y, z, temp, pts, (pts-u64LastPTS));

    //if((MSENSOR_DATA_GYRO==enDataType)&&(pts-u64LastPTS > 1200)&&(u64Times >20))
    //{
    //    //HI_TRACE_MSENSOR(HI_DBG_ERR,"enDataType:%d  x:%d y:%d z:%d temp:%d pts:%llu intel:%lld\n",enDataType, x, y, z, temp, pts, (pts-u64LastPTS));
    //    osal_printk("enDataType:%d  x:%d y:%d z:%d temp:%d pts:%llu intel:%lld\n",enDataType, x, y, z, temp, pts, (pts-u64LastPTS));
    //    u64LastPTS = pts;
    //    return HI_FAILURE;
    //}

    u64LastPTS = pts;
    u64Times++;

    //Step 1:judge if any user overflow
    if ((HI_U8 *)g_astBufInfo[enDataType][DATA_TYPE_Y].pStartAddr - (HI_U8 *)g_astBufInfo[enDataType][DATA_TYPE_X].pWritePointer > sizeof(x)*WR_GAP)
    {
        pTempPointer = (HI_U8 *)g_astBufInfo[enDataType][DATA_TYPE_X].pWritePointer + sizeof(x) * WR_GAP;
    }
    else
    {
        pTempPointer = (HI_U8 *)g_astBufInfo[enDataType][DATA_TYPE_X].pStartAddr + sizeof(x) * WR_GAP
                       - ((HI_U8 *)g_astBufInfo[enDataType][DATA_TYPE_Y].pStartAddr - (HI_U8 *)g_astBufInfo[enDataType][DATA_TYPE_X].pWritePointer);
    }


    //if(MSENSOR_DATA_GYRO == enDataType)
    //{
    //    if(pTempPointer >= g_stUserMng.pstUserContext[0]->pReadPointer[enDataType][DATA_TYPE_X])
    //    {    
    //        HI_TRACE_MSENSOR(HI_DBG_ERR,"#pTempPointer:%p pReadPointer:%p Itl:%d\n",pTempPointer,g_stUserMng.pstUserContext[0]->pReadPointer[enDataType][DATA_TYPE_X],
    //            (HI_U32 *)pTempPointer - (HI_U32 *)g_stUserMng.pstUserContext[0]->pReadPointer[enDataType][DATA_TYPE_X]);   
    //    }
    //    else
    //    {
    //        HI_TRACE_MSENSOR(HI_DBG_ERR,"#pTempPointer:%p pReadPointer:%p Itl:%d\n",pTempPointer,g_stUserMng.pstUserContext[0]->pReadPointer[enDataType][DATA_TYPE_X],
    //            (HI_U32 *)g_astBufInfo[enDataType][DATA_TYPE_Y].pStartAddr-(HI_U32 *)g_stUserMng.pstUserContext[0]->pReadPointer[enDataType][DATA_TYPE_X]+(HI_U32 *)pTempPointer
    //            -(HI_U32 *)g_astBufInfo[enDataType][DATA_TYPE_X].pStartAddr);  
    //    }
    //}

    for (i = 0; i < MAX_USER_NUM; i++)
    {
        if (HI_NULL != g_stUserMng.pstUserContext[i])
        {
            if ((pTempPointer == g_astBufInfo[enDataType][DATA_TYPE_Y].pStartAddr
                 && g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_X] == g_astBufInfo[enDataType][DATA_TYPE_X].pStartAddr)
                || pTempPointer == g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_X])
            {

                if(MSENSOR_DATA_GYRO == enDataType)
                {
                    HI_TRACE_MSENSOR(HI_DBG_ERR,"******enDataType:%d ---!@@--------!overflow!\n",enDataType);
                    HI_TRACE_MSENSOR(HI_DBG_ERR,"******pTempPointer:%p g_astBufInfo[enDataType][DATA_TYPE_Y].pStartAddr:%p\n",pTempPointer,g_astBufInfo[enDataType][DATA_TYPE_Y].pStartAddr);
                    HI_TRACE_MSENSOR(HI_DBG_ERR,"******pReadPointer[enDataType][DATA_TYPE_X]:%p pStartAddr:%p\n",g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_X] , g_astBufInfo[enDataType][DATA_TYPE_X].pStartAddr);
                    HI_TRACE_MSENSOR(HI_DBG_ERR,"******pTempPointer:%p pReadPointer[enDataType][DATA_TYPE_X]:%p\n",pTempPointer,g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_X]);   
                }
                
                //osal_printk("******enDataType:%d ---!@@--------!overflow!\n",enDataType);
                //osal_printk("******pTempPointer:%p g_astBufInfo[enDataType][DATA_TYPE_Y].pStartAddr:%p\n",pTempPointer,g_astBufInfo[enDataType][DATA_TYPE_Y].pStartAddr);
                //osal_printk("******pReadPointer[enDataType][DATA_TYPE_X]:%p pStartAddr:%p\n",g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_X] , g_astBufInfo[enDataType][DATA_TYPE_X].pStartAddr);
                //osal_printk("******pTempPointer:%p pReadPointer[enDataType][DATA_TYPE_X]:%p\n",pTempPointer,g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_X]);
                g_stMotionsensorProcInfo.au32BufOverflow[enDataType]++;
                g_stMotionsensorProcInfo.as32BufOverflowID[enDataType] = i;
                osal_spin_lock(&g_stUserMng.stUserSync.read_lock[i]);


                #if 0
                g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_X]            = g_astBufInfo[enDataType][DATA_TYPE_X].pWritePointer;
                g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_Y]            = g_astBufInfo[enDataType][DATA_TYPE_Y].pWritePointer;
                g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_Z]            = g_astBufInfo[enDataType][DATA_TYPE_Z].pWritePointer;
                g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_TEMP]         = g_astBufInfo[enDataType][DATA_TYPE_TEMP].pWritePointer;
                g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_PTS]          = g_astBufInfo[enDataType][DATA_TYPE_PTS].pWritePointer;
                #else
                if((HI_U32 *)g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_X] + 1 >= (HI_U32 *)g_astBufInfo[enDataType][DATA_TYPE_Y].pStartAddr)
                {
                    g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_X]    = g_astBufInfo[enDataType][DATA_TYPE_Y].pStartAddr; 
                    g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_Y]    = g_astBufInfo[enDataType][DATA_TYPE_Y].pStartAddr;
                    g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_Z]    = g_astBufInfo[enDataType][DATA_TYPE_Z].pStartAddr;
                    g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_TEMP] = g_astBufInfo[enDataType][DATA_TYPE_TEMP].pStartAddr;
                    g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_PTS]  = g_astBufInfo[enDataType][DATA_TYPE_PTS].pStartAddr;
                }
                else
                {
                    g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_X]    = ((HI_U32 *)g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_X]) + 1; 
                    g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_Y]    = ((HI_U32 *)g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_Y])  + 1;
                    g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_Z]    = ((HI_U32 *)g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_Z]) + 1;
                    g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_TEMP] = ((HI_U32 *)g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_TEMP])+ 1;
                    g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_PTS]  = ((HI_U64 *)g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_PTS]) + 1;
                }
                #endif

                //osal_printk("******(*Read):%d\n",*(HI_U64 *)(g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_PTS]));
                osal_spin_unlock(&g_stUserMng.stUserSync.read_lock[i]);

                if(MSENSOR_DATA_GYRO == enDataType)
                {
                    HI_TRACE_MSENSOR(HI_DBG_ERR, "PUSH->->->(*Read)::%lld\n", *(HI_U64*)(g_stUserMng.pstUserContext[i]->pReadPointer[enDataType][DATA_TYPE_PTS]));
                }

            }
        }
    }

    #if 1
    //Step 2:write data
    *(int*)g_astBufInfo[enDataType][DATA_TYPE_X].pWritePointer    = x;
    *(int*)g_astBufInfo[enDataType][DATA_TYPE_Y].pWritePointer    = y;
    *(int*)g_astBufInfo[enDataType][DATA_TYPE_Z].pWritePointer    = z;
    *(int*)g_astBufInfo[enDataType][DATA_TYPE_TEMP].pWritePointer = temp;
    *(unsigned long long int*)g_astBufInfo[enDataType][DATA_TYPE_PTS].pWritePointer = pts;
    #endif

    ///HI_TRACE_MSENSOR(HI_DBG_ERR, "@@@write-pts:%lld\n", pts);


    //if(Cnt%200 == 1)
    //{
    //    osal_printk("fun:%s enDataType:%d ---pWritePointer:%p--x:%d y:%d z:%d TEMP:%d pts:%lld\n",__func__,enDataType,g_astBufInfo[enDataType][DATA_TYPE_X].pWritePointer,*(int*)g_astBufInfo[enDataType][DATA_TYPE_X].pWritePointer,
    //        *(int*)g_astBufInfo[enDataType][DATA_TYPE_Y].pWritePointer,*(int*)g_astBufInfo[enDataType][DATA_TYPE_Z].pWritePointer,*(int*)g_astBufInfo[enDataType][DATA_TYPE_TEMP].pWritePointer,
    //        *(unsigned long long int*)g_astBufInfo[enDataType][DATA_TYPE_PTS].pWritePointer);
    //
    //}
    //Cnt++;

    //Step 3:calculate next write pointer
    pNextWritePointer = (HI_U8 *)g_astBufInfo[enDataType][DATA_TYPE_X].pWritePointer + sizeof(x);

    if (pNextWritePointer >= g_astBufInfo[enDataType][DATA_TYPE_Y].pStartAddr)
    {
        //osal_printk("=======circle round====\n");
        g_astBufInfo[enDataType][DATA_TYPE_X].pWritePointer    = g_astBufInfo[enDataType][DATA_TYPE_X].pStartAddr;
        g_astBufInfo[enDataType][DATA_TYPE_Y].pWritePointer    = g_astBufInfo[enDataType][DATA_TYPE_Y].pStartAddr;
        g_astBufInfo[enDataType][DATA_TYPE_Z].pWritePointer    = g_astBufInfo[enDataType][DATA_TYPE_Z].pStartAddr;
        g_astBufInfo[enDataType][DATA_TYPE_TEMP].pWritePointer = g_astBufInfo[enDataType][DATA_TYPE_TEMP].pStartAddr;
        g_astBufInfo[enDataType][DATA_TYPE_PTS].pWritePointer  = g_astBufInfo[enDataType][DATA_TYPE_PTS].pStartAddr;
    }
    else
    {
        g_astBufInfo[enDataType][DATA_TYPE_X].pWritePointer    = (HI_U8 *)g_astBufInfo[enDataType][DATA_TYPE_X].pWritePointer + sizeof(x);
        g_astBufInfo[enDataType][DATA_TYPE_Y].pWritePointer    = (HI_U8 *)g_astBufInfo[enDataType][DATA_TYPE_Y].pWritePointer + sizeof(y);
        g_astBufInfo[enDataType][DATA_TYPE_Z].pWritePointer    = (HI_U8 *)g_astBufInfo[enDataType][DATA_TYPE_Z].pWritePointer + sizeof(z);
        g_astBufInfo[enDataType][DATA_TYPE_TEMP].pWritePointer = (HI_U8 *)g_astBufInfo[enDataType][DATA_TYPE_TEMP].pWritePointer + sizeof(temp);
        g_astBufInfo[enDataType][DATA_TYPE_PTS].pWritePointer  = (HI_U8 *)g_astBufInfo[enDataType][DATA_TYPE_PTS].pWritePointer + sizeof(pts);
    }

    return HI_SUCCESS;
}


#if 0
int MOTIONSENSOR_BUF_WriteUsrData(MOTIONSENSOR_USRDATA_S* pstUsrData)
{
    int i;
    int ret;

    for (i = 0; i < pstUsrData->u32UsrDataNum; i++)
    {
        ret = MOTIONSENSOR_BUF_WriteData(pstUsrData->enDataType, pstUsrData->stData[i].x,
                                pstUsrData->stData[i].y, pstUsrData->stData[i].z, pstUsrData->stData[i].pts);

        if (HI_SUCCESS != ret)
        {
            osal_printk("MOTIONSENSOR_BUF_WriteData failed\n");
            return ret;
        }
    }

    return HI_SUCCESS;
}
#endif

HI_S32 MOTIONSENSOR_BUF_ReleaseData(HI_VOID* pstMSensorData)
{
    //unsigned long mngflags;
    //unsigned long flags;
    MSENSOR_DATA_INFO_S* pstMSensorDataInfo = HI_NULL;

    pstMSensorDataInfo = (MSENSOR_DATA_INFO_S*)pstMSensorData;

    if((pstMSensorDataInfo->s32ID < 0)||(pstMSensorDataInfo->s32ID >= MAX_USER_NUM))
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"*ps32Id(%d) is out of range[0,%d].\n",pstMSensorDataInfo->s32ID,MAX_USER_NUM);
        return HI_FAILURE;
    }

    if (HI_FALSE == g_bBufInit)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"when ReleaseData, motionsensor buf not be inited yet\n");
        return HI_FAILURE;
    }

    if (HI_NULL == g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID])
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"MOTIONSENSOR_BUF_Release: s32Id is NULL.\n");
        return HI_FAILURE;
    }

    if(pstMSensorDataInfo->enDataType >= MSENSOR_DATA_BUTT)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"enDataType:%d is out of range.\n",pstMSensorDataInfo->enDataType);
        return HI_FAILURE;
    }

    if (HI_TRUE == g_bAlreadyReleased[pstMSensorDataInfo->s32ID])
    {
        return HI_SUCCESS;
    }

    osal_spin_lock(&g_stUserMng.stUserSync.read_lock[pstMSensorDataInfo->s32ID]);

    if (0 == pstMSensorDataInfo->astMSensorData[1].u32Num)
    {
        if (pstMSensorDataInfo->astMSensorData[0].u32Num > 0)
        {
            if (((HI_U64)(HI_UL)pstMSensorDataInfo->astMSensorData[0].ps32XPhyAddr + pstMSensorDataInfo->astMSensorData[0].u32Num * sizeof(int) - g_s64Offset) >= ((HI_U64)(HI_UL)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_Y].pStartAddr))
            {
                g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_X]    = g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_X].pStartAddr;
                g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Y]    = g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_Y].pStartAddr;
                g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Z]    = g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_Z].pStartAddr;
                g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP] = g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP].pStartAddr;
                g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS]  = g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pStartAddr;
            }
            else
            {
                g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_X]    = (HI_U8 *)pstMSensorDataInfo->astMSensorData[0].ps32XPhyAddr    + pstMSensorDataInfo->astMSensorData[0].u32Num * sizeof(int) - g_s64Offset;
                g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Y]    = (HI_U8 *)pstMSensorDataInfo->astMSensorData[0].ps32YPhyAddr    + pstMSensorDataInfo->astMSensorData[0].u32Num * sizeof(int) - g_s64Offset;
                g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Z]    = (HI_U8 *)pstMSensorDataInfo->astMSensorData[0].ps32ZPhyAddr    + pstMSensorDataInfo->astMSensorData[0].u32Num * sizeof(int) - g_s64Offset;
                g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP] = (HI_U8 *)pstMSensorDataInfo->astMSensorData[0].ps32TempPhyAddr + pstMSensorDataInfo->astMSensorData[0].u32Num * sizeof(int) - g_s64Offset;
                g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS]  = (HI_U8 *)pstMSensorDataInfo->astMSensorData[0].pu64PTSPhyAddr  + pstMSensorDataInfo->astMSensorData[0].u32Num * sizeof(unsigned long long int) - g_s64Offset;
            }
        }
    }
    else
    {
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_X]    = (HI_U8 *)pstMSensorDataInfo->astMSensorData[1].ps32XPhyAddr    + pstMSensorDataInfo->astMSensorData[1].u32Num * sizeof(int) - g_s64Offset;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Y]    = (HI_U8 *)pstMSensorDataInfo->astMSensorData[1].ps32YPhyAddr    + pstMSensorDataInfo->astMSensorData[1].u32Num * sizeof(int) - g_s64Offset;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Z]    = (HI_U8 *)pstMSensorDataInfo->astMSensorData[1].ps32ZPhyAddr    + pstMSensorDataInfo->astMSensorData[1].u32Num * sizeof(int) - g_s64Offset;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP] = (HI_U8 *)pstMSensorDataInfo->astMSensorData[1].ps32TempPhyAddr + pstMSensorDataInfo->astMSensorData[1].u32Num * sizeof(int) - g_s64Offset;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS]  = (HI_U8 *)pstMSensorDataInfo->astMSensorData[1].pu64PTSPhyAddr  + pstMSensorDataInfo->astMSensorData[1].u32Num * sizeof(unsigned long long int) - g_s64Offset;
    }

    //HI_TRACE_MSENSOR(HI_DBG_ERR,"Release::::X-pReadPointer:%p *pPtsReadPointer:%lld N0:%d N1:%d\n",
    //    g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_X],
    //    *(HI_U64 *)g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS],
    //    pstMSensorDataInfo->astMSensorData[0].u32Num,pstMSensorDataInfo->astMSensorData[1].u32Num);


    pstMSensorDataInfo->astMSensorData[0].u32Num = 0;
    pstMSensorDataInfo->astMSensorData[1].u32Num = 0;
    g_bAlreadyReleased[pstMSensorDataInfo->s32ID] = HI_TRUE;

    osal_spin_unlock(&g_stUserMng.stUserSync.read_lock[pstMSensorDataInfo->s32ID]);

    return HI_SUCCESS;
}

HI_S32 MOTIONSENSOR_BUF_ArriveBackStartAddr(MSENSOR_DATA_INFO_S* pstMSensorDataInfo)
{
    HI_S32           length[2] = {0};
    HI_S32           buftotal_len =0;
    HI_BOOL          bLoopFlag = false;
    HI_U32  u32Interval;
    HI_U64* pPtsReadPointer;

    buftotal_len = (unsigned int*)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_Y].pStartAddr
                        - (unsigned int*)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_X].pStartAddr;

    
    //HI_TRACE_MSENSOR(HI_DBG_ERR,"**##### buftotal_len:%d!\n",buftotal_len);

    pPtsReadPointer = g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS];

    while (1)
    {
        if ((*pPtsReadPointer <= pstMSensorDataInfo->u64BeginPts)
            || pPtsReadPointer == g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pWritePointer)
        {
            //HI_TRACE_MSENSOR(HI_DBG_ERR,"*pPtsReadPointer:%lld u64BeginPts:%lld!\n",*pPtsReadPointer,pstMSensorDataInfo->u64BeginPts);
            //HI_TRACE_MSENSOR(HI_DBG_ERR,"pPtsReadPointer:%p pWritePointer:%p!\n",pPtsReadPointer,g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pWritePointer);

            if( pPtsReadPointer == g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pWritePointer)
            {
                HI_TRACE_MSENSOR(HI_DBG_ERR,"Arrive to WriteAddr!\n");
            }
            
            break;
        }

        if(pPtsReadPointer == g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pStartAddr)
        {
            bLoopFlag = HI_TRUE;
            pPtsReadPointer = ( unsigned long long int* )g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pStartAddr + buftotal_len - 1;            
            //HI_TRACE_MSENSOR(HI_DBG_ERR,"*************round to end pPtsReadPointer:%p *pPtsReadPointer:%lld buftotal_len:%d!\n",pPtsReadPointer, *pPtsReadPointer, buftotal_len);
        }
        else
        {
            pPtsReadPointer--;
        }
        
        g_bForward = HI_FALSE;

        HI_TRACE_MSENSOR(HI_DBG_DEBUG,"#####pPtsReadPointer:%p %lld\n",pPtsReadPointer,*pPtsReadPointer);

        if (HI_TRUE == bLoopFlag)
        {
            length[1]++;
        }
        else
        {
            length[0]++;
        }

    }

    if(length[1]>0)
    {
        u32Interval = (HI_U64 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pStartAddr + buftotal_len - pPtsReadPointer;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_X]    = (HI_U32 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_Y].pStartAddr    - u32Interval;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Y]    = (HI_U32 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_Z].pStartAddr    - u32Interval;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Z]    = (HI_U32 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP].pStartAddr - u32Interval;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP] = (HI_U32 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pStartAddr  - u32Interval;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS]  = pPtsReadPointer;
        //osal_printk("#####pts-start %lld\n",*(HI_U64 *)g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS]);

    }
    else
    {
        u32Interval = (HI_U64 *)g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS] - pPtsReadPointer;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_X]    = 
            (HI_U32 *)g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_X]    - u32Interval;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Y]    = 
            (HI_U32 *)g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Y]    - u32Interval;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Z]    =
            (HI_U32 *)g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Z]    - u32Interval;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP] =
            (HI_U32 *)g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP] - u32Interval;
        
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS]  = pPtsReadPointer;
    }


    return HI_SUCCESS;
}


int MOTIONSENSOR_BUF_GetData(HI_VOID* pstMSensorData)
{
    HI_S32 ret = 0;
    HI_U64 * pPtsReadPointer;
    HI_BOOL bLoopFlag = false;
    HI_BOOL bFirstFound = false;
    HI_U32 u32Interval;
    //unsigned long flags;
    //unsigned long mngflags;
    MSENSOR_DATA_INFO_S  stDataInfo = {0};
    MSENSOR_DATA_INFO_S* pstMSensorDataInfo = HI_NULL;

    HI_VOID* pXVirAddr = HI_NULL;
    HI_VOID* pYVirAddr = HI_NULL;
    HI_VOID* pZVirAddr = HI_NULL;
    HI_VOID* pTempVirAddr = HI_NULL;
    //HI_U64 u64nowpts = 0;

    if (HI_FALSE == g_bBufInit)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"Get Data,But Motionsensor buffer hasn't been initialised.\n");
        return HI_FAILURE;
    }

    if(HI_NULL == pstMSensorData)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"pstMotionsensorData(%p) is NULL!!!\n",pstMSensorData);
        return HI_FAILURE;
    }

    pstMSensorDataInfo = &stDataInfo;
    osal_memcpy(pstMSensorDataInfo,pstMSensorData,sizeof(MSENSOR_DATA_INFO_S));
    //osal_printk("---b=%llu,end=%llu\n",pstMSensorDataInfo->u64BeginPts,pstMSensorDataInfo->u64EndPts);

    if((pstMSensorDataInfo->s32ID < 0)||(pstMSensorDataInfo->s32ID >= MAX_USER_NUM))
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"s32ID(%d) is out of range[0,%d].\n",pstMSensorDataInfo->s32ID,MAX_USER_NUM);
        return HI_FAILURE;
    }

    if(pstMSensorDataInfo->enDataType >= MSENSOR_DATA_BUTT)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"enDataType:%d out of range.\n",pstMSensorDataInfo->enDataType);
        return HI_FAILURE;
    }

    //HI_TRACE_MSENSOR(HI_DBG_ERR,"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    //osal_printk("fun:%s line:%d\n",__func__,__LINE__);

    if(pstMSensorDataInfo->enDataType == MSENSOR_DATA_GYRO)
    {
        MOTIONSENSOR_BUF_WriteData2Buf();
    }

    //osal_spin_lock_irqsave(&g_stUserMng.msensormng_lock, &mngflags);
    ret = MOTIONSENSOR_BUF_ArriveBackStartAddr(pstMSensorDataInfo);
    if(HI_SUCCESS!=ret)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"enDataType:%d has no data.\n",pstMSensorDataInfo->enDataType);
        //osal_spin_lock_irqsave(&g_stUserMng.msensormng_lock, &mngflags);
        return HI_FAILURE;
    }

    if (HI_NULL == g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID])
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"MOTIONSENSOR_BUF_ReadData: s32Id is NULL.\n");
        //osal_spin_unlock_irqrestore(&g_stUserMng.msensormng_lock, &mngflags);
        return HI_FAILURE;
    }

    if (HI_FALSE == g_bAlreadyReleased[pstMSensorDataInfo->s32ID])
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"please release last read\n");
        //osal_spin_unlock_irqrestore(&g_stUserMng.msensormng_lock, &mngflags);
        return HI_FAILURE;
    }

    pstMSensorDataInfo->astMSensorData[0].u32Num = 0;
    pstMSensorDataInfo->astMSensorData[1].u32Num = 0;
    //osal_printk("s32ID:%d enDataType:%d DATA_TYPE_PTS:%d \n",pstMotionsensorDataInfo->s32ID,pstMotionsensorDataInfo->enDataType,DATA_TYPE_PTS);

    osal_spin_lock(&g_stUserMng.stUserSync.read_lock[pstMSensorDataInfo->s32ID]);

    pPtsReadPointer = g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS];

    //osal_printk("~~~~~fun:%s line:%d pPtsReadPointer:%p *pPtsReadPointer:%lld\n",__func__,__LINE__,pPtsReadPointer,*pPtsReadPointer);

    //osal_printk("fun:%s line:%d pPtsReadPointer:%p\n",__func__,__LINE__,pPtsReadPointer);
    while (1)
    {
        if (((pPtsReadPointer == g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pWritePointer) && (HI_TRUE == g_bForward))
            || *pPtsReadPointer > pstMSensorDataInfo->u64EndPts)
        //if (((pPtsReadPointer == g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pWritePointer) /*&& (HI_TRUE == g_bForward)*/)
        //    || *pPtsReadPointer > pstMSensorDataInfo->u64EndPts)
        {

            if(pPtsReadPointer == g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pWritePointer)
            {
                //HI_TRACE_MSENSOR(HI_DBG_ERR,"==g_bForward:%d pPtsReadPointer:%p pWritePointer:%p *(pWritePointer-1):%lld *pPtsReadPointer:%lld BP:%lld EP:%lld N0:%d N1:%d\n",g_bForward,
                //    pPtsReadPointer,g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pWritePointer,*(HI_U64 *)((HI_U64)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pWritePointer-sizeof(HI_U64)),
                //    *(pPtsReadPointer-1),pstMSensorDataInfo->u64BeginPts,pstMSensorDataInfo->u64EndPts,pstMSensorDataInfo->astMSensorData[0].u32Num,pstMSensorDataInfo->astMSensorData[1].u32Num);

               //osal_printk("==g_bForward:%d pPtsReadPointer:%p pWritePointer:%p *(pWritePointer-1):%lld *pPtsReadPointer:%lld BP:%lld EP:%lld u64nowpts:%d\n",g_bForward,
               //     pPtsReadPointer,g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pWritePointer,*(HI_U64 *)((HI_U64)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pWritePointer-sizeof(HI_U64)),
               //     *(pPtsReadPointer-1),pstMSensorDataInfo->u64BeginPts,pstMSensorDataInfo->u64EndPts);   
            }

            if(*pPtsReadPointer > pstMSensorDataInfo->u64EndPts)
            {
                //osal_printk("pPointer:%p *pPtsRead:%lld u64BeginPts:%lld u64EndPts:%lld N0:%d N1:%d\n",pPtsReadPointer,
                //    *pPtsReadPointer,pstMSensorDataInfo->u64BeginPts,pstMSensorDataInfo->u64EndPts,pstMSensorDataInfo->astMSensorData[0].u32Num,pstMSensorDataInfo->astMSensorData[1].u32Num);
                //HI_TRACE_MSENSOR(HI_DBG_ERR,"pPointer:%p *pPtsRead:%lld u64BeginPts:%lld u64EndPts:%lld N0:%d N1:%d\n",pPtsReadPointer,
                //    *pPtsReadPointer,pstMSensorDataInfo->u64BeginPts,pstMSensorDataInfo->u64EndPts,pstMSensorDataInfo->astMSensorData[0].u32Num,pstMSensorDataInfo->astMSensorData[1].u32Num);
            }

            break;
        }


        if (*pPtsReadPointer >= pstMSensorDataInfo->u64BeginPts && *pPtsReadPointer <= pstMSensorDataInfo->u64EndPts)
        {
            if (HI_TRUE == bLoopFlag)
            {
                pstMSensorDataInfo->astMSensorData[1].u32Num++;
            }
            else
            {
                pstMSensorDataInfo->astMSensorData[0].u32Num++;

                if (!bFirstFound)
                {
                    u32Interval = ((HI_U8 *)pPtsReadPointer - (HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pStartAddr) >> 1;
                    g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_X]    = (HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_X].pStartAddr + u32Interval;
                    g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Y]    = (HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_Y].pStartAddr + u32Interval;
                    g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Z]    = (HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_Z].pStartAddr + u32Interval;
                    g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP] = (HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP].pStartAddr + u32Interval;
                    g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS]  = pPtsReadPointer;
                    bFirstFound = true;
                }
            }
        }

        pPtsReadPointer++;
        g_bForward = HI_TRUE;


        ///////HI_TRACE_MSENSOR(HI_DBG_ERR,"#####pPtsReadPointer:%p pWritePointer:%p \n",pPtsReadPointer,g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pWritePointer);

        if (pPtsReadPointer >= ((unsigned long long int*)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pStartAddr
                                + ((HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_Y].pStartAddr - (HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_X].pStartAddr) / sizeof(int)))
        {
            pPtsReadPointer = g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pStartAddr;

            if (bFirstFound)
            {
                bLoopFlag = true;
            }
        }

    }

    ///osal_printk("====pPtsReadPointer end=%llu\n",*(pPtsReadPointer-1));

    if (pstMSensorDataInfo->astMSensorData[0].u32Num > 0)
    {
        pXVirAddr    = g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_X];
        pYVirAddr    = g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Y];
        pZVirAddr    = g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Z];
        pTempVirAddr = g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP];

        pstMSensorDataInfo->astMSensorData[0].ps32XPhyAddr    =  (HI_VOID *)((HI_U8 *)pXVirAddr + g_s64Offset);
        pstMSensorDataInfo->astMSensorData[0].ps32YPhyAddr    =  (HI_VOID *)((HI_U8 *)pYVirAddr + g_s64Offset);
        pstMSensorDataInfo->astMSensorData[0].ps32ZPhyAddr    =  (HI_VOID *)((HI_U8 *)pZVirAddr + g_s64Offset);
        pstMSensorDataInfo->astMSensorData[0].ps32TempPhyAddr =  (HI_VOID *)((HI_U8 *)pTempVirAddr + g_s64Offset);
        pstMSensorDataInfo->astMSensorData[0].pu64PTSPhyAddr  =  (HI_VOID *)((HI_U8 *)g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS] + g_s64Offset);
        //if(pstMotionsensorDataInfo->u64BeginPts > *((unsigned long long *)g_stUserMng.pstUserContext[pstMotionsensorDataInfo->s32ID]->pReadPointer[pstMotionsensorDataInfo->enDataType][DATA_TYPE_PTS]))
        //{
            //osal_printk("wrong!u64BeginPts = %llu, readpts=%llu\n",pstMotionsensorDataInfo->u64BeginPts,
            //    *((unsigned long long *)g_stUserMng.pstUserContext[pstMotionsensorDataInfo->s32ID]->pReadPointer[pstMotionsensorDataInfo->enDataType][DATA_TYPE_PTS]));
        //}
    }

    ///osal_printk("l=%d\n",__LINE__);

    if (pstMSensorDataInfo->astMSensorData[1].u32Num > 0)
    {
        //osal_printk("l=%d\n",__LINE__);
        pstMSensorDataInfo->astMSensorData[1].ps32XPhyAddr    =  (HI_VOID *)((HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_X].pStartAddr    + g_s64Offset);
        pstMSensorDataInfo->astMSensorData[1].ps32YPhyAddr    =  (HI_VOID *)((HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_Y].pStartAddr    + g_s64Offset);
        pstMSensorDataInfo->astMSensorData[1].ps32ZPhyAddr    =  (HI_VOID *)((HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_Z].pStartAddr    + g_s64Offset);
        pstMSensorDataInfo->astMSensorData[1].ps32TempPhyAddr =  (HI_VOID *)((HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP].pStartAddr + g_s64Offset);
        pstMSensorDataInfo->astMSensorData[1].pu64PTSPhyAddr  =  (HI_VOID *)((HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pStartAddr  + g_s64Offset);
    }

    if (0 == pstMSensorDataInfo->astMSensorData[0].u32Num && 0 == pstMSensorDataInfo->astMSensorData[1].u32Num)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR, "No data! BgPts:%12lld EdPts:%12lld\n",pstMSensorDataInfo->u64BeginPts, pstMSensorDataInfo->u64EndPts);
        //HI_ASSERT(0);
        g_bAlreadyReleased[pstMSensorDataInfo->s32ID] = HI_TRUE;
        g_stMotionsensorProcInfo.au32BufDataUnmatch[pstMSensorDataInfo->enDataType]++;
        g_stMotionsensorProcInfo.as32BufDataUnmatchID[pstMSensorDataInfo->enDataType] = pstMSensorDataInfo->s32ID;
        u32Interval = ((HI_U8 *)pPtsReadPointer - (HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS].pStartAddr) >> 1;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_X]    = (HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_X].pStartAddr    + u32Interval;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Y]    = (HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_Y].pStartAddr    + u32Interval;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_Z]    = (HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_Z].pStartAddr    + u32Interval;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP] = (HI_U8 *)g_astBufInfo[pstMSensorDataInfo->enDataType][DATA_TYPE_TEMP].pStartAddr + u32Interval;
        g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS]  = pPtsReadPointer;
    }
    else
    {
        //osal_printk("l=%d\n",__LINE__);
        g_bAlreadyReleased[pstMSensorDataInfo->s32ID] = HI_FALSE;
    }

    ///osal_printk("l=%d\n",__LINE__);

    pstMSensorDataInfo->s64AddrOffset = g_s64Offset;

   osal_spin_unlock(&g_stUserMng.stUserSync.read_lock[pstMSensorDataInfo->s32ID]);

    //osal_spin_unlock_irqrestore(&g_stUserMng.msensormng_lock, &mngflags);

    osal_memcpy(pstMSensorData,pstMSensorDataInfo,sizeof(MSENSOR_DATA_INFO_S));

    MOTIONSENSOR_BUF_ReleaseData(pstMSensorDataInfo);

    ///osal_printk("RRRRRRR-pReadPointer[%d][%d]:%p\n",pstMSensorDataInfo->s32ID,pstMSensorDataInfo->enDataType,g_stUserMng.pstUserContext[pstMSensorDataInfo->s32ID]->pReadPointer[pstMSensorDataInfo->enDataType][DATA_TYPE_PTS]);

    return HI_SUCCESS;
}


HI_S32 MOTIONSENSOR_BUF_SyncInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i=0;

    for(i=0;i<MAX_USER_NUM;i++)
    {
        s32Ret = osal_spin_lock_init(&g_stUserMng.stUserSync.read_lock[i]);
        if(HI_SUCCESS!=s32Ret)
        {
            HI_TRACE_MSENSOR(HI_DBG_ERR,"spin_lock_init failed!!!!\n");
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 MOTIONSENSOR_BUF_SyncDeInit(HI_VOID)
{
    HI_S32 i=0;

    for(i=0;i<MAX_USER_NUM;i++)
    {
        //osal_printk("UUUUUU fun:%s line:%d UUUUUU\n",__func__,__LINE__);
        osal_spin_lock_destory(&g_stUserMng.stUserSync.read_lock[i]);
    }

    return HI_SUCCESS;
}

HI_S32 MOTIONSENSOR_BUF_LockInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    osal_memset(&g_stUserMng, 0, sizeof(MSENSOR_BUF_USER_MNG_S));

    s32Ret = osal_spin_lock_init(&g_stUserMng.msensormng_lock);

    if (HI_SUCCESS != s32Ret)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"spin_lock_init failed!!!!\n");
        return HI_FAILURE;
    }

    MOTIONSENSOR_BUF_SyncInit();

    osal_mutex_init(&g_stUserMng.buf_mng_mutex);

    return HI_SUCCESS;
}

HI_VOID MOTIONSENSOR_BUF_LockDeInit(HI_VOID)
{
    osal_mutex_destory(&g_stUserMng.buf_mng_mutex);

    MOTIONSENSOR_BUF_SyncDeInit();

    osal_spin_lock_destory(&g_stUserMng.msensormng_lock);
}

