#include "los_base.h"
#include "linux/device.h"
#include "linux/kernel.h"
#include "linux/i2c.h"
#include "linux/delay.h"
#include "linux/errno.h"
#include "linux/io.h"
#include "linux/module.h"
#include "i2c-hisilicon.h"
#include "i2c.h"
#include "hisoc/i2c.h"
#include "linux/platform_device.h"

#ifdef LOSCFG_DRIVERS_HIDMAC
#include "hi_dmac.h"
#endif
#define I2C_WAIT_TIME_OUT       0x1000000
#define I2C_WAIT_IDLE_TIME_OUT  0x1000000

static void hi_i2c_abortprocess(struct i2c_driver_data *pdata)
{
    unsigned int auto_status;
    unsigned int tx_src;

    tx_src = readl(pdata->regbase + I2C_TX_ABRT_SRC);
    i2c_err("tx_src: 0x%x!\n", tx_src);

    /* disable i2c */
    writel(0, pdata->regbase + I2C_ENABLE_REG);

    auto_status = readl(pdata->regbase + I2C_AUTO_REG);

    /* clear 0xB0 err status */
    /* auto_mst_tx_abrt_clr
       auto_tx_cmd_fifo_over_clr
       auto_rx_cmd_fifo_under_clr
       auto_rx_cmd_fifo_over_clr
     */
    auto_status |= 0x0f000000;
    writel(auto_status, pdata->regbase + I2C_AUTO_REG);
    writel(0x1, pdata->regbase + I2C_CLR_INTR_REG);

    /* enable i2c */
    writel(0x1, pdata->regbase + I2C_ENABLE_REG);
}

void hi_i2c_set_rate(struct i2c_driver_data* pdata)
{
    unsigned int apb_clk, scl_h, scl_l, hold;

    /* get apb bus clk for diff plat */
    apb_clk = pdata->clk;

    /* set SCLH and SCLL depend on apb_clk and def_rate */
    if (pdata->freq <= I2C_DFT_RATE) {
        scl_h = (apb_clk / I2C_DFT_RATE) / 2;
        scl_l = scl_h;
    } else {
        scl_h = (apb_clk * 36) / (pdata->freq * 100);
        scl_l = (apb_clk * 64) / (pdata->freq * 100);
    }

    writel(scl_h, pdata->regbase + I2C_SCL_H_REG);
    writel(scl_l, pdata->regbase + I2C_SCL_L_REG);

    /* set hi_i2c hold time */
    hold = scl_h / 2;
    writel(hold, pdata->regbase + I2C_SDA_HOLD_REG);
}

void hi_i2c_hw_init(struct i2c_driver_data* pdata)
{
    unsigned int temp, rx_fifo, tx_fifo;
    struct hi_platform_i2c * hi_data = (struct hi_platform_i2c *)pdata->private;

    /* unlock hi_i2c controller to access */
    writel(HI_I2C_UNLOCK_VALUE, pdata->regbase + I2C_LOCK_REG);

    /* disable hi_i2c controller */
    temp = readl(pdata->regbase + I2C_ENABLE_REG);
    writel((temp & ~HI_I2C_ENABLE), pdata->regbase + I2C_ENABLE_REG);

    /* clear interrupt */
    writel(0x1, pdata->regbase + I2C_CLR_INTR_REG);
    /* clear interrupt mask */
    writel(0xfff, pdata->regbase + I2C_INTR_MASK_REG);

    /* disable hi_i2c auto_mode */
    writel(HI_I2C_AUTO_MODE_OFF, pdata->regbase + I2C_AUTO_REG);

    /* set hi_i2c in fast mode */
    writel(HI_I2C_FAST_MODE, pdata->regbase + I2C_CON_REG);

    /* set hi_i2c rate */
    hi_i2c_set_rate(pdata);

    rx_fifo = CONFIG_HI_I2C_RX_FIFO;
    tx_fifo = CONFIG_HI_I2C_TX_FIFO;

    /* set hi_i2c fifo */
    writel(rx_fifo, pdata->regbase + I2C_RX_TL_REG);
    writel(tx_fifo, pdata->regbase + I2C_TX_TL_REG);

    /* enable hi_i2c controller */
    temp = readl(pdata->regbase + I2C_ENABLE_REG);
    writel((temp | HI_I2C_ENABLE), pdata->regbase + I2C_ENABLE_REG);

    hi_data->g_last_dev_addr = 0;
    hi_data->g_last_mode = I2C_MODE_NONE;

    pdata->msgs = NULL;
    pdata->msg_num = 0;
}

int hi_i2c_wait_idle(struct i2c_driver_data *pdata)
{
    unsigned int  work_status;
    unsigned int  auto_status;
    unsigned int  int_raw_status = 0;
    unsigned int  i = 0;
    int ret = 0;

    work_status = readl(pdata->regbase + I2C_STATUS_REG);
    auto_status = readl(pdata->regbase + I2C_AUTO_REG);

    while (!(IS_FIFO_EMPTY(auto_status) && IS_I2C_IDLE(work_status))) {
        if (i > I2C_WAIT_IDLE_TIME_OUT) {
            i2c_err("wait i2c idle timeout!"\
                    "auto_status: 0x%x, work_status: 0x%x\n",
                    auto_status, work_status);
            ret = -1;
            break;
        }
        i++;
        i2c_msg("===== i: %d, auto_status: 0x%x,"\
            "work_status: 0x%x.\n", i, auto_status, work_status);
        work_status = readl(pdata->regbase + I2C_STATUS_REG);
        auto_status = readl(pdata->regbase + I2C_AUTO_REG);
    };

#ifdef LOSCFG_DRIVERS_HIDMAC
    unsigned int dmac_finish;

    dmac_finish = readl(IO_ADDRESS(DMAC_REG_BASE) + 0x010c);
    while (dmac_finish & 0xfff) {
        if (i > 0x10000)
            break;
        i++;
        dmac_finish = readl(IO_ADDRESS(DMAC_REG_BASE) + 0x010c);
    }
#endif

    int_raw_status = readl(pdata->regbase + I2C_INTR_RAW_REG);

    if ((int_raw_status & I2C_RAW_TX_ABORT) == I2C_RAW_TX_ABORT) {
        i2c_err("transmit error, int_raw_status: 0x%x!\n",
                int_raw_status);
        hi_i2c_abortprocess(pdata);
        ret = -1;
    }

    return ret;
}

/* wait until tx fifo is not full */
int hi_i2c_wait_txfifo_notfull(struct i2c_driver_data *pdata)
{
    unsigned int  auto_status = 0;
    unsigned int  int_raw_status = 0;
    unsigned int  i = 0;
    int ret = 0;

    auto_status = readl(pdata->regbase + I2C_AUTO_REG);

    while ((auto_status & I2c_AUTO_TX_FIFO_NOT_FULL)
            != I2c_AUTO_TX_FIFO_NOT_FULL) {
        if (i > I2C_WAIT_TIME_OUT) {
            i2c_err("wait timeout, auto_status: 0x%x!\n",
                    auto_status);
            ret = -1;
            break;
        }

        i++;

        i2c_msg("===== i: %d, auto_status: 0x%x\n", i, auto_status);

        auto_status = readl(pdata->regbase + I2C_AUTO_REG);
    };

    int_raw_status = readl(pdata->regbase + I2C_INTR_RAW_REG);

    if ((int_raw_status & I2C_RAW_TX_ABORT) == I2C_RAW_TX_ABORT) {
        i2c_err("transmit error, int_raw_status: 0x%x!\n",
                int_raw_status);
        i2c_err("tx_abrt_cause is %x.\n",
                readl(pdata->regbase + I2C_TX_ABRT_SRC));
        hi_i2c_abortprocess(pdata);
        ret = -1;
    }

    return ret;
}

/* wait until tx fifo is not empty */
int hi_i2c_wait_rxfifo_notempty(struct i2c_driver_data *pdata)
{
    unsigned int  auto_status = 0;
    unsigned int  int_raw_status = 0;
    unsigned int  i = 0;
    int ret = 0;

    auto_status = readl(pdata->regbase + I2C_AUTO_REG);
    while ((auto_status & I2C_AUTO_RX_FIFO_NOT_EMPTY)
            != I2C_AUTO_RX_FIFO_NOT_EMPTY) {
        if (i > I2C_WAIT_TIME_OUT) {
            i2c_err("wait timeout! auto_status: 0x%x\n",
                    auto_status);
            ret = -1;
            break;
        }
        i++;

        auto_status = readl(pdata->regbase + I2C_AUTO_REG);
        //i2c_msg("===== i: %d, auto_status: 0x%x\n", i, auto_status);
    };

    if (ret == -1) {
        int_raw_status = readl(pdata->regbase + I2C_INTR_RAW_REG);
        if ((int_raw_status & I2C_RAW_TX_ABORT) == I2C_RAW_TX_ABORT) {
            i2c_err("transmit error, int_raw_status: 0x%x!\n",
                    int_raw_status);
            i2c_err("tx_abrt_cause is %x.\n",
                readl(pdata->regbase + I2C_TX_ABRT_SRC));
            hi_i2c_abortprocess(pdata);
            ret = -1;
        }
    }

    return ret;
}

int hi_i2c_set_dev_addr_and_mode(struct i2c_driver_data *pdata,
        i2c_mode_e work_mode)
{
    unsigned int dev_addr = pdata->msgs->addr;
    struct hi_platform_i2c * hi_data = (struct hi_platform_i2c *)pdata->private;

    /* wait until all cmd in fifo is finished and i2c is idle */
    if (hi_i2c_wait_idle(pdata) < 0) {
        i2c_err("wait i2c idle time out.\n");
        return -1;
    }

    if ((hi_data->g_last_dev_addr == dev_addr)
            && (hi_data->g_last_mode == work_mode))
        return 0;

    /* disable i2c */
    writel(0x0, pdata->regbase + I2C_ENABLE_REG);
    /* clear err status */
    writel(0x0f000000, pdata->regbase + I2C_AUTO_REG);

    /* different device, need to reinit i2c ctrl */
    if ((hi_data->g_last_dev_addr) != dev_addr) {
        /* set slave dev addr */
#ifndef I2C_DEVADDR_WITH_RWBIT
        writel(((dev_addr & 0xff)), pdata->regbase + I2C_TAR_REG);
#else
        writel(((dev_addr & 0xff) >> 1), pdata->regbase + I2C_TAR_REG);
#endif
        hi_data->g_last_dev_addr = dev_addr;
    }

    if (hi_data->g_last_mode != work_mode) {
        /* set auto mode */
        if (work_mode == I2C_MODE_AUTO) {
            writel(0x0, pdata->regbase + I2C_DMA_CMD0);
            writel(0x80000000, pdata->regbase + I2C_AUTO_REG);
            hi_data->g_last_mode = work_mode;
        } else if (work_mode == I2C_MODE_DMA) {
            writel(0x0, pdata->regbase + I2C_AUTO_REG);
            hi_data->g_last_mode = work_mode;
        } else {
            i2c_err("invalid i2c mode\n");
            return -1;
        }
    }

    /*  enable i2c */
    writel(0x1, pdata->regbase + I2C_ENABLE_REG);

    return 0;
}

int hi_i2c_write(struct i2c_driver_data *pdata)
{
    unsigned int reg_val;
    unsigned int temp_reg;
    unsigned int temp_data;
    unsigned int temp_auto_reg;
    struct i2c_msg *msgs = pdata->msgs;

    if (hi_i2c_set_dev_addr_and_mode(pdata, I2C_MODE_AUTO) < 0)
        return -1;

    temp_auto_reg = HI_I2C_WRITE;

    if (msgs->flags & I2C_M_16BIT_REG) {
        /* 16bit reg addr */
        temp_auto_reg |= I2C_AUTO_ADDR;

        /* switch high byte and low byte */
        temp_reg = msgs->buf[pdata->msg_idx] << 8;

        pdata->msg_idx++;

        temp_reg |= msgs->buf[pdata->msg_idx];

        pdata->msg_idx++;
    } else {
        temp_reg = msgs->buf[pdata->msg_idx];
        pdata->msg_idx++;
    }

    if (msgs->flags & I2C_M_16BIT_DATA) {
        /* 16bit data */
        temp_auto_reg |= I2C_AUTO_DATA;

        /* switch high byte and low byte */
        temp_data =  msgs->buf[pdata->msg_idx] << 8;

        pdata->msg_idx++;

        temp_data |= msgs->buf[pdata->msg_idx];

        pdata->msg_idx++;
    } else {
        temp_data = msgs->buf[pdata->msg_idx];
        pdata->msg_idx++;
    }

    writel(temp_auto_reg, pdata->regbase + I2C_AUTO_REG);
    i2c_msg("temp_auto_reg: 0x%x\n", temp_auto_reg);

    /* set write reg&data */
    reg_val = (temp_reg << REG_SHIFT) | temp_data;

    /* wait until tx fifo not full */
    if (hi_i2c_wait_txfifo_notfull(pdata) < 0)
        return -1;

    i2c_msg("reg_val = %x\n", reg_val);

    writel(reg_val, pdata->regbase + I2C_TX_RX_REG);

    i2c_msg("dev_addr =%x, reg_addr = %x, Data = %x\n",
        pdata->msgs->addr, pdata->msgs->buf[0], pdata->msgs->buf[1]);

    return pdata->msg_idx;
}

unsigned int hi_i2c_read(struct i2c_driver_data *pdata)
{
    unsigned int reg_val;
    unsigned int temp_reg;
    unsigned int ret_data = 0xffff;
    unsigned int temp_auto_reg;
    unsigned int data_num = 0;
    struct i2c_msg *msgs = pdata->msgs;

    if (hi_i2c_set_dev_addr_and_mode(pdata, I2C_MODE_AUTO) < 0)
        return -1;

    temp_auto_reg = HI_I2C_READ;

    if (msgs->flags & I2C_M_16BIT_REG) {
        /* 16bit reg addr */
        temp_auto_reg |= I2C_AUTO_ADDR;

        /* switch high byte and low byte */
        temp_reg = msgs->buf[pdata->msg_idx] << 8;
        pdata->msg_idx++;
        temp_reg |= msgs->buf[pdata->msg_idx];
    } else {
        temp_reg = msgs->buf[pdata->msg_idx];
        pdata->msg_idx++;
    }

    if (msgs->flags & I2C_M_16BIT_DATA)
        /* 16bit data */
        temp_auto_reg |= I2C_AUTO_DATA;

    writel(temp_auto_reg, pdata->regbase + I2C_AUTO_REG);

    /* 1. write addr */
    reg_val = temp_reg << REG_SHIFT;

    /* wait until tx fifo not full  */
    if (hi_i2c_wait_txfifo_notfull(pdata) < 0)
        return -1;

    /* regaddr */
    writel(reg_val, pdata->regbase + I2C_TX_RX_REG);

    /* 2. read return data */
    /* wait until rx fifo not empty  */

    if (hi_i2c_wait_rxfifo_notempty(pdata) < 0)
        return -1;

    ret_data = readl(pdata->regbase + I2C_TX_RX_REG) & DATA_16BIT_MASK;

    if (msgs->flags & I2C_M_16BIT_DATA) {
        pdata->msgs->buf[0] = ret_data & DATA_8BIT_MASK;
        pdata->msgs->buf[1] = (ret_data >> 8) & DATA_8BIT_MASK;
        data_num = 2;
    } else {
        pdata->msgs->buf[0] = ret_data & DATA_8BIT_MASK;
        data_num = 1;
    }

    writel(0x1, pdata->regbase + I2C_CLR_INTR_REG);

    return data_num;
}
/*
 * dma functions
 */
#ifdef LOSCFG_DRIVERS_HIDMAC
extern int dmac_channel_free(unsigned int channel);
extern int dmac_wait(unsigned int channel);
extern int do_dma_llim2p(unsigned int memaddr, unsigned int peri_addr, unsigned int length);
extern int do_dma_m2p(unsigned int memaddr, unsigned int peri_addr, unsigned int length);
extern int do_dma_llip2m(unsigned int memaddr, unsigned int peri_addr, unsigned int length);
extern int do_dma_p2m(unsigned int memaddr, unsigned int peri_addr, unsigned int length);
void hi_i2c_dma_enable(struct i2c_driver_data *pdata, unsigned int dir)
{
    /* register I2C_DMA_CR bit[0] is the tdmae; bit[1] is the rdmae */
    writel((1 << dir), pdata->regbase + I2C_DMA_CTRL_REG);
}

void hi_i2c_dmac_config(struct i2c_driver_data *pdata, unsigned int dir)
{
    /* 1. enable RX(0) or TX(1) in DMA mode */
    hi_i2c_dma_enable(pdata, dir);//register I2C_DMA_CR bit[0] is the tdmae; bit[1] is the rdmae

    /* 2. set dma fifo */
    writel(4, pdata->regbase + I2C_DMA_TDLR);
    writel(4, pdata->regbase + I2C_DMA_RDLR);
}

void hi_i2c_start_rx(struct i2c_driver_data *pdata, unsigned int reg_addr, unsigned int length)
{
    unsigned int reg;
    writel(reg_addr, pdata->regbase + I2C_DMA_CMD1);
    writel((length - 1), pdata->regbase + I2C_DMA_CMD2);

    reg = readl(pdata->regbase + I2C_DMA_CMD0);

    /*start tx*/
    reg &= ~I2C_DMA_CMD0_RW;
    writel((I2C_DMA_CMD0_MODE_EN | reg), pdata->regbase + I2C_DMA_CMD0);
}

void hi_i2c_start_tx(struct i2c_driver_data *pdata, unsigned int reg_addr, unsigned int length)
{
    unsigned int reg;

    writel(reg_addr, pdata->regbase + I2C_DMA_CMD1);
    writel((length - 1), pdata->regbase + I2C_DMA_CMD2);

    reg = readl(pdata->regbase + I2C_DMA_CMD0);
    /*start rx*/
    writel(((I2C_DMA_CMD0_RW | I2C_DMA_CMD0_MODE_EN) | reg), pdata->regbase + I2C_DMA_CMD0);
}

int wait_add_cnt_full(struct i2c_driver_data *pdata)
{
    unsigned int temp_cmd_reg;
    unsigned int i = 0;
    int ret = 1;

    temp_cmd_reg = readl(pdata->regbase + I2C_DMA_CMD0);
    while ((temp_cmd_reg & I2C_DMA_CMD0_ADD_CNT_FULL) != I2C_DMA_CMD0_ADD_CNT_FULL)
    {
        if (i > I2C_WAIT_TIME_OUT)
        {
            i2c_err("wait timeout! IC2_DMA_CMD0: 0x%x\n", temp_cmd_reg);
            ret = 0;
            break;
        }
        i++;

        temp_cmd_reg = readl(pdata->regbase + I2C_DMA_CMD0);
        i2c_msg("===== i: %d, I2C_DMA_CMD0: 0x%x\n", i, temp_cmd_reg);
    };
    return ret;
}

int dma_to_i2c_lli(unsigned int src, unsigned int dst, unsigned int length)
{
    int chan;

    chan = do_dma_llim2p(src, dst, length);
    if (chan == -1)
        i2c_err("dma_to_i2c error\n");

    return chan;
}

int dma_to_i2c(unsigned int src, unsigned int dst, unsigned int length)
{
    int chan;

    chan = do_dma_m2p(src, dst, length);
    if (chan == -1)
        i2c_err("dma_to_i2c error\n");

    return chan;
}

int i2c_to_dma_lli(unsigned int src, unsigned int dst, unsigned int length)
{
    int chan;

    chan = do_dma_llip2m(dst, src, length);
    if (chan == -1)
        i2c_err("dma_p2m error...\n");
    return chan;
}

int i2c_to_dma(unsigned int src, unsigned int dst, unsigned int length)
{
    int chan;

    chan = do_dma_p2m(dst, src, length);
    if (chan == -1)
        i2c_err("dma_p2m error...\n");
    return chan;
}

int hi_i2c_dma_write(struct i2c_driver_data *pdata)
{
    int chan;
    int memalign_num = 0;
    unsigned short data_num = 0;
    struct i2c_msg *msgs = pdata->msgs;
    unsigned int temp_reg = 0;
    char *allign_data_addr = NULL;

    /* 1. switch i2c devaddr and dma mode*/
    if (hi_i2c_set_dev_addr_and_mode(pdata, I2C_MODE_DMA) < 0)
        return -1;

    if (msgs->flags & I2C_M_16BIT_REG) {
        /* 16bit reg addr */
        /* switch high byte and low byte */
        temp_reg =  msgs->buf[pdata->msg_idx] << 8;
        pdata->msg_idx++;
        temp_reg |= msgs->buf[pdata->msg_idx];
        pdata->msg_idx++;

        writel(I2C_DMA_CMD0_ADDR_MODE_16, pdata->regbase + I2C_DMA_CMD0);

        // slave addr should not be contained in the data transferred through dma
        data_num = msgs->len - 2;
    } else {
        temp_reg = msgs->buf[pdata->msg_idx];
        pdata->msg_idx++;

        writel(I2C_DMA_CMD0_ADDR_MODE_8, pdata->regbase + I2C_DMA_CMD0);

        // slave addr should not be contained in the data transferred through dma
        data_num = msgs->len - 1;
    }

    memalign_num = data_num / CACHE_ALIGNED_SIZE;
    if ((data_num % CACHE_ALIGNED_SIZE) != 0)
        memalign_num++;

    allign_data_addr = (char *)memalign(CACHE_ALIGNED_SIZE, CACHE_ALIGNED_SIZE * memalign_num);
    if(allign_data_addr == NULL)
        return -1;

    memcpy(allign_data_addr, &(msgs->buf[msgs->len - data_num]), data_num);

    /*clean cache*/
    dma_cache_clean((AARCHPTR)allign_data_addr, (AARCHPTR)(allign_data_addr + (CACHE_ALIGNED_SIZE * memalign_num)));

    /* 2. config i2c into DMA mode */
    //register I2C_DMA_CR bit[0] is the tdmae; bit[1] is the rdmae
    hi_i2c_dmac_config(pdata, 0x1);

    /* 3. start i2c logic to write */
    hi_i2c_start_tx(pdata, temp_reg, data_num);
    /* wait add_cnt_full to 1 */
    if(wait_add_cnt_full(pdata) > 0)
    {
        /* 4. transmit DATA from DMAC to I2C in DMA mode */
        // slave addr should not be contained in the data transferred through dma
        chan = dma_to_i2c((unsigned int)allign_data_addr,
                (unsigned int)(pdata->regbase + I2C_DATA_CMD_REG), data_num);
        if (dmac_wait(chan) != DMAC_CHN_SUCCESS) {
            free(allign_data_addr);
            i2c_err("dma wait failed\n");
            return -1;
        }

        dmac_channel_free(chan);
    }
    free(allign_data_addr);
    return 0;
}

unsigned int hi_i2c_dma_read(struct i2c_driver_data *pdata)
{
    int chan;
    int memalign_num = 0;
    unsigned short data_num = 0;
    struct i2c_msg *msgs = pdata->msgs;
    unsigned int temp_reg = msgs->addr;
    char *allign_data_addr = NULL;

    /* 1. switch i2c devaddr and dma mode*/
    if (hi_i2c_set_dev_addr_and_mode(pdata, I2C_MODE_DMA) < 0)
        return -1;

    if (msgs->flags & I2C_M_16BIT_REG) {
        /* 16bit reg addr */
        /* switch high byte and low byte */

        temp_reg =  msgs->buf[pdata->msg_idx] << 8;
        pdata->msg_idx++;
        temp_reg |= msgs->buf[pdata->msg_idx];
        pdata->msg_idx++;

        writel(I2C_DMA_CMD0_ADDR_MODE_16, pdata->regbase + I2C_DMA_CMD0);

        //slave addr should not be contained in the data transferred through dma
        data_num = msgs->len - 2;

    } else {
        temp_reg = msgs->buf[pdata->msg_idx];
        pdata->msg_idx++;

        writel(I2C_DMA_CMD0_ADDR_MODE_8, pdata->regbase + I2C_DMA_CMD0);

        //slave addr should not be contained in the data transferred through dma
        data_num = msgs->len - 1;
    }

    /* 2. config i2c into DMA mode */
    //register I2C_DMA_CR bit[0] is the tdmae; bit[1] is the rdmae
    hi_i2c_dmac_config(pdata, 0x0);

    memalign_num = data_num / CACHE_ALIGNED_SIZE;
    if ((data_num % CACHE_ALIGNED_SIZE) != 0)
        memalign_num++;

    allign_data_addr = (char *)memalign(CACHE_ALIGNED_SIZE, CACHE_ALIGNED_SIZE * memalign_num);
    if(allign_data_addr == NULL)
        return -1;

    dma_cache_inv((int)allign_data_addr, (int)(allign_data_addr + CACHE_ALIGNED_SIZE * memalign_num));

    /* 3. transmit DATA from I2C to DMAC in DMA mode */
    //do dma to i2c without linked list item
    chan = i2c_to_dma((unsigned int)(pdata->regbase + I2C_DATA_CMD_REG), (unsigned int)allign_data_addr, data_num);

    /* 4. start i2c logic to read */
    hi_i2c_start_rx(pdata, temp_reg, data_num);
    if (dmac_wait(chan) != DMAC_CHN_SUCCESS) {
        free(allign_data_addr);
        i2c_err("dma wait failed\n");
        return -1;
    }

    memcpy(msgs->buf, allign_data_addr,  data_num);

    dmac_channel_free(chan);

    free(allign_data_addr);

    return 0;
}

#endif

int hi_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    struct i2c_driver_data *pdata;
    int errorcode;
    unsigned short data_len = 0;

    pdata = (struct i2c_driver_data *)adap->dev.driver_data;
    pdata->msgs = msgs;
    pdata->msg_num = msgs->len;
    pdata->msg_idx = 0;

    data_len = pdata->msg_num;

    if (msgs->flags & I2C_M_16BIT_REG)
        data_len -= 2;
    else
        data_len -=1;
    if (msgs->flags & I2C_M_16BIT_DATA){
        if((data_len % 2) !=0){
            i2c_err("Invalid argument!\n");
            return -EINVAL;
        }
        data_len /= 2;
    }

    if (msgs->flags & I2C_M_RD)
    {
        if (data_len == 1)
            errorcode = hi_i2c_read(pdata);
        else{
#ifdef LOSCFG_DRIVERS_HIDMAC
            errorcode = hi_i2c_dma_read(pdata);
#endif

#ifndef LOSCFG_DRIVERS_HIDMAC
            i2c_err("Invalid data length !\n");
            i2c_err("If you want to read more than one data once, you should open the DMA!\n");
            return -EINVAL;
#endif
        }
    }
    else
    {
        if (data_len == 1)
            errorcode = hi_i2c_write(pdata);
        else{
#ifdef LOSCFG_DRIVERS_HIDMAC
            errorcode = hi_i2c_dma_write(pdata);
#endif
#ifndef LOSCFG_DRIVERS_HIDMAC
            i2c_err("Invalid data length !\n");
            i2c_err("If you want to write more than one data once, you should open the DMA!\n");
            return -EINVAL;
#endif
        }
    }

    return errorcode;
}

static const struct i2c_algorithm hi_i2c_algo = {
            .master_xfer    = &hi_i2c_xfer,
};

static int i2c_hisilicon_suspend(struct device *dev)
{
    return 0;
}

static int i2c_hisilicon_resume(struct device *dev)
{
    return 0;
}

static const struct dev_pm_op i2c_hisilicon_dev_pm_ops = {
    .suspend = i2c_hisilicon_suspend,
    .resume = i2c_hisilicon_resume,
};

static int i2c_hisilicon_init(struct platform_device *dev)
{
    int index = 0,ret = -1;
    struct i2c_adapter *adapter;
    struct i2c_driver_data* pdata = NULL;
    struct hi_platform_i2c* hpi = NULL;
    struct resource *res = NULL;
    struct i2c_hisilicon_platform_data *plat = dev_get_platdata(&dev->dev);

    index = dev->id;
    if(index < 0)
    {
        pdata = dev_get_drvdata(&dev->dev);
        if(NULL == pdata)
        {
            i2c_err("i2c platform dev is wrong!\n");
            return -1;
        }
        if(NULL == pdata->adapter)
        {
            i2c_err("i2c platform dev is wrong!\n");
            return -1;
        }

        adapter = pdata->adapter;
        goto HW_INIT;
    }

    pdata = &hi_i2c_data[index];
    res = platform_get_resource(dev, IORESOURCE_MEM, 0);
    if(NULL == res)
    {
        i2c_err("platform_get_resource IORESOURCE_MEM fail!\n");
        return -1;
    }
    pdata->regbase = platform_ioremap_resource(res);
    pdata->freq = plat->freq;
    pdata->irq = platform_get_irq(dev, 0);
    if(LOS_NOK == pdata->irq)
    {
        i2c_err("platform_get_irq fail!\n");
        return -1;
    }
    pdata->clk = get_host_clock(index);
    hpi = &hi_i2c_platform_data[index];
    pdata->private = (void *)(hpi);
 
    adapter = zalloc(sizeof(struct i2c_adapter));
    if(!adapter)
    {
        i2c_err("i2c_adapter zalloc fail!\n");
        return -1;
    }
    pdata->adapter = adapter;
HW_INIT:
    hi_i2c_hw_init(pdata);

    adapter->algo = &hi_i2c_algo;
    if (index >= 0) {
        adapter->nr = index;
        if (i2c_add_numbered_adapter(adapter)) {
            free(adapter);
            return -1; /* change it to the standard error code later */
        }
    } else {
        if (i2c_register_adapter(adapter)) {
            return -1; /* change it to the standard error code later */
        }
    }

    platform_set_drvdata(dev, pdata);/* used by suspend/resume */
    adapter->dev.driver_data = (void*)pdata;

    return LOS_OK;
}

static int i2c_hisilicon_deinit(struct platform_device *dev)
{
    /*TODO: do i2c bus i2c_add_numbered_adapter/i2c_unregister_adapter later*/

    /*TODO: i2c HW deinit  later*/
 
    struct i2c_driver_data* pdata = NULL;

    pdata = platform_get_drvdata(dev);
    if((NULL == pdata) || (NULL == pdata->adapter))
    {
        i2c_err("i2c did not init yet!\n");
        return LOS_OK;
    }

    if(dev->id >= 0)
    {
        free(pdata->adapter);
        pdata->adapter = NULL;
    }
    return LOS_OK;
}

static int i2c_hisilicon_probe(struct platform_device *dev)
{
    return i2c_hisilicon_init(dev);
}

static int i2c_hisilicon_remove(struct platform_device *dev)
{
    return i2c_hisilicon_deinit(dev);
}

static struct platform_driver i2c_hisilicon_driver = {
    .probe      = i2c_hisilicon_probe,
    .remove     = i2c_hisilicon_remove,
    .driver     = {
        .name   = I2C_DRIVER_NAME,
        .pm = &i2c_hisilicon_dev_pm_ops,
    },
#if 0
    .id_table   = i2c_hisilicon_id_table,
#endif
};

int i2c_adap_hisilicon_init(void)
{
    return platform_driver_register(&i2c_hisilicon_driver);
}

void i2c_adap_hisilicon_exit(void)
{
    platform_driver_unregister(&i2c_hisilicon_driver);
}

module_init(i2c_adap_hisilicon_init);
module_exit(i2c_adap_hisilicon_exit);
