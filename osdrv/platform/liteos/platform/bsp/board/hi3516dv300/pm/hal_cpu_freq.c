#include "los_printf.h"
#include "asm/delay.h"

#include "sys_pm.h"
#include "hi_io.h"
#include "pmc.h"


static void HI_SetCpuApll(unsigned rate)
{
    unsigned int u32Frac, u32Postdiv1, u32Postdiv2, u32Fbdiv, u32Refdiv;

    switch (rate)
    {
        case (400):
        {
            u32Frac = 0;
            u32Postdiv1 = 1;
            u32Postdiv2 = 1;
            u32Refdiv = 24;
            u32Fbdiv = 400;
            break;
        }
        case (600):
        {
            u32Frac = 0;
            u32Postdiv1 = 2;
            u32Postdiv2 = 1;
            u32Refdiv = 1;
            u32Fbdiv = 50;
            break;
        }
        case (732):
        {
            u32Frac = 0;
            u32Postdiv1 = 1;
            u32Postdiv2 = 1;
            u32Refdiv = 24;
            u32Fbdiv = 732;
            break;
        }
        case (850):
        {
            u32Frac = 0;
            u32Postdiv1 = 1;
            u32Postdiv2 = 1;
            u32Refdiv = 24;
            u32Fbdiv = 850;
            break;
        }
        default:
            return;
    }
    HI_RegSetBitEx(u32Frac, 0, 24, PERI_CRG0);
    HI_RegSetBitEx(u32Postdiv1, 24, 3, PERI_CRG0);
    HI_RegSetBitEx(u32Postdiv2, 28, 3, PERI_CRG0);
    HI_RegSetBitEx(u32Fbdiv, 0, 12, PERI_CRG1);
    HI_RegSetBitEx(u32Refdiv, 12, 6, PERI_CRG1);

    return;
}

void hal_cpu_set_freq_clk(unsigned int freq)
{
    HI_RegSetBitEx(0x1, 8, 2, PERI_CRG12);
    HI_SetCpuApll(freq);

    /* 3.wait for the success of APLL LOCK */
    while (1)
    {
        unsigned long value;
        HI_RegRead(&value, PERI_CRG58);
        if (value & 0x1)
            break;
    }
    HI_RegSetBitEx(0x0, 8, 2, PERI_CRG12);
    return;
}

int hal_cpu_get_freq_clk(void)
{
    unsigned long value;
    unsigned cpu_sel;
    unsigned int clk_rate;

    HI_RegRead(&value, PERI_CRG12);
    cpu_sel = value & 0x300;
    if(0x100 == cpu_sel)
    {
        clk_rate = 500;
    }
    else if(0x200 == cpu_sel)
    {
        clk_rate = 400;
    }
    else
    {
        unsigned long cpu_sel_addr0, cpu_sel_addr1;
        unsigned fbdiv, frac, refdiv, pstdiv1, pstdiv2;
        if(0x0 == cpu_sel)
        {
            cpu_sel_addr0 = PERI_CRG0;
            cpu_sel_addr1 = PERI_CRG1;
        }
        else
        {
            cpu_sel_addr0 = PERI_CRG2;
            cpu_sel_addr1 = PERI_CRG3;
        }
        HI_RegRead(&value, cpu_sel_addr0);
        frac    = 0xFFFFFF & value;
        pstdiv1 = ((0x7 << 24) & value) >> 24;
        pstdiv2 = ((0x7 << 28) & value) >> 28;
        HI_RegRead(&value, cpu_sel_addr1);
        fbdiv   = 0xFFF & value;
        refdiv  = ((0x3F << 12) & value) >> 12;
        clk_rate = 24 * (fbdiv + (frac >> 24)) / (refdiv * pstdiv1 * pstdiv2);
    }

    return clk_rate;
}

