#ifndef __GPIO_DEV_H__
#define __GPIO_DEV_H__

#include "gpio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef LOSCFG_DEBUG_VERSION
#define LOSCFG_DEBUG_VERSION
#endif
#ifdef LOSCFG_DEBUG_VERSION
  #define gpio_err(x...) \
      do { \
                  dprintf("%s->%d: ", __func__, __LINE__); \
                  dprintf(x); \
                  dprintf("\n"); \
          } while (0)
    #undef GPIO_DEBUG
#else
    #define gpio_err(x...) do { } while (0)
    #undef GPIO_DEBUG
#endif

#ifdef GPIO_DEBUG
  #define gpio_msg(x...) \
      do { \
                  dprintf("%s (line:%d) ", __func__, __LINE__); \
                  dprintf(x); \
          } while (0)
#else
    #define gpio_msg(x...) do { } while (0)
#endif

struct gpio_ops {
	int (*setdir)(struct gpio_descriptor *gd,gpio_groupbit_info* gpio_info);
    int (*getdir)(struct gpio_descriptor *gd,gpio_groupbit_info* gpio_info);
	int (*readbit)(struct gpio_descriptor *gd,gpio_groupbit_info* gpio_info);
	int (*writebit)(struct gpio_descriptor *gd,gpio_groupbit_info* gpio_info);
};


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
