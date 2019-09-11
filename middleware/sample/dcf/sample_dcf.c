#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hi_dcf_wrapper.h"
#include "dcf_wrapper_app.h"

typedef enum DCF_WORK_MODE_E
{
    DCF_WORK_MODE_REC = 0x00,
    DCF_WORK_MODE_BOARD_PLAYBAK = 0x01,
    DCF_WORK_MODE_CLIENT_PLAYBAK = 0x02,
    DCF_WORK_MODE_BUTT
}DCF_WORK_MODE_E;

typedef enum DCF_OBJ_TYPE_E
{
    DCF_OBJ_TYPE_NORMMP4 = 0x00, //NORM0001.MP4/NORM0001.LRV/NORM0001.THM
    DCF_OBJ_TYPE_SLOWMP4 = 0x01, //SLOW0002.MP4/SLOW0002.LRV/NORM0002.THM
    DCF_OBJ_TYPE_LOOPMP4 = 0x02, //L001003.MP4/L001003.LRV/L001003.THM
    DCF_OBJ_TYPE_LPSEMP4 = 0x03, //LPSE0004.MP4/LPSE0004.LRV/LPSE0004.THM
    DCF_OBJ_TYPE_SINGJPG = 0x04, //SING0005.JPG
    DCF_OBJ_TYPE_DLAYJPG = 0x05, //DLAY0006.JPG
    DCF_OBJ_TYPE_BUSTJPG = 0x06, //B0020007.JPG
    DCF_OBJ_TYPE_CONTJPG = 0x07, //C0030008.JPG
    DCF_OBJ_TYPE_LPSEJPG = 0x08, //L0040009.JPG
    DCF_OBJ_TYPE_BUTT
}DCF_OBJ_TYPE_E;

void ShowExeHelp(char* sampleName)
{
    printf("\nUsage: %s [mode][set]\n"
           "mode: it is a must parameter\n"
           "  -R: rec mode\n"
           "  -B: board playbak mode\n"
           "  -C: client playbak mode\n"
           "\n"
           "set: it is a set parameter\n"
           " -NORMMP4:  rec file will be NORMMP4 or get Grp/Obj Cnt of NORMMP4\n"
           " -SLOWMP4:  rec file will be SLOWMP4 or get Grp/Obj Cnt of SLOWMP4\n"
           " -LOOPMP4:  rec file will be LOOPMP4 or get Grp/Obj Cnt of LOOPMP4\n"
           " -LPSEMP4:  rec file will be LPSEMP4 or get Grp/Obj Cnt of LPSEMP4\n"
           " -SINGJPG:  rec file will be SINGJPG or get Grp/Obj Cnt of SINGJPG\n"
           " -DLAYJPG:  rec file will be DLAYJPG or get Grp/Obj Cnt of DLAYJPG\n"
           " -BUSTJPG:  rec file will be BUSTJPG or get Grp/Obj Cnt of BUSTJPG\n"
           " -CONTJPG:  rec file will be CONTJPG or get Grp/Obj Cnt of CONTJPG\n"
           " -LPSEJPG:  rec file will be LPSEJPG or get Grp/Obj Cnt of LPSEJPG\n"
           "\n",
           sampleName);
}

void ShowWorkModeHelp(DCF_WORK_MODE_E enWorkMode, DCF_OBJ_TYPE_E enObjTyp)
{
    if(DCF_WORK_MODE_REC == enWorkMode)
    {
        printf("cmd[new] :NewObj with TypIdx:%d\n", enObjTyp);
    }
    else if(DCF_WORK_MODE_BOARD_PLAYBAK == enWorkMode)
    {
        printf("DCF_WORK_MODE_BOARD_PLAYBAK step as fllows:\n"
            "PlayBak Grp:GetFirstGrp/GetLastGrp----->GetNextGrp/GetPrevGrp\n"
            "PlayBak Obj:GetCurGrpFirstObj/GetCurGrpLastObj------->GetCurGrpNextObj/GetCurGrpPrevObj\n"
            "\n"
            "cmd description\n"
            "cmd[gfg] :GetFirstGrp \n"
            "cmd[glg] :GetLastGrp \n"
            "cmd[gng] :GetNextGrp \n"
            "cmd[gpg] :GetPrevGrp \n"
            "cmd[dcg] :DelCurGrp \n"
            "cmd[gfo] :GetCurGrpFirstObj \n"
            "cmd[glo] :GetCurGrpLastObj \n"
            "cmd[gno] :GetCurGrpNextObj \n"
            "cmd[gpo] :GetCurGrpPrevObj \n"
            "cmd[dco] :DelCurObj \n"
            "cmd[dfo] :DelFirstObj \n"
            "cmd[gtgc] :GetTotalGrpCnt \n"
            "cmd[ggc] :GetGrpCntByTyp with typIdx:%d \n"
            "cmd[gtoc] :GetTotalObjCnt \n"
            "cmd[goc] :GetObjCntByTyp with typIdx:%d \n"
            "\n",
            enObjTyp, enObjTyp);
    }
    else
    {
        printf("DCF_WORK_MODE_CLIENT_PLAYBAK:\n"
            "\n"
            "cmd description\n"
            "cmd[gnl] :GetNextObjList \n"
            "cmd[gpl] :GetPrevObjList \n");
    }

    printf("cmd[h]:show help info\n"
        "cmd[exit]:exit the sample\n"
        "\n");
}

void Rec(char* cmd, DCF_OBJ_TYPE_E enObjTyp)
{
    if (0 == strncmp(cmd, "new", 3))
    {
        NewObj(enObjTyp);
    }
    else
    {
        ShowWorkModeHelp(DCF_WORK_MODE_REC, enObjTyp);
    }
}

void BoardPlayBak(char* cmd, DCF_OBJ_TYPE_E enObjTyp)
{
    if (0 == strncmp(cmd, "gfg", 3))
    {
        GetFirstGrp();
    }
    else if (0 == strncmp(cmd, "glg", 3))
    {
        GetLastGrp();
    }
    else if (0 == strncmp(cmd, "gng", 3))
    {
        GetNextGrp();
    }
    else if (0 == strncmp(cmd, "gpg", 3))
    {
        GetPrevGrp();
    }
    else if (0 == strncmp(cmd, "gfo", 3))
    {
        GetCurGrpFirstObj();
    }
    else if (0 == strncmp(cmd, "glo", 3))
    {
        GetCurGrpLastObj();
    }
    else if (0 == strncmp(cmd, "gno", 2))
    {
        GetCurGrpNextObj();
    }
    else if (0 == strncmp(cmd, "gpo", 3))
    {
        GetCurGrpPrevObj();
    }
    else if (0 == strncmp(cmd, "dfo", 3))
    {
        DelFirstObj();
    }
    else if (0 == strncmp(cmd, "ggc", 3))
    {
        GetGrpCntByTyp(enObjTyp);
    }
    else if (0 == strncmp(cmd, "gtgc", 4))
    {
        GetTotalGrpCnt();
    }
    else if (0 == strncmp(cmd, "goc", 3))
    {
        GetObjCntByTyp(enObjTyp);
    }
    else if (0 == strncmp(cmd, "gtoc", 4))
    {
        GetTotalObjCnt();
    }
    else if (0 == strncmp(cmd, "dcg", 3))
    {
        DelCurGrp();
    }
    else if (0 == strncmp(cmd, "dco", 3))
    {
        DelCurObj();
    }
    else
    {
        ShowWorkModeHelp(DCF_WORK_MODE_BOARD_PLAYBAK, enObjTyp);
    }
}

void ClientPlayBak(char* cmd, DCF_OBJ_TYPE_E enObjTyp)
{
    if (0 == strncmp(cmd, "gnl", 3))
    {
        GetNextObjList();
    }
    else if (0 == strncmp(cmd, "gpl", 3))
    {
        GetPrevObjList();
    }
    else
    {
        ShowWorkModeHelp(DCF_WORK_MODE_CLIENT_PLAYBAK, enObjTyp);
    }
}

#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    DCF_WORK_MODE_E enWorkMode = DCF_WORK_MODE_BUTT;
    DCF_OBJ_TYPE_E enObjTyp = DCF_OBJ_TYPE_BUTT;

    if (argc != 3)
    {
        ShowExeHelp(argv[0]);
        return 1;
    }

    if (!strcmp("-R", argv[1]))
    {
        enWorkMode = DCF_WORK_MODE_REC;
    }
    else if (!strcmp("-B", argv[1]))
    {
        enWorkMode = DCF_WORK_MODE_BOARD_PLAYBAK;
    }
    else if (!strcmp("-C", argv[1]))
    {
        enWorkMode = DCF_WORK_MODE_CLIENT_PLAYBAK;
    }
    else
    {
        // incorrect parameter
        ShowExeHelp(argv[0]);
        return 1;
    }

    if (!strcmp("-NORMMP4", argv[2]))
    {
        enObjTyp = DCF_OBJ_TYPE_NORMMP4;
    }
    else if (!strcmp("-SLOWMP4", argv[2]))
    {
        enObjTyp = DCF_OBJ_TYPE_SLOWMP4;
    }
    else if (!strcmp("-LOOPMP4", argv[2]))
    {
        enObjTyp = DCF_OBJ_TYPE_LOOPMP4;
    }
    else if (!strcmp("-LPSEMP4", argv[2]))
    {
        enObjTyp = DCF_OBJ_TYPE_LPSEMP4;
    }
    else if (!strcmp("-SINGJPG", argv[2]))
    {
        enObjTyp = DCF_OBJ_TYPE_SINGJPG;
    }
    else if (!strcmp("-DLAYJPG", argv[2]))
    {
        enObjTyp = DCF_OBJ_TYPE_DLAYJPG;
    }
    else if (!strcmp("-BUSTJPG", argv[2]))
    {
        enObjTyp = DCF_OBJ_TYPE_BUSTJPG;
    }
    else if (!strcmp("-CONTJPG", argv[2]))
    {
        enObjTyp = DCF_OBJ_TYPE_CONTJPG;
    }
    else if (!strcmp("-LPSEJPG", argv[2]))
    {
        enObjTyp = DCF_OBJ_TYPE_LPSEJPG;
    }
    else
    {
        // incorrect parameter
        ShowExeHelp(argv[0]);
        return 1;
    }

    if (HI_SUCCESS != FileOptInit())
    {
        printf("FileOptInit fail\n");
        return -1;
    }

    char cmd[516] = {0};
    printf("Input CMD: ");
    while (NULL != fgets(cmd, 516, stdin))
    {
        if(0 == strncmp(cmd, "h", 1))
        {
            ShowWorkModeHelp(enWorkMode, enObjTyp);
            printf("Input CMD: ");
            continue;
        }
        else if(0 == strncmp(cmd, "exit", 4))
        {
            printf("will exit\n");
            break;
        }

        if(DCF_WORK_MODE_REC == enWorkMode)
        {
            Rec(cmd, enObjTyp);
        }
        else if(DCF_WORK_MODE_BOARD_PLAYBAK == enWorkMode)
        {
            BoardPlayBak(cmd, enObjTyp);
        }
        else//DCF_WORK_MODE_CLIENT_PLAYBAK == enWorkMode
        {
            ClientPlayBak(cmd, enObjTyp);
        }
        printf("Input CMD: ");
    }
    FileOptDeInit();
    return 0;
}
