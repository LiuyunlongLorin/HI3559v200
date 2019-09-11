#ifndef  __HI_HAL_CPU_FREQ_H__
#define  __HI_HAL_CPU_FREQ_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* Begin of #ifdef __cplusplus */
#include <linux/cpu.h>
#include <linux/clk.h>


#define CPUFREQ_NAME          "hi3559a_cpufreq"
#define DEF_TRANS_LATENCY       1000000
#define MANAGER_CPU             3
#define MANAGER_CPU_EX          3
#define COMPATIBLE              "hisilicon,hi3559av100"
#define MANAGER_CLK             "a73_mux"
#define MANAGER_CLK_EX          "800m"
#define VARIABLE_CLK            "apll"

int hal_cpu_freq_init_fixed_clk(struct device *cpu_dev);
void hal_cpu_freq_release_fixed_clk(void);
struct clk *hal_cpu_freq_get_clk(unsigned int index, unsigned long rate);
struct clk *hal_cpu_freq_get_lowest_fixed_clk(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif/* End of #ifndef __HI_HAL_CPU_FREQ_H__*/

