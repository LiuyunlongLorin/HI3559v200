#ifndef _NETDEV_H_
#define _NETDEV_H_



#include "los_typedef.h"
#include "hal_tables.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef struct net_netdevtab_entry {
    const char        *name;
    BOOL             (*init)(struct net_netdevtab_entry *tab);
    void              *device_instance;  // Local data, instance specific
    unsigned long     status;
}DEV_HAL_TABLE_TYPE net_netdevtab_entry_t;

#define DEV_NETDEVTAB_STATUS_AVAIL   0x0001

extern net_netdevtab_entry_t __NETDEVTAB__[], __NETDEVTAB_END__;

#define NETDEVTAB_ENTRY(_l,_name,_init,_instance)  \
 bool _init(struct net_netdevtab_entry *tab);                  \
net_netdevtab_entry_t _l DEV_HAL_TABLE_ENTRY(netdev) = {             \
   _name,                                                            \
   _init,                                                            \
   _instance                                                         \
};

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif // _NETDEV_H_
