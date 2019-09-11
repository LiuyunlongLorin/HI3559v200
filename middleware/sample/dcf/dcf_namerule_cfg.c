#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hi_dcf_wrapper.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_DCF_WRAPPER_NAMERULE_CFG_S g_stNameRuleCfg_DV =
{
    "/tmp",
    "HSCAM",
    9,
    {
        //NORM0001.MP4/NORM0001.LRV/NORM0001.THM
        {
            3,
            {
                {
                    "NORM",
                    "MP4"
                },
                {
                    "NORM",
                    "LRV"
                },
                {
                    "NORM",
                    "THM"
                }
            }
        },
        //SLOW0002.MP4/SLOW0002.LRV/NORM0002.THM
        {
            3,
            {
                {
                    "SLOW",
                    "MP4"
                },
                {
                    "SLOW",
                    "LRV"
                },
                {
                    "SLOW",
                    "THM"
                }
            }
        },
        //L001003.MP4/L001003.LRV/L001003.THM
        {
            3,
            {
                {
                    "L",
                    "MP4"
                },
                {
                    "L",
                    "LRV"
                },
                {
                    "L",
                    "THM"
                }
            }
        },
        //LPSE0004.MP4/LPSE0004.LRV/LPSE0004.THM
        {
            3,
            {
                {
                    "LPSE",
                    "MP4"
                },
                {
                    "LPSE",
                    "LRV"
                },
                {
                    "LPSE",
                    "THM"
                }
            }
        },
        //SING0005.JPG
        {
            1,
            {
                {
                    "SING",
                    "JPG"
                }
            }
        },
        //DLAY0006.JPG
        {
            1,
            {
                {
                    "DLAY",
                    "JPG"
                }
            }
        },
        //B0020007.JPG
        {
            1,
            {
                {
                    "B",
                    "JPG"
                }
            }
        },
        //C0030008.JPG
        {
            1,
            {
                {
                    "C",
                    "JPG"
                }
            }
        },
        //L0040009.JPG
        {
            1,
            {
                {
                    "L",
                    "JPG"
                }
            }
        }
    }
};
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
