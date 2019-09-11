/** @defgroup wakelock Wakelock
 *  @ingroup linux
*/

#ifndef __WAKELOCK_H__
#define __WAKELOCK_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

enum {
    WAKE_LOCK_SUSPEND, /* Prevent suspend */
    WAKE_LOCK_IDLE
};

struct wakeup_source{
    const char          *name;
};

struct wake_lock{
    struct wakeup_source    ws;
    unsigned int bitmap_pos;
};

/**
 * @ingroup  wakelock
 * @brief Activate the wake lock.
 *
 * @par Description:
 * This API is used to activate the wake lock.
 *
 * @attention
 * <ul>
 * <li>Use this function,you must run wake_lock_init first !</li>
 * <li>please make sure the parameter lock is valid,otherwise the system maybe crash!</li>
 * </ul>
 *
 * @param  lock [IN] Type #struct wake_lock   struct of the lock.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>Wakelock.h: the header file that contains the API declaration.</li></ul>
 * @see wake_unlock.
 * @since Huawei LiteOS V100R001C00
 */
void wake_lock(struct wake_lock *lock);

/**
 * @ingroup  wakelock
 * @brief Unlock the wake lock.
 *
 * @par Description:
 * This API is used to Unlock the wake lock,and make it invalid.
 *
 * @attention
 * <ul>
 * <li>Use this function,you must run wake_lock_init first !</li>
 * <li>please make sure the parameter lock is valid,otherwise the system maybe crash!</li>
 * </ul>
 *
 * @param  lock [IN] Type #struct wake_lock   struct of the lock.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>Wakelock.h: the header file that contains the API declaration.</li></ul>
 * @see wake_lock.
 * @since Huawei LiteOS V100R001C00
 */
void wake_unlock(struct wake_lock *lock);

/**
 * @ingroup  wakelock
 * @brief judge whether the wake lock is active or not.
 *
 * @par Description:
 * This API is used to judge whether the wake lock is active or not.
 *
 * @attention
 * <ul>
 * <li>Use this function,you must run wake_lock_init first !</li>
 * <li>please make sure the parameter lock is valid,otherwise the system maybe crash!</li>
 * </ul>
 *
 * @param  lock [IN] Type #struct wake_lock   struct of the lock.
 *
 * @retval #1 the wake lock is active.
 * @retval #0 the wake lock is not active.
 * @par Dependency:
 * <ul><li>Wakelock.h: the header file that contains the API declaration.</li></ul>
 * @see none.
 * @since Huawei LiteOS V100R001C00
 */
int wake_lock_active(struct wake_lock *lock);

/**
 * @ingroup  wakelock
 * @brief initial a new wakelock.
 *
 * @par Description:
 * This API is used to initial a wakelock.
 *
 * @attention
 * <ul>
 * <li>DO NOT init more than 32 wack_lock!.</li>
 * <li>please make sure the parameter lock type name are valid,otherwise the system maybe crash!</li>
 * </ul>
 *
 * @param  lock [IN] Type #struct wake_lock   struct of the lock.
 * @param  type [IN] Type #int   type of the new wakelock,not supported now.
 * @param  name [IN] Type #const char   name of the new wakelock.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>Wakelock.h: the header file that contains the API declaration.</li></ul>
 * @see none.
 * @since Huawei LiteOS V100R001C00
 */
void wake_lock_init(struct wake_lock *lock, int type, const char *name);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __WAKELOCK_H__ */

