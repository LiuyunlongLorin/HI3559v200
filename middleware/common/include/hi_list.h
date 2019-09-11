/**
* Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file        hi_list.h
* @brief      Simple doubly linked list implementation
* @author   HiMobileCam middleware develop team
* @date      2016.06.06
*/
#ifndef __HI_LIST_H__
#define __HI_LIST_H__

#include "linux_cbb_list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/** free list node*/
#define HI_Free(p)  do{if(NULL != (p)){free(p);(p) = NULL;}}while(0)

/**define list head struct*/
typedef struct cbb_list_head List_Head_S;

/**define hi list head struct*/
typedef List_Head_S HI_List_Head_S;

/**init list head node*/
#define HI_LIST_INIT_HEAD_DEFINE(name) LIST_HEAD_INIT(name)

/**list head node define*/
#define HI_LIST_HEAD_DEFINE(name) LIST_HEAD(name)

/**init list head node pointer*/
#define HI_LIST_INIT_HEAD_PTR(ptr) INIT_LIST_HEAD(ptr)

/**
 * add a new entry
 * @pstruNew: input, new entry to be added
 * @pstruHead: input, the entry after witch new entry will be add
 */
#define HI_List_Add(pstruNew, pstruHead) cbb_list_add(pstruNew, pstruHead)

/**
 * @new: input, pstruNew entry to be added
 * @head: input, pstruHead the entry before witch new entry will be add
 * this is useful for implementing a queue.
 */
#define HI_List_Add_Tail(pstruNew, pstruHead) cbb_list_add_tail(pstruNew, pstruHead)

/*
 * Delete a list entry by making the prev/next entries point to each other.
 * @pstruEntry: input, the entry to delete
 */
#define HI_List_Del(pstruEntry) cbb_list_del(pstruEntry)

/**
 * Delete a entry from list and reinitialize it.
 * @pstruEntry: input, the element to delete from the list.
 */
#define HI_List_Del_Init(pstruEntry) cbb_list_del_init(pstruEntry)

/**
 * tests whether a list is empty
 * @pstruHead: input, the list to test.
 */
#define HI_List_Empty(pstruHead) cbb_list_empty(pstruHead)

/**
 * join two lists
 * @pstruList: input, the new list to add.
 * @pstruHead: input, the entry after witch to add the new list.
 */
#define HI_List_Splice(pstruList, pstruHead) cbb_list_splice(pstruList, pstruHead)

/**
 * get the struct address from a list entry
 * @ptr:   input, the pointer of the entry
 * @type: input, the pointer struct type that should return
 * @member: the name of the entry within the struct type.
 */
#define HI_LIST_ENTRY(ptr, type, member) cbb_list_entry(ptr, type, member)

/**
 *  iterate over a list
 * @pos:   input, output, the &struct HI_List_Head_S to use as a loop counter.
 * @head:  input, the head of the list.
 */
#define HI_List_For_Each(pos, head) cbb_list_for_each(pos, head)

/**
 * iterate over a list safe against removal of list entry
 * @pos:   input, output,  the &struct HI_List_Head_S to use as a loop counter.
 * @n:     input, another &struct HI_List_Head_S to use as temporary storage
 * @head:  input, the head of the list.
 */
#define HI_List_For_Each_Safe(pos, n, head) cbb_list_for_each_safe(pos, n, head)

/**
 *  iterate over list of given type
 * @pos:    input, output, the type * to use as a loop cursor.
 * @n:  input, another &struct HI_List_Head_S to use as temporary storage
 * @head:  input,  the head for your list.
 * @member: input, the name of the list_struct within the struct.
 */
#define HI_List_For_Each_Entry_Safe(pos, n, head, member) cbb_list_for_each_entry_safe(pos, n, head, member)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__HI_LIST_H__*/
