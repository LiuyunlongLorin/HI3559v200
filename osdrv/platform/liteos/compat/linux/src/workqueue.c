/*----------------------------------------------------------------------------
 *      Huawei - Huawei LiteOS
 *----------------------------------------------------------------------------
 *      Name:    Workqueue.c
 *      Purpose: High-precision-software-timer-related functions
 *      Rev.:    V1.0.0
 *----------------------------------------------------------------------------
 *

 * Copyright (c) 2014, Huawei Technologies Co., Ltd.
 * All rights reserved.
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 *THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *---------------------------------------------------------------------------*/

#include "linux/workqueue.h"
#include "los_swtmr.ph"
#include "los_event.h"

struct workqueue_struct *g_pstSystemWq;
pthread_mutex_t wq_mutex = PTHREAD_MUTEX_INITIALIZER;
extern SORTLINK_ATTRIBUTE_S               g_stSwtmrSortLink;

static cpu_workqueue_struct *init_cpu_workqueue(struct workqueue_struct *wq, int cpu);
static int create_workqueue_thread(cpu_workqueue_struct *cwq, int cpu);
static void worker_thread(cpu_workqueue_struct *__cwq);
static void run_workqueue(cpu_workqueue_struct *cwq);

/*
 * @ingroup workqueue
 * Obtain the first work in a workqueue.
 *
 */
#define worklist_entry(ptr, type, member)  ((type *)((char *)(ptr)-((AARCHPTR)&(((type*)0)->member))))

/*
 * @ingroup workqueue
 * Traverse a workqueue.
 *
 */
#define LIST_FOR_WORK(pos, list_object, type, field)      \
        for (pos = LOS_DL_LIST_ENTRY((list_object)->next, type, field);     \
             &pos->field != (list_object);                            \
             pos = LOS_DL_LIST_ENTRY(pos->field.next, type, field))

#define LIST_FOR_WORK_DEL(pos, pstnext, list_object, type, field)      \
                for (pos = LOS_DL_LIST_ENTRY((list_object)->next, type, field),     \
                        pstnext = LOS_DL_LIST_ENTRY(pos->field.next, type, field);     \
                     &pos->field != (list_object);                            \
                     pos = pstnext, pstnext = LOS_DL_LIST_ENTRY(pos->field.next, type, field))

void delayed_work_timer_fn(unsigned long __data);
void init_delayed_work(struct delayed_work *_work, work_func_t _func)
{
    if (NULL == _work || NULL == _func)
    {
        return;
    }
    INIT_WORK((&((_work)->work)), _func);
    (_work)->timer.function = delayed_work_timer_fn;
    (_work)->timer.data = (unsigned long)(_work);
    (_work)->timer.flag = TIMER_UNVALID;
    (_work)->work.work_status = 0;
}

static unsigned int wq_is_empty(cpu_workqueue_struct *cwq)
{
    UINTPTR uwIntSave;
    UINT32 uwRet;

    uwIntSave = LOS_IntLock();
    uwRet = list_empty(&cwq->worklist);
    LOS_IntRestore(uwIntSave);

    return uwRet;
}

struct workqueue_struct *__create_workqueue_key(char *name,
                        int singlethread,
                        int freezeable,
                        int rt,
                        struct lock_class_key *key,
                        const char *lock_name)
{
    struct workqueue_struct *wq;
    cpu_workqueue_struct *cwq;
    UINT32 uwRet;

    if (NULL == name)
        return (struct workqueue_struct *)NULL;

    wq = (struct workqueue_struct *)LOS_MemAlloc(m_aucSysMem0, sizeof(struct workqueue_struct));
    if (NULL == wq)
    {
        return (struct workqueue_struct *)NULL;
    }

    wq->cpu_wq = (cpu_workqueue_struct *)LOS_MemAlloc(m_aucSysMem0, sizeof(cpu_workqueue_struct));
    if (NULL == wq->cpu_wq)
    {
        (VOID)LOS_MemFree(m_aucSysMem0, (VOID *)wq);
        return (struct workqueue_struct *)NULL;
    }

    wq->name = name;
    wq->singlethread = singlethread;
    wq->freezeable = freezeable;
    wq->rt = rt;
    wq->delayed_work_count =0;
    INIT_LIST_HEAD(&wq->list);
    (VOID)LOS_EventInit(&wq->wq_event);

    if (singlethread)
    {
        cwq = init_cpu_workqueue(wq, singlethread);
        uwRet = create_workqueue_thread(cwq, singlethread);
    }
    else
    {
        return (struct workqueue_struct *)NULL;
    }

    if (uwRet)
    {
        destroy_workqueue(wq);
        wq = (struct workqueue_struct *)NULL;
    }

    return wq;
}

struct workqueue_struct * create_singlethread_workqueue(char *name)
{
    return __create_workqueue_key(name,1,0,0,(struct lock_class_key *)NULL, (const char *)NULL);
}

static cpu_workqueue_struct *init_cpu_workqueue(struct workqueue_struct *wq, int cpu)
{
    cpu_workqueue_struct *cwq = wq->cpu_wq;

    cwq->wq = wq;
    INIT_LIST_HEAD(&cwq->worklist);

    return cwq;
}

static int create_workqueue_thread(cpu_workqueue_struct *cwq, int cpu)
{
    struct workqueue_struct *wq = cwq->wq;
    TSK_INIT_PARAM_S os_task_init_param;
    UINT32 uwRet;

    os_task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)worker_thread;
    os_task_init_param.uwStackSize  = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    os_task_init_param.pcName       = wq->name;
    os_task_init_param.usTaskPrio   = 1;
    os_task_init_param.auwArgs[0] = (AARCHPTR)cwq;
    os_task_init_param.uwResved   = LOS_TASK_STATUS_DETACHED;

    uwRet = LOS_TaskCreate(&cwq->wq->wq_id, &os_task_init_param);
    if(LOS_OK != uwRet)
    {
        return LOS_NOK;
    }

    cwq->thread = (task_struct*)OS_TCB_FROM_TID(cwq->wq->wq_id);
    (VOID)LOS_TaskYield();

    return LOS_OK;
}

static void worker_thread(cpu_workqueue_struct *__cwq)
{
    cpu_workqueue_struct *cwq = __cwq;

    for (;;)
    {
        if (wq_is_empty(cwq))
        {
            (VOID)LOS_EventRead(&(cwq->wq->wq_event), 0x01, LOS_WAITMODE_OR |
                LOS_WAITMODE_CLR, LOS_WAIT_FOREVER);
        }
        run_workqueue(cwq);
    }
}

static void run_workqueue(cpu_workqueue_struct *cwq)
{
    struct work_struct *work;
    UINTPTR uwIntSave;
    work_func_t f;

    if(!wq_is_empty(cwq))
    {
        uwIntSave = LOS_IntLock();
        work = worklist_entry(cwq->worklist.next, struct work_struct, entry);/*lint !e413*/
        work->work_status |= WORK_STRUCT_RUNNING;
        list_del_init(cwq->worklist.next);
        f = work->func;
        LOS_IntRestore(uwIntSave);

        cwq->current_work = work;
        f(work);
        cwq->current_work = (struct work_struct *)NULL;

        if(work->work_status & WORK_STRUCT_RUNNING)
            work->work_status &= ~(WORK_STRUCT_RUNNING | WORK_STRUCT_PENDING);
    }
}

static void __list_add(struct list_head * _new, struct list_head * prev, struct list_head * next)
{
    next->prev = _new;
    _new->next = next;
    _new->prev = prev;
    prev->next = _new;
}

static void worklist_add(struct list_head *_new, struct list_head *head, unsigned int work_pri)
{
    struct work_struct *work;
    struct list_head * list = head;
    do{
        list = list->next;
        if(list == head)
            break;
        work = worklist_entry(list, struct work_struct, entry);/*lint !e413*/
    }while(work->work_pri <= work_pri);

    __list_add(_new, list->prev, list);
}

static void insert_work(cpu_workqueue_struct *cwq, struct work_struct *work, struct list_head *head)
{
    worklist_add(&work->entry, head, work->work_pri);
    (VOID)LOS_EventWrite(&(cwq->wq->wq_event), 0x01);
}

static void __queue_work(cpu_workqueue_struct *cwq, struct work_struct *work)
{
    insert_work(cwq, work, &cwq->worklist);
}

bool queue_work_on(struct workqueue_struct *wq, struct work_struct *work)
{
    bool uwRet = FALSE;
    struct work_struct *pstwork;

    if (wq_is_empty(wq->cpu_wq))
    {
        uwRet = TRUE;
    }
    else
    {
        LIST_FOR_WORK(pstwork, &wq->cpu_wq->worklist, struct work_struct, entry)/*lint !e413*/
        {
            if(pstwork == work)
            {
                return FALSE;
            }
        }
        uwRet = TRUE;
    }
    __queue_work(wq->cpu_wq, work);

    return uwRet;
}

bool queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
    bool uwRet = FALSE;
    UINTPTR uwIntSave;

    if(NULL == wq || NULL == wq->name || NULL == work)
        return FALSE;

    uwIntSave = LOS_IntLock();

    work->work_status = WORK_STRUCT_PENDING;
    uwRet = queue_work_on(wq, work);

    LOS_IntRestore(uwIntSave);
    return uwRet;
}

bool cancel_work_sync(struct work_struct *work)
{
    bool uwRet = FALSE;

    if(NULL == work)
        return FALSE;

    if(!work->work_status)
    {
        uwRet = FALSE;
    }
    else if(work->work_status & WORK_STRUCT_RUNNING)
    {
        uwRet = FALSE;
    }
    else
    {
        uwRet = TRUE;
    }
    while(work->work_status)
    {
        (void)usleep(10000);
    }
    return uwRet;
}

bool flush_work(struct work_struct *work)
{
    if(NULL == work)
        return FALSE;

    if(work->work_status & WORK_STRUCT_PENDING)
    {
        while(work->work_status)
        {
            (void)usleep(10000);
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void delayed_work_timer_fn(unsigned long __data)
{
    struct delayed_work *dwork = (struct delayed_work *)__data;
    UINTPTR uwIntSave;

    /* should have been called from irqsafe timer with irq already off */
    uwIntSave = LOS_IntLock();
    dwork->wq->delayed_work_count--;
    (void)queue_work_on(dwork->wq, &dwork->work);
    (void)del_timer(&dwork->timer);
    LOS_IntRestore(uwIntSave);
}

bool queue_delayed_work(struct workqueue_struct *wq, struct delayed_work *dwork, unsigned int delaytime)
{
    bool  uwRet = TRUE;
    UINTPTR uwIntSave;
    struct work_struct *pstwork;
    SWTMR_CTRL_S *pstCurSwTmr;
    UINT32 i;
    LOS_DL_LIST *pstListObject;
    SORTLINK_LIST *pstListSorted = NULL;

    if((NULL == wq) || (NULL == wq->name) || (NULL == wq->cpu_wq) || (NULL == dwork))
        return FALSE;

    dwork->wq = wq;
    if(delaytime == 0)
    {
        uwRet = queue_work(dwork->wq, &dwork->work);
        return uwRet;
    }
    else
    {
        uwIntSave = LOS_IntLock();
        for(i = 0; i < OS_TSK_SORTLINK_LEN; i++)
        {
            pstListObject = g_stSwtmrSortLink.pstSortLink + i;
            if(!LOS_ListEmpty(pstListObject))
            {
                pstListSorted = LOS_DL_LIST_ENTRY((pstListObject)->pstNext, SORTLINK_LIST, stList); /*lint !e413*/
                do
                {
                    pstCurSwTmr = LOS_DL_LIST_ENTRY(pstListSorted, SWTMR_CTRL_S, stSortList);
                    if(((struct delayed_work *)pstCurSwTmr->uwArg) == dwork)
                    {
                        LOS_IntRestore(uwIntSave);
                        return FALSE;
                    }
                    pstListSorted = LOS_DL_LIST_ENTRY(pstListSorted->stList.pstNext, SORTLINK_LIST, stList); /*lint !e413*/
                }while(&pstListSorted->stList != (pstListObject));
            }
        }

        if (wq_is_empty(wq->cpu_wq))
        {
            uwRet = TRUE;
        }
        else
        {
            LIST_FOR_WORK(pstwork, &wq->cpu_wq->worklist, struct work_struct, entry)/*lint !e413*/
            {
                if(pstwork == &dwork->work)
                {
                    LOS_IntRestore(uwIntSave);
                    return FALSE;
                }
            }
        }
    }

    dwork->timer.expires = delaytime;
    add_timer(&dwork->timer);
    wq->delayed_work_count++;
    dwork->work.work_status = WORK_STRUCT_PENDING;

    LOS_IntRestore(uwIntSave);
    return uwRet;
}

bool cancel_delayed_work(struct delayed_work *dwork)
{
    SWTMR_CTRL_S *pstCurSwTmr;
    struct work_struct *pstwork;
    struct work_struct *pstworknext;
    UINTPTR uwIntSave;
    bool  uwRet = FALSE;
    LOS_DL_LIST *pstListObject;
    SORTLINK_LIST *pstListSorted = NULL;
    UINT32 i;

    if ((NULL == dwork) || (NULL == dwork->wq))
        return FALSE;

    uwIntSave = LOS_IntLock();
    if(dwork->work.work_status & WORK_STRUCT_PENDING)
    {

        for(i = 0; i < OS_TSK_SORTLINK_LEN; i++)
        {
            pstListObject = g_stSwtmrSortLink.pstSortLink + i;
            if(!LOS_ListEmpty(pstListObject))
            {
                pstListSorted = LOS_DL_LIST_ENTRY((pstListObject)->pstNext, SORTLINK_LIST, stList); /*lint !e413*/
                do
                {
                    pstCurSwTmr = LOS_DL_LIST_ENTRY(pstListSorted, SWTMR_CTRL_S, stSortList);
                    if(pstCurSwTmr->usTimerID == dwork->timer.timerid && pstCurSwTmr->ucState == OS_SWTMR_STATUS_TICKING)
                    {
                        (void)del_timer(&dwork->timer);
                        dwork->work.work_status = 0;
                        dwork->wq->delayed_work_count--;
                        LOS_IntRestore(uwIntSave);
                        return TRUE;
                    }

                    pstListSorted = LOS_DL_LIST_ENTRY(pstListSorted->stList.pstNext, SORTLINK_LIST, stList); /*lint !e413*/
                }while(&pstListSorted->stList != (pstListObject));
            }
        }

        if(dwork->work.work_status & WORK_STRUCT_RUNNING)
        {
            uwRet = FALSE;
        }
        else if(dwork->work.work_status & WORK_STRUCT_PENDING)
        {
            LIST_FOR_WORK_DEL(pstwork, pstworknext, &dwork->wq->cpu_wq->worklist, struct work_struct, entry)/*lint !e413*/
            {
                if(pstwork == &dwork->work)
                {
                    list_del_init(&pstwork->entry);
                    dwork->work.work_status = 0;
                    uwRet = TRUE;
                    break;
                }
            }
        }
        LOS_IntRestore(uwIntSave);
        return uwRet;
    }
    else
    {
        LOS_IntRestore(uwIntSave);
        return FALSE;
    }
}

bool cancel_delayed_work_sync(struct delayed_work *dwork)
{
    bool  uwRet = FALSE;

    if(NULL == dwork)
        return FALSE;

    LOS_TaskLock();

    uwRet = cancel_delayed_work(dwork);

    LOS_TaskUnlock();
    return uwRet;
}

bool flush_delayed_work(struct delayed_work *dwork)
{
    bool  uwRet = FALSE;
    SWTMR_CTRL_S *pstCurSwTmr;
    UINTPTR uwIntSave;
    LOS_DL_LIST *pstListObject;
    SORTLINK_LIST *pstListSorted = NULL;
    UINT32 i;

    if((NULL == dwork) || (NULL == dwork->wq))
        return FALSE;

    uwIntSave = LOS_IntLock();
    if(dwork->work.work_status & WORK_STRUCT_PENDING)
    {
        for(i = 0; i < OS_TSK_SORTLINK_LEN; i++)
        {
            pstListObject = g_stSwtmrSortLink.pstSortLink + i;
            if(!LOS_ListEmpty(pstListObject))
            {
                pstListSorted = LOS_DL_LIST_ENTRY((pstListObject)->pstNext, SORTLINK_LIST, stList); /*lint !e413*/
                do
                {
                    pstCurSwTmr = LOS_DL_LIST_ENTRY(pstListSorted, SWTMR_CTRL_S, stSortList);
                    if(pstCurSwTmr->usTimerID == dwork->timer.timerid && pstCurSwTmr->ucState == OS_SWTMR_STATUS_TICKING)
                    {
                        (void)del_timer(&dwork->timer);
                        dwork->wq->delayed_work_count--;
                        (void)queue_work(dwork->wq, &dwork->work);
                        break;
                    }

                    pstListSorted = LOS_DL_LIST_ENTRY(pstListSorted->stList.pstNext, SORTLINK_LIST, stList); /*lint !e413*/
                }while(&pstListSorted->stList != (pstListObject));
            }
        }

        (void)flush_work(&dwork->work);
        uwRet = TRUE;
    }
    else
    {
        uwRet = FALSE;
    }

    LOS_IntRestore(uwIntSave);
    return uwRet;
}

unsigned int work_busy(struct work_struct *work)
{
    unsigned int uwRet = 0;

    if(NULL == work)
        return FALSE;

    if(work->work_status & WORK_STRUCT_PENDING)
    {
        uwRet |= WORK_BUSY_PENDING;
    }
    if(work->work_status & WORK_STRUCT_RUNNING)
    {
        uwRet |= WORK_BUSY_RUNNING;
    }
    return uwRet;
}

bool schedule_work(struct work_struct *work)
{
    bool  uwRet = FALSE;
    UINTPTR uwIntSave;

    if(NULL == g_pstSystemWq || NULL == g_pstSystemWq->name || NULL == work)
        return FALSE;

    uwIntSave = LOS_IntLock();

    work->work_status = WORK_STRUCT_PENDING;
    uwRet = queue_work_on(g_pstSystemWq, work);

    LOS_IntRestore(uwIntSave);
    return uwRet;
}

bool schedule_delayed_work(struct delayed_work *dwork, unsigned int delaytime)
{
    bool  uwRet = FALSE;
    UINTPTR uwIntSave;

    if(NULL == g_pstSystemWq || NULL == dwork)
        return FALSE;

    uwIntSave = LOS_IntLock();

    uwRet = queue_delayed_work(g_pstSystemWq, dwork, delaytime);

    LOS_IntRestore(uwIntSave);
    return uwRet;
}

void drain_workqueue(struct workqueue_struct *wq)
{
    while(1)
    {
        (void)usleep(10000);
        if(wq_is_empty(wq->cpu_wq) && wq->delayed_work_count == 0)
        {
            break;
        }
    }
}

void destroy_workqueue(struct workqueue_struct *wq)
{
    if(NULL == wq)
        return;

    /* drain it before proceeding with destruction */
    drain_workqueue(wq);

    (VOID)pthread_mutex_lock(&wq_mutex);
    wq->name = (char *)NULL;
    list_del_init(&wq->list);
    (VOID)LOS_TaskDelete(wq->wq_id);
    (VOID)LOS_EventDestroy(&(wq->wq_event));

    (VOID)LOS_MemFree(m_aucSysMem0, (VOID *)(wq->cpu_wq));
    (VOID)LOS_MemFree(m_aucSysMem0, (VOID *)(wq));
    (VOID)pthread_mutex_unlock(&wq_mutex);
}
