

#include "hi_type.h"
#include "hi_mipi.h"

extern int mipi_rx_mod_init(void);
extern void mipi_rx_mod_exit(void);

int mipi_rx_ModInit(void)
{
    return mipi_rx_mod_init();
}
void mipi_rx_ModExit(void)
{
    mipi_rx_mod_exit();
}
