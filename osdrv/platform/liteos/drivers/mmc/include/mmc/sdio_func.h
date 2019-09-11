#ifndef _SDIO_FUNC_H
#define _SDIO_FUNC_H

#include "mmc/mmc_os_adapt.h"
#include "mmc/core.h"
#include "mmc/host.h"
#include "mmc/card.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef void (sdio_irq_handler_t)(struct sdio_func *);

struct sdio_embedded_func {
    uint32_t max_blk_size;
    uint16_t func_code;
};

/*
 * SDIO function CIS tuple
 */
struct sdio_func_tuple {
    struct sdio_func_tuple *next;
    uint8_t code;
    uint8_t size;
    uint8_t data[0];
};

/* SDIO function struct */
struct sdio_func {
    struct mmc_card     *card;
    union {
        uint32_t data;
        struct {
            uint32_t present : 1;
            uint32_t reserved :31;
        }bits;
    } state;        /* function state */
    uint32_t        max_blk_size;    /* max block size */
    uint32_t        cur_blk_size;    /* cur block size */
    uint32_t        func_num;        /* function number */
    uint8_t         func_class;        /* function code */
    uint16_t        manufacturer_id;  /* manufacturer id */
    uint16_t        device_id;        /* device id */
    uint32_t        en_timeout_ms;    /* */
    sdio_irq_handler_t    *sdio_irq_handler;    /* IRQ handle */

    struct sdio_func_tuple *tuple_link;
    void            *data;
};

#define is_sdio_func_present(func)    ((func)->state.bits.present)
#define set_sdio_func_present(func)    ((func)->state.bits.present = 1)
#define clr_sdio_func_present(func)    ((func)->state.bits.present = 0)

/* funcions declaration to extern caller */
/*
 * SDIO I/O operations
 */
extern struct sdio_func *sdio_get_func(uint32_t func_num,
        uint32_t manufacturer_id, uint32_t device_id);
extern int sdio_en_func(struct sdio_func *func);
extern int sdio_dis_func(struct sdio_func *func);
extern int sdio_enable_blksz_for_byte_mode(struct sdio_func *func, uint32_t enable);
extern int sdio_set_cur_blk_size(struct sdio_func *func, uint32_t blksz);

extern int sdio_require_irq(struct sdio_func *func, sdio_irq_handler_t *handler);
extern int sdio_release_irq(struct sdio_func *func);

/*extern uint32_t sdio_align_size(struct sdio_func *func, uint32_t sz);*/

extern uint8_t sdio_read_byte(struct sdio_func *func, uint32_t addr, int *err_ret);
extern uint8_t sdio_read_byte_ext(struct sdio_func *func, uint32_t addr, int *err_ret,
    unsigned in);

extern int sdio_read_incr_block(struct sdio_func *func, void *dst,
    uint32_t addr, int count);
extern int sdio_read_fifo_block(struct sdio_func *func, void *dst,
    uint32_t addr, int count);
extern int sdio_write_incr_block(struct sdio_func *func, uint32_t addr,
    void *src, int count);
extern int sdio_write_fifo_block(struct sdio_func *func, uint32_t addr,
    void *src, int count);

extern void sdio_write_byte(struct sdio_func *func, uint8_t byte,
    uint32_t addr, int *err_ret);

extern uint8_t sdio_write_byte_raw(struct sdio_func *func, uint8_t write_byte,
    uint32_t addr, int *err_ret);

extern unsigned char sdio_func0_read_byte(struct sdio_func *func,
    uint32_t addr, int *err_ret);
extern void sdio_func0_write_byte(struct sdio_func *func, unsigned char byte,
    uint32_t addr, int *err_ret);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* _SDIO_FUNC_H */
