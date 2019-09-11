#ifndef _MMC_BLOCK_H
#define _MMC_BLOCK_H

#include "mmc/core.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#undef  CARD_ERR_RESET_ENABLE
#define MAX_BLOCK_COUNT 3
#define MMC_CARDBUS_BLOCK_SIZE      512
#define BLOCK_THREAD_STACKSZ 0x1800


/* block ioctl */
#define RT_DEVICE_CTRL_BLK_GETGEOME     0x10            /**< get geometry information   */
#define RT_DEVICE_CARD_STATUS           0x1            /**< get card status   */
#define RT_DEVICE_CARD_AU_SIZE          0x2            /**< get card au size   */
#define RT_DEVICE_BLOCK_ERROR_COUNT     0x3           /**< get block error count   */


/* request error state */

#define MMC_REQ_SUCCESS   0
#define MMC_REQ_RETRY     1
#define MMC_REQ_ABORT     2

typedef unsigned long sector_t;

struct blk_node_no {
    int node_flag[MAX_BLOCK_COUNT];
    mmc_mutex flag_lock;
    int blk_max_count;
    struct mmc_block *blocks[3];
};

/**
 * block device geometry structure
 */
struct rt_device_blk_geometry
{
    unsigned long sector_count;                           /**< count of sectors */
    unsigned long bytes_per_sector;                       /**< number of bytes per sector */
    unsigned long block_size;                             /**< number of bytes to erase one block */
};

struct mmc_req_entity {
    struct mmc_request    mrq;
    struct mmc_cmd    cmd;
    struct mmc_data       data;
};


struct mmc_req {
    struct request* request;

    struct mmc_req_entity req_entity;
    struct scatterlist sg;
#ifdef LOSCFG_DRIVERS_MMC_SPEEDUP
    mmc_event* p_thread_event;  /* not a good idea*/
    int wr_pos;
#endif
};

struct mmc_req_queue {
    struct mmc_card *card;

    mmc_thread thread;
    struct request_queue* queue;
    unsigned int thread_state;
#define THREAD_SHOULD_STOP  (1<<0)
#define THREAD_STATE_SUSPEND    (1<<1)
#define set_thread_runing(state) (state &= ~(THREAD_STATE_SUSPEND))
#define set_thread_suspend(state) (state |= (THREAD_STATE_SUSPEND))
    mmc_event thread_event;
#define THREAD_STOPPED  (1<<1)
#define WAIT_THREAD_STOP_TIMEOUT (LOSCFG_BASE_CORE_TICK_PER_SECOND * 10)  // 10s
#define THREAD_RUNNING  (1<<2)
#define THREAD_RETIREVE_REQ (1<<3)
#define WAIT_THREAD_RUNING_TIMEOUT (LOSCFG_BASE_CORE_TICK_PER_SECOND)     // 1s

#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
    struct mmc_req mreq;
#else
    #define MMC_MAX_REQS_SEND_ONCE  (32)
    struct mmc_req mreq[MMC_MAX_REQS_SEND_ONCE];
    unsigned int used;
    unsigned int start;
    unsigned int end;
    unsigned int capacity;
#endif
};

struct mmc_req_data {
    struct mmc_card * card;

    mmc_mutex mmc_data_queuelock;
    struct mmc_req_queue mmc_data_queue;
};

struct request_queue {
   struct list_head queuehead;
   mmc_mutex *p_mux_queue_lock;
   unsigned int request_cnt;
   unsigned int state;
#define BLK_QUEUE_NORMAL (1 << 0)
#define IS_BLK_QUEUE_NORMAL(blk)    (((blk)->state) & BLK_QUEUE_NORMAL)
#define SET_BLK_QUEUE_NORMAL(blk)   (((blk)->state) |= BLK_QUEUE_NORMAL)
#define CLR_BLK_QUEUE_NORMAL(blk)   (((blk)->state) &= ~(BLK_QUEUE_NORMAL))
};

struct request {
    struct list_head queuelist;
    struct request_queue *q;

    unsigned int req_flags;

    sector_t sector_s;
    unsigned int sector_sz;
    unsigned char *buffer;  //FIXME

    int error;
    unsigned int timeout;

    unsigned int retries;
#define REQUEST_RETRIES_TIMES 3

    EVENT_CB_S wait_event;
#define REQUEST_EVENT_ANSWER (1<<0)
};

#define BLOCK_NODE_NAMESZ 32
struct mmc_block {
    int node_no;
    char  node_name[BLOCK_NODE_NAMESZ];
    int error_count;                        //collect error count
    unsigned long long blk_capacity;

    struct block_operations bops;

    mmc_mutex queue_lock;
    struct request_queue queue;
    mmc_atomic_t cur_reqs; /* current requests in block */
    int block_status;
#define BLOCK_NORMAL    (0)
#define BLOCK_ABNORMAL  (-1)

    void* dev;
    void* data;
};

extern unsigned int emmc_raw_write(char * buffer, unsigned int start_sector, unsigned int nsectors);
extern unsigned int emmc_raw_read(char * buffer, unsigned int start_sector, unsigned int nsectors);
extern unsigned int mmc_direct_write(unsigned int host_idx, char * buffer, unsigned int start_sector, unsigned int nsectors);
extern unsigned int mmc_direct_read(unsigned int host_idx, char * buffer, unsigned int start_sector, unsigned int nsectors);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
