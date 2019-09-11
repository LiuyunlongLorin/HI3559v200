#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hi_fstool.h"

void ShowHelp()
{
    #ifdef __HuaweiLite__
    printf("mkfs usage:sample clusterSize(KB)  mode partition\n"
        "\n"
        "description\n"
        "clusterSize(KB) : 64 \n"
        "mode : -a(advanced) or -d(discard) \n"
        "partition : such as /dev/mmcblk0p0 \n");
    #else
    printf("mkfs usage:./himkfs.vfat clusterSize(KB)  mode partition\n"
        "\n"
        "description\n"
        "clusterSize(KB) : 64 \n"
        "mode : -a(advanced) or -d(discard) \n"
        "partition : such as /dev/mmcblk0p1 \n");
    #endif
}


#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    int ret = 0;
    unsigned int cluSize = 0;
    if (((argc == 2) && !strcmp("--help", argv[1])) || argc != 4)
    {
        ShowHelp();
        return 1;
    }

    cluSize = 1024* atoi(argv[1]);
    HI_FSTOOL_FORMAT_CFG_S stFstoolCfg;
    stFstoolCfg.u32ClusterSize = cluSize;
    stFstoolCfg.enable4KAlignCheck = 0;
    if (!strcmp("-a", argv[2]))
    {
        stFstoolCfg.enMode = HI_FSTOOL_FORMAT_MODE_ADVANCED;
        ret = HI_FSTOOL_Format(argv[3], &stFstoolCfg);
    }
    else if (!strcmp("-d", argv[2]))
    {
        stFstoolCfg.enMode = HI_FSTOOL_FORMAT_MODE_DISCARD;
        ret = HI_FSTOOL_Format(argv[3], &stFstoolCfg);
    }
    else
    {
        ShowHelp();
        return 1;
    }

    if (0 != ret)
    {
        printf("[himkfs]:mkfs fail with errno:0x%x\n",  ret);
    }

    return ret;
}
