/** @defgroup linux Linux
 *  @defgroup delay Delay
 *  @ingroup linux
*/
#ifndef __ASM_DELAY_H__
#define __ASM_DELAY_H__

#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


extern VOID LOS_Udelay(UINT32 usecs);
extern VOID LOS_Mdelay(UINT32 msecs);

#ifndef udelay
#define udelay(n)   LOS_Udelay(n)
#endif
#ifndef mdelay
#define mdelay(n)   LOS_Mdelay(n)
#endif
/**
*@ingroup delay
*@brief Delay current task.
*
*@par Description:
*<ul>
*<li>This API is used to delay current task.</li>
*</ul>
*@attention
*<ul>
*<li>The unit of param is millisecond, and the minimum is 1 tick</li>
*</ul>
*
*@param msecs     [IN] Type #unsigned int time to delay current task.
*@retval None.
*@par Dependency:
*<ul><li>delay.h: the header file that contains the API declaration.</li></ul>
*@see
*@since Huawei LiteOS V100R001C00
*/
extern void msleep(unsigned int msecs);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
