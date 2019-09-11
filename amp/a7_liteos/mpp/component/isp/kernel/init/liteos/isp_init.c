
extern int ISP_ModInit(void);
extern void ISP_ModExit(void);

int isp_mod_init(void){
    ISP_ModInit();
    return 0;
}
void isp_mod_exit(void){
    ISP_ModExit();
}


