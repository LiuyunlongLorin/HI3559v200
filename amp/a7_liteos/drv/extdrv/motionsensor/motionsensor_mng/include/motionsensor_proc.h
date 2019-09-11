#ifndef __MOTIONSENSOR_PROC__
#define __MOTIONSENSOR_PROC__
#include "motionsensor_ext.h"

#define MAX_LEN 10

typedef struct
{

    char aszGyroName[MAX_LEN];
    char aszAccelName[MAX_LEN];
    char aszMagName[MAX_LEN];

    unsigned long long int au64BufAddr[MSENSOR_DATA_BUTT];
    unsigned int au32BufSize[MSENSOR_DATA_BUTT];
    unsigned int au32BufOverflow[MSENSOR_DATA_BUTT];
    unsigned int au32BufDataUnmatch[MSENSOR_DATA_BUTT];
    int as32BufOverflowID[MSENSOR_DATA_BUTT];
    int as32BufDataUnmatchID[MSENSOR_DATA_BUTT];
}MOTIONSENSOR_PROC_INFO_S;

extern MOTIONSENSOR_PROC_INFO_S g_stMotionsensorProcInfo;

int MOTIONSENSOR_PROC_Init(void);
void MOTIONSENSOR_PROC_Exit(void);


#endif

