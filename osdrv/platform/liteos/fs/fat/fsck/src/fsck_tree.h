#ifndef __FSCK_TREE_H__
#define __FSCK_TREE_H__

#include <sys/cdefs.h>

#define RBT_HEAD(name, type)                     \
struct name {                               \
    struct type *rbh_root; \
}

#define RBT_INITIALIZER(root)                        \
    { NULL }

#define RBT_INIT(root) do {                      \
    (root)->rbh_root = NULL;                    \
} while (0)

#define RBT_BLACK    0
#define RBT_RED      1
#define RBT_ENTRY(type)                          \
struct {                                \
    struct type *rbe_left;    \
    struct type *rbe_right;   \
    struct type *rbe_parent;  \
    int rbe_color;            \
}

#define RBT_LEFT(elm, field)     (elm)->field.rbe_left
#define RBT_RIGHT(elm, field)    (elm)->field.rbe_right
#define RBT_PARENT(elm, field)   (elm)->field.rbe_parent
#define RBT_COLOR(elm, field)    (elm)->field.rbe_color
#define RBT_ROOT(head)           (head)->rbh_root
#define RBT_EMPTY(head)          (RBT_ROOT(head) == NULL)

#define RBT_SET(elm, parent, field) do {                 \
    RBT_PARENT(elm, field) = parent;                 \
    RBT_LEFT(elm, field) = RBT_RIGHT(elm, field) = NULL;      \
    RBT_COLOR(elm, field) = RBT_RED;                  \
} while (0)

#define RBT_SET_BLACKRED(black, red, field) do {             \
    RBT_COLOR(black, field) = RBT_BLACK;              \
    RBT_COLOR(red, field) = RBT_RED;                  \
} while (0)

#ifndef RBT_AUGMENT
#define RBT_AUGMENT(x)   do {} while (0)
#endif

#define RBT_ROTATE_LEFT(head, elm, tmp, field) do {          \
    (tmp) = RBT_RIGHT(elm, field);                   \
    RBT_RIGHT(elm, field) = RBT_LEFT(tmp, field);         \
    if (RBT_RIGHT(elm, field) != NULL) {             \
        RBT_PARENT(RBT_LEFT(tmp, field), field) = (elm);      \
    }                               \
    RBT_AUGMENT(elm);                        \
    RBT_PARENT(tmp, field) = RBT_PARENT(elm, field);          \
    if (RBT_PARENT(tmp, field) != NULL) {                \
        if ((elm) == RBT_LEFT(RBT_PARENT(elm, field), field)) \
            RBT_LEFT(RBT_PARENT(elm, field), field) = (tmp);  \
        else                            \
            RBT_RIGHT(RBT_PARENT(elm, field), field) = (tmp); \
    } else                              \
        (head)->rbh_root = (tmp);               \
    RBT_LEFT(tmp, field) = (elm);                    \
    RBT_PARENT(elm, field) = (tmp);                  \
    RBT_AUGMENT(tmp);                        \
    if ((RBT_PARENT(tmp, field)))                    \
        RBT_AUGMENT(RBT_PARENT(tmp, field));          \
} while (0)

#define RBT_ROTATE_RIGHT(head, elm, tmp, field) do {         \
    (tmp) = RBT_LEFT(elm, field);                    \
    RBT_LEFT(elm, field) = RBT_RIGHT(tmp, field);         \
    if (RBT_LEFT(elm, field) != NULL) {              \
        RBT_PARENT(RBT_RIGHT(tmp, field), field) = (elm);     \
    }                               \
    RBT_AUGMENT(elm);                        \
    RBT_PARENT(tmp, field) = RBT_PARENT(elm, field);          \
    if (RBT_PARENT(tmp, field) != NULL) {                \
        if ((elm) == RBT_LEFT(RBT_PARENT(elm, field), field)) \
            RBT_LEFT(RBT_PARENT(elm, field), field) = (tmp);  \
        else                            \
            RBT_RIGHT(RBT_PARENT(elm, field), field) = (tmp); \
    } else                              \
        (head)->rbh_root = (tmp);               \
    RBT_RIGHT(tmp, field) = (elm);                   \
    RBT_PARENT(elm, field) = (tmp);                  \
    RBT_AUGMENT(tmp);                        \
    if ((RBT_PARENT(tmp, field)))                    \
        RBT_AUGMENT(RBT_PARENT(tmp, field));          \
} while ( 0)

#define RBT_PROTOTYPE_INTERNAL(name, type, field, cmp, attr)     \
attr void name##_RBT_INSERT_COLOR(struct name *, struct type *);     \
attr void name##_RBT_REMOVE_COLOR(struct name *, struct type *, struct type *);\
attr struct type *name##_RBT_REMOVE(struct name *, struct type *);   \
attr struct type *name##_RBT_INSERT(struct name *, struct type *);   \
attr struct type *name##_RBT_FIND(struct name *, struct type *);     \
attr struct type *name##_RBT_NEXT(struct type *);            \
attr struct type *name##_RBT_MIN(struct name *);         \
                                    \

#define RBT_GENERATE(name, type, field, cmp)             \
    RBT_GENERATE_INTERNAL(name, type, field, cmp,)
#define RBT_GENERATE_STATIC(name, type, field, cmp)          \
    RBT_GENERATE_INTERNAL(name, type, field, cmp, __unused static)
#define RBT_GENERATE_INTERNAL(name, type, field, cmp, attr)      \
attr void                               \
name##_RBT_INSERT_COLOR(struct name *head, struct type *elm)     \
{                                   \
    struct type *parent, *gparent, *tmp;                \
    while ((parent = RBT_PARENT(elm, field)) != NULL &&      \
        RBT_COLOR(parent, field) == RBT_RED) {            \
        gparent = RBT_PARENT(parent, field);         \
        if (parent == RBT_LEFT(gparent, field)) {        \
            tmp = RBT_RIGHT(gparent, field);         \
            if (tmp && RBT_COLOR(tmp, field) == RBT_RED) {    \
                RBT_COLOR(tmp, field) = RBT_BLACK;    \
                RBT_SET_BLACKRED(parent, gparent, field);\
                elm = gparent;              \
                continue;               \
            }                       \
            if (RBT_RIGHT(parent, field) == elm) {       \
                RBT_ROTATE_LEFT(head, parent, tmp, field);\
                tmp = parent;               \
                parent = elm;               \
                elm = tmp;              \
            }                       \
            RBT_SET_BLACKRED(parent, gparent, field);    \
            RBT_ROTATE_RIGHT(head, gparent, tmp, field); \
        } else {                        \
            tmp = RBT_LEFT(gparent, field);          \
            if (tmp && RBT_COLOR(tmp, field) == RBT_RED) {    \
                RBT_COLOR(tmp, field) = RBT_BLACK;    \
                RBT_SET_BLACKRED(parent, gparent, field);\
                elm = gparent;              \
                continue;               \
            }                       \
            if (RBT_LEFT(parent, field) == elm) {        \
                RBT_ROTATE_RIGHT(head, parent, tmp, field);\
                tmp = parent;               \
                parent = elm;               \
                elm = tmp;              \
            }                       \
            RBT_SET_BLACKRED(parent, gparent, field);    \
            RBT_ROTATE_LEFT(head, gparent, tmp, field);  \
        }                           \
    }                               \
    RBT_COLOR(head->rbh_root, field) = RBT_BLACK;         \
}                                   \
                                    \
attr void                               \
name##_RBT_REMOVE_COLOR(struct name *head, struct type *parent, struct type *elm) \
{                                   \
    struct type *tmp;                       \
    while ((elm == NULL || RBT_COLOR(elm, field) == RBT_BLACK) && \
        elm != RBT_ROOT(head)) {                 \
        if (RBT_LEFT(parent, field) == elm) {            \
            tmp = RBT_RIGHT(parent, field);          \
            if (RBT_COLOR(tmp, field) == RBT_RED) {       \
                RBT_SET_BLACKRED(tmp, parent, field);    \
                RBT_ROTATE_LEFT(head, parent, tmp, field);\
                tmp = RBT_RIGHT(parent, field);      \
            }                       \
            if ((RBT_LEFT(tmp, field) == NULL ||     \
                RBT_COLOR(RBT_LEFT(tmp, field), field) == RBT_BLACK) &&\
                (RBT_RIGHT(tmp, field) == NULL ||        \
                RBT_COLOR(RBT_RIGHT(tmp, field), field) == RBT_BLACK)) {\
                RBT_COLOR(tmp, field) = RBT_RED;      \
                elm = parent;               \
                parent = RBT_PARENT(elm, field);     \
            } else {                    \
                if (RBT_RIGHT(tmp, field) == NULL || \
                    RBT_COLOR(RBT_RIGHT(tmp, field), field) == RBT_BLACK) {\
                    struct type *oleft;     \
                    oleft = RBT_LEFT(tmp, field);    \
                    if (oleft != NULL)      \
                        RBT_COLOR(oleft, field) = RBT_BLACK;\
                    RBT_COLOR(tmp, field) = RBT_RED;  \
                    RBT_ROTATE_RIGHT(head, tmp, oleft, field);\
                    tmp = RBT_RIGHT(parent, field);  \
                }                   \
                RBT_COLOR(tmp, field) = RBT_COLOR(parent, field);\
                RBT_COLOR(parent, field) = RBT_BLACK; \
                if (RBT_RIGHT(tmp, field))       \
                    RBT_COLOR(RBT_RIGHT(tmp, field), field) = RBT_BLACK;\
                RBT_ROTATE_LEFT(head, parent, tmp, field);\
                elm = RBT_ROOT(head);            \
                break;                  \
            }                       \
        } else {                        \
            tmp = RBT_LEFT(parent, field);           \
            if (RBT_COLOR(tmp, field) == RBT_RED) {       \
                RBT_SET_BLACKRED(tmp, parent, field);    \
                RBT_ROTATE_RIGHT(head, parent, tmp, field);\
                tmp = RBT_LEFT(parent, field);       \
            }                       \
            if ((RBT_LEFT(tmp, field) == NULL ||     \
                RBT_COLOR(RBT_LEFT(tmp, field), field) == RBT_BLACK) &&\
                (RBT_RIGHT(tmp, field) == NULL ||        \
                RBT_COLOR(RBT_RIGHT(tmp, field), field) == RBT_BLACK)) {\
                RBT_COLOR(tmp, field) = RBT_RED;      \
                elm = parent;               \
                parent = RBT_PARENT(elm, field);     \
            } else {                    \
                if (RBT_LEFT(tmp, field) == NULL ||  \
                    RBT_COLOR(RBT_LEFT(tmp, field), field) == RBT_BLACK) {\
                    struct type *oright;        \
                    oright = RBT_RIGHT(tmp, field);  \
                    if (oright != NULL)     \
                        RBT_COLOR(oright, field) = RBT_BLACK;\
                    RBT_COLOR(tmp, field) = RBT_RED;  \
                    RBT_ROTATE_LEFT(head, tmp, oright, field);\
                    tmp = RBT_LEFT(parent, field);   \
                }                   \
                RBT_COLOR(tmp, field) = RBT_COLOR(parent, field);\
                RBT_COLOR(parent, field) = RBT_BLACK; \
                if (RBT_LEFT(tmp, field))        \
                    RBT_COLOR(RBT_LEFT(tmp, field), field) = RBT_BLACK;\
                RBT_ROTATE_RIGHT(head, parent, tmp, field);\
                elm = RBT_ROOT(head);            \
                break;                  \
            }                       \
        }                           \
    }                               \
    if (elm)                            \
        RBT_COLOR(elm, field) = RBT_BLACK;            \
}                                   \
                                    \
attr struct type *                          \
name##_RBT_REMOVE(struct name *head, struct type *elm)           \
{                                   \
    struct type *child, *parent, *old = elm;            \
    int color;                          \
    if (RBT_LEFT(elm, field) == NULL)                \
        child = RBT_RIGHT(elm, field);               \
    else if (RBT_RIGHT(elm, field) == NULL)              \
        child = RBT_LEFT(elm, field);                \
    else {                              \
        struct type *left;                  \
        elm = RBT_RIGHT(elm, field);             \
        while ((left = RBT_LEFT(elm, field)) != NULL)        \
            elm = left;                 \
        child = RBT_RIGHT(elm, field);               \
        parent = RBT_PARENT(elm, field);             \
        color = RBT_COLOR(elm, field);               \
        if (child)                      \
            RBT_PARENT(child, field) = parent;       \
        if (parent) {                       \
            if (RBT_LEFT(parent, field) == elm)      \
                RBT_LEFT(parent, field) = child;     \
            else                        \
                RBT_RIGHT(parent, field) = child;    \
            RBT_AUGMENT(parent);             \
        } else                          \
            RBT_ROOT(head) = child;              \
        if (RBT_PARENT(elm, field) == old)           \
            parent = elm;                   \
        (elm)->field = (old)->field;                \
        if (RBT_PARENT(old, field)) {                \
            if (RBT_LEFT(RBT_PARENT(old, field), field) == old)\
                RBT_LEFT(RBT_PARENT(old, field), field) = elm;\
            else                        \
                RBT_RIGHT(RBT_PARENT(old, field), field) = elm;\
            RBT_AUGMENT(RBT_PARENT(old, field));      \
        } else                          \
            RBT_ROOT(head) = elm;                \
        RBT_PARENT(RBT_LEFT(old, field), field) = elm;        \
        if (RBT_RIGHT(old, field))               \
            RBT_PARENT(RBT_RIGHT(old, field), field) = elm;   \
        if (parent) {                       \
            left = parent;                  \
            do {                        \
                RBT_AUGMENT(left);           \
            } while ((left = RBT_PARENT(left, field)) != NULL); \
        }                           \
        goto color;                     \
    }                               \
    parent = RBT_PARENT(elm, field);                 \
    color = RBT_COLOR(elm, field);                   \
    if (child)                          \
        RBT_PARENT(child, field) = parent;           \
    if (parent) {                           \
        if (RBT_LEFT(parent, field) == elm)          \
            RBT_LEFT(parent, field) = child;         \
        else                            \
            RBT_RIGHT(parent, field) = child;        \
        RBT_AUGMENT(parent);                 \
    } else                              \
        RBT_ROOT(head) = child;                  \
color:                                  \
    if (color == RBT_BLACK)                      \
        name##_RBT_REMOVE_COLOR(head, parent, child);        \
    return old;                         \
}                                   \
                                    \
attr struct type *                          \
name##_RBT_INSERT(struct name *head, struct type *elm)           \
{                                   \
    struct type *tmp;                       \
    struct type *parent = NULL;                 \
    int comp = 0;                           \
    tmp = RBT_ROOT(head);                        \
    while (tmp) {                           \
        parent = tmp;                       \
        comp = (cmp)(elm, parent);              \
        if (comp < 0)                       \
            tmp = RBT_LEFT(tmp, field);          \
        else if (comp > 0)                  \
            tmp = RBT_RIGHT(tmp, field);         \
        else                            \
            return tmp;                 \
    }                               \
    RBT_SET(elm, parent, field);                 \
    if (parent != NULL) {                       \
        if (comp < 0)                       \
            RBT_LEFT(parent, field) = elm;           \
        else                            \
            RBT_RIGHT(parent, field) = elm;          \
        RBT_AUGMENT(parent);                 \
    } else                              \
        RBT_ROOT(head) = elm;                    \
    name##_RBT_INSERT_COLOR(head, elm);              \
    return NULL;                            \
}                                   \
                                    \
attr struct type *                          \
name##_RBT_FIND(struct name *head, struct type *elm)         \
{                                   \
    struct type *tmp = RBT_ROOT(head);               \
    int comp;                           \
    while (tmp) {                           \
        comp = cmp(elm, tmp);                   \
        if (comp < 0)                       \
            tmp = RBT_LEFT(tmp, field);          \
        else if (comp > 0)                  \
            tmp = RBT_RIGHT(tmp, field);         \
        else                            \
            return tmp;                 \
    }                               \
    return NULL;                            \
}                                   \
                                    \
attr struct type *                          \
name##_RBT_NEXT(struct type *elm)                    \
{                                   \
    if (RBT_RIGHT(elm, field)) {                 \
        elm = RBT_RIGHT(elm, field);             \
        while (RBT_LEFT(elm, field))             \
            elm = RBT_LEFT(elm, field);          \
    } else {                            \
        if (RBT_PARENT(elm, field) &&                \
            (elm == RBT_LEFT(RBT_PARENT(elm, field), field))) \
            elm = RBT_PARENT(elm, field);            \
        else {                          \
            while (RBT_PARENT(elm, field) &&         \
                (elm == RBT_RIGHT(RBT_PARENT(elm, field), field)))\
                elm = RBT_PARENT(elm, field);        \
            elm = RBT_PARENT(elm, field);            \
        }                           \
    }                               \
    return elm;                         \
}                                   \
                                    \
attr struct type *                          \
name##_RBT_MIN(struct name *head)                \
{                                   \
    struct type *tmp = RBT_ROOT(head);               \
    struct type *parent = NULL;                 \
    while (tmp) {                           \
        parent = tmp;                       \
        tmp = RBT_LEFT(tmp, field);          \
    }                               \
    return parent;                      \
}

#endif  /* _SYS_TREE_H_ */
