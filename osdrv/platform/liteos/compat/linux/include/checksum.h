
#ifndef __ASM_ARM_CHECKSUM_H
#define __ASM_ARM_CHECKSUM_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

unsigned int csum_partial(const void *buff, int len, unsigned int sum);

/**
 *@ingroup libc
 *@brief Check and reverse the data.
 *
 * @par Description:
 * This API is used to check and reverse the data. The function is used for converting 32-bits accumulation into 16-bits checksum.
 *
 * @attention
 * <ul>
 * <li>The checksum calculation can be divided into two steps: 1.accumulation; 2.reverse.</li>
 * <li>This function is used to complate the second step. The input value get from the first step.</li>
 * </ul>
 *
 *@param sum [IN] the value of sum for the data which is needed to check and reverse.
 *
 *@retval uint16_t value of checksum.
 *@par Dependency:
 *<ul><li>checksum.h: the header file that contains the API declaration.</li></ul>
 *@see None
 *@since Huawei LiteOS V100R001C00
 */
#ifdef LOSCFG_ARCH_CORTEX_A53_AARCH64
static inline unsigned short csum_fold(unsigned int csum)
{
	unsigned int sum = (unsigned int)csum;
	sum += (sum >> 16) | (sum << 16);
	return ~(unsigned short)(sum >> 16);
}

#else
 static inline unsigned short csum_fold(unsigned int sum)
{
	__asm__(
	"add	%0, %1, %1, ror #16	@ csum_fold"
	: "=r" (sum)
	: "r" (sum)
	: "cc");
	return ( unsigned short)(~(unsigned int)sum >> 16);
}
#endif /* LOSCFG_ARCH_CORTEX_A53_AARCH64 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
