/** @defgroup rbtree Rbtree
 *  @ingroup linux
*/

#ifndef    _LINUX_RBTREE_H
#define    _LINUX_RBTREE_H

#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


struct rb_node {
    int rb_color;            /* node color */
    struct rb_node *rb_parent;    /* parent element */
    struct rb_node *rb_right;    /* right element */
    struct rb_node *rb_left;    /* left element */
};

struct rb_root {
    struct rb_node *rb_node; /* root of the tree */
};

enum {
    RB_BLACK  =  0,
    RB_RED    =  1
};

#define rb_entry(p, container, field)     /*lint -e(413)*/   \
    ((container *) ((char *)p - ((char *)&(((container *)0)->field))))

#define RB_ROOT /*lint -e1086*/((struct rb_root){NULL})


static inline void rb_link_node(struct rb_node * node, struct rb_node * parent,
                struct rb_node ** rb_link)
{
    node->rb_color = RB_RED;
    node->rb_left = node->rb_right = (struct rb_node *)NULL;
    node->rb_parent = parent;

    *rb_link = node;
}

/**
 * @ingroup  rbtree
 * @brief Iterating through the elements stored in an rbtree (in sort order).
 *
 * @par Description:
 * This API is used to start iterating with a pointer to the root of the tree.
 *
 * @attention
 * <ul>
 * <li>please make sure the parameter root is valid,otherwise the system maybe crash!.</li>
 * </ul>
 *
 * @param  root [IN] Type #struct rb_root pointer to the root node of the current tree .
 *
 * @retval #rb_node* a pointer to the node structure contained in the first element in the tree.
 * @par Dependency:
 * <ul><li>Rbtree.h: the header file that contains the API declaration.</li></ul>
 * @see rb_replace_node
 * @since Huawei LiteOS V100R001C00
 */
extern struct rb_node *rb_first(struct rb_root *root);

/**
 * @ingroup  rbtree
 * @brief Iterating through the elements stored in an rbtree (in sort order).
 *
 * @par Description:
 * This API is used to fetch the  previous node on the current node.
 *
 * @attention
 * <ul>
 * <li>This will return NULL when there are no more nodes left.</li>
 * <li>please make sure the parameter elm is valid,otherwise the system maybe crash!</li>
 * </ul>
 *
 * @param  elm [IN] Type #struct rb_node pointer to the current node .
 *
 * @retval #rb_node* return the  previous node on the current node.
 * @par Dependency:
 * <ul><li>Rbtree.h: the header file that contains the API declaration.</li></ul>
 * @see rb_next
 * @since Huawei LiteOS V100R001C00
 */
extern struct rb_node *rb_prev(struct rb_node *elm);

/**
 * @ingroup  rbtree
 * @brief Iterating through the elements stored in an rbtree (in sort order).
 *
 * @par Description:
 * This API is used to fetch the  next node on the current node.
 *
 * @attention
 * <ul>
 * <li>This will return NULL when there are no more nodes left.</li>
 * <li>please make sure the parameter elm is valid,otherwise the system maybe crash!</li>
 * </ul>
 *
 * @param  elm  [IN] Type #struct rb_node pointer to the current node .
 *
 * @retval #rb_node* return the  next node on the current node.
 * @par Dependency:
 * <ul><li>Rbtree.h: the header file that contains the API declaration.</li></ul>
 * @see rb_prev
 * @since Huawei LiteOS V100R001C00
 */
extern struct rb_node *rb_next(struct rb_node *elm);

/**
 * @ingroup  rbtree
 * @brief Remove an existing node from a tree.
 *
 * @par Description:
 * This API is used to remove an existing node from a tree.
 *
 * @attention
 * <ul>
 * <li>please make sure the parameter elm and head are valid,otherwise the system maybe crash!.</li>
 * </ul>
 *
 * @param  elm [IN] Type #struct rb_node pointer to the node which need to remove .
 * @param  head [IN] Type #struct rb_root pointer to the root node of the current tree .
 *
 * @retval none.
 * @par Dependency:
 * <ul><li>Rbtree.h: the header file that contains the API declaration.</li></ul>
 * @see rb_insert_color
 * @since Huawei LiteOS V100R001C00
 */
extern void rb_erase(struct rb_node *elm, struct rb_root *head);

/**
 * @ingroup  rbtree
 * @brief Inserting data into an rbtree.
 *
 * @par Description:
 * This API is used to insert data into an rbtree and then set color.
 *
 * @attention
 * <ul>
 * <li>Inserting data in the tree involves first searching for the place to insert thenew node</li>
 * <li>After insert the new data,we also needs a link to its parent node for rebalancing purposes</li>
 * <li>please make sure the parameter elm and head are valid,otherwise the system maybe crash!</li>
 * </ul>
 *
 * @param  elm [IN] Type #struct rb_node pointer to the node which need to insert .
 * @param  head [IN] Type #struct rb_root pointer to the root node of the current tree .
 *
 * @retval none.
 * @par Dependency:
 * <ul><li>Rbtree.h: the header file that contains the API declaration.</li></ul>
 * @see rb_erase
 * @since Huawei LiteOS V100R001C00
 */
extern void rb_insert_color(struct rb_node *elm, struct rb_root *head);

/**
 * @ingroup  rbtree
 * @brief Replace an existing node in a tree.
 *
 * @par Description:
 * This API is used to replace an existing node in a tree with a new one with the same key.
 *
 * @attention
 * <ul>
 * <li>Replacing a node this way does not re-sort the tree</li>
 * <li>If the new node doesn'thave the same key as the old node, the rbtree will probably become corrupted.</li>
 * <li>please make sure the parameter victim,newnode and root are valid,otherwise the system maybe crash!</li>
 * </ul>
 *
 * @param  victim [IN] Type #struct rb_node pointer to the old node which need to replace .
 * @param  newnode [IN] Type #struct rb_node pointer to the new node which used to replace .
 * @param  head [IN] Type #struct rb_root pointer to the root node of the current tree .
 *
 * @retval none.
 * @par Dependency:
 * <ul><li>Rbtree.h: the header file that contains the API declaration.</li></ul>
 * @see none.
 * @since Huawei LiteOS V100R001C00
 */
extern void rb_replace_node(struct rb_node *victim, struct rb_node *newnode,
                struct rb_root *root);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif    /* _LINUX_RBTREE_H */
