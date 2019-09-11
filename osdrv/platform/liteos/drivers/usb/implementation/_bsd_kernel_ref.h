/*-
 * Copyright (c) 1995 Terrence R. Lambert
 * All rights reserved.
 *
 * Copyright (c) 1990, 1993
 *    The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the University of
 *    California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *    @(#)kernel.h    8.3 (Berkeley) 1/21/94
 * $FreeBSD$
 */
/*-
 * Copyright (c) 1997 Berkeley Software Design, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Berkeley Software Design Inc's name may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY BERKELEY SOFTWARE DESIGN INC ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL BERKELEY SOFTWARE DESIGN INC BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *    from BSDI $Id: mutex.h,v 2.7.2.35 2000/04/27 03:10:26 cp Exp $
 * $FreeBSD$
 */

#ifndef _BSD_KERNEL_H_
#define    _BSD_KERNEL_H_

#include "los_typedef.h"
#include "stdint.h"

#define    _KERNEL
#define    __FreeBSD_version 1000000

#define    rebooting 0
#define    M_USB 0
#define    M_USBDEV 0
#define    USB_PROC_MAX 3
#define    M_DEVBUF 1


#define    SYSCTL_DECL(...)
#define    SYSCTL_INT(...)
#define    TUNABLE_INT(...)
#define    MALLOC_DECLARE(...)
#define    MALLOC_DEFINE(...)
#define    EVENTHANDLER_DECLARE(...)
#define    EVENTHANDLER_INVOKE(...)
#define    SCHEDULER_STOPPED(x) (0)
#define    PI_SWI(...) (0)
#define    UNIQ_NAME(x) x
#define    UNIQ_NAME_STR(x) #x
#define    DEVCLASS_MAXUNIT 32
#define    MOD_LOAD 1
#define    MOD_UNLOAD 2

#define    DEVMETHOD(what, func) {#what, (void *)&func}
#define    DEVMETHOD_END {0,0}
#define    DRIVER_MODULE(name, busname, driver, devclass, evh, arg)    \
/*   static  */struct module_data bsd_##name##_##busname##_driver_mod = {    \
    evh, arg, #busname, #name, #busname "/" #name,            \
    &driver, &devclass, { 0, 0 } };

#define    MODULE_DEPEND(...)

#define    hz  LOSCFG_BASE_CORE_TICK_PER_SECOND
#ifndef PAGE_SIZE
#define    PAGE_SIZE 4096
#endif

/*
 * Mutex types and options passed to mtx_init().  MTX_QUIET and MTX_DUPOK
 * can also be passed in.
 */
#define    MTX_DEF        0x00000000    /* DEFAULT (sleep) lock */
#define MTX_SPIN    0x00000001    /* Spin lock (disables interrupts) */
#define MTX_RECURSE    0x00000004    /* Option: lock allowed to recurse */
#define    MTX_NOWITNESS    0x00000008    /* Don't do any witness checking. */
#define MTX_NOPROFILE   0x00000020    /* Don't profile this lock */

#define    DEVICE_ATTACH(dev, ...) \
  (((device_attach_t *)(device_get_method(dev, "device_attach")))(dev,## __VA_ARGS__))
#define    DEVICE_DETACH(dev, ...) \
  (((device_detach_t *)(device_get_method(dev, "device_detach")))(dev,## __VA_ARGS__))
#define    DEVICE_PROBE(dev, ...) \
  (((device_probe_t *)(device_get_method(dev, "device_probe")))(dev,## __VA_ARGS__))
#define    DEVICE_RESUME(dev, ...) \
  (((device_resume_t *)(device_get_method(dev, "device_resume")))(dev,## __VA_ARGS__))
#define    DEVICE_SHUTDOWN(dev, ...) \
  (((device_shutdown_t *)(device_get_method(dev, "device_shutdown")))(dev,## __VA_ARGS__))
#define    DEVICE_SUSPEND(dev, ...) \
  (((device_suspend_t *)(device_get_method(dev, "device_suspend")))(dev,## __VA_ARGS__))
#define    USB_HANDLE_REQUEST(dev, ...) \
  (((usb_handle_request_t *)(device_get_method(dev, "usb_handle_request")))(dev,## __VA_ARGS__))
#define    USB_TAKE_CONTROLLER(dev, ...) \
  (((usb_take_controller_t *)(device_get_method(dev, "usb_take_controller")))(dev,## __VA_ARGS__))

enum {
    SI_SUB_DUMMY = 0x0000000,
    SI_SUB_LOCK = 0x1B00000,
    SI_SUB_KLD = 0x2000000,
    SI_SUB_DRIVERS = 0x3100000,
    SI_SUB_PSEUDO = 0x7000000,
    SI_SUB_KICK_SCHEDULER = 0xa000000,
    SI_SUB_RUN_SCHEDULER = 0xfffffff
};

enum {
    SI_ORDER_FIRST = 0x0000000,
    SI_ORDER_SECOND = 0x0000001,
    SI_ORDER_THIRD = 0x0000002,
    SI_ORDER_FOURTH = 0x0000003,
    SI_ORDER_MIDDLE = 0x1000000,
    SI_ORDER_ANY = 0xfffffff    /* last */
};

typedef unsigned long bus_addr_t;
typedef unsigned long bus_size_t;

typedef void *bus_dmamap_t;
typedef void *bus_dma_tag_t;

typedef void *bus_space_tag_t;
typedef unsigned long bus_space_handle_t;

typedef struct bus_dma_segment {
    bus_addr_t    ds_addr;    /* DMA address */
    bus_size_t    ds_len;        /* length of transfer */
} bus_dma_segment_t;

typedef enum {
    BUS_DMA_LOCK    = 0x01,
    BUS_DMA_UNLOCK    = 0x02,
} bus_dma_lock_op_t;


/* SYSINIT API */

struct sysinit {
    void    (*func) (void *arg);
    void   *data;
};

extern struct mtx Giant;

#define ticks (LOS_TickCountGet() & 0xFFFFFFFF)

/* DEVICE API */

struct driver;
struct devclass;
struct device_;
struct module;
struct module_data;

typedef struct driver driver_t;
typedef struct devclass *devclass_t;
typedef struct device_ *device_t;
typedef int (driver_filter_t)(void *);
#define    FILTER_STRAY        0x01
#define    FILTER_HANDLED        0x02
#define    FILTER_SCHEDULE_THREAD    0x04

typedef int device_attach_t (device_t dev);
typedef int device_detach_t (device_t dev);
typedef int device_resume_t (device_t dev);
typedef int device_shutdown_t (device_t dev);
typedef int device_probe_t (device_t dev);
typedef int device_suspend_t (device_t dev);

typedef int bus_child_location_str_t (device_t parent, device_t child, char *buf, size_t buflen);
typedef int bus_child_pnpinfo_str_t (device_t parent, device_t child, char *buf, size_t buflen);
typedef void bus_driver_added_t (device_t dev, driver_t *driver);

struct device_method {
    const char *desc;
    void  *const func;
};

typedef struct device_method device_method_t;

struct device_ {
    TAILQ_HEAD(device_list, device_) dev_children;
    TAILQ_ENTRY(device_) dev_link;

    struct device_ *dev_parent;
    const struct module_data *dev_module;
    void   *dev_sc;
    void   *dev_aux;
    driver_filter_t *dev_irq_filter;
    driver_intr_t *dev_irq_fn;
    void   *dev_irq_arg;

    uint16_t dev_unit;

    char    dev_nameunit[64];
    char    dev_desc[64];

    uint8_t    dev_res_alloc:1;
    uint8_t    dev_quiet:1;
    uint8_t    dev_softc_set:1;
    uint8_t    dev_softc_alloc:1;
    uint8_t    dev_attached:1;
    uint8_t    dev_fixed_class:1;
    uint8_t    dev_unit_manual:1;
};

struct devclass {
    device_t dev_list[DEVCLASS_MAXUNIT];
};

struct driver {
    const char *name;
    const struct device_method *methods;
    uint32_t size;
};

struct module_data {
    int     (*callback) (struct module *, int, void *arg);
    void   *arg;
    const char *bus_name;
    const char *mod_name;
    const char *long_name;
    const struct driver *driver;
    struct devclass **devclass_pp;
    TAILQ_ENTRY(module_data) entry;
};


device_t device_get_parent(device_t dev);
void   *device_get_method(device_t dev, const char *what);
const char *device_get_name(device_t dev);
const char *device_get_nameunit(device_t dev);

#define    device_printf(dev, fmt,...) \
    dprintf("%s: " fmt, device_get_nameunit(dev),## __VA_ARGS__)
device_t device_add_child(device_t dev, const char *name, int unit);
void    device_quiet(device_t dev);
void    device_set_interrupt(device_t dev, driver_filter_t *, driver_intr_t *, void *);
void    device_run_interrupts(device_t parent);
void    device_set_ivars(device_t dev, void *ivars);
void   *device_get_ivars(device_t dev);
const char *device_get_desc(device_t dev);
int        device_probe_and_attach(device_t dev);
struct module_data *bus_get_device(const char *busname);
int        device_detach(device_t dev);
void   *device_get_softc(device_t dev);
void    device_set_softc(device_t dev, void *softc);
int        device_delete_child(device_t dev, device_t child);
int        device_delete_children(device_t dev);
int        device_is_attached(device_t dev);
void    device_set_desc(device_t dev, const char *desc);
void    device_set_desc_copy(device_t dev, const char *desc);
int        device_get_unit(device_t dev);
void   *devclass_get_softc(devclass_t dc, int unit);
int        devclass_get_maxunit(devclass_t dc);
device_t devclass_get_device(devclass_t dc, int unit);
devclass_t devclass_find(const char *classname);

#define    bus_get_dma_tag(...) (NULL)
int        bus_generic_detach(device_t dev);
int        bus_generic_resume(device_t dev);
int        bus_generic_shutdown(device_t dev);
int        bus_generic_suspend(device_t dev);
int        bus_generic_print_child(device_t dev, device_t child);

/* BUS SPACE API */

void    module_register(void *);
/* USB */

typedef int usb_handle_request_t (device_t dev, const void *req, void **pptr, uint16_t *plen, uint16_t offset, uint8_t *pstate);
typedef int usb_take_controller_t (device_t dev);

/* set some defaults */

#ifndef USB_POOL_SIZE
#define    USB_POOL_SIZE (1024*1024)    /* 1 MByte */
#endif

#ifndef    __DECONST
#define    __DECONST(type, var)    ((type)(__uintptr_t)(const void *)(var))
#endif

/* BUS SPACE API */

void    bus_space_write_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, uint8_t data);
void    bus_space_write_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, uint16_t data);
void    bus_space_write_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, uint32_t data);

uint8_t    bus_space_read_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset);
uint16_t bus_space_read_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset);
uint32_t bus_space_read_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset);

void    bus_space_read_multi_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, uint8_t *datap, bus_size_t count);
void    bus_space_read_multi_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, uint16_t *datap, bus_size_t count);
void    bus_space_read_multi_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, uint32_t *datap, bus_size_t count);

void    bus_space_write_multi_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, uint8_t *datap, bus_size_t count);
void    bus_space_write_multi_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, uint16_t *datap, bus_size_t count);
void    bus_space_write_multi_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset, uint32_t *datap, bus_size_t count);

void    bus_space_read_region_1(bus_space_tag_t space, bus_space_handle_t handle, bus_size_t offset, uint8_t *datap, bus_size_t count);
void    bus_space_write_region_1(bus_space_tag_t space, bus_space_handle_t handle, bus_size_t offset, uint8_t *datap, bus_size_t count);
void    bus_space_read_region_4(bus_space_tag_t space, bus_space_handle_t handle, bus_size_t offset, uint32_t *datap, bus_size_t count);
void    bus_space_write_region_4(bus_space_tag_t space, bus_space_handle_t handle, bus_size_t offset, uint32_t *datap, bus_size_t count);

void    bus_space_barrier(bus_space_tag_t space, bus_space_handle_t handle, bus_size_t offset, bus_size_t length, int flags);

extern void devclass_module_dump(void);

#endif                    /* _BSD_KERNEL_H_ */
