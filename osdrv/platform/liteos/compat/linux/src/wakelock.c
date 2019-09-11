#include "linux/wakelock.h"
#include "los_task.h"

#define SET_BIT(bitmap, pos) (bitmap |= (1u << pos))
#define CLEAR_BIT(bitmap, pos) (bitmap &= ~(1u << pos))
#define CHECK_BIT(bitmap, pos) ((bitmap & (1u << pos)) ? 1:0)

struct wakelock_bitmap{
    unsigned int lock;  //indicate which bit is locked
    unsigned int used; //indicate used bit
};

struct wakelock_bitmap wl_bitmap;

static inline int find_empty_bit(unsigned int bitmap)
{
    unsigned int t = 1;
    int i = 0;
    for(i=0; i<32; i++)
    {
        if(!(t & bitmap))  break;
        else t <<= 1;
    }
    if(i == 32)
        return -1;
    else
        return i;
}

/*
initial a wakelock
attention: DO NOT init more than 32 wack_lock!
*/
void wake_lock_init(struct wake_lock *lock, int type, const char *name)
{
    unsigned int intsave;
    if (NULL == lock)
    {
        return;
    }

    lock->ws.name = name;

    intsave = LOS_IntLock();
    lock ->bitmap_pos = find_empty_bit(wl_bitmap.used);
    SET_BIT(wl_bitmap.used, lock->bitmap_pos);
    CLEAR_BIT(wl_bitmap.lock, lock->bitmap_pos);
    LOS_IntRestore(intsave);
}

void wake_lock(struct wake_lock *lock)
{
    unsigned int intsave;
    if (NULL == lock)
    {
        return;
    }

    intsave = LOS_IntLock();
    SET_BIT(wl_bitmap.lock, lock->bitmap_pos);
    LOS_IntRestore(intsave);
}

void wake_unlock(struct wake_lock *lock)
{
    unsigned int intsave;
    if (NULL == lock)
    {
        return;
    }

    intsave = LOS_IntLock();
    CLEAR_BIT(wl_bitmap.lock, lock->bitmap_pos);
    LOS_IntRestore(intsave);
}

/*judge whether the wake lock is active or not*/
int wake_lock_active(struct wake_lock *lock)
{
    int ret;
    unsigned int intsave;
    if (NULL == lock)
    {
        return -1;
    }
    intsave = LOS_IntLock();
    ret = CHECK_BIT(wl_bitmap.lock, lock->bitmap_pos);
    LOS_IntRestore(intsave);
    return ret;
}

void wake_lock_destroy(struct wake_lock *lock)
{
    unsigned int intsave;
    if (NULL == lock)
    {
        return;
    }
    intsave = LOS_IntLock();
    CLEAR_BIT(wl_bitmap.used, lock->bitmap_pos);
    CLEAR_BIT(wl_bitmap.lock, lock->bitmap_pos);
    LOS_IntRestore(intsave);
}


