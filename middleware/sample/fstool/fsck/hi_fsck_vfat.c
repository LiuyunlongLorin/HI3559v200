#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hi_fstool.h"


void ShowHelp()
{
    #ifdef __HuaweiLite__
    printf("fsck usage:sample partition\n");
    #else
    printf("fsck usage:./hifsck.vfat partition\n");
    #endif
}


#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    int ret = 0;
    if ((argc != 2) || !strcmp("--help", argv[1]))
    {
        ShowHelp();
        return 1;
    }
    HI_FSTOOL_CHECK_CFG_S stCheckCfg;
    stCheckCfg.enMode = HI_FSTOOL_CHECK_MODE_ALWAYS;
    stCheckCfg.u32FragmentThr = 25;
    ret = HI_FSTOOL_Check(argv[1],&stCheckCfg);
    if (ret == HI_ERR_FSTOOL_FRAGMENT_SEVERELY)
    {
        printf("[hifsck]:file system fragment severely, and this can be ignored\n");
    }
    else if (0 == ret)
    {
        printf("[hifsck]:file system is well\n");
    }
    if (0 != ret)
    {
        printf("[hifsck]:file system check with errno:0x%x\n", ret);
    }

    return ret;
}

