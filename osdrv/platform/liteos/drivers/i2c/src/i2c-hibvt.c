#include "linux/kernel.h"
#include "linux/i2c.h"
#include "linux/delay.h"
#include "linux/errno.h"
#include "linux/io.h"
#include "i2c-hibvt.h"
#include "i2c.h"
#include "hisoc/i2c.h"
#include "linux/platform_device.h"
#include "linux/module.h"
#include "los_config.h"
#include "los_mux.h"

#ifdef LOSCFG_DRIVERS_HIEDMAC
#include "hiedmac.h"
#endif


static inline void hibvt_i2c_disable(struct i2c_driver_data *i2c);
static inline void hibvt_i2c_cfg_irq(struct i2c_driver_data *i2c,unsigned int flag);
static inline unsigned int hibvt_i2c_clr_irq(struct i2c_driver_data *i2c);

static void hibvt_i2c_rescue(struct i2c_driver_data *i2c)
{
	unsigned int val;
	unsigned int time_cnt;
	int index;

	hibvt_i2c_disable(i2c);
	hibvt_i2c_cfg_irq(i2c, 0);
	hibvt_i2c_clr_irq(i2c);

	val = (0x1 << GPIO_MODE_SHIFT) | (0x1 << FORCE_SCL_OEN_SHIFT) | (0x1 << FORCE_SDA_OEN_SHIFT);
	writel(val, i2c->regbase + HIBVT_I2C_CTRL2);

	time_cnt = 0;
	do {
		for (index = 0; index < 9; index++) {
			val = (0x1 << GPIO_MODE_SHIFT) | 0x1;
			writel(val, i2c->regbase + HIBVT_I2C_CTRL2);

			udelay(5);

			val = (0x1 << GPIO_MODE_SHIFT) | (0x1 << FORCE_SCL_OEN_SHIFT) | (0x1 << FORCE_SDA_OEN_SHIFT);
			writel(val, i2c->regbase + HIBVT_I2C_CTRL2);

			udelay(5);
		}

		time_cnt++;
		if (time_cnt > I2C_WAIT_TIMEOUT) {
			i2c_msg("wait Timeout!\n");
			goto disable_rescue;
		}

		val = readl(i2c->regbase + HIBVT_I2C_CTRL2);
	} while(!(val & (0x1 << CHECK_SDA_IN_SHIFT)));


	val = (0x1 << GPIO_MODE_SHIFT) | (0x1 << FORCE_SCL_OEN_SHIFT) | (0x1 << FORCE_SDA_OEN_SHIFT);
	writel(val, i2c->regbase + HIBVT_I2C_CTRL2);

	val = (0x1 << GPIO_MODE_SHIFT) | (0x1 << FORCE_SCL_OEN_SHIFT);
	writel(val, i2c->regbase + HIBVT_I2C_CTRL2);

	udelay(10);

	val = (0x1 << GPIO_MODE_SHIFT) | (0x1 << FORCE_SCL_OEN_SHIFT) | (0x1 << FORCE_SDA_OEN_SHIFT);
	writel(val, i2c->regbase + HIBVT_I2C_CTRL2);

disable_rescue:
	val = (0x1 << FORCE_SCL_OEN_SHIFT) | 0x1;
	writel(val, i2c->regbase + HIBVT_I2C_CTRL2);
}

static inline void hibvt_i2c_disable(struct i2c_driver_data *i2c)
{
    unsigned int val;

    val = readl(i2c->regbase + HIBVT_I2C_GLB);
    val &= ~GLB_EN_MASK;
    writel(val, i2c->regbase + HIBVT_I2C_GLB);
}

static inline void hibvt_i2c_disable_irq(struct i2c_driver_data *i2c,
		unsigned int flag)
{
	unsigned int val;

	val = readl(i2c->regbase + HIBVT_I2C_INTR_EN);
	val &= ~flag;
	writel(val, i2c->regbase + HIBVT_I2C_INTR_EN);
}

static inline unsigned int hibvt_i2c_clr_irq(struct i2c_driver_data *i2c)
{
	unsigned int val;

	val = readl(i2c->regbase + HIBVT_I2C_INTR_STAT);
	writel(INTR_ALL_MASK, i2c->regbase + HIBVT_I2C_INTR_RAW);

	return val;
}

static void hibvt_i2c_set_freq(struct i2c_driver_data *i2c)
{
	unsigned int max_freq = 0, freq = 0;
	unsigned int clk_rate;
	unsigned int val;

	freq = i2c->freq;
	clk_rate = i2c->clk;
	max_freq = clk_rate >> 1;

	if (freq > max_freq) {
		i2c->freq = max_freq;
		freq = i2c->freq;
	}

	if (freq <= 100000) {
		val = clk_rate / (freq * 2);
		writel(val, i2c->regbase + HIBVT_I2C_SCL_H);
		writel(val, i2c->regbase + HIBVT_I2C_SCL_L);
	} else {
		val = (clk_rate * 36) / (freq * 100);
		writel(val, i2c->regbase + HIBVT_I2C_SCL_H);
		val = (clk_rate * 64) / (freq * 100);
		writel(val, i2c->regbase + HIBVT_I2C_SCL_L);
	}
	val = readl(i2c->regbase + HIBVT_I2C_GLB);
	val &= ~GLB_SDA_HOLD_MASK;
	val |= ((0xa << GLB_SDA_HOLD_SHIFT) & GLB_SDA_HOLD_MASK);
	writel(val, i2c->regbase + HIBVT_I2C_GLB);
}

/*
 * set i2c controller TX and RX FIFO water
 */
static inline void hibvt_i2c_set_water(struct i2c_driver_data *i2c)
{
	writel(I2C_TXF_WATER, i2c->regbase + HIBVT_I2C_TX_WATER);
	writel(I2C_RXF_WATER, i2c->regbase + HIBVT_I2C_RX_WATER);
}

/*
 *  initialise the controller, set i2c bus interface freq
 */
static void hibvt_i2c_hw_init(struct i2c_driver_data *i2c)
{
    hibvt_i2c_disable(i2c);
    hibvt_i2c_disable_irq(i2c, INTR_ALL_MASK);
    hibvt_i2c_set_freq(i2c);
    hibvt_i2c_set_water(i2c);
}

static inline void hibvt_i2c_cmdreg_set(struct i2c_driver_data *i2c,
		unsigned int cmd, unsigned int *offset)
{
	i2c_msg("hii2c reg: offset=0x%x, cmd=0x%x...\n",
			*offset * 4, cmd);
	writel(cmd, i2c->regbase + HIBVT_I2C_CMD_BASE + *offset * 4);
	(*offset)++;
}

static void hibvt_i2c_cfg_cmd(struct i2c_driver_data *i2c)
{
	struct i2c_msg *msg = i2c->msgs;
	int offset = 0;

	if (i2c->msg_idx == 0)
		hibvt_i2c_cmdreg_set(i2c, CMD_TX_S, &offset);
	else
		hibvt_i2c_cmdreg_set(i2c, CMD_TX_RS, &offset);

	if (msg->flags & I2C_M_TEN) {
		if (i2c->msg_idx == 0) {
			hibvt_i2c_cmdreg_set(i2c, CMD_TX_D1_2, &offset);
			hibvt_i2c_cmdreg_set(i2c, CMD_TX_D1_1, &offset);
		} else {
			hibvt_i2c_cmdreg_set(i2c, CMD_TX_D1_2, &offset);
		}
	} else {
		hibvt_i2c_cmdreg_set(i2c, CMD_TX_D1_1, &offset);
	}

	if (msg->flags & I2C_M_IGNORE_NAK)
		hibvt_i2c_cmdreg_set(i2c, CMD_IGN_ACK, &offset);
    else
		hibvt_i2c_cmdreg_set(i2c, CMD_RX_ACK, &offset);

	if (msg->flags & I2C_M_RD) {
		if (msg->len >= 2) {
			writel(offset, i2c->regbase + HIBVT_I2C_DST1);
			writel(msg->len - 2, i2c->regbase + HIBVT_I2C_LOOP1);
			hibvt_i2c_cmdreg_set(i2c, CMD_RX_FIFO, &offset);
			hibvt_i2c_cmdreg_set(i2c, CMD_TX_ACK, &offset);
			hibvt_i2c_cmdreg_set(i2c, CMD_JMP1, &offset);
		}
		hibvt_i2c_cmdreg_set(i2c, CMD_RX_FIFO, &offset);
		hibvt_i2c_cmdreg_set(i2c, CMD_TX_NACK, &offset);
	} else {
		writel(offset, i2c->regbase + HIBVT_I2C_DST1);
		writel(msg->len - 1, i2c->regbase + HIBVT_I2C_LOOP1);
		hibvt_i2c_cmdreg_set(i2c, CMD_UP_TXF, &offset);
		hibvt_i2c_cmdreg_set(i2c, CMD_TX_FIFO, &offset);

		if (msg->flags & I2C_M_IGNORE_NAK)
			hibvt_i2c_cmdreg_set(i2c, CMD_IGN_ACK, &offset);
		else
			hibvt_i2c_cmdreg_set(i2c, CMD_RX_ACK, &offset);

		hibvt_i2c_cmdreg_set(i2c, CMD_JMP1, &offset);
	}

	if ((i2c->msg_idx == (i2c->msg_num - 1))
            || (msg->flags & I2C_M_STOP)) {
		i2c_msg("run to %s %d...TX STOP\n",
				__func__, __LINE__);
		hibvt_i2c_cmdreg_set(i2c, CMD_TX_P, &offset);
	}

	hibvt_i2c_cmdreg_set(i2c, CMD_EXIT, &offset);

}

static inline void hibvt_i2c_enable(struct i2c_driver_data *i2c)
{
	unsigned int val;

	val = readl(i2c->regbase + HIBVT_I2C_GLB);
	val |= GLB_EN_MASK;
	writel(val, i2c->regbase + HIBVT_I2C_GLB);
}

/*
 * config i2c slave addr
 */
static inline void hibvt_i2c_set_addr(struct i2c_driver_data *i2c)
{
	struct i2c_msg *msg = i2c->msgs;
	unsigned int addr;

	if (msg->flags & I2C_M_TEN) {
		/* First byte is 11110XX0 where XX is upper 2 bits */
		addr = ((msg->addr & 0x300) << 1) | 0xf000;
		if (msg->flags & I2C_M_RD)
			addr |= 1 << 8;

		/* Second byte is the remaining 8 bits */
		addr |= msg->addr & 0xff;
	} else {
		addr = (msg->addr & 0x7f) << 1;
		if (msg->flags & I2C_M_RD)
			addr |= 1;
	}

	writel(addr, i2c->regbase + HIBVT_I2C_DATA1);
}

/*
 * Start command sequence
 */
static inline void hibvt_i2c_start_cmd(struct i2c_driver_data *i2c)
{
	unsigned int val;

	val = readl(i2c->regbase + HIBVT_I2C_CTRL1);
	val |= CTRL1_CMD_START_MASK;
	writel(val, i2c->regbase + HIBVT_I2C_CTRL1);
}


static int hibvt_i2c_wait_rx_noempty(struct i2c_driver_data *i2c)
{
	unsigned int time_cnt = 0;
	unsigned int val;

	do {
		val = readl(i2c->regbase + HIBVT_I2C_STAT);
		if (val & STAT_RXF_NOE_MASK)
			return 0;

		udelay(50);
	} while (time_cnt++ < I2C_TIMEOUT_COUNT);

	hibvt_i2c_rescue(i2c);

	i2c_msg("wait rx no empty timeout, RIS: 0x%x, SR: 0x%x\n",
			readl(i2c->regbase + HIBVT_I2C_INTR_RAW), val);
	return -EIO;
}

static int hibvt_i2c_wait_tx_nofull(struct i2c_driver_data *i2c)
{
	unsigned int time_cnt = 0;
	unsigned int val;

	do {
		val = readl(i2c->regbase + HIBVT_I2C_STAT);
		if (val & STAT_TXF_NOF_MASK)
			return 0;

		udelay(50);
	} while (time_cnt++ < I2C_TIMEOUT_COUNT);

	hibvt_i2c_rescue(i2c);

	i2c_msg("wait rx no empty timeout, RIS: 0x%x, SR: 0x%x\n",
			readl(i2c->regbase + HIBVT_I2C_INTR_RAW), val);
	return -EIO;
}

static int hibvt_i2c_wait_idle(struct i2c_driver_data *i2c)
{
	unsigned int time_cnt = 0;
	unsigned int val;

	do {
		val = readl(i2c->regbase + HIBVT_I2C_INTR_RAW);
		if (val & (INTR_ABORT_MASK)) {
			i2c_err("wait idle abort!, RIS: 0x%x\n",val);
			return -EIO;
		}

		if (val & INTR_CMD_DONE_MASK)
			return 0;

		udelay(50);
	} while (time_cnt++ < I2C_WAIT_TIMEOUT);

	hibvt_i2c_rescue(i2c);

	i2c_msg("wait idle timeout, RIS: 0x%x, SR: 0x%x\n",
			val, readl(i2c->regbase + HIBVT_I2C_STAT));

	return -EIO;
}

static int hibvt_i2c_polling_xfer_one_msg(struct i2c_driver_data *i2c)
{
    int status;
    unsigned int val;
    struct i2c_msg *msg = i2c->msgs;
    unsigned int msg_buf_ptr = 0;

    i2c_msg("[%s,%d]msg->flags=0x%x, len=0x%x\n",
            __func__, __LINE__, msg->flags, msg->len);

    hibvt_i2c_enable(i2c);
    hibvt_i2c_clr_irq(i2c);
    hibvt_i2c_set_addr(i2c);
    hibvt_i2c_cfg_cmd(i2c);
    hibvt_i2c_start_cmd(i2c);

	if (msg->flags & I2C_M_RD) {
		while (msg_buf_ptr < msg->len) {
			status = hibvt_i2c_wait_rx_noempty(i2c);
			if (status)
				goto end;

			val = readl(i2c->regbase + HIBVT_I2C_RXF);
			msg->buf[msg_buf_ptr] = val;
			msg_buf_ptr++;

		}
	} else {
		while (msg_buf_ptr < msg->len) {
			status = hibvt_i2c_wait_tx_nofull(i2c);
			if (status)
				goto end;

			val = msg->buf[msg_buf_ptr];
			writel(val, i2c->regbase + HIBVT_I2C_TXF);
			msg_buf_ptr++;
		}
	}

	status = hibvt_i2c_wait_idle(i2c);
end:
	hibvt_i2c_disable(i2c);

	return status;
}

static inline void hibvt_i2c_cfg_irq(struct i2c_driver_data *i2c,
		unsigned int flag)
{
	writel(flag, i2c->regbase + HIBVT_I2C_INTR_EN);
}

#ifdef LOSCFG_DRIVERS_HIEDMAC
#define i2c_event_signal(event, bit)    LOS_EventWrite(event, bit)
#define i2c_event_wait(event, bit, timeout)  LOS_EventRead(event, bit,\
                                                     LOS_WAITMODE_OR+LOS_WAITMODE_CLR, timeout)
#define i2c_event_destroy(event)  LOS_EventDestroy(event)
#define WAIT_DMA_DONE 0x1
#define WATI_DMA_TIMEOUT 0xffff

void i2c_callback(void* callback_data, int ret_status)
{
    EVENT_CB_S * wait_event = (EVENT_CB_S *)callback_data;

    if (ret_status == DMAC_CHN_SUCCESS)
        dprintf("dmac trasfer success!\n");
    else
        dprintf("dmac trasfer fail!\n");

    i2c_event_signal(wait_event,WAIT_DMA_DONE);
}

static int hibvt_i2c_do_dma_read(struct i2c_driver_data *i2c,
        char* dma_buf)
{

    int chan, status = 0;
    struct i2c_msg *msg = i2c->msgs;
    int j = 0;
    unsigned int val;
    EVENT_CB_S wait_event;

    LOS_EventInit(&wait_event);
    hibvt_i2c_set_freq(i2c);
    writel(0x0, i2c->regbase+ HIBVT_I2C_RX_WATER);
    hibvt_i2c_enable(i2c);
    hibvt_i2c_clr_irq(i2c);
    hibvt_i2c_set_addr(i2c);
    hibvt_i2c_cfg_cmd(i2c);


    val = readl(i2c->regbase + HIBVT_I2C_CTRL1);
    val &= ~(CTRL1_DMA_MASK);
    val |= CTRL1_CMD_START_MASK | CTRL1_DMA_R;

    i2c_msg("%s,%d,i2c,lzptest val = 0x%x!\n",__func__,__LINE__,val);
    writel(val, i2c->regbase+ HIBVT_I2C_CTRL1);

    do_dma_p2m((AARCHPTR)dma_buf, (unsigned long)(i2c->regbase + HIBVT_I2C_RXF), msg->len, &i2c_callback, (void*)&wait_event);

    i2c_event_wait(&wait_event,WAIT_DMA_DONE,WATI_DMA_TIMEOUT);
    i2c_event_destroy(&wait_event);

    hibvt_i2c_disable(i2c);
    return 0;
}

static int hibvt_i2c_do_dma_write(struct i2c_driver_data *i2c,
        char* dma_buf)
{

    int chan, status = 0;
    struct i2c_msg *msg = i2c->msgs;
    int j = 0;
    unsigned int val;

    EVENT_CB_S wait_event;

    LOS_EventInit(&wait_event);
    hibvt_i2c_set_freq(i2c);
    writel(0x1, i2c->regbase + HIBVT_I2C_TX_WATER);
    hibvt_i2c_enable(i2c);
    hibvt_i2c_clr_irq(i2c);
    hibvt_i2c_set_addr(i2c);
    hibvt_i2c_cfg_cmd(i2c);

    val = readl(i2c->regbase + HIBVT_I2C_CTRL1);
    val &= ~(CTRL1_DMA_MASK);
    val |= CTRL1_CMD_START_MASK | CTRL1_DMA_W;

    i2c_msg("%s,%d,i2c,lzptest val = 0x%x!\n",__func__,__LINE__,val);
    writel(val, i2c->regbase+ HIBVT_I2C_CTRL1);

    do_dma_m2p((AARCHPTR)dma_buf, (unsigned long)(i2c->regbase + HIBVT_I2C_TXF), msg->len, &i2c_callback, (void*)&wait_event);

    i2c_event_wait(&wait_event,WAIT_DMA_DONE,WATI_DMA_TIMEOUT);
    i2c_event_destroy(&wait_event);

    hibvt_i2c_disable(i2c);
    return 0;
}

static int hibvt_i2c_dma_xfer_one_msg(struct i2c_driver_data *i2c)
{
    char* dma_buf_align = NULL;
    int status;
    struct i2c_msg *msg = i2c->msgs;

    dma_buf_align = memalign(64,64);
    if (!dma_buf_align) {
        i2c_msg("Can not malloc space for dma_buf!!!\n");
        return -ENOMEM;
    }
    if (msg->flags & I2C_M_RD) {
        i2c_msg("%s,%d, dma read  len = %d,\n",__func__,__LINE__ ,msg->len);
        status = hibvt_i2c_do_dma_read(i2c, dma_buf_align);

        memcpy((void *)(msg->buf), (void *)(dma_buf_align),  msg->len);
    } else {
        i2c_msg("%s,%d, dma write\n",__func__,__LINE__);
        memcpy( (void *)(dma_buf_align),(void *)(msg->buf),  msg->len);
        status = hibvt_i2c_do_dma_write(i2c, dma_buf_align);
    }
    kfree(dma_buf_align);
    return status;
}
#endif

static int hibvt_i2c_interrupt_xfer_one_msg(struct i2c_driver_data *i2c)
{
	int status;
	struct i2c_msg *msg = i2c->msgs;
	unsigned int event_status;
	unsigned long flags;
    unsigned int msg_buf_ptr = 0;
    struct hibvt_platform_i2c *hpi = (struct hibvt_platform_i2c *) i2c->private;

	i2c_msg("[%s,%d]msg->flags=0x%x, len=0x%x\n",
			__func__, __LINE__, msg->flags, msg->len);

	msg_buf_ptr = 0;
	hpi->status = -EIO;

    /* lock irq here */
    flags = LOS_IntLock();
	hibvt_i2c_enable(i2c);
	hibvt_i2c_clr_irq(i2c);
	if (msg->flags & I2C_M_RD)
		hibvt_i2c_cfg_irq(i2c, INTR_USE_MASK & ~INTR_TX_MASK);
	else
		hibvt_i2c_cfg_irq(i2c, INTR_USE_MASK & ~INTR_RX_MASK);

	hibvt_i2c_set_addr(i2c);
	hibvt_i2c_cfg_cmd(i2c);
	hibvt_i2c_start_cmd(i2c);
    LOS_IntRestore(flags);

    event_status = LOS_EventRead(&(hpi->msg_event), I2C_WAIT_RESPOND,
            LOS_WAITMODE_OR+LOS_WAITMODE_CLR, I2C_WAIT_TIMEOUT);

	if (event_status == LOS_ERRNO_EVENT_READ_TIMEOUT) {
		hibvt_i2c_disable_irq(i2c, INTR_ALL_MASK);
		status = -EIO;
		i2c_err("%s timeout\n",
			 msg->flags & I2C_M_RD ? "rx" : "tx");
	} else {
		status = hpi->status;
	}

	hibvt_i2c_disable(i2c);

	return status;
}

int hibvt_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    struct i2c_driver_data *i2c = (struct i2c_driver_data *)adap->dev.driver_data;
    int status = 0;
    unsigned long irq_save;

    if(!msgs) {
        i2c_err("invalid argument\n");
        return -EIO;
    }

    irq_save = LOS_IntLock();
    i2c->msgs = msgs;
    i2c->msg_num = num;
    i2c->msg_idx = 0;

#ifdef HIBVT_I2C_INTERRUPT_MODE
    while (i2c->msg_idx < i2c->msg_num) {
        status = hibvt_i2c_interrupt_xfer_one_msg(i2c);
        if (status)
            break;

        i2c->msgs++;
        i2c->msg_idx++;
    }
#else
    while (i2c->msg_idx < i2c->msg_num) {
        status = hibvt_i2c_polling_xfer_one_msg(i2c);
        if (status)
            break;

        i2c->msgs++;
        i2c->msg_idx++;
    }
#endif

	if (!status || i2c->msg_idx > 0)
		status = i2c->msg_idx;

	LOS_IntRestore(irq_save);

	return status;
}


int hi_i2c_transfer(struct i2c_adapter *adapter, struct i2c_msg *msgs, int count)
{
    int ret;

    ret = hibvt_i2c_xfer(adapter, msgs, count);

    return ret;
}

int hi_i2c_master_send(struct i2c_client *client, const char *buf, int count)
{
    int ret;
    struct i2c_msg msg;
    struct i2c_adapter *adapter = client->adapter;

    msg.addr = client->addr;
    msg.flags = client->flags & (~I2C_M_RD);
    msg.len = count;
    msg.buf = (char *)buf;

    ret = hi_i2c_transfer(adapter, &msg, 1);

    return (ret == 1) ? count : ret;
}

int hi_i2c_master_recv(const struct i2c_client *client, char *buf, int count)
{
    int ret;
    struct i2c_msg msg;
    struct i2c_adapter *adapter = client->adapter;

    msg.addr = client->addr;
    msg.flags = client->flags;

    msg.flags |= I2C_M_RD;
    msg.len = count;
    msg.buf = buf;

    ret = hi_i2c_transfer(adapter, &msg, 1);

    /*
     * If everything went ok (i.e. 1 msg received), return #bytes received,
     * else error code.
     */
    return (ret == 1) ? count : ret;
}

static irqreturn_t hibvt_i2c_isr(int irq, void *dev_i2c)
{
    struct i2c_driver_data *i2c = (struct i2c_driver_data *)dev_i2c;
    struct hibvt_platform_i2c *hpi = (struct hibvt_platform_i2c *) i2c->private;
    unsigned int irq_status;
    struct i2c_msg *msg = i2c->msgs;
    unsigned int msg_buf_ptr = 0;

    /* FIXME: lock i2c here */

    irq_status = hibvt_i2c_clr_irq(i2c);
    i2c_msg("%s RIS:  0x%x\n", __func__, irq_status);

    if (!irq_status) {
        i2c_msg("no irq\n");
        goto end;
    }

    if (irq_status & INTR_ABORT_MASK) {
       i2c_err("irq abort,RIS: 0x%x\n", irq_status);
       hpi->status = -EIO;
       hibvt_i2c_disable_irq(i2c, INTR_ALL_MASK);

       LOS_EventWrite(&(hpi->msg_event), I2C_WAIT_RESPOND);
       goto end;
    }

    if (msg->flags & I2C_M_RD) {
        while((readl(i2c->regbase + HIBVT_I2C_STAT) & STAT_RXF_NOE_MASK)
                && (msg_buf_ptr < msg->len)) {
            msg->buf[msg_buf_ptr] =
                readl(i2c->regbase + HIBVT_I2C_RXF);
            msg_buf_ptr++;
        }
    } else {
        while ((readl(i2c->regbase + HIBVT_I2C_STAT) & STAT_TXF_NOF_MASK)
                && (msg_buf_ptr < msg->len)) {
            writel(msg->buf[msg_buf_ptr],
                    i2c->regbase + HIBVT_I2C_TXF);
            msg_buf_ptr++;
        }
    }

	if (msg_buf_ptr >= msg->len)
		hibvt_i2c_disable_irq(i2c, INTR_TX_MASK | INTR_RX_MASK);

	if (irq_status & INTR_CMD_DONE_MASK) {
		i2c_msg("cmd done\n");
		hpi->status =  0;
		hibvt_i2c_disable_irq(i2c, INTR_ALL_MASK);

        LOS_EventWrite(&(hpi->msg_event), I2C_WAIT_RESPOND);
	}
end:
    /* unlock here*/
    return IRQ_HANDLED;
}

extern struct i2c_adapter adapter_array[];

static const struct i2c_algorithm hibvt_i2c_algo = {
    .master_xfer        = &hibvt_i2c_xfer,
};

static int i2c_hibvt_suspend(struct device *dev)
{    
    /*TODO: do i2c bus suspend later*/

    /*TODO: do i2c HW suspend */
    return 0;
}

static int i2c_hibvt_resume(struct device *dev)
{
    /*TODO: do i2c bus resume later*/

    /*TODO: do i2c HW resume */
    return 0;
}

static const struct dev_pm_op i2c_hibvt_dev_pm_ops = {
    .suspend = i2c_hibvt_suspend,
    .resume = i2c_hibvt_resume,
};

static int i2c_hibvt_init(struct platform_device *dev)
{
    int index = 0;
    int ret = 0;
    struct i2c_adapter *adapter = NULL;
    struct i2c_driver_data* pdata = NULL;
    struct hibvt_platform_i2c * hpi = NULL;
    struct resource *res = NULL;
    struct i2c_hibvt_platform_data *plat = dev_get_platdata(&dev->dev);

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
    hibvt_i2c_hw_init(pdata);

#ifdef HIBVT_I2C_INTERRUPT_MODE
    LOS_EventInit(&(hpi->msg_event));
    ret = request_irq(pdata->irq, (irq_handler_t)hibvt_i2c_isr, 0, "HIBVT_I2C", pdata);
    if(ret) {
        i2c_err("regist irq fail!\n");
        free(adapter);
        return -1;
    }
#endif

    adapter->algo = &hibvt_i2c_algo;
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

static int i2c_hibvt_deinit(struct platform_device *dev)
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

static int i2c_hibvt_probe(struct platform_device *dev)
{
    return i2c_hibvt_init(dev);
}

static int i2c_hibvt_remove(struct platform_device *dev)
{
    return i2c_hibvt_deinit(dev);
}

static struct platform_driver i2c_hibvt_driver = {
    .probe      = i2c_hibvt_probe,
    .remove     = i2c_hibvt_remove,
    .driver     = {
        .name   = I2C_DRIVER_NAME,
        .pm = &i2c_hibvt_dev_pm_ops,
    },
};

int i2c_adap_hibvt_init(void)
{
    return platform_driver_register(&i2c_hibvt_driver);
}

void i2c_adap_hibvt_exit(void)
{
    platform_driver_unregister(&i2c_hibvt_driver);
}

module_init(i2c_adap_hibvt_init);
module_exit(i2c_adap_hibvt_exit);

