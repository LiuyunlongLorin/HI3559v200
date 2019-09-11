#include "motionsensor_buf.h"
#include "motionsensor_exe.h"
#include "motionsensor_proc.h"
#include "hi_osal.h"
#include <linux/kernel.h>

#define  MOTIONSENSOR_INFO "motionsensor_info"

#define  MOTIONSENSOR_VERSION_INFO "MOTIONSENSOR debug V0.0.0.1"

extern MSENSOR_BUF_INFO_S g_astBufInfo[MSENSOR_DATA_BUTT][DATA_TYPE_BUTT];

MOTIONSENSOR_PROC_INFO_S g_stMotionsensorProcInfo;


static int MOTIONSENSOR_PROC_Show(osal_proc_entry_t *s)
{
    int ret;
    ret = HI_MOTIONSENSOR_GetProcInfo();
    if(ret)
    {
        printk("HI_MOTIONSENSOR_GetProcInfo failed!\n");
        return -1;
    }

    osal_seq_printf(s, "[motionsensor] Version:["MOTIONSENSOR_VERSION_INFO"], Build Time["__DATE__", "__TIME__"]\n");


    if(g_stMotionsensorProcInfo.aszGyroName[0])
    {
        osal_seq_printf(s, "------GyroSensorName------\n");
        osal_seq_printf(s, "%20s", g_stMotionsensorProcInfo.aszGyroName);
        osal_seq_printf(s, "\n");

        osal_seq_printf(s, "%20s%16s%16s%16s%16s%16s\n", "buf_addr","buf_size", "overflow","data_unmatch","overflowID","data_unmatchID");
        osal_seq_printf(s, "%20llx", g_stMotionsensorProcInfo.au64BufAddr[MSENSOR_DATA_GYRO]);
        osal_seq_printf(s, "%16u", g_stMotionsensorProcInfo.au32BufSize[MSENSOR_DATA_GYRO]);
        osal_seq_printf(s, "%16u", g_stMotionsensorProcInfo.au32BufOverflow[MSENSOR_DATA_GYRO]);
        osal_seq_printf(s, "%16u", g_stMotionsensorProcInfo.au32BufDataUnmatch[MSENSOR_DATA_GYRO]);
        osal_seq_printf(s, "%16d", g_stMotionsensorProcInfo.as32BufOverflowID[MSENSOR_DATA_GYRO]);
        osal_seq_printf(s, "%16d", g_stMotionsensorProcInfo.as32BufDataUnmatchID[MSENSOR_DATA_GYRO]);

        osal_seq_printf(s, "\n");

        osal_seq_printf(s, "%20s%20s%20s%20s%20s%20s%20s%20s\n", "x-startaddr","x-startaddr" ,"y-startaddr","y-startaddr","z-startaddr","z-startaddr","temp-startaddr","temp-startaddr");
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_GYRO][DATA_TYPE_X].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_GYRO][DATA_TYPE_X].pWritePointer);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_GYRO][DATA_TYPE_Y].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_GYRO][DATA_TYPE_Y].pWritePointer);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_GYRO][DATA_TYPE_Z].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_GYRO][DATA_TYPE_Z].pWritePointer);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_GYRO][DATA_TYPE_TEMP].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_GYRO][DATA_TYPE_TEMP].pWritePointer);
        
        osal_seq_printf(s, "\n");

        osal_seq_printf(s, "%20s%20s\n", "pts-startaddr","pts-startaddr");
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_GYRO][DATA_TYPE_PTS].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_GYRO][DATA_TYPE_PTS].pWritePointer);
        
        osal_seq_printf(s, "\n");
        
    }

    if(g_stMotionsensorProcInfo.aszAccelName[0])
    {
        osal_seq_printf(s, "------AccelSensorName------\n");
        osal_seq_printf(s, "%21s", g_stMotionsensorProcInfo.aszAccelName);

        osal_seq_printf(s, "\n");

        osal_seq_printf(s, "%20s%16s%16s%16s%16s%16s\n", "buf_addr","buf_size", "overflow","data_unmatch","overflowID","data_unmatchID");
        osal_seq_printf(s, "%20llx", g_stMotionsensorProcInfo.au64BufAddr[MSENSOR_DATA_ACC]);
        osal_seq_printf(s, "%16u", g_stMotionsensorProcInfo.au32BufSize[MSENSOR_DATA_ACC]);
        osal_seq_printf(s, "%16u", g_stMotionsensorProcInfo.au32BufOverflow[MSENSOR_DATA_ACC]);
        osal_seq_printf(s, "%16u", g_stMotionsensorProcInfo.au32BufDataUnmatch[MSENSOR_DATA_ACC]);
        osal_seq_printf(s, "%16d", g_stMotionsensorProcInfo.as32BufOverflowID[MSENSOR_DATA_ACC]);
        osal_seq_printf(s, "%16d", g_stMotionsensorProcInfo.as32BufDataUnmatchID[MSENSOR_DATA_ACC]);
        osal_seq_printf(s, "\n");

        osal_seq_printf(s, "%20s%20s%20s%20s%20s%20s%20s%20s\n", "x-startaddr","x-startaddr" ,"y-startaddr","y-startaddr","z-startaddr","z-startaddr","temp-startaddr","temp-startaddr");
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_ACC][DATA_TYPE_X].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_ACC][DATA_TYPE_X].pWritePointer);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_ACC][DATA_TYPE_Y].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_ACC][DATA_TYPE_Y].pWritePointer);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_ACC][DATA_TYPE_Z].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_ACC][DATA_TYPE_Z].pWritePointer);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_ACC][DATA_TYPE_TEMP].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_ACC][DATA_TYPE_TEMP].pWritePointer);
        
        osal_seq_printf(s, "\n");

        osal_seq_printf(s, "%20s%20s\n", "pts-startaddr","pts-startaddr");
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_ACC][DATA_TYPE_PTS].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_ACC][DATA_TYPE_PTS].pWritePointer);
        
        osal_seq_printf(s, "\n");

    }

    if(g_stMotionsensorProcInfo.aszMagName[0])
    {
        osal_seq_printf(s, "------MagSensorName------\n");
        osal_seq_printf(s, "%19s", g_stMotionsensorProcInfo.aszMagName);
        osal_seq_printf(s, "\n");

        osal_seq_printf(s, "%20s%16s%16s%16s%16s%16s\n", "buf_addr","buf_size", "overflow","data_unmatch","overflowID","data_unmatchID");
        osal_seq_printf(s, "%20llx", g_stMotionsensorProcInfo.au64BufAddr[MSENSOR_DATA_MAGN]);
        osal_seq_printf(s, "%16u", g_stMotionsensorProcInfo.au32BufSize[MSENSOR_DATA_MAGN]);
        osal_seq_printf(s, "%16u", g_stMotionsensorProcInfo.au32BufOverflow[MSENSOR_DATA_MAGN]);
        osal_seq_printf(s, "%16u", g_stMotionsensorProcInfo.au32BufDataUnmatch[MSENSOR_DATA_MAGN]);
        osal_seq_printf(s, "%16d", g_stMotionsensorProcInfo.as32BufOverflowID[MSENSOR_DATA_MAGN]);
        osal_seq_printf(s, "%16d", g_stMotionsensorProcInfo.as32BufDataUnmatchID[MSENSOR_DATA_MAGN]);
        osal_seq_printf(s, "\n");
        
        osal_seq_printf(s, "%20s%20s%20s%20s%20s%20s%20s%20s\n", "x-startaddr","x-startaddr" ,"y-startaddr","y-startaddr","z-startaddr","z-startaddr","temp-startaddr","temp-startaddr");
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_MAGN][DATA_TYPE_X].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_MAGN][DATA_TYPE_X].pWritePointer);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_MAGN][DATA_TYPE_Y].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_MAGN][DATA_TYPE_Y].pWritePointer);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_MAGN][DATA_TYPE_Z].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_MAGN][DATA_TYPE_Z].pWritePointer);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_MAGN][DATA_TYPE_TEMP].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_MAGN][DATA_TYPE_TEMP].pWritePointer);
        
        osal_seq_printf(s, "\n");

        osal_seq_printf(s, "%20s%20s\n", "pts-startaddr","pts-startaddr");
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_MAGN][DATA_TYPE_PTS].pStartAddr);
        osal_seq_printf(s, "%20p", g_astBufInfo[MSENSOR_DATA_MAGN][DATA_TYPE_PTS].pWritePointer);
        
        osal_seq_printf(s, "\n");

    }

    return 0;
}


int MOTIONSENSOR_PROC_Init(void)
{
    osal_proc_entry_t *motionsensor_entry;
    osal_memset(&g_stMotionsensorProcInfo, 0, sizeof(g_stMotionsensorProcInfo));

    motionsensor_entry = osal_create_proc_entry(MOTIONSENSOR_INFO, NULL);
    if(motionsensor_entry == NULL)
    {
        printk("osal_create_proc_entry failed!\n");
        return -1;
    }
    
    motionsensor_entry->read = MOTIONSENSOR_PROC_Show;
    motionsensor_entry->write = NULL;

    return 0;
}

void  MOTIONSENSOR_PROC_Exit(void)
{
     osal_remove_proc_entry(MOTIONSENSOR_INFO, NULL);
}



