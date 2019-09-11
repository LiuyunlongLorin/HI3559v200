

#include "hi_type.h"
#include "hi_mipi_tx.h"

extern int mipi_tx_module_init(void);
extern void mipi_tx_module_exit(void);

int mipi_tx_ModInit(void)
{
    return mipi_tx_module_init();
}
void mipi_tx_ModExit(void)
{
    return mipi_tx_module_exit();
}
