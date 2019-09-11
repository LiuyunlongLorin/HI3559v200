#include "los_config.h"
#ifdef LOSCFG_SHELL
#include "los_typedef.h"
#include "sys/stat.h"
#include "stdlib.h"
#include "shcmd.h"
#include "shell.h"
#include "securec.h"
#include "inode/inode.h"

#if defined(__LP64__)
#define  timeval64      timeval
#define  settimeofday64 settimeofday
#define  gettimeofday64 gettimeofday
#define  localtime64    localtime
#define  ctime64        ctime
#define  mktime64       mktime
#include "time.h"
#else
#include "time64.h"
#endif

#define  DATE_ERR_INFO      1
#define  DATE_HELP_INFO     0
#define  DATE_ERR           (-1)
#define  DATE_OK            0
#define  DATE_BASE_YEAR     1900
#define  LEAPYEAR(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

extern char *strptime(const char *buf, const char *fmt, struct tm *tm);
static const int s_wMonLengths[2][12] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

static void osCopyTm(struct tm *  pstDest, struct tm * pstSrc)
{
    if( pstSrc == NULL ) {
        (void)memset_s(pstDest,sizeof(struct tm), 0, sizeof(struct tm));
    }
    else {
            pstDest->tm_sec = pstSrc->tm_sec;
            pstDest->tm_min = pstSrc->tm_min;
            pstDest->tm_hour = pstSrc->tm_hour;
            pstDest->tm_mday = pstSrc->tm_mday;
            pstDest->tm_mon = pstSrc->tm_mon;
            pstDest->tm_year = pstSrc->tm_year;
            pstDest->tm_wday = pstSrc->tm_wday;
            pstDest->tm_yday = pstSrc->tm_yday;
            pstDest->tm_isdst = pstSrc->tm_isdst;
            pstDest->tm_gmtoff = pstSrc->tm_gmtoff;
            pstDest->tm_zone = pstSrc->tm_zone;
    }
}

static void osCmdUsageDate(int swOrder)
{
    if(swOrder)
    {
        PRINTK("date: invalid option or parameter.\n");
        PRINTK("Try 'date --help' for more information.\n");
        return ;
    }
    PRINTK("\nUsage: date [+FORMAT]\n");
    PRINTK("   or: date [-s] [YY/MM/DD] [hh:mm:ss]\n");
    PRINTK("Display the current time in the given FORMAT, or set the system date.\n");
    PRINTK("FORMAT controls the output.  Interpreted sequences are:\n");
    PRINTK("  %%b     The abbreviated month name according to the current locale.\n");
    PRINTK("  %%B     The full month name according to the current locale.\n");
    PRINTK("  %%C     The century number (year/100) as a 2-digit integer. (SU)\n");
    PRINTK("  %%d     The day of the month as a decimal number (range 01 to 31).\n");
    PRINTK("  %%e     Like %%d, the day of the month as a decimal number, \n");
    PRINTK("         but a leading zero is replaced by a space.\n");
    PRINTK("  %%h     Equivalent to %%b.  (SU)\n");
    PRINTK("  %%H     The hour as a decimal number using a 24-hour clock (range 00 to 23).\n");
    PRINTK("  %%I     The  hour as a decimal number using a 12-hour clock (range 01 to 12).\n");
    PRINTK("  %%j     The day of the year as a decimal number (range 001 to 366).\n");
    PRINTK("  %%k     The hour (24-hour clock) as a decimal number (range  0  to  23); \n");
    PRINTK("         single digits are preceded by a blank.  (See also %H.)  (TZ)\n");
    PRINTK("  %%l     The  hour  (12-hour  clock) as a decimal number (range 1 to 12); \n");
    PRINTK("         single digits are preceded by a blank.  (See also %I.)  (TZ)\n");
    PRINTK("  %%m     The month as a decimal number (range 01 to 12).\n");
    PRINTK("  %%M     The minute as a decimal number (range 00 to 59).\n");
    PRINTK("  %%n     A newline character. (SU)\n");
    PRINTK("  %%p     Either \"AM\" or \"PM\" according to the given time  value, \n");
    PRINTK("         or the corresponding  strings  for the current locale.\n");
    PRINTK("         Noon is treated as \"PM\" and midnight as \"AM\".\n");
    PRINTK("  %%P     Like %%p but in lowercase: \"am\" or \"pm\" \n");
    PRINTK("         or a corresponding string for the current locale. (GNU)\n");
    PRINTK("  %%s     The number of seconds since the Epoch, that is,\n");
    PRINTK("         since 1970-01-01 00:00:00 UTC. (TZ)\n");
    PRINTK("  %%S     The second as a decimal number (range 00 to 60).\n");
    PRINTK("         (The range is up to 60 to allow for occasional leap seconds.)\n");
    PRINTK("  %%t     A tab character. (SU)\n");
    PRINTK("  %%y     The year as a decimal number without a century (range 00 to 99).\n");
    PRINTK("  %%Y     The year as a decimal number including the century.\n");
    PRINTK("  %%%%     A literal '%%' character.\n");
    PRINTK("\nExamples:\n");
    PRINTK("Set system date (2017-01-01)\n");
    PRINTK("$ date -s 20170101\n");
    PRINTK("Set system time (12:00:00)\n");
    PRINTK("$ date -s 12:00:00\n");
    PRINTK("Show the time with format Year-Month-Day\n");
    PRINTK("$ date +%%Y-%%m-%%d\n");
}

static int osStrToTm(char *pStr,struct tm *pstTm)
{
    char *pcRet = NULL;
    if(strlen(pStr) == 8)
    {
        if(pStr[2] == ':')
        {
            pcRet = strptime(pStr,"%H:%M:%S",pstTm);
        }
        else
        {
            pcRet = strptime(pStr,"%Y%m%d",pstTm);
        }
    }
    else if(strlen(pStr) == 10)
    {
        pcRet = strptime(pStr,"%Y/%m/%d",pstTm);
    }
    else if(strlen(pStr) == 5)
    {
        if(pStr[2] == ':')
        {
            pcRet = strptime(pStr,"%H:%M",pstTm);
        }
        else if(pStr[2] == '/')
        {
            pcRet = strptime(pStr,"%m/%d",pstTm);
        }
    }
    else if(strlen(pStr) == 7)
    {
        if(pStr[4] == '/')
        {
            pcRet = strptime(pStr,"%Y/%m",pstTm);
        };
    }

    if(pstTm->tm_year < 70)
    {
        PRINTK("\nUsage: date -s set system time range from 1970.\n");
        return DATE_ERR;
    }

    if(pstTm->tm_mday > s_wMonLengths[(int)LEAPYEAR(pstTm->tm_year + DATE_BASE_YEAR)][pstTm->tm_mon])
    {
        return DATE_ERR;
    }

    /* Seconds (0-59), leap seconds shall not be used when set time. */
    if (pstTm->tm_sec < 0 || pstTm->tm_sec > 59)
    {
        return DATE_ERR;
    }

    return pcRet == NULL ? DATE_ERR : DATE_OK;
}

static int osFormatPrintTime(char *pcFormat)
{
    char acTimebuf[SHOW_MAX_LEN] = {0};
    struct tm *pstTm = NULL;
    struct timeval64 stNowTime = {0};

    if((strlen(pcFormat) < 2))
    {
        osCmdUsageDate(DATE_ERR_INFO);
        return DATE_ERR;
    }

    if(gettimeofday64(&stNowTime,NULL))
    {
        return DATE_ERR;
    }

    if((pstTm = localtime64(&(stNowTime.tv_sec))) == NULL)
    {
        return DATE_ERR;
    }

    if(strftime(acTimebuf,SHOW_MAX_LEN - 1,pcFormat + 1,pstTm))/*lint !e586*/
    {
        PRINTK("%s\n",acTimebuf);
    }
    else
    {
        osCmdUsageDate(DATE_ERR_INFO);
        return DATE_ERR;
    }
    return DATE_OK;
}

static int osDateSetTime(char *pcTimeStr)
{
    struct tm stTm = {0};
    struct timeval64 stNowTime = {0},stSetTime = {0};

    if(gettimeofday64(&stNowTime,NULL))
    {
        PRINTK("settime failed...\n");
        return DATE_ERR;
    }

    stSetTime.tv_usec = stNowTime.tv_usec;
    osCopyTm(&stTm,localtime64(&(stNowTime.tv_sec)));

    if(osStrToTm(pcTimeStr,&stTm))
    {
        osCmdUsageDate(DATE_ERR_INFO);
        return DATE_ERR;
    }

    stSetTime.tv_sec = mktime64(&stTm);

    if(settimeofday64(&stSetTime,NULL))
    {
        PRINTK("settime failed...\n");
        return DATE_ERR;
    }

    return DATE_OK;
}

#ifdef  LOSCFG_FS_VFS
static int osViewFileTime(char *filename)
{
    struct stat stat_buf = {0};
    char *fullpath = NULL;
    int ret = 0;
    char * shell_working_directory = osShellGetWorkingDirtectory();

    ret = vfs_normalize_path(shell_working_directory, filename, &fullpath);
    if (ret < 0)
    {
        set_errno(-ret);
        perror("date error");
        return DATE_ERR;
    }

    if((stat(fullpath,&stat_buf)) != 0)
    {
        osCmdUsageDate(DATE_ERR_INFO);
        free(fullpath);
        return DATE_ERR;
    }
    PRINTK("%s\n",ctime(&(stat_buf.st_mtim.tv_sec)));
    free(fullpath);
    return DATE_OK;
}
#endif

int osShellCmdDate(int argc, char **argv)
{
    struct timeval64 stNowTime = {0};

    if (argc == 1)
    {
        if(gettimeofday64(&stNowTime,NULL))
        {
            return DATE_ERR;
        }
        PRINTK("%s\n",ctime64(&(stNowTime.tv_sec)));

        return DATE_OK;
    }

    if(argc == 2)
    {
        if(!(strcmp(argv[1],"--help")))
        {
            osCmdUsageDate(DATE_HELP_INFO);
            return DATE_OK;
        }
        if(!(strncmp(argv[1],"+",1)))
        {
            return osFormatPrintTime(argv[1]);
        }
    }

    if(argc > 2)
    {
        if(!(strcmp(argv[1],"-s")))
        {
            return osDateSetTime(argv[2]);
        }
#ifdef  LOSCFG_FS_VFS
        else if(!(strcmp(argv[1],"-r")))
        {
            return osViewFileTime(argv[2]);
        }
#endif
    }

    osCmdUsageDate(DATE_ERR_INFO);
    return DATE_OK;
}

SHELLCMD_ENTRY(date_shellcmd, CMD_TYPE_STD, "date", XARGS, (CMD_CBK_FUNC)osShellCmdDate); /*lint !e19*/

#endif
