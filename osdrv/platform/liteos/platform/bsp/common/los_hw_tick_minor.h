#include "los_hwi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef LOSCFG_ARCH_CORTEX_M7
typedef  struct tagOsTimerRegs{
    UINT32  uwLOAD;
    UINT32  uwVALUE;
    UINT32  uwCONTROL;
    UINT32  uwINTCLR;
    UINT32  uwRIS;
    UINT32  uwMCR;
    UINT32  uwMIS;
    UINT32  uwBGLOAD;
} OS_TIMER_REG_S;

#define  OS_TIMER_TICK_BASE_REG ((OS_TIMER_REG_S *)(TIMER_TICK_REG_BASE))
#define  SysTick OS_TIMER_TICK_BASE_REG
#endif
/**
 *@ingroup platform
 *@brief get cpu cycle.
 *
 *@par Description:
 *This API is used to get the cpu step's cycle.
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param puwCntHi	[IN] For storage cycle hi32bit.
 *@param puwCntLo	[IN] For storage cycle low32bit.
 *
 *@retval NONE
 *@par Dependency:
 *<ul><li>los_hw_tick_minor.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_GetCpuCycle
 *@since Huawei LiteOS V100R001C00
 */
extern VOID LOS_GetCpuCycle(UINT32 *puwCntHi, UINT32 *puwCntLo);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


