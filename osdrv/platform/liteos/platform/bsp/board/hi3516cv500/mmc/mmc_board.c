#include "hisoc/mmc.h"

#define REG_SDIO0_DLL_CTRL      0x158
#define REG_SDIO0_DLL_STATUS    0x15c
#define REG_SDIO1_DLL_CTRL      0x160
#define REG_SDIO1_DLL_STATUS    0x164
#define REG_DLL_CTRL(id)        (id == 0 ? \
                REG_SDIO0_DLL_CTRL : REG_SDIO1_DLL_CTRL)
#define REG_DLL_STATUS(id)      (id == 0 ? \
                REG_SDIO0_DLL_STATUS : REG_SDIO1_DLL_STATUS)

#define SDIO_DRV_DLL_SLAVE_EN   (0x1 << 4)
#define SDIO_DRV_DLL_SSEL_MODE  (0x1 << 7)
#define SDIO_DRV_DLL_SSEL_MASK  (0xff << 8)
#define SDIO_SAM_DLL_SLAVE_EN   (0x1 << 20)

#define SDIO_DRV_DLL_READY      (0x1 << 9)
#define SDIO_DRV_DLL_LOCK       (0x1 << 10)
#define SDIO_SAM_DLL_READY      (0x1 << 25)
#define SDIO_SAM_DLL_LOCK       (0x1 << 26)

#define REG_SDIO_DLL_SRST       0x168
#define SDIO0_DLL_SRST_REQ      (0x3 << 2)
#define SDIO1_DLL_SRST_REQ      0x3

#define REG_SDIO0_CLK_CTRL      0x178
#define REG_SDIO1_CLK_CTRL      0x17c
#define SDIO_DRV_PHASE_SEL_MASK (0x1f << 2)
#define SDIO_DRV_SEL(phase)     ((phase) << 2)
#define REG_EMMC_CLK_CTRL       0x180
#define REG_CLK_CTRL(id)        (id == 0 ? \
                REG_SDIO0_CLK_CTRL : REG_SDIO1_CLK_CTRL)

#define REG_MISC_CTRL14         0x14
#define SDIO0_PWRSW_CTRL_1V8    (0x1 << 8)
#define SDIO0_PWRSW_EN          (0x1 << 9)
#define SDIO0_PWRSW_SEL_1V8     (0x1 << 10)

#define REG_PHYINITCTRL         0x4
#define PHY_INIT_EN             (0x1 << 0)
#define PHY_DLYMEAS_EN          (0x1 << 2)

#define REG_PHY_CTL2            0x248
#define PHY_DLYMEAS_UPDATE      (0x1 << 14)
#define REG_DLY_CTL             0x250
#define PHY_DLY_CODE_R_MASK     (0x7ff << 0)

#define REG_DLY_CTL1            0x254
#define PHY_DLY_CTL1_INV_CLK    (0x1 << 31)
#define PHY_DLY1_CODE_MASK      (0x3ff << 21)
#define PHY_DLY1(dly)           ((dly) << 21)

#define REG_IOCTL_RONSEL_1_0    0x264
#define REG_IOCTL_OD_RONSEL_2   0x268

#define REG_CTRL_SDIO0_CLK      0x93c
#define REG_CTRL_SDIO0_CMD      0x940
#define REG_CTRL_SDIO0_DATA0    0x944
#define REG_CTRL_SDIO0_DATA1    0x948
#define REG_CTRL_SDIO0_DATA2    0x94c
#define REG_CTRL_SDIO0_DATA3    0x950
#define REG_CTRL_SDIO1_CLK      0x954
#define REG_CTRL_SDIO1_CMD      0x958
#define REG_CTRL_SDIO1_DATA0    0x95c
#define REG_CTRL_SDIO1_DATA1    0x960
#define REG_CTRL_SDIO1_DATA2    0x964
#define REG_CTRL_SDIO1_DATA3    0x968

static unsigned int sdr50_drv[] = {0x80, 0x60, 0x60, 0x60, 0x60, 0x60};
static unsigned int sdr25_12_drv[] = {0x60, 0x50, 0x50, 0x50, 0x50, 0x50};
static unsigned int drv[] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80};

unsigned int sdhci_set_clock(struct sdhc_host *host, unsigned int cfg_clock) /*lint -e550 -e574*/
{
    unsigned int temp = 0;
    unsigned int reg_value = 0;
    if (cfg_clock >= MMC_FREQ_200M)
        temp |= SDIO0_CLK_SEL_200M;
    else if (cfg_clock >= MMC_FREQ_150M)
        temp |= SDIO0_CLK_SEL_150M;
    else if (cfg_clock >= MMC_FREQ_125M)
        temp |= SDIO0_CLK_SEL_125M;
    else if (cfg_clock >= MMC_FREQ_100M)
        temp |= SDIO0_CLK_SEL_100M;
    else if (cfg_clock >= MMC_FREQ_50M)
        temp |= SDIO0_CLK_SEL_50M;
    else if (cfg_clock >= MMC_FREQ_25M)
        temp |= SDIO0_CLK_SEL_25M;
    else if (cfg_clock >= MMC_FREQ_400K)
        temp |= SDIO0_CLK_SEL_400K;
    else if (cfg_clock >= MMC_FREQ_100K)
        temp |= SDIO0_CLK_SEL_100K;
    else {
        mmc_err("no suitable clock! clock = %d\n",cfg_clock);
        return 0;
    }
    mmc_trace(4,"set clock reg value = 0x%08x",temp); /*lint !e506*/

    switch (host->id) {
        case 0:
            reg_value = sdhci_crg_readl(PERI_CRG94);
            reg_value &= ~(MMC_FREQ_MASK << MMC_FREQ_SHIFT);
            reg_value |= ((temp & MMC_FREQ_MASK) << MMC_FREQ_SHIFT);
            sdhci_crg_writel(reg_value, PERI_CRG94);

            break;
        case 1:
            reg_value = sdhci_crg_readl(PERI_CRG95);
            reg_value &= ~(MMC_FREQ_MASK << MMC_FREQ_SHIFT);
            reg_value |= ((temp & MMC_FREQ_MASK) << MMC_FREQ_SHIFT);
            sdhci_crg_writel(reg_value, PERI_CRG95);
            break;
        case 2:
            reg_value = sdhci_crg_readl(PERI_CRG96);
            reg_value &= ~(MMC_FREQ_MASK << MMC_FREQ_SHIFT);
            reg_value |= ((temp & MMC_FREQ_MASK) << MMC_FREQ_SHIFT);
            sdhci_crg_writel(reg_value, PERI_CRG96);
            break;
        default:
            mmc_err("host is not support! = %d\n", host->id);
            break;
    }
    return cfg_clock;
} /*lint +e550 +e574*/

void sdhci_set_bus_width(struct sdhc_host *host, int width)
{

}

void sdhci_hisi_assert_reset_dll(struct sdhc_host *host)
{
    unsigned int reg = 0;

    // i think i should lock here.
    reg = readl(CRG_REG_BASE + REG_SDIO_DLL_SRST);
    reg |= (host->id == 0 ? SDIO0_DLL_SRST_REQ : SDIO1_DLL_SRST_REQ);
    writel(reg, CRG_REG_BASE + REG_SDIO_DLL_SRST);
}

void sdhci_hisi_deassert_reset_dll(struct sdhc_host *host)
{
    unsigned int reg = 0;

    // i think i should lock here.
    reg = readl(CRG_REG_BASE + REG_SDIO_DLL_SRST);
    reg &= ~(host->id == 0 ? SDIO0_DLL_SRST_REQ : SDIO1_DLL_SRST_REQ);
    writel(reg, CRG_REG_BASE + REG_SDIO_DLL_SRST);
}

void sdhci_hisi_set_drv_dll(struct sdhc_host *host)
{
    unsigned int reg, dly, timeout;
    if (host->id == 2) {
        if (host->timing == TIMING_MMC_HS200) {
            reg = readl(EMMC_PHY_BASE + REG_PHYINITCTRL);
            reg |= PHY_DLYMEAS_EN | PHY_INIT_EN;
            writel(reg, EMMC_PHY_BASE + REG_PHYINITCTRL);

            timeout = 10;
            do {

                reg = readl(EMMC_PHY_BASE + REG_PHYINITCTRL);
                if (!(reg & (PHY_DLYMEAS_EN | PHY_INIT_EN)))
                    break;
                mmc_delay_us(100);
                if (timeout-- == 0) {
                    printk("wait mmc phy cal done time out\n");
                    return;
                }
            } while(1);/*lint !e506*/


            reg = readl(EMMC_PHY_BASE + REG_PHY_CTL2);
            reg |= PHY_DLYMEAS_UPDATE;
            writel(reg, EMMC_PHY_BASE + REG_PHY_CTL2);

            reg = readl(EMMC_PHY_BASE + REG_DLY_CTL);
            dly = reg & PHY_DLY_CODE_R_MASK;
            dly = dly / 3;
            reg = readl(EMMC_PHY_BASE + REG_DLY_CTL1);
            reg &= ~(PHY_DLY1_CODE_MASK | PHY_DLY_CTL1_INV_CLK);/*lint !e648*/
            reg |= PHY_DLY1(dly);
            writel(reg, EMMC_PHY_BASE + REG_DLY_CTL1);

            reg = readl(EMMC_PHY_BASE + REG_PHY_CTL2);
            reg &= ~PHY_DLYMEAS_UPDATE;
            writel(reg, EMMC_PHY_BASE + REG_PHY_CTL2);

        }

        return ;
    }
    if (host->clock > 50000000) {
        timeout = 20;
        do {
            reg = readl(CRG_REG_BASE + REG_DLL_STATUS(host->id));
            if (reg & SDIO_DRV_DLL_LOCK)
                break;
            mmc_delay_ms(1);
            if (--timeout == 0) {
                mmc_err("drv_dll never lock!\n");
                return;
            }
        } while(1);/*lint !e506*/

        reg = readl(CRG_REG_BASE + REG_CLK_CTRL(host->id));
        reg &= ~SDIO_DRV_PHASE_SEL_MASK;
        reg |= SDIO_DRV_SEL(0x11);
        writel(reg, CRG_REG_BASE + REG_CLK_CTRL(host->id));

        reg = readl(CRG_REG_BASE + REG_DLL_CTRL(host->id));
        reg &= ~(SDIO_DRV_DLL_SSEL_MODE | SDIO_DRV_DLL_SSEL_MASK);
        reg |= SDIO_DRV_DLL_SLAVE_EN;
        writel(reg, CRG_REG_BASE + REG_DLL_CTRL(host->id));

        timeout = 20;

        do {
            reg = readl(CRG_REG_BASE + REG_DLL_STATUS(host->id));
            if (reg & SDIO_DRV_DLL_READY)
                break;

            mmc_delay_ms(1);
            if (--timeout == 0) {
                mmc_err("drv_dll never ready.\n");
                return;
            }
        } while(1); /*lint !e506*/
    } else {
        reg = readl(CRG_REG_BASE + REG_DLL_CTRL(host->id));
        reg &= ~SDIO_DRV_DLL_SSEL_MASK;
        reg |= SDIO_DRV_DLL_SSEL_MODE;
        if (host->timing == TIMING_SD_HS || host->timing == TIMING_UHS_SDR25) {
            reg |= (0x90 << 8);
        } else {
            reg |= (0xff << 8);
        }
        writel(reg, CRG_REG_BASE + REG_DLL_CTRL(host->id));
    }
}

void sdhci_hisi_sam_dll_slave_en(struct sdhc_host *host,
        unsigned int phase)
{
    unsigned int reg, timeout = 20;

    reg = readl(CRG_REG_BASE + REG_DLL_CTRL(host->id));
    reg |= SDIO_SAM_DLL_SLAVE_EN;
    writel(reg, CRG_REG_BASE + REG_DLL_CTRL(host->id));
    do {
        reg = readl(CRG_REG_BASE + REG_DLL_STATUS(host->id));
        if (reg & SDIO_SAM_DLL_READY || !phase)
            break;

        mmc_delay_ms(1);
        if (--timeout == 0) {
            mmc_err(" sam_dll never ready.\n");
            return;
        }
    } while (1);/*lint !e506*/

    mmc_delay_ms(1);
}

void sdhci_hisi_set_samp_phase(struct sdhc_host *host, unsigned int phase)
{
    unsigned int reg;

    reg = sdhc_readl(host, SDHC_AUTO_TUNING_STATUS);
    reg &= ~SDHC_CENTER_PH_CODE_MASK;
    reg |= phase;
    sdhc_writel(host, reg, SDHC_AUTO_TUNING_STATUS);

    if (host->id != 2)
        sdhci_hisi_sam_dll_slave_en(host, phase);
}

void sdhci_set_uhs_signaling(struct sdhc_host *host, enum mmc_bus_timing timing)
{
    unsigned int ctrl_2 = 0;
    unsigned int reg_addr, start, end;
    unsigned int devid = host->id;
    unsigned int *pin_drv_cap;

    mmc_trace(4,"timing = %d\n",timing); /*lint !e506*/
    ctrl_2 = sdhc_readw(host, SDHC_HOST_CONTROL2);
    /* Select Bus Speed Mode for host */
    ctrl_2 &= ~SDHC_CTRL_UHS_MASK;
    if ((timing == TIMING_MMC_HS200) ||
            (timing == TIMING_UHS_SDR104))
        ctrl_2 |= SDHC_CTRL_UHS_SDR104;
    else if (timing == TIMING_UHS_SDR12)
        ctrl_2 |= SDHC_CTRL_UHS_SDR12;
    else if (timing == TIMING_UHS_SDR25)
        ctrl_2 |= SDHC_CTRL_UHS_SDR25;
    else if (timing == TIMING_UHS_SDR50)
        ctrl_2 |= SDHC_CTRL_UHS_SDR50;
    else if ((timing == TIMING_UHS_DDR50) ||
            (timing == TIMING_UHS_DDR52))
        ctrl_2 |= SDHC_CTRL_UHS_DDR50;
    else if (timing == TIMING_MMC_HS400)
        ctrl_2 |= SDHC_CTRL_HS400;
    sdhc_writew(host, ctrl_2, SDHC_HOST_CONTROL2);

    if (host->id == 2) {
        if (timing == TIMING_MMC_HS200) {
            sdhc_writel(host, SDHC_CARD_IS_EMMC, SDHC_EMMC_CTRL);
            /* set drv strength to 40ohm */
            writel(0x7ff, EMMC_PHY_BASE + REG_IOCTL_RONSEL_1_0);
            writel(0x7ff, EMMC_PHY_BASE + REG_IOCTL_OD_RONSEL_2);
        }
    } else {
        start = devid == 0 ? REG_CTRL_SDIO0_CLK : REG_CTRL_SDIO1_CLK;
        end = devid == 0 ? REG_CTRL_SDIO0_DATA3 : REG_CTRL_SDIO1_DATA3;

        if (timing == TIMING_UHS_SDR50) {
                pin_drv_cap = sdr50_drv;
        } else if (timing == TIMING_UHS_SDR12 ||
                        timing == TIMING_UHS_SDR25) {
                pin_drv_cap = sdr25_12_drv;
        } else {
                pin_drv_cap = drv;
        }

        for (reg_addr = start; reg_addr <= end; reg_addr += 4) {
            writel(*pin_drv_cap, (IO_MUX_REG_BASE + reg_addr));
            pin_drv_cap++;
        }
    }
}

void sdhci_host_quirks(struct sdhc_host *host)
{
    switch (host->id) {
        case MMC0:
            host->flags &= ~(SDHC_CAN_DO_SDMA | SDHC_AUTO_CMD23 | SDHC_AUTO_CMD12);
            break;
        case MMC1:
            host->flags &= ~(SDHC_CAN_DO_SDMA | SDHC_AUTO_CMD23 | SDHC_AUTO_CMD12);
            break;
        case MMC2:
            host->flags &= ~(SDHC_CAN_DO_SDMA | SDHC_AUTO_CMD23 | SDHC_AUTO_CMD12);
            sdhc_writew(host, 0x1, 0x52c);
            mmc_trace(3,"EMMC CTRL(0x52c):%x\n", sdhc_readw(host, 0x52c)); /*lint !e506*/
            mmc_trace(3,"REG(0x534):%x\n", sdhc_readw(host, 0x534)); /*lint !e506*/

            break;
        default:
            break;
    }
}

void sdhci_pltfm_init(struct sdhc_host *host) /*lint -e550*/
{
    struct mmc_host * mmc = host->mmc;

    host->quirks.bits.quirk_broken_dma = 1;
    host->quirks.bits.quirk_data_timeout_use_sdclk = 1;
    host->quirks.bits.quirk_inverted_write_protect = 1;
    host->quirks.bits.quirk_cap_clock_base_broken = 1;
    host->quirks.bits.quirk_broken_timeout_val = 1;
    host->quirks2.bits.quirk2_broken_ddr50 = 1;


    switch (host->id) {
        case MMC0:
            host->max_clk = CONFIG_MMC0_CCLK_MAX;
            break;
        case MMC1:
            host->max_clk = CONFIG_MMC1_CCLK_MAX;
            break;
        case MMC2:
            host->max_clk = CONFIG_MMC2_CCLK_MAX;
            mmc->caps.bits.cap_nonremovable = 1;
            mmc->caps.bits.cap_8_bit = 1;
            mmc->caps2.bits.caps2_HS200_1v8_SDR = 1;
            mmc->caps2.bits.caps2_HS200_1v2_SDR = 1;
            break;
        default:
            break;
    }
}/*lint +e550*/
/* end of file board.c */
