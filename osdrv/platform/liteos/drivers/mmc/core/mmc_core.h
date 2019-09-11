/* mmc_core.h begin */
#ifndef _MMC_CORE_H
#define _MMC_CORE_H

/* Order's important */
#include "mmc/mmc_os_adapt.h"
#include "mmc/mmc.h"
#include "mmc/sd.h"
#include "mmc/sdio.h"
#include "mmc/core.h"
#include "mmc/host.h"
#include "mmc/card.h"
#include "mmc/sdio_func.h"
#include "mmc/block.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


/* this headers include all mmc functions declaration */
/* functions declaration */

/* in block.c */
int block_init(void);
int mmc_block_init(void* dev);
int mmc_block_deinit(void* dev);

/* in card.c */
struct mmc_card * mmc_alloc_card(struct mmc_host *host);
void mmc_free_card(struct mmc_host *host);
void mmc_del_card(struct mmc_host *host);
void mmc_reset_card(struct mmc_card *card);
int mmc_add_card(struct mmc_card *card);

/* in core.c */
void mmc_wait_for_req(struct mmc_host *cur_host, struct mmc_request *cur_mrq);
int mmc_wait_for_cmd(struct mmc_host *cur_host, struct mmc_cmd *cur_cmd, int retries);
void mmc_set_data_timeout(struct mmc_data *cur_data, const struct mmc_card *cur_card);
unsigned int mmc_get_bits(unsigned int *bits, int start, int size);
uint32_t mmc_select_voltage(struct mmc_host *host, uint32_t ocr);
void mmc_start(struct mmc_card* card, struct mmc_req_entity* req_entity);
int sent_stop(struct mmc_card *card, int dir, unsigned int * stop_status);

/* in host.c */
int mmc_do_detect(struct mmc_host *host);
int mmc_alloc_mmc_list(void);
int set_mmc_host(struct mmc_host *host, uint32_t mmc_idx);
void mmc_free_mmc_list(void);
int card_reset(struct mmc_card *card);

/* in mmc.c */
int mmc_assume_mmc(struct mmc_card *);
int mmc_app_cmd(struct mmc_card *card);
void mmc_idle_card(struct mmc_card *card);
int get_card_status(struct mmc_card *card, unsigned int *status);
/* in mmc_ops.c */
int mmc_all_send_cid(struct mmc_card *card, uint32_t *cid);
int mmc_send_csd(struct mmc_card *card, uint32_t *csd);
int mmc_select_card(struct mmc_card *card);
int mmc_send_op_cond(struct mmc_card *card, uint32_t ocr, uint32_t *rocr);
int mmc_set_relative_addr(struct mmc_card *card);
int mmc_send_ext_csd(struct mmc_card *card, unsigned char *ext_csd);
int mmc_send_status(struct mmc_card *card, uint32_t *status);
int mmc_switch(struct mmc_card *card, unsigned char set, unsigned char index,
        unsigned char value,
        unsigned int timeout_ms);

/* in sd.c */
int mmc_assume_sd(struct mmc_card *card);
int sd_switch_func_hs(struct mmc_card *card);
void sd_go_highspeed(struct mmc_card *card);
int sd_get_cid(struct mmc_card *card, union mmc_ocr ocr,
        uint32_t *cid, uint32_t *rocr);
void sd_decode_cid(struct mmc_card *card);
int sd_get_csd(struct mmc_card *card);
int sd_setup_card(struct mmc_card *card);
void sd_go_highspeed(struct mmc_card *card);
unsigned int mmc_sd_get_max_clock(struct mmc_card *card);
int sd_card_reset(struct mmc_card *card);
/* in sd_ops.c */
int sd_switch_func(struct mmc_card *card, int mode, int group,
    unsigned char value, unsigned char *resp);
int sd_send_if_cond(struct mmc_card *card, uint32_t ocr);
int sd_app_set_bus_width(struct mmc_card *card, enum mmc_bus_width width);
int sd_app_send_op_cond(struct mmc_card *card, uint32_t ocr, uint32_t *rocr);
int sd_signal_volt_switch(struct mmc_card *card);
int sd_app_send_scr(struct mmc_card *card, uint32_t *scr);
int sd_send_relative_addr(struct mmc_card *card, uint32_t *rca);
int sd_app_sd_status(struct mmc_card *card, void *ssr);

/* in sdio.c */
int sdio_reset(struct mmc_card *card);
int sdio_reset_comm(struct mmc_card *card);
int mmc_assume_sdio(struct mmc_card *card);
/* sdio_ops.c */
int sdio_rw_direct(struct mmc_card *card, int write, uint32_t fn,
    uint32_t addr, uint8_t in, uint8_t *out);
int sdio_read_cis(struct mmc_card *card, struct sdio_func *func);
int sdio_read_func_cis(struct sdio_func *fc);
int sdio_io_send_op_cond(struct mmc_card *card, uint32_t ocr, uint32_t *rocr);
int sdio_io_rw_extended(struct mmc_card *card, int write, uint32_t fn,
            uint32_t addr, int incr_addr, uint8_t *buf, uint32_t blocks, uint32_t blksz);
int sdio_io_rw_direct(struct mmc_card *card, int write, uint32_t fn,
    uint32_t addr, uint8_t in, uint8_t *out);
/* in sdio_func.c */
void sdio_delete_func(struct mmc_card *card);

/* extern functions declaration,
 * hardware drivers should adapt the functions.
 * */

extern void mmc_do_request(struct mmc_host *mmc, struct mmc_request *mrq);
extern void mmc_set_clock(struct mmc_host *mmc, unsigned int clock);
extern void mmc_set_power_mode(struct mmc_host *mmc, unsigned int mode);
extern void mmc_set_bus_width(struct mmc_host *mmc, enum mmc_bus_width width);
extern void mmc_set_timing(struct mmc_host *mmc, enum mmc_bus_timing timing);
extern int mmc_voltage_switch(struct mmc_host *mmc, enum signal_volt volt);
extern int mmc_get_readonly(struct mmc_host *mmc);
extern void mmc_set_bus_mode(struct mmc_host *mmc, enum bus_mode mode);
extern int mmc_execute_tuning(struct mmc_host *mmc, uint32_t cmd_code);
extern void mmc_set_sdio_irq(struct mmc_host *mmc, int enable);
extern void mmc_hw_reset(struct mmc_host *mmc);
extern void mmc_hw_init(struct mmc_host *mmc);
extern int mmc_get_card_detect(struct mmc_host *mmc);
extern int mmc_get_host_idx(struct mmc_host *mmc);
extern int get_emmc_host_id(void);
extern uint32_t get_mmc_max_num(void);
extern void mmc_priv_fn(struct mmc_card *card);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif

/* mmc_core.h end */
