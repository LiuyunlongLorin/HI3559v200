#ifndef _COMPAT_LINUX_FB_H
#define _COMPAT_LINUX_FB_H
#include "stdlib.h"
#include <linux/workqueue.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include "liteos/fb.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

struct vm_area_struct;
typedef struct fb_info fb_info_t;
typedef  struct fb_var_screeninfo  fb_var_screeninfo_t;
typedef  struct fb_fix_screeninfo   fb_fix_screeninfo_t;
typedef struct fb_cmap   fb_cmap_t;
typedef struct fb_ops  fb_ops_t;
typedef unsigned long ulong;

struct fb_ops {
    int (*fb_open)(fb_info_t *info, int user);
    int (*fb_release)(fb_info_t *info, int user);

    /* pan display */
    int (*fb_pan_display)(fb_var_screeninfo_t *var, fb_info_t *info);

    /* perform fb specific ioctl (optional) */
    int (*fb_ioctl)(fb_info_t *info, unsigned int cmd,
            ulong arg);
    /* checks var and eventually tweaks it to something supported,
     * DO NOT MODIFY PAR */
    int (*fb_check_var)(fb_var_screeninfo_t *var, fb_info_t *info);

    /* set the video mode according to info->var */
    int (*fb_set_par)(fb_info_t *info);
    /* set color registers in batch */
    int (*fb_setcmap)(fb_cmap_t *cmap, fb_info_t *info);
    /* set color register */
    int (*fb_setcolreg)(unsigned regno, unsigned red, unsigned green,
                unsigned blue, unsigned transp, fb_info_t *info);

    struct module *owner;
};

/* FBINFO_* = fb_info.flags bit flags */
#define COMPAT_FBINFO_MODULE                    0x0001
#define COMPAT_FBINFO_HWACCEL_XPAN        0x1000
#define COMPAT_FBINFO_HWACCEL_YPAN        0x2000
#define COMPAT_FBINFO_MISC_USEREVENT      0x10000

struct fb_info {
    atomic_t count;
    int node;
    int flags;
    pthread_mutex_t lock;
    fb_var_screeninfo_t var;    /* Current var */
    fb_fix_screeninfo_t fix;    /* Current fix */
    fb_cmap_t cmap;        /* Current cmap */
    fb_ops_t *fbops;
    char __iomem *screen_base;    /* Virtual address */
    ulong screen_size;    /* Amount of ioremapped VRAM or 0 */
    void *par;
};

#ifdef MODULE
#define FBINFO_DEFAULT    COMPAT_FBINFO_MODULE
#else
#define FBINFO_DEFAULT    0
#endif

#define COMPAT_FBINFO_FLAG_DEFAULT    FBINFO_DEFAULT

/* drivers/video/fbmem.c */
extern int register_framebuffer(fb_info_t *info);
extern int unregister_framebuffer(fb_info_t *info);
extern int fb_set_var(fb_info_t *info, fb_var_screeninfo_t *var);
extern int fb_pan_display(fb_info_t *info, fb_var_screeninfo_t *var);
extern fb_info_t *framebuffer_alloc(size_t size, struct device *dev);
extern void framebuffer_release(fb_info_t *info);

/* drivers/video/fbcmap.c */
extern int fb_alloc_cmap(fb_cmap_t *colormap, int length, int transp);
extern void fb_dealloc_cmap(fb_cmap_t *colormap);
extern int fb_set_cmap(fb_cmap_t *colormap, fb_info_t *fb_info);
extern int fb_set_user_cmap(fb_cmap_t *colormap, fb_info_t *fb_info);
extern int fb_copy_cmap(const fb_cmap_t *from, fb_cmap_t *to);
extern int fb_cmap_to_user(const fb_cmap_t *from, fb_cmap_t *to);
extern const fb_cmap_t *fb_default_cmap(int length);

/* FBINFO_* = fb_info.flags bit flags */
#define FBINFO_MODULE                       COMPAT_FBINFO_MODULE    /* Low-level driver is a module */
#define FBINFO_HWACCEL_XPAN        COMPAT_FBINFO_HWACCEL_XPAN /* optional */
#define FBINFO_HWACCEL_YPAN        COMPAT_FBINFO_HWACCEL_YPAN /* optional */
#define FBINFO_MISC_USEREVENT     COMPAT_FBINFO_MISC_USEREVENT /* event request from userspace */

#define FBINFO_FLAG_DEFAULT    COMPAT_FBINFO_FLAG_DEFAULT

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* _COMPAT_LINUX_FB_H */
