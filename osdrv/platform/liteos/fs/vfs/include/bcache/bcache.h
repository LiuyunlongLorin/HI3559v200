#ifndef VFSONCE_BCACHE_H
#define VFSONCE_BCACHE_H

#include "linux/rbtree.h"
#include "los_list.h"
#include "pthread.h"

#ifdef LOSCFG_SHELL
#include "reset_shell.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#define VFSIMP_BLOCK_LIB_STATISTICS
#define ALIGN_LIB(_x_) (((_x_) + (HALARC_ALIGNMENT-1)) & ~(HALARC_ALIGNMENT-1))
#define ALIGN_DISP(_x_) (HALARC_ALIGNMENT - (_x_ & (HALARC_ALIGNMENT - 1)))
#define BLIB_PREREAD_PRIO    12

#ifdef VFSIMP_BLOCK_LIB_STATISTICS
#define STAT_INIT(_bc_) do { \
        _bc_->stat.n_gets = _bc_->stat.n_shoots = _bc_->stat.n_reads = _bc_->stat.n_writes = 0;  \
    } while (0)
#define STAT(_bc_, _group_) ((_bc_)->stat._group_++)
#else // VFSIMP_BLOCK_LIB_STATISTICS
#define STAT_INIT(_bc_)
#define STAT(_bc_, _group_)
#endif // not VFSIMP_BLOCK_LIB_STATISTICS

typedef int (*block_cache_bread_fn) (
    void*,         // private data
    void*,         // block buffer
    unsigned int*, // number of blocks to read
    unsigned long long   // starting block number
);

typedef int (*block_cache_bwrite_fn) (
    void*,         // private data
    const void*,   // block buffer
    unsigned int*, // number of blocks to write
    unsigned long long   // starting block number
);

typedef void (*block_cache_preread_fn) (
    void*,    // block cache instance space holder
    void*    // block data
);

typedef struct {
    unsigned int  n_gets;     // number of block gets
    unsigned int  n_reads;    // number of block reads
    unsigned int  n_writes;   // number of block writes
    unsigned int  n_shoots;   // number of cache shoot
} block_cache_stat_t;

typedef struct {
    void                    *priv;              // private data
    LOS_DL_LIST             list_head;          // head of block list
    struct rb_root          rb_root;            // block red-black tree root
    unsigned int            block_size;         // block size in bytes
    unsigned int            block_size_log2;    // block size log2
    unsigned int            block_count;        // block count of the disk
    unsigned int            sector_size;        // device sector size in bytes
    unsigned int            sector_per_block;   // device sector size in bytes
    unsigned char           *mem_base;          // memory base
    unsigned int            mem_size;           // memory size
    unsigned int            preread_task_id;    // preread task id
    unsigned int            bnum_now;           // current preread block number
    LOS_DL_LIST             free_list_head;     // list of free blocks
    block_cache_bread_fn       bread_fn;           // block read function
    block_cache_bwrite_fn      bwrite_fn;          // block write function
    block_cache_preread_fn    preread_fn;        //block preread function
#ifdef VFSIMP_BLOCK_LIB_STATISTICS
    block_cache_stat_t         stat;               // statistics
#endif
    unsigned char           *rw_buffer;         //buffer for blib block
    pthread_mutex_t         bcache_mutex;         //mutex for blib
    EVENT_CB_S             bcache_event;           // event for blib
} los_bcache_t;


typedef void * los_io_handle_t;

typedef struct {
    unsigned int    sector_size;          // Sector size in bytes
    unsigned int    sector_size_log2;     // Sector size log2
    los_io_handle_t dev_t;
    unsigned char  *bcache_mem;
    los_bcache_t      bcache;

}los_bdisk;



/**********************************************************************
* Creates a block lib instance on top of IO system
*   (los_io_bread and los_io_bwrite)
*
*   handle     - los_io_handle_t
*   mem_base   - block cache memory base
*   mem_size   - block cache memory size
*   block_size - block size
*   bc         - block cache instance space holder
*
*   returns ENOERR if create succeded
************************************************************************/
int block_cache_io_create(los_io_handle_t     handle,
                               void               *mem_base,
                               unsigned int        mem_size,
                               unsigned int        block_size,
                               los_bcache_t         *bc);


/**********************************************************************
* Creates a block lib instance
*
*   priv_data  - private data to pass to bread_fn and bwrite_fn
*   mem_base   - block cache memory base
*   mem_size   - block cache memory size
*   block_size - block size
*   bread_fn   - function which reads blocks
*   bwrite_fn  - function which writes blocks
*   bc         - block cache instance space holder
*
*   returns ENOERR if create succeded
************************************************************************/
int block_cache_create(void               *priv_data,
                          void                   *mem_base,
                          unsigned int        mem_size,
                          unsigned int        block_size,
                          block_cache_bread_fn   bread_fn,
                          block_cache_bwrite_fn  bwrite_fn,
                          los_bcache_t         *bc);


/**********************************************************************
* Deletes a block cache instance
*
*   bc - block cache instance
*
*   The block cache is synced before
*
*   returns ENOERR if delete succeded
************************************************************************/
int block_cache_delete(los_bcache_t *bc);

/**********************************************************************
* Reads data
*
*   bc   - block lib instance
*   buf  - data buffer ptr
*   len  - number of bytes to read
*   bnum - starting block number
*   pos  - starting position inside starting block
*
*   returns ENOERR if read succeded
*
*   The block number is automatically adjusted if
*   position is greater than block size
************************************************************************/
int block_cache_read(los_bcache_t   *bc,
                        void         *buf,
                        unsigned int *len,
                        unsigned int  bnum,
                        unsigned long long  pos);

/**********************************************************************
* Writes data
*
*   bl   - block lib instance
*   buf  - data buffer ptr
*   len  - number of bytes to write
*   bnum - starting block number
*   pos  - starting position inside starting block
*
*   returns ENOERR if write succeded
*
*   The block number is automatically adjusted if
*   position is greater than block size
************************************************************************/
int block_cache_write(los_bcache_t   *bc,
                         const void   *buf,
                         unsigned int *len,
                         unsigned int  bnum,
                         unsigned long long  pos);



/**********************************************************************
* Syncs block cache - ie write modified blocks
*
*   bc - block lib instance
*
*   returns ENOERR if sync succeded
************************************************************************/
int block_cache_sync(los_bcache_t *bc);

/**********************************************************************
* Syncs block - ie write if modified
*
*   bc  - block lib instance
*   num - block number to sync
*
*   returns ENOERR if sync succeded
************************************************************************/
int block_cache_sync_block(los_bcache_t *bc, unsigned int num);

/**********************************************************************
* Gets block size
*
*   bc  - block cache instance
*
*   returns the current block size
************************************************************************/
static inline unsigned int block_cache_get_block_size(los_bcache_t *bc)
{
    return bc->block_size;
}


/**********************************************************************
* Gets log2 of block size
*
*   bc  - block lib instance
*
*   returns log2 of the current block size
************************************************************************/
static inline unsigned int block_cache_get_block_size_log2(los_bcache_t *bc)
{
    return bc->block_size_log2;
}

/**********************************************************************
* Gets block cache statistics
*
*   bc - block lib instance
*
*   returns ENOERR if get succeded
************************************************************************/
los_bcache_t *block_cache_init(void *dev_node,
                                unsigned int sector_size,
                                unsigned int sector_per_block,
                                unsigned int block_num,
                                unsigned int block_count);

void block_cache_deinit(los_bcache_t *bc);

unsigned int bcache_async_preread_init(los_bcache_t  *bc);
void resume_async_preread(void *arg1, void *arg2);
unsigned int bcache_async_preread_deinit(los_bcache_t *bc);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif // VFSONCE_BCACHE_H

// --------------------------------------------------------------------
// EOF bcache.h
