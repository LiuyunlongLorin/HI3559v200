
#ifndef FS_JFFS2_LIST_H
#define FS_JFFS2_LIST_H

#include <stddef.h>
#include <linux/wait.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef struct list_head {
    struct list_head *next;
    struct list_head *prev;
}list_head_t;

struct hlist_head {
    struct hlist_node *first;
};

typedef struct hlist_node {
    struct hlist_node *next, **pprev;
}hlist_node_t;

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name)     list_head_t name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD( _list_ )              \
do {                               \
(_list_)->next = (_list_)->prev = (_list_);   \
} while(0)

#define container_of LOS_DL_LIST_ENTRY

/* Is list empty? */
static __inline__ int
list_empty(list_head_t *pList)
{
    return pList->next == pList;
}

/* Insert an entry after the specified entry */
static __inline__ void
list_add(list_head_t *newnode, list_head_t *afternode)
{
    list_head_t *pstnext = afternode->next;
    newnode->prev = afternode;
    newnode->next = pstnext;
    pstnext->prev = newnode;
    afternode->next = newnode;
}

/* Insert an entry _before_ the specified entry */
static __inline__ void
list_add_tail(list_head_t *newnode, list_head_t *beforenode)
{
    list_head_t *pstprev = beforenode->prev;
    newnode->next = beforenode;
    newnode->prev = pstprev;
    pstprev->next = newnode;
    beforenode->prev = newnode;
}

/* Delete the specified entry */
static __inline__ void
list_del(list_head_t *node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;
}

/* list_entry - returns the address of the container structure */
#define list_entry(ptr, type, member)      ((type *)((unsigned long)(ptr)-((unsigned long)(&((type *)1)->member) - 1)))

/* list_for_each - using _ent_, iterate through list _list_ */
#define list_for_each(_node_, _list_)   \
    for ( (_node_) = (_list_)->next;      \
    (_node_) != (_list_);                 \
    (_node_) = (_node_)->next )

/* list_for_each_entry - iterate over all items in a list */
#define list_for_each_entry(_list_, _head_, _item_)                     \
for ((_list_) = list_entry((_head_)->next, __typeof__(*_list_), _item_); \
     &((_list_)->_item_) != (_head_);                                 \
     (_list_) = list_entry((_list_)->_item_.next, __typeof__(*_list_), _item_))

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head);  pos = n, n = pos->next)

/* list_first_entry - get the first element from a list */
#define list_first_entry(ptr, type, member)     list_entry((ptr)->next, type, member)

/* ist_is_last - tests whether @list is the last entry in list @head */
static inline int list_is_last(const list_head_t *plist, const list_head_t *phead)
{
    return plist->next == phead;
}

/* list_for_each_entry_safe - iterate over list of given type safe against removal of list entry */
#define list_for_each_entry_safe(pos, n, head, member)            \
    for (pos = list_entry((head)->next, __typeof__(*pos), member),    \
        n = list_entry(pos->member.next, __typeof__(*pos), member);    \
         &pos->member != (head);                     \
         pos = n, n = list_entry(n->member.next, __typeof__(*n), member))

/* list_for_each_entry_reverse - iterate backwards over list of given type */
#define list_for_each_entry_reverse(pos, head, member)            \
        for (pos = list_entry((head)->prev, __typeof__(*pos), member);  \
             &pos->member != (head);    \
             pos = list_entry(pos->member.prev, __typeof__(*pos), member))

/* Delete a list entry by making the prev/next entries */
static inline void __list_del(list_head_t * prev, list_head_t * next)
{
    next->prev = prev;
    prev->next = next;
}

/* list_del - deletes entry from list */
static inline void __list_del_entry(list_head_t *pEntry)
{
    __list_del(pEntry->prev, pEntry->next);
}

/* list_move - delete from one list and add as another's head */
static inline void list_move(list_head_t *pList, list_head_t *head)
{
    __list_del_entry(pList);
    list_add(pList, head);
}

static inline void list_del_init(list_head_t *pEntry)
{
    __list_del(pEntry->prev, pEntry->next);
    INIT_LIST_HEAD(pEntry);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
/* -----------------------------------------------------------------------*/
#endif /* #ifndef FS_JFFS2_LIST_H */
/* EOF list.h */
