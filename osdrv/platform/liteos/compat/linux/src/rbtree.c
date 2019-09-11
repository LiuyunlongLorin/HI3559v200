#include <linux/rbtree.h>


#ifndef RB_AUGMENT
#define RB_AUGMENT(x)
#endif

#define RB_TREE_HEAD(root)        (root)->rb_node
#define RB_TREE_COLOR(node)        (node)->rb_color
#define RB_TREE_PARENT(node)        (node)->rb_parent
#define RB_TREE_RIGHT(node)        (node)->rb_right
#define RB_TREE_LEFT(node)        (node)->rb_left

#define RB_SET_BLACKRED(black, red) do {            \
    RB_TREE_COLOR(red) = RB_RED;                    \
    RB_TREE_COLOR(black) = RB_BLACK;                \
} while (0)


#define RB_SET(elm, parent) do {                \
    RB_TREE_COLOR(elm) = RB_RED;                \
    RB_TREE_PARENT(elm) = parent;                \
    RB_TREE_LEFT(elm) = RB_TREE_RIGHT(elm) = NULL;    \
} while (0)


#define RB_ROTATE_RIGHT(head, elm, node) do {            \
    (node) = RB_TREE_LEFT(elm);                    \
    if ((RB_TREE_LEFT(elm) = RB_TREE_RIGHT(node))) {            \
        RB_TREE_PARENT(RB_TREE_RIGHT(node)) = (elm);        \
    }                            \
    RB_AUGMENT(elm);                    \
    if ((RB_TREE_PARENT(node) = RB_TREE_PARENT(elm))) {        \
        if ((elm) == RB_TREE_LEFT(RB_TREE_PARENT(elm)))        \
            RB_TREE_LEFT(RB_TREE_PARENT(elm)) = (node);    \
        else                        \
            RB_TREE_RIGHT(RB_TREE_PARENT(elm)) = (node);    \
    } else                            \
        (head)->rb_node = (node);            \
    RB_TREE_RIGHT(node) = (elm);                    \
    RB_TREE_PARENT(elm) = (node);                    \
    RB_AUGMENT(node);                    \
    if ((RB_TREE_PARENT(node)))                    \
        RB_AUGMENT(RB_TREE_PARENT(node));            \
} while(0)


#define RB_ROTATE_LEFT(head, elm, node) do {            \
    (node) = RB_TREE_RIGHT(elm);                    \
    if ((RB_TREE_RIGHT(elm) = RB_TREE_LEFT(node))) {            \
        RB_TREE_PARENT(RB_TREE_LEFT(node)) = (elm);        \
    }                            \
    RB_AUGMENT(elm);                    \
    if ((RB_TREE_PARENT(node) = RB_TREE_PARENT(elm))) {        \
        if ((elm) == RB_TREE_LEFT(RB_TREE_PARENT(elm)))        \
            RB_TREE_LEFT(RB_TREE_PARENT(elm)) = (node);    \
        else                        \
            RB_TREE_RIGHT(RB_TREE_PARENT(elm)) = (node);    \
    } else                            \
        (head)->rb_node = (node);            \
    RB_TREE_LEFT(node) = (elm);                    \
    RB_TREE_PARENT(elm) = (node);                    \
    RB_AUGMENT(node);                    \
    if ((RB_TREE_PARENT(node)))                    \
        RB_AUGMENT(RB_TREE_PARENT(node));            \
} while (0)


static void rb_remove_color(struct rb_root *head, struct rb_node *parent, struct rb_node *elm)
{
    struct rb_node *pnode;

    while ((elm == NULL || RB_TREE_COLOR(elm) == RB_BLACK) &&
        elm != RB_TREE_HEAD(head)) {
        if (parent == NULL) {
            return;
        }
        if (RB_TREE_LEFT(parent) != elm) {
            pnode = RB_TREE_LEFT(parent);
            if (RB_TREE_COLOR(pnode) == RB_RED) {
                RB_SET_BLACKRED(pnode, parent);
                RB_ROTATE_RIGHT(head, parent, pnode);/*lint !e548*/
                pnode = RB_TREE_LEFT(parent);
            }
            if ((RB_TREE_LEFT(pnode) == NULL ||
                RB_TREE_COLOR(RB_TREE_LEFT(pnode)) == RB_BLACK) &&
                (RB_TREE_RIGHT(pnode) == NULL ||
                RB_TREE_COLOR(RB_TREE_RIGHT(pnode)) == RB_BLACK)) {
                RB_TREE_COLOR(pnode) = RB_RED;
                elm = parent;
                parent = RB_TREE_PARENT(elm);
            } else {
                if (RB_TREE_LEFT(pnode) == NULL ||
                    RB_TREE_COLOR(RB_TREE_LEFT(pnode)) == RB_BLACK) {
                    struct rb_node *oright;
                    if ((oright = RB_TREE_RIGHT(pnode)))
                        RB_TREE_COLOR(oright) = RB_BLACK;
                    RB_TREE_COLOR(pnode) = RB_RED;
                    RB_ROTATE_LEFT(head, pnode, oright);/*lint !e548*/
                    pnode = RB_TREE_LEFT(parent);
                }
                RB_TREE_COLOR(pnode) = RB_TREE_COLOR(parent);
                RB_TREE_COLOR(parent) = RB_BLACK;
                if (RB_TREE_LEFT(pnode))
                    RB_TREE_COLOR(RB_TREE_LEFT(pnode)) = RB_BLACK;
                RB_ROTATE_RIGHT(head, parent, pnode);/*lint !e548*/
                elm = RB_TREE_HEAD(head);
                break;
            }
        } else {
            pnode = RB_TREE_RIGHT(parent);
            if (RB_TREE_COLOR(pnode) == RB_RED) {
                RB_SET_BLACKRED(pnode, parent);
                RB_ROTATE_LEFT(head, parent, pnode);/*lint !e548*/
                pnode = RB_TREE_RIGHT(parent);
            }
            if ((RB_TREE_LEFT(pnode) == NULL ||
                RB_TREE_COLOR(RB_TREE_LEFT(pnode)) == RB_BLACK) &&
                (RB_TREE_RIGHT(pnode) == NULL ||
                RB_TREE_COLOR(RB_TREE_RIGHT(pnode)) == RB_BLACK)) {
                RB_TREE_COLOR(pnode) = RB_RED;
                elm = parent;
                parent = RB_TREE_PARENT(elm);
            } else {
                if (RB_TREE_RIGHT(pnode) == NULL ||
                    RB_TREE_COLOR(RB_TREE_RIGHT(pnode)) == RB_BLACK) {
                    struct rb_node *oleft;
                    if ((oleft = RB_TREE_LEFT(pnode)))
                        RB_TREE_COLOR(oleft) = RB_BLACK;
                    RB_TREE_COLOR(pnode) = RB_RED;
                    RB_ROTATE_RIGHT(head, pnode, oleft);/*lint !e548*/
                    pnode = RB_TREE_RIGHT(parent);
                }
                RB_TREE_COLOR(pnode) = RB_TREE_COLOR(parent);
                RB_TREE_COLOR(parent) = RB_BLACK;
                if (RB_TREE_RIGHT(pnode))
                    RB_TREE_COLOR(RB_TREE_RIGHT(pnode)) = RB_BLACK;
                RB_ROTATE_LEFT(head, parent, pnode);/*lint !e548*/
                elm = RB_TREE_HEAD(head);
                break;
            }
        }
    }
    if (elm)
        RB_TREE_COLOR(elm) = RB_BLACK;
}


void rb_insert_color(struct rb_node *elm, struct rb_root *head)
{
    struct rb_node *parent, *gparent, *pnode;

    if ((NULL == elm) || (NULL == head))
    {
        return;
    }

    while ((parent = RB_TREE_PARENT(elm)) &&
        RB_TREE_COLOR(parent) == RB_RED) {
        gparent = RB_TREE_PARENT(parent);
        if (parent != RB_TREE_LEFT(gparent)) {
            pnode = RB_TREE_LEFT(gparent);
            if (pnode && RB_TREE_COLOR(pnode) == RB_RED) {
                RB_TREE_COLOR(pnode) = RB_BLACK;
                RB_SET_BLACKRED(parent, gparent);
                elm = gparent;
                continue;
            }
            if (RB_TREE_LEFT(parent) == elm) {
                RB_ROTATE_RIGHT(head, parent, pnode);/*lint !e548*/
                pnode = parent;
                parent = elm;
                elm = pnode;
            }
            RB_SET_BLACKRED(parent, gparent);
            RB_ROTATE_LEFT(head, gparent, pnode);/*lint !e548*/
        } else {
            pnode = RB_TREE_RIGHT(gparent);
            if (pnode && RB_TREE_COLOR(pnode) == RB_RED) {
                RB_TREE_COLOR(pnode) = RB_BLACK;
                RB_SET_BLACKRED(parent, gparent);
                elm = gparent;
                continue;
            }
            if (RB_TREE_RIGHT(parent) == elm) {
                RB_ROTATE_LEFT(head, parent, pnode);/*lint !e548*/
                pnode = parent;
                parent = elm;
                elm = pnode;
            }
            RB_SET_BLACKRED(parent, gparent);
            RB_ROTATE_RIGHT(head, gparent, pnode);/*lint !e548*/
        }
    }
    RB_TREE_COLOR(head->rb_node) = RB_BLACK;
}


void rb_replace_node(struct rb_node *victimnode, struct rb_node *newnode, struct rb_root *root)
{
    struct rb_node *pnode = NULL;
    if ((NULL == victimnode) || (NULL == newnode) || (NULL == root))
    {
        return;
    }

    pnode = victimnode->rb_parent;

    if (!pnode) {
            root->rb_node = newnode;
    }
    else {
            if (victimnode != pnode->rb_left)
                    pnode->rb_right = newnode;
            else
                    pnode->rb_left = newnode;
    }

    if (victimnode->rb_right)
            victimnode->rb_right->rb_parent = newnode;
    if (victimnode->rb_left)
            victimnode->rb_left->rb_parent = newnode;

    *newnode = *victimnode;
}

struct rb_node *rb_first(struct rb_root *root)
{
    struct rb_node  *pnode = NULL;
    if (NULL == root)
        return NULL;

    pnode = root->rb_node;
    if (!pnode)
            return NULL;
    while (pnode->rb_left)
            pnode = pnode->rb_left;
    return pnode;
}


struct rb_node *rb_prev(struct rb_node *elm)
{
    if (NULL == elm)
        return NULL;

    if (RB_TREE_LEFT(elm)) {
        elm = RB_TREE_LEFT(elm);
        while (RB_TREE_RIGHT(elm))
            elm = RB_TREE_RIGHT(elm);
    } else {
        if (RB_TREE_PARENT(elm) &&
            (elm == RB_TREE_RIGHT(RB_TREE_PARENT(elm))))
            elm = RB_TREE_PARENT(elm);
        else {
            while (RB_TREE_PARENT(elm) &&
                (elm == RB_TREE_LEFT(RB_TREE_PARENT(elm))))
                elm = RB_TREE_PARENT(elm);
            elm = RB_TREE_PARENT(elm);
        }
    }
    return (elm);
}


struct rb_node *rb_next(struct rb_node *elm)
{
    if(NULL == elm)
        return NULL;

    if (RB_TREE_RIGHT(elm)) {
        elm = RB_TREE_RIGHT(elm);
        while (RB_TREE_LEFT(elm))
            elm = RB_TREE_LEFT(elm);
    } else {
        if (RB_TREE_PARENT(elm) &&
            (elm == RB_TREE_LEFT(RB_TREE_PARENT(elm))))
            elm = RB_TREE_PARENT(elm);
        else {
            while (RB_TREE_PARENT(elm) &&
                (elm == RB_TREE_RIGHT(RB_TREE_PARENT(elm))))
                elm = RB_TREE_PARENT(elm);
            elm = RB_TREE_PARENT(elm);
        }
    }
    return (elm);
}


void rb_erase(struct rb_node *elm, struct rb_root *head)
{
    struct rb_node *child, *parent, *pnode = elm;
    int color;
    if(NULL == elm || NULL == head)
        return;

    if (RB_TREE_RIGHT(elm) == NULL)
        child = RB_TREE_LEFT(elm);
    else if (RB_TREE_LEFT(elm) == NULL)
        child = RB_TREE_RIGHT(elm);
    else {
        struct rb_node *left;
        elm = RB_TREE_RIGHT(elm);
        while ((left = RB_TREE_LEFT(elm)))
            elm = left;
        child = RB_TREE_RIGHT(elm);
        parent = RB_TREE_PARENT(elm);
        color = RB_TREE_COLOR(elm);
        if (child)
            RB_TREE_PARENT(child) = parent;
        if (!parent)
            RB_TREE_HEAD(head) = child;
        else {
            if (RB_TREE_LEFT(parent) == elm)
                RB_TREE_LEFT(parent) = child;
            else
                RB_TREE_RIGHT(parent) = child;
            RB_AUGMENT(parent);
        }

        if (RB_TREE_PARENT(elm) == pnode)
            parent = elm;
        *(elm) = *(pnode);
        if (!RB_TREE_PARENT(pnode))
            RB_TREE_HEAD(head) = elm;
        else {
            if (RB_TREE_LEFT(RB_TREE_PARENT(pnode)) == pnode)
                RB_TREE_LEFT(RB_TREE_PARENT(pnode)) = elm;
            else
                RB_TREE_RIGHT(RB_TREE_PARENT(pnode)) = elm;
            RB_AUGMENT(RB_TREE_PARENT(pnode));
        }

        RB_TREE_PARENT(RB_TREE_LEFT(pnode)) = elm;
        if (RB_TREE_RIGHT(pnode))
            RB_TREE_PARENT(RB_TREE_RIGHT(pnode)) = elm;
        if (parent) {
            left = parent;
            do {
                RB_AUGMENT(left);
            } while ((left = RB_TREE_PARENT(left)));
        }
        goto color;
    }
    parent = RB_TREE_PARENT(elm);
    color = RB_TREE_COLOR(elm);
    if (child)
        RB_TREE_PARENT(child) = parent;
    if (!parent)
        RB_TREE_HEAD(head) = child;
    else {
        if (RB_TREE_LEFT(parent) == elm)
            RB_TREE_LEFT(parent) = child;
        else
            RB_TREE_RIGHT(parent) = child;
        RB_AUGMENT(parent);
    }

color:
    if (color == RB_BLACK)
        rb_remove_color(head, parent, child);
}
