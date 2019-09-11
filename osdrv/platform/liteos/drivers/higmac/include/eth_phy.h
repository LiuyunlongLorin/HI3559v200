#ifndef PHYONCE_DEVS_ETH_PHY_H_
#define PHYONCE_DEVS_ETH_PHY_H_

#include "los_typedef.h"
#include "higmac.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define externC

#undef HISI_ETH_PHY_DBG

#define MAX_PHY_ADDR    31

// Physical device access - defined by hardware instance
typedef struct {
    BOOL init_done;
    void (*init)(void);
    void (*reset)(void);
    int phy_addr;
    int phy_mode;
} eth_phy_access_t;

#define ETH_PHY_STAT_LINK  0x0001   // Link up/down
#define ETH_PHY_STAT_100MB 0x0002   // Connection is 100Mb/10Mb
#define ETH_PHY_STAT_FDX   0x0004   // Connection is full/half duplex

bool higmac_get_phy_stat(struct higmac_netdev_local *pld, eth_phy_access_t *f, int *state);
int miiphy_link(struct higmac_netdev_local *pld, eth_phy_access_t *f);
int miiphy_speed(struct higmac_netdev_local *pld, eth_phy_access_t *f);
int miiphy_duplex(struct higmac_netdev_local *pld, eth_phy_access_t *f);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
