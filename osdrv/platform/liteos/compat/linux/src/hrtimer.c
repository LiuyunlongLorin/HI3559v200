/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "linux/hrtimer.h"
#include "los_task.ph"
#include "los_membox.ph"
#include "los_memory.ph"
#include "los_hwi.h"
#include "asm/hal_platform_ints.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

LITE_OS_SEC_BSS struct hrtimer_list_node *g_HrtimerList;

static void hrtimer_list_scan(void);

unsigned int hrtimers_init(void)
{
    unsigned int uwRet;

    g_HrtimerList = (struct hrtimer_list_node *)NULL;
    /*Initialize the timer*/
    hrtimer_clock_initialize();
    /*Create interrupt of the timer*/
    uwRet = LOS_HwiCreate(NUM_HAL_INTERRUPT_HRTIMER, 0, 0, hrtimer_list_scan, 0);
    if(uwRet != LOS_OK)
        return LOS_NOK;
    hal_interrupt_unmask(NUM_HAL_INTERRUPT_HRTIMER);

    return LOS_OK;
}

static void handler_node_add(struct hrtimer_list_node *pstHrtimer, struct handler_list_node *pstHandlerNode)
{
    struct handler_list_node *ptemp = (struct handler_list_node *)NULL;

    if(NULL == pstHrtimer)
    {
        return;
    }

    ptemp = pstHrtimer->HandlerHead;
    if(NULL== ptemp)
    {
        pstHrtimer->HandlerHead = pstHandlerNode;
    }
    else
    {
        while(ptemp->pstNext != NULL)
        {
            ptemp = ptemp->pstNext;
        }
        ptemp->pstNext = pstHandlerNode;
    }//FIFO

    return;
}

static void hrtimer_node_add(struct hrtimer_list_node *pstHrtimer, struct handler_list_node *pstHandlerNode)
{
    struct hrtimer_list_node *pstPrev = (struct hrtimer_list_node *)NULL;
    struct hrtimer_list_node *pstCur = (struct hrtimer_list_node *)NULL;
    struct hrtimer_list_node *ptemp = (struct hrtimer_list_node *)NULL;
    unsigned int temp;

    if(NULL == g_HrtimerList)
    {
        g_HrtimerList = pstHrtimer;
        hrtimer_clock_start(pstHrtimer->set_time_reload);
    }
    else
    {
        temp = get_hrtimer_clock_value();
        g_HrtimerList->set_time_reload = temp;

        pstCur = g_HrtimerList;
        while (pstCur != NULL)
        {
            if (pstCur->set_time_reload > pstHrtimer->set_time_reload)
            {
                break;
            }
            if(pstCur->set_time_reload == pstHrtimer->set_time_reload)
            {
                handler_node_add(pstCur, pstHandlerNode);
                (void)LOS_MemFree(m_aucSysMem0, (void *)pstHrtimer);
                return;
            }
           pstHrtimer->set_time_reload -= pstCur->set_time_reload;
           pstPrev = pstCur;
           pstCur = pstCur->pstNext;
        }
        if(pstCur == g_HrtimerList)
        {
            ptemp = g_HrtimerList;
            hrtimer_clock_stop();
            hrtimer_clock_start(pstHrtimer->set_time_reload);

            ptemp->set_time_reload -= pstHrtimer->set_time_reload;
            pstHrtimer->pstNext = pstCur;
            g_HrtimerList = pstHrtimer;
        }
        else if(pstCur == NULL)
        {
            pstPrev->pstNext = pstHrtimer;
        }
        else
        {
            pstHrtimer->pstNext = pstCur;
            pstPrev->pstNext = pstHrtimer;
            pstCur->set_time_reload -= pstHrtimer->set_time_reload;
        }
    }
    if(pstHandlerNode != NULL)
    {
        handler_node_add(pstHrtimer, pstHandlerNode);
    }
}

static void hrtimer_handler_run(void)
{
    struct hrtimer_list_node *pstHrtimer = (struct hrtimer_list_node *)NULL;
    struct handler_list_node *pstCur = (struct handler_list_node *)NULL;
    struct handler_list_node *pstHandler = (struct handler_list_node *)NULL;
    struct handler_list_node *pstHandlerTail = (struct handler_list_node *)NULL;
    struct handler_list_node *pstHandlerHead = (struct handler_list_node *)NULL;
    struct hrtimer timer;
    enum hrtimer_restart restart;

    pstHrtimer = g_HrtimerList;
    timer._softexpires.tv.usec = pstHrtimer->_softexpires.tv.usec;
    timer._softexpires.tv.sec = pstHrtimer->_softexpires.tv.sec;
    g_HrtimerList = pstHrtimer->pstNext;

    if(g_HrtimerList != NULL)
    {
        hrtimer_clock_stop();
        hrtimer_clock_start(g_HrtimerList->set_time_reload);
    }

    pstHandler = pstHrtimer->HandlerHead;
    pstHrtimer->pstNext = (struct hrtimer_list_node *)NULL;
    pstHrtimer->HandlerHead = (struct handler_list_node *)NULL;

    while(pstHandler != NULL)
    {
        if(pstHandler->pfnHandler != NULL)
        {
            timer.function = pstHandler->pfnHandler;
            restart = pstHandler->pfnHandler(&timer);   /*lint !e534*/
            pstCur = pstHandler;
            pstHandler = pstHandler->pstNext;
            pstCur->pstNext = (struct handler_list_node *)NULL;

            if(restart == HRTIMER_NORESTART)
            {
                (void)LOS_MemFree(m_aucSysMem0, (void *)pstCur);
            }
            else if(restart == HRTIMER_RESTART)
            {
                if(pstHandlerHead != NULL)
                {
                    pstHandlerTail->pstNext = pstCur;  /*lint !e613*/
                    pstHandlerTail = pstCur;
                }
                else
                {
                    pstHandlerHead = pstCur;
                    pstHandlerTail = pstCur;
                }
            }
            else
            {
                PRINT_ERR("The return value of hrtimer function is not defined!\n");
            }
        }
    }

    if (pstHandlerHead == NULL)
    {
        (void)LOS_MemFree(m_aucSysMem0, (void *)pstHrtimer);
    }
    else
    {
        pstHrtimer->set_time_reload = (unsigned int)((pstHrtimer->_softexpires.tv.sec*1000000 + pstHrtimer->_softexpires.tv.usec) * HRTIMER_PERUS); /*lint !e653*/
        hrtimer_node_add(pstHrtimer, pstHandlerHead);
    }
}

static void hrtimer_list_scan(void)
{
    hrtimer_clock_irqclear();

    if (g_HrtimerList != NULL)
    {
        hrtimer_handler_run();
    }
}

static void get_handler_node_position(const struct hrtimer *timer, struct hrtimer_list_node *hrtimer_node,
    struct handler_list_node **pstPrev, struct handler_list_node **pstCur)
{
    struct handler_list_node *pstHandlerCur = (struct handler_list_node *)NULL;
    struct handler_list_node *pstHandlerPrev = (struct handler_list_node *)NULL;
    unsigned int uvIntSave;

    uvIntSave = LOS_IntLock();
    pstHandlerCur = hrtimer_node->HandlerHead;
    while(pstHandlerCur != NULL)
    {
        if (pstHandlerCur->pfnHandler == timer->function &&
            pstHandlerCur->_softexpires.tv.sec == timer->_softexpires.tv.sec &&
            pstHandlerCur->_softexpires.tv.usec == timer->_softexpires.tv.usec)
        {
            *pstPrev = pstHandlerPrev;
            *pstCur = pstHandlerCur;
            LOS_IntRestore(uvIntSave);
            return;
        }
        pstHandlerPrev = pstHandlerCur;
        pstHandlerCur = pstHandlerCur->pstNext;
    }

    LOS_IntRestore(uvIntSave);
}

static void get_hrtimer_node_position(const struct hrtimer *timer, struct hrtimer_list_node **pstPrev, struct hrtimer_list_node **pstCur)
{
    struct handler_list_node *pstHandlerCur = (struct handler_list_node *)NULL;
    unsigned int uvIntSave;

    uvIntSave = LOS_IntLock();
    *pstCur = g_HrtimerList;
    while (*pstCur != NULL)
    {
        pstHandlerCur = (*pstCur)->HandlerHead;
        while(pstHandlerCur != NULL)
        {
            if (pstHandlerCur->pfnHandler == timer->function &&
                pstHandlerCur->_softexpires.tv.sec == timer->_softexpires.tv.sec &&
                pstHandlerCur->_softexpires.tv.usec == timer->_softexpires.tv.usec)
            {
                LOS_IntRestore(uvIntSave);
                return;
            }
            pstHandlerCur = pstHandlerCur->pstNext;
        }
        *pstPrev = *pstCur;
        *pstCur = (*pstCur)->pstNext;
    }
    LOS_IntRestore(uvIntSave);
}

static unsigned int change_node_position(struct hrtimer_list_node *pstPrev, struct hrtimer_list_node *pstCur,
    struct handler_list_node *pstHandlerPrev, struct handler_list_node *pstHandlerCur, union ktime time)
{
    struct hrtimer_list_node *pstHrtimer = (struct hrtimer_list_node *)NULL;
    unsigned int uvIntSave;

    uvIntSave = LOS_IntLock();
    if (pstHandlerPrev != NULL || pstHandlerCur->pstNext != NULL)
    {
        if (pstHandlerPrev == NULL)
        {
            pstCur->HandlerHead = pstHandlerCur->pstNext;
        }
        else
        {
            pstHandlerPrev->pstNext = pstHandlerCur->pstNext;
        }

        pstHandlerCur->pstNext = (struct handler_list_node *)NULL;
        pstHandlerCur->_softexpires = time;
        pstHrtimer = (struct hrtimer_list_node *)LOS_MemAlloc(m_aucSysMem0, sizeof(struct hrtimer_list_node));
        if(NULL == pstHrtimer)
        {
            LOS_IntRestore(uvIntSave);
            return LOS_NOK;
        }
        pstHrtimer->_softexpires = time;
        pstHrtimer->set_time_reload = (unsigned int)((time.tv.sec * 1000000 + time.tv.usec) * HRTIMER_PERUS); /*lint !e653*/
        pstHrtimer->HandlerHead = (struct handler_list_node *)NULL;
        pstHrtimer->pstNext = (struct hrtimer_list_node *)NULL;

        hrtimer_node_add(pstHrtimer, pstHandlerCur);
    }
    else
    {
        if(pstCur->pstNext != NULL)
        {
            if(pstCur == g_HrtimerList)
            {
                g_HrtimerList = pstCur->pstNext;
                g_HrtimerList->set_time_reload += get_hrtimer_clock_value();
                hrtimer_clock_stop();
                hrtimer_clock_start(g_HrtimerList->set_time_reload);
            }
            else
            {
                pstPrev->pstNext = pstCur->pstNext;
                pstCur->pstNext->set_time_reload += pstCur->set_time_reload;
            }
        }
        else
        {
            if(pstCur == g_HrtimerList)
            {
                g_HrtimerList = (struct hrtimer_list_node *)NULL;
                hrtimer_clock_stop();
            }
            else
            {
                pstPrev->pstNext = (struct hrtimer_list_node *)NULL;
            }
        }
        pstCur->pstNext = (struct hrtimer_list_node *)NULL;
        pstCur->_softexpires = time;
        pstCur->set_time_reload = (unsigned int)((time.tv.sec * 1000000 + time.tv.usec) * HRTIMER_PERUS); /*lint !e653*/
        pstCur->HandlerHead->_softexpires = time;
        hrtimer_node_add(pstCur, (struct handler_list_node *)NULL);
    }
    LOS_IntRestore(uvIntSave);

    return LOS_OK;
}

static void cancel_handler_node(const struct hrtimer *timer, struct hrtimer_list_node *pstCur)
{
    struct handler_list_node *pstHandlerCur = (struct handler_list_node *)NULL;
    struct handler_list_node *pstHandlerPrev = (struct handler_list_node *)NULL;

    pstHandlerCur = pstCur->HandlerHead;
    pstHandlerPrev = pstHandlerCur;
    while(pstHandlerCur != NULL)
    {
        if(pstHandlerCur->pfnHandler == timer->function &&
            pstHandlerCur->_softexpires.tv.sec == timer->_softexpires.tv.sec &&
            pstHandlerCur->_softexpires.tv.usec == timer->_softexpires.tv.usec)
        {
            if(pstHandlerCur == pstCur->HandlerHead)
            {
                pstCur->HandlerHead = pstHandlerCur->pstNext;
            }
            else
            {
                pstHandlerPrev->pstNext = pstHandlerCur->pstNext;
            }
            pstHandlerCur->pstNext = (struct handler_list_node *)NULL;
            (void)LOS_MemFree(m_aucSysMem0, (void *)pstHandlerCur);
            break;
        }
        pstHandlerPrev = pstHandlerCur;
        pstHandlerCur = pstHandlerCur->pstNext;
    }
}

static unsigned int check_time(union ktime *time)
{
    if((time->tv.sec == 0) && (time->tv.usec == 0))
    {
        return LOS_NOK;
    }

    if(time->tv.usec >= 1000000)
    {
        time->tv.sec += time->tv.usec /1000000;
        time->tv.usec = time->tv.usec % 1000000;
    }

    return LOS_OK;
}

void hrtimer_init(struct hrtimer *timer, clockid_t clock_id, enum hrtimer_mode mode)
{
    if(NULL == timer || mode != HRTIMER_MODE_REL)
        PRINT_ERR("The timer is NULL OR The mode is not HRTIMER_MODE_REL!\n");
    return;
}

int hrtimer_create(struct hrtimer *timer, union ktime time, Handler handler)
{
    unsigned int uwRet;

    if(NULL == timer)
    {
        return -1;
    }

    if(NULL == handler)
    {
        return -1;
    }

    uwRet = check_time(&time);
    if(uwRet != LOS_OK)
    {
        return -1;
    }

    timer->_softexpires.tv.sec = time.tv.sec;
    timer->_softexpires.tv.usec = time.tv.usec;
    timer->function = handler;

    return 0;
}

int hrtimer_start(struct hrtimer *timer, union ktime time, const enum hrtimer_mode mode)
{
    struct hrtimer_list_node *pstHrtimer = (struct hrtimer_list_node *)NULL;
    struct hrtimer_list_node *pstPrev = (struct hrtimer_list_node *)NULL;
    struct hrtimer_list_node *pstCur = (struct hrtimer_list_node *)NULL;
    struct handler_list_node *pstHandler = (struct handler_list_node *)NULL;
    struct handler_list_node *pstHandlerPrev = (struct handler_list_node *)NULL;
    struct handler_list_node *pstHandlerCur = (struct handler_list_node *)NULL;
    unsigned int uvIntSave;
    unsigned int uwRet;

    if(NULL == timer || mode != HRTIMER_MODE_REL)
    {
        return -1;
    }

    uwRet = check_time(&time);
    if(uwRet != LOS_OK)
    {
        return -1;
    }

    get_hrtimer_node_position(timer, &pstPrev, &pstCur);
    if(pstCur == NULL)
    {
        pstHrtimer = (struct hrtimer_list_node *)LOS_MemAlloc(m_aucSysMem0, sizeof(struct hrtimer_list_node));
        if(NULL == pstHrtimer)
        {
            return -1;
        }
        pstHrtimer->_softexpires = time;
        pstHrtimer->set_time_reload = (unsigned int)((time.tv.sec*1000000 + time.tv.usec) * HRTIMER_PERUS); /*lint !e653*/
        pstHrtimer->HandlerHead = (struct handler_list_node *)NULL;
        pstHrtimer->pstNext = (struct hrtimer_list_node *)NULL;

        pstHandler = (struct handler_list_node *)LOS_MemAlloc(m_aucSysMem0, sizeof(struct handler_list_node));
        if(NULL == pstHandler)
        {
            (void)LOS_MemFree(m_aucSysMem0, (void *)pstHrtimer);/*lint !e424*/
            return -1;
        }
        pstHandler->_softexpires = time;
        if(NULL == timer->function)
        {
            (void)LOS_MemFree(m_aucSysMem0, (void *)pstHrtimer);/*lint !e424*/
            (void)LOS_MemFree(m_aucSysMem0, (void *)pstHandler); /*lint !e424*/
            return -1;
        }

        pstHandler->pfnHandler = timer->function;
        pstHandler->pstNext = (struct handler_list_node *)NULL;

        uvIntSave = LOS_IntLock();
        hrtimer_node_add(pstHrtimer, pstHandler);
        LOS_IntRestore(uvIntSave);
        return 0;
    }
    else
    {
        get_handler_node_position(timer, pstCur, &pstHandlerPrev, &pstHandlerCur);

        uwRet = change_node_position(pstPrev, pstCur, pstHandlerPrev, pstHandlerCur, time);
        if (uwRet != LOS_OK)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
}

int hrtimer_cancel(struct hrtimer *timer)
{
    struct hrtimer_list_node *pstPrev = (struct hrtimer_list_node *)NULL;
    struct hrtimer_list_node *pstCur = (struct hrtimer_list_node *)NULL;
    unsigned int uvIntSave;

    if(NULL == timer)
    {
        return -1;
    }

    uvIntSave = LOS_IntLock();
    pstCur = g_HrtimerList;
    if(pstCur == NULL)
    {
        LOS_IntRestore(uvIntSave);
        return 0;
    }

    get_hrtimer_node_position(timer, &pstPrev, &pstCur);

    if (pstCur == NULL)
    {
        LOS_IntRestore(uvIntSave);
        return 0;
    }
    else if(pstCur == g_HrtimerList)
    {
        cancel_handler_node(timer, pstCur);

        if(pstCur ->HandlerHead == NULL)
        {
            g_HrtimerList = pstCur->pstNext;
            if(NULL != g_HrtimerList)
            {
                g_HrtimerList->set_time_reload += get_hrtimer_clock_value();
                hrtimer_clock_stop();
                hrtimer_clock_start(g_HrtimerList->set_time_reload);
            }
            else
            {
                hrtimer_clock_stop();
            }
            pstCur->pstNext = (struct hrtimer_list_node *)NULL;
            (void)LOS_MemFree(m_aucSysMem0, (void *)pstCur);
        }
    }
    else
    {
        cancel_handler_node(timer, pstCur);

        if(pstCur ->HandlerHead == NULL)
        {
            if(pstCur->pstNext == NULL)
            {
                pstPrev->pstNext = (struct hrtimer_list_node *)NULL;
            }
            else
            {
                pstPrev->pstNext = pstCur->pstNext;
                pstPrev->pstNext->set_time_reload += pstCur->set_time_reload;
            }
            pstCur->pstNext = (struct hrtimer_list_node *)NULL;
            (void)LOS_MemFree(m_aucSysMem0, (void *)pstCur);
        }
    }
    LOS_IntRestore(uvIntSave);
    return 1;
}

u64 hrtimer_forward(struct hrtimer *timer, union ktime interval)
{
    struct hrtimer_list_node *pstPrev = (struct hrtimer_list_node *)NULL;
    struct hrtimer_list_node *pstCur = (struct hrtimer_list_node *)NULL;
    struct handler_list_node *pstHandlerPrev = (struct handler_list_node *)NULL;
    struct handler_list_node *pstHandlerCur = (struct handler_list_node *)NULL;
    unsigned int uwRet;

    if(NULL == timer)
    {
        return 0;
    }

    uwRet = check_time(&interval);
    if(uwRet != LOS_OK)
    {
        return 0;
    }

    get_hrtimer_node_position(timer, &pstPrev, &pstCur);
    if(pstCur == NULL)
    {
        return 0;
    }
    else
    {
        get_handler_node_position(timer, pstCur, &pstHandlerPrev, &pstHandlerCur);
        timer->_softexpires = interval;
        uwRet = change_node_position(pstPrev, pstCur, pstHandlerPrev, pstHandlerCur, interval);
        if (uwRet != LOS_OK)
        {
            return 0;
        }
        else
        {
            return (u64)((interval.tv.sec * 1000000 + interval.tv.usec) * HRTIMER_PERUS); /*lint !e647 !e653*/
        }
    }
}

int hrtimer_is_queued(struct hrtimer *timer)
{
    struct hrtimer_list_node *pstCur = (struct hrtimer_list_node *)NULL;
    struct handler_list_node *pstHandler = (struct handler_list_node *)NULL;
    int wRet = LOS_NOK;
    unsigned int uvIntSave;

    if(NULL == timer)
    {
        return -1;
    }

    uvIntSave = LOS_IntLock();
    pstCur = g_HrtimerList;
    while (pstCur != NULL)
    {
        pstHandler = pstCur->HandlerHead;
        while(pstHandler != NULL)
        {
            if (pstHandler->pfnHandler == timer->function)
            {
                break;
            }
            pstHandler = pstHandler->pstNext;
        }

        if(pstHandler != NULL && pstHandler->pfnHandler == timer->function &&
            pstHandler->_softexpires.tv.sec == timer->_softexpires.tv.sec &&
            pstHandler->_softexpires.tv.usec == timer->_softexpires.tv.usec)
        {
            wRet = LOS_OK;
            break;
        }
        pstCur = pstCur->pstNext;
    }
    LOS_IntRestore(uvIntSave);

    return wRet;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
