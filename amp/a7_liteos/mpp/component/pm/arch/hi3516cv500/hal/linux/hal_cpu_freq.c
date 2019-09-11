#include "hal_cpu_freq.h"

static struct clk *fixed_24m_clk;
static struct clk *fixed_1000m_clk;

int hal_cpu_freq_init_fixed_clk(struct device *cpu_dev)
{
    fixed_24m_clk = clk_get(cpu_dev, "24m");
    fixed_1000m_clk = clk_get(cpu_dev, "1000m");
    if(IS_ERR(fixed_24m_clk) || IS_ERR(fixed_1000m_clk)) {
        printk("failed to init fixed clocks\n");
        return -ENOENT;
    }
    return 0;
}

void hal_cpu_freq_release_fixed_clk(void)
{
    if(fixed_24m_clk) {
        clk_put(fixed_24m_clk);
    }
    if(fixed_1000m_clk) {
        clk_put(fixed_1000m_clk);
    }
}



struct clk *hal_cpu_freq_get_clk(unsigned int index, unsigned long rate)
{
    if (rate == 24000 && fixed_24m_clk) {
        return fixed_24m_clk;
    } else if (rate == 1000000 && fixed_1000m_clk) {
        return fixed_1000m_clk;
    } else {
        return NULL;
    }
}

struct clk *hal_cpu_freq_get_lowest_fixed_clk(void)
{
    return fixed_1000m_clk;
}


