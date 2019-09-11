/** @defgroup timer Timer
 *  @ingroup linux
*/

#ifndef __LINUX_TIMER_H__
#define __LINUX_TIMER_H__

/* Not yet */

#include <linux/kernel.h>


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

struct timer_list {
        unsigned long expires;
        void (*function)(unsigned long);
        unsigned long data;

        UINT16 timerid;
        unsigned int flag;
        BOOL created;
#define TIMER_VALID    0xABCDDCBA
#define TIMER_UNVALID  0xDCBAABCD
} ;


/**
 * timer_pending - is a timer pending?
 * @timer: the timer in question
 *
 * timer_pending will tell whether a given timer is currently pending,
 * or not. Callers must ensure serialization wrt. other operations done
 * to this timer, eg. interrupt contexts, or other CPUs on SMP.
 *
 * return value: 1 if the timer is pending, 0 if not.
 */
static inline int timer_pending(const struct timer_list * timer)
{
    return 0;
}

/**
 * @ingroup  timer
 * @brief Initialize a timer.
 *
 *@par Description:
 * This API is used to initialize a timer.
 *
 * @attention
 * <ul>
 * <li>The parameter timer must be valid, otherwise, the system would be abnormal.</li>
 * </ul>
 *
 * @param  timer   [IN] timer handle.
 *
 * @retval  None.
 * @par Dependency:
 * <ul><li>timer.h: the header file that contains the API declaration.</li></ul>
 * @see none.
 * @since Huawei LiteOS V100R001C00
 */
extern void init_timer(struct timer_list *timer);

/**
 * @ingroup  timer
 * @brief create a timer and start it.
 *
 *@par Description:
 * This API is used to create a timer and start it.
 *
 * @attention
 * <ul>
 * <li>The parameter timer must be valid, otherwise, the system would be abnormal. </li>
 * <li>Please make sure the domain expires, function, data, timerid inside of timer is valid, otherwise, create timer would failure. Please refer to LOS_SwtmrCreate() for detail</li>
 * <li>Do not forget to initialize the structure with 'init_timer()' before the first time calling this function.</li>
 * </ul>
 *
 * @param  timer   [IN] timer handle.
 *
 * @retval  None.
 * @par Dependency:
 * <ul><li>timer.h: the header file that contains the API declaration.</li></ul>
 * @see none.
 * @since Huawei LiteOS V100R001C00
 */
extern void add_timer(struct timer_list *timer);

/**
 * @ingroup  timer
 * @brief delete a timer.
 *
 *@par Description:
 * This API is used to delete a timer.
 *
 * @attention
 * <ul>
 * <li>The parameter timer must be valid, otherwise, the system would be abnormal. </li>
 * <li>Please make sure the domain timerid is valid, otherwise, delete timer would be failed.</li>
 * </ul>
 *
 * @param  timer   [IN] timer handle.
 *
 * @retval  #0 Delete an inactive timer or delete an active timer failure.
 * @retval  #1 Delete an active timer successful.
 * @par Dependency:
 * <ul><li>timer.h: the header file that contains the API declaration.</li></ul>
 * @see none.
 * @since Huawei LiteOS V100R001C00
 */
extern int del_timer(struct timer_list * timer);

/**
 * @ingroup  timer
 * @brief delete a timer(as same as del_timer()).
 *
 *@par Description:
 * This API is used to delete a timer.
 *
 * @attention
 * <ul>
 * <li>The parameter t must be valid, otherwise, the system would be abnormal. </li>
 * <li>Please make sure the domain timerid is valid, otherwise, delete timer would be failed.</li>
 * </ul>
 *
 * @param  timer   [IN] timer handle.
 *
 * @retval  #0 Delete an inactive timer or delete an active timer failure.
 * @retval  #1 Delete an active timer successful.
 * @par Dependency:
 * <ul><li>timer.h: the header file that contains the API declaration.</li></ul>
 * @see none.
 * @since Huawei LiteOS V100R001C00
 */
#define del_timer_sync(t)        del_timer(t)

/**
 * @ingroup  timer
 * @brief modifity a timer.
 *
 *@par Description:
 * This API is used to modifity a timer. It takes effert only if the timer is in using. acturly the specified timer would be delete and then add again, so you can refer to add_timer() for details about the input parameters.
 *
 * @attention
 * <ul>
 * <li>The parameter timer must be valid, otherwise, the system would be abnormal. </li>
 * <li>The parameter expires must not be 0, otherwise, modify timer would be failed. </li>
 * <li>Please make sure the domain expires, function, data, timerid inside of timer is valid, otherwise, create timer would failure. Please refer to LOS_SwtmrCreate() for detail</li>
 * </ul>
 *
 * @param  timer   [IN] timer handle.
 * @param  expires  [IN] Timing duration of the software timer to be created (unit: milisecond).
 *
 * @retval  None.
 * @par Dependency:
 * <ul><li>timer.h: the header file that contains the API declaration.</li></ul>
 * @see none.
 * @since Huawei LiteOS V100R001C00
 */
extern int mod_timer(struct timer_list *timer, unsigned long expires);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __LINUX_TIMER_H__ */

