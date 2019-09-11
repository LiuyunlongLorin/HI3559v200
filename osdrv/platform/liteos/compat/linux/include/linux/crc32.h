/** @defgroup crc32 Crc32
 *  @ingroup linux
*/

#ifndef CRC32_H
#define CRC32_H
#include "los_typedef.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup  crc32
 * @brief Accumulate the value of crc32.
 *
 * @par Description:
 * This API is used to accumulate the value of crc32.
 *
 * @attention
 * <ul>
 * <li>please make sure the parameter s is valid,otherwise the system maybe crash!</li>
 * </ul>
 *
 * @param  crc32val [IN/OUT] Type #unsigned int the result of  crc32 calculation.
 * @param  s [IN] Type #unsigned char  the data needs to crc32 calculate..
 * @param  len [IN] Type #int  the length of the data needs to crc32 calculate..
 *
 * @retval unsigned int return the crc value.
 * @par Dependency:
 * <ul><li>Crc32.h: the header file that contains the API declaration.</li></ul>
 * @see none.
 * @since Huawei LiteOS V100R001C00
 */
extern unsigned int crc32_accumulate(unsigned int crc32val, unsigned char *s, int len);
#define crc32(val, s, len) crc32_accumulate(val, (unsigned char *)s, len)


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
