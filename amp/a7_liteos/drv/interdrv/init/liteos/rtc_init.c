#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#ifndef NULL
#define NULL  ((void *)0)
#endif

extern int  rtc_init(void);
extern void  rtc_exit(void);

int rtc_mod_init()
{
    return rtc_init();
}

void rtc_mod_exit()
{
    rtc_exit();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* __cplusplus */
