#ifndef _RAMFS_H_
#define _RAMFS_H_

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef LOSCFG_FS_RAMFS

extern off_t g_ramfs_alloc_unit;

#define RAMFSPKG_FS_RAM_SIMPLE

#ifdef LOSCFG_ARCH_CORTEX_A53_AARCH64
#define RAMFSNUM_RAMFS_DIRENT_SIZE          40
#else
#define RAMFSNUM_RAMFS_DIRENT_SIZE          32
#endif
#define RAMFSNUM_RAMFS_REALLOC_INCREMENT    (RAMFSNUM_RAMFS_DIRENT_SIZE * 2)

#define RAMFSPKG_FS_RAM_RET_DIRENT_DTYPE

#ifndef RAMFSPKG_FS_RAM_SIMPLE

#define RAMFSPKG_FS_RAM_BLOCKS_ARRAY

#define RAMFSPKG_FS_RAM_BLOCKS_ARRAY_EXTERN

#define RAMFSNUM_FS_RAM_BLOCKS_ARRAY_SIZE   128
#define RAMFSNUM_RAMFS_BLOCK_SIZE           64

#define RAMFSNUM_RAMFS_BLOCKS_DIRECT        1
#define RAMFSNUM_RAMFS_BLOCKS_INDIRECT1     1
#define RAMFSNUM_RAMFS_BLOCKS_INDIRECT2     1

#endif

/** Timestamp support
Provides the current time as a time_t timestamp for use in filesystem
data strucures. */
time_t ramfs_timestamp(void);

/** Data typedefs */
/** Some forward typedefs for the main data structures. */

struct ramfs_node;
typedef struct ramfs_node ramfs_node;

struct ramfs_dirent;
typedef struct ramfs_dirent ramfs_dirent;

#ifndef RAMFSPKG_FS_RAM_SIMPLE

typedef UINT8 ramfs_block[RAMFSNUM_RAMFS_BLOCK_SIZE];

#endif

/** Sizes derived from configuration */

/** Simple malloc based allocator parameters */

#ifdef RAMFSPKG_FS_RAM_SIMPLE

#define RAMFS_FILESIZE_MAX      UINT_MAX

#else

/** The maximum size of a file */
#define RAMFS_FILESIZE_MAX      RAMFS_INDIRECT2_MAX

/** The maximum data offset for data accessed from the two level indirect blocks */
#define RAMFS_INDIRECT2_MAX     (RAMFS_INDIRECT1_MAX+                   \
                                 (RAMFSNUM_RAMFS_BLOCKS_INDIRECT2*        \
                                  RAMFS_INDIRECT2_BLOCK_EXTENT))

/** The maximum data offset for data accessed from the single level indirect blocks */
#define RAMFS_INDIRECT1_MAX     (RAMFS_DIRECT_MAX+                      \
                                 (RAMFSNUM_RAMFS_BLOCKS_INDIRECT1*        \
                                  RAMFS_INDIRECT1_BLOCK_EXTENT))

/** The maximum data offset for data directly accessed from the node */
#define RAMFS_DIRECT_MAX        (RAMFSNUM_RAMFS_BLOCKS_DIRECT*RAMFSNUM_RAMFS_BLOCK_SIZE)

/** number of bytes contained in a two level indirect block */
#define RAMFS_INDIRECT2_BLOCK_EXTENT (RAMFS_INDIRECT_PER_BLOCK* \
                                      RAMFS_INDIRECT_PER_BLOCK* \
                                      RAMFSNUM_RAMFS_BLOCK_SIZE)

/** Number of bytes contained in a one level indirect block */
#define RAMFS_INDIRECT1_BLOCK_EXTENT (RAMFS_INDIRECT_PER_BLOCK* \
                                      RAMFSNUM_RAMFS_BLOCK_SIZE)

/** The number of directory entries that can be stored in a single data block */
#define RAMFS_DIRENT_PER_BLOCK  (RAMFSNUM_RAMFS_BLOCK_SIZE/sizeof(ramfs_dirent))

/** The number of indirect pointers that can be stored in a single data block */
#define RAMFS_INDIRECT_PER_BLOCK (RAMFSNUM_RAMFS_BLOCK_SIZE/sizeof(ramfs_block *))

/** The number of nodes per block */
#define RAMFS_NODES_PER_BLOCK (RAMFSNUM_RAMFS_BLOCK_SIZE/sizeof(ramfs_node))


#endif

/** forward definitions */

struct ramfs_mtab_entry;
typedef struct ramfs_mtab_entry ramfs_mtab_entry;

struct  ramfs_fstab_entry;
typedef struct  ramfs_fstab_entry  ramfs_fstab_entry;

struct RAMFS_FILEOPS_TAG;
typedef struct RAMFS_FILEOPS_TAG ramfs_fileops_tag;

struct RAMFS_FILE_TAG;
typedef struct RAMFS_FILE_TAG ramfs_file;

struct RAMFS_IOVEC_TAG;
typedef struct RAMFS_IOVEC_TAG ramfs_iovec;

struct RAMFS_UIO_TAG;
typedef struct RAMFS_UIO_TAG ramfs_uio;

struct RAMFS_SELINFO_TAG;
typedef struct RAMFS_SELINFO_TAG ramfs_selinfo;


/** File and directory node */
/** This data structure represents a file or directory. */

struct ramfs_node
{
    time_t              ctime;          /** last changed status time */
    time_t              mtime;          /** last modified time */
    time_t              atime;          /** last access time */
    size_t              size;           /** size of file in bytes */
    nlink_t             nlink;          /** number of links to this node */
    UINT32              refcnt;         /** open file/current dir references */
    mode_t              mode;           /** node type */

#ifdef RAMFSPKG_FS_RAM_SIMPLE

    /** The data storage in this case consists of a single
        malloced memory block, together with its size.*/

    UINT8               *data;          /** malloced data buffer */
    size_t              datasize;       /** size of data block */

#else

    /** The data storage in this case consists of arrays of pointers
        to data blocks. */

#if  RAMFSNUM_RAMFS_BLOCKS_INDIRECT2 > 0
    /** Two level indirection */
    ramfs_block         ***indirect2[RAMFSNUM_RAMFS_BLOCKS_INDIRECT2];
#endif

#if  RAMFSNUM_RAMFS_BLOCKS_INDIRECT1 > 0
    /** Single level indirection */
    ramfs_block         **indirect1[RAMFSNUM_RAMFS_BLOCKS_INDIRECT1];
#endif

#if RAMFSNUM_RAMFS_BLOCKS_DIRECT > 0
    /** Directly accessible blocks from the inode. */
    ramfs_block         *direct[RAMFSNUM_RAMFS_BLOCKS_DIRECT];
#endif

#endif

};

/** Directory search data */
/** Parameters for a directory search. The fields of this structure are
updated as we follow a pathname through the directory tree. */

struct ramfs_dirsearch
{
    const char          *name;          /** last name fragment used */
    int                 namelen;        /** name fragment length */
    ramfs_node          *node;          /** Node found */
    const char          *path;          /** path to follow */
    ramfs_node          *dir;           /** directory to search */
    BOOL                last;           /** last name in path */
};

/** Directory entry. */
/** Fixed sized entry containing a fragment of the name of a file/directory. */

struct ramfs_dirent
{
#define RAMFS_NAME_MAX      RAMFSNUM_RAMFS_DIRENT_SIZE - \
                            sizeof(ramfs_node *) - \
                            sizeof(UINT32) - \
                            sizeof(off_t)

    /** Name fragment, fills rest of entry. */
char                    name[RAMFS_NAME_MAX];
unsigned int            inuse:1,        /** entry in use */
                        first:1,        /** first directory entry fragment */
                        last:1,         /** last directory entry fragment */
                        namelen:8,      /** bytes in whole name */
                        fraglen:8;      /** bytes in name fragment */
off_t                   next;           /** offset of next dirent */
ramfs_node              *node;          /** pointer to node */

};

typedef struct ramfs_dirsearch ramfs_dirsearch;

static int del_direntry( ramfs_node *dir, const char *name, int len );

BOOL ramfs_directory_empty(ramfs_node *dir);
int ramfs_mount    ( ramfs_fstab_entry *fste, ramfs_mtab_entry *mte );
int ramfs_umount   ( ramfs_mtab_entry *mte );
int ramfs_open     ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name,
                             int mode,  ramfs_file *fte );
int ramfs_unlink   ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name );
int ramfs_mkdir    ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name );
int ramfs_rmdir    ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name );
int ramfs_rename   ( ramfs_mtab_entry *mte, AARCHPTR dir1, const char *name1,
                             AARCHPTR dir2, const char *name2 );
static int ramfs_link     ( ramfs_mtab_entry *mte, AARCHPTR dir1, const char *name1,
                             AARCHPTR dir2, const char *name2, int type );
int ramfs_opendir  ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name,
                             ramfs_file *fte );
static int ramfs_chdir    ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name,
                             AARCHPTR *dir_out );
int ramfs_stat     ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name,
                             struct stat *stat_buf);
static int ramfs_getinfo  ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name,
                             int key, void *buf, int len );
static int ramfs_setinfo  ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name,
                             int key, void *buf, int len );

/** File operations */
int ramfs_fo_read      (struct RAMFS_FILE_TAG *fp, struct RAMFS_UIO_TAG *uio);
int ramfs_fo_write     (struct RAMFS_FILE_TAG *fp, struct RAMFS_UIO_TAG *uio);
int ramfs_fo_lseek     (struct RAMFS_FILE_TAG *fp, off_t *pos, int whence );
static int ramfs_fo_ioctl     (struct RAMFS_FILE_TAG *fp, UINT32 com,
                                UINT32 data);
int ramfs_fo_fsync     (struct RAMFS_FILE_TAG *fp, int mode );
int ramfs_fo_close     (struct RAMFS_FILE_TAG *fp);
int ramfs_fo_fstat     (struct RAMFS_FILE_TAG *fp, struct stat *stat_buf );
static int ramfs_fo_getinfo   (struct RAMFS_FILE_TAG *fp, int key, void *buf, int len );
static int ramfs_fo_setinfo   (struct RAMFS_FILE_TAG *fp, int key, void *buf, int len );

/** Directory operations */
int ramfs_fo_dirread      (struct RAMFS_FILE_TAG *fp, struct RAMFS_UIO_TAG *uio);
int ramfs_fo_dirlseek     (struct RAMFS_FILE_TAG *fp, off_t *pos, int whence );


/** Filesystem table entry */

typedef int     ramfs_fsop_mount    ( ramfs_fstab_entry *fste, ramfs_mtab_entry *mte );
typedef int     ramfs_fsop_umount   ( ramfs_mtab_entry *mte );
typedef int     ramfs_fsop_open     ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name,
                                    int mode,  ramfs_file *fte );
typedef int     ramfs_fsop_unlink   ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name );
typedef int     ramfs_fsop_mkdir    ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name );
typedef int     ramfs_fsop_rmdir    ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name );
typedef int     ramfs_fsop_rename   ( ramfs_mtab_entry *mte, AARCHPTR dir1, const char *name1,
                                    AARCHPTR dir2, const char *name2 );
typedef int     ramfs_fsop_link     ( ramfs_mtab_entry *mte, AARCHPTR dir1, const char *name1,
                                    AARCHPTR dir2, const char *name2, int type );
typedef int     ramfs_fsop_opendir  ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name,
                                    ramfs_file *fte );
typedef int     ramfs_fsop_chdir    ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name,
                                    AARCHPTR *dir_out );
typedef int     ramfs_fsop_stat     ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name,
                                    struct stat *buf);
typedef int     ramfs_fsop_getinfo  ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name,
                                    int key, void *buf, int len );
typedef int     ramfs_fsop_setinfo  ( ramfs_mtab_entry *mte, AARCHPTR dir, const char *name,
                                    int key, void *buf, int len );


struct ramfs_fstab_entry
{
    const char            *name;          /** filesystem name */
    ramfs_fsop_setinfo    *setinfo;
    ramfs_fsop_getinfo    *getinfo;
    ramfs_fsop_stat       *stat;
    ramfs_fsop_chdir      *chdir;
    ramfs_fsop_opendir    *opendir;
    ramfs_fsop_link       *link;
    ramfs_fsop_rename     *rename;
    ramfs_fsop_rmdir      *rmdir;
    ramfs_fsop_mkdir      *mkdir;
    ramfs_fsop_unlink     *unlink;
    ramfs_fsop_open       *open;
    ramfs_fsop_umount     *umount;
    ramfs_fsop_mount      *mount;
    UINT32                syncmode;       /** synchronization mode */
    UINT32                data;           /** private data value */
} RAMFS_HAL_TABLE_TYPE;

/** Keys for getinfo() and setinfo() */

#define FS_INFO_CHMOD           7       /* chmod() */
#ifdef  RAMFSSEM_FILEIO_INFO_DISK_USAGE
#define FS_INFO_DISK_USAGE      6       /* get_disk_usage()    */
#endif
#define FS_INFO_ATTRIB          5       /* ramfs_fs_(get|set)_attrib() */
#define FS_INFO_SYNC            4       /* ramfs_fs_fssync() */
#define FS_INFO_GETCWD          3       /* getcwd() */
#define FS_INFO_ACCESS          2       /* access() */
#define FS_INFO_CONF            1       /* pathconf() */


/** Types for link() */

#define RAMFS_FSLINK_SOFT         2       /* form a soft link */
#define RAMFS_FSLINK_HARD         1       /* form a hard link */


/** getinfo() and setinfo() buffers structures. */

struct ramfs_getcwd_info
{
    size_t      size;           /* size of buffer */
    char        *buf;           /* buffer for cwd string */
};

struct ramfs_fs_disk_usage
{
    UINT64 total_blocks;
    UINT64 free_blocks;
    UINT32 block_size;
};

/** Mount table entry */

struct ramfs_mtab_entry
{
    const char          *name;          /** name of mount point */
    const char          *fsname;        /** name of implementing filesystem */
    const char          *devname;       /** name of hardware device */
    UINT32              data;           /** private data value */

    /** The following are filled in after a successful mount operation */
    ramfs_fstab_entry   *fs;            /** pointer to fstab entry */
    AARCHPTR            root;           /** root directory pointer */
    BOOL                valid;          /** Valid entry */
};

/** IO vector descriptors */

struct RAMFS_IOVEC_TAG
{
    ssize_t        iov_len;             /* Length. */
    void           *iov_base;           /* Base address. */
};

enum    ramfs_uio_rw { UIO_READ, UIO_WRITE };

/* Segment flag values. */
enum ramfs_uio_seg
{
    UIO_SYSSPACE,               /* from system space */
    UIO_USERSPACE               /* from user data space */
};

struct RAMFS_UIO_TAG
{
    enum ramfs_uio_rw       uio_rw;        /* see above */
    enum ramfs_uio_seg      uio_segflg;    /* see above */
    ssize_t                 uio_resid; /* residual i/o count */
    off_t                   uio_offset;    /* offset into file this uio corresponds to */
    INT32                   uio_iovcnt;    /* number of iovecs in array */
    struct RAMFS_IOVEC_TAG *uio_iov;    /* pointer to array of iovecs */
};

// Limits
#define UIO_SMALLIOV    8       /* 8 on stack, else malloc */

/** Description of open file */

typedef int ramfs_fileop_readwrite (struct RAMFS_FILE_TAG *fp, struct RAMFS_UIO_TAG *uio);
typedef ramfs_fileop_readwrite ramfs_fileop_read;
typedef ramfs_fileop_readwrite ramfs_fileop_write;
typedef int ramfs_fileop_lseek   (struct RAMFS_FILE_TAG *fp, off_t *pos, int whence );
typedef int ramfs_fileop_ioctl   (struct RAMFS_FILE_TAG *fp, UINT32 com,
                                UINT32 data);
typedef BOOL ramfs_fileop_select  (struct RAMFS_FILE_TAG *fp, int which, UINT32 info);
typedef int ramfs_fileop_fsync   (struct RAMFS_FILE_TAG *fp, int mode );
typedef int ramfs_fileop_close   (struct RAMFS_FILE_TAG *fp);
typedef int ramfs_fileop_fstat   (struct RAMFS_FILE_TAG *fp, struct stat *buf );
typedef int ramfs_fileop_getinfo (struct RAMFS_FILE_TAG *fp, int key, void *buf, int len );
typedef int ramfs_fileop_setinfo (struct RAMFS_FILE_TAG *fp, int key, void *buf, int len );

struct RAMFS_FILEOPS_TAG
{
    ramfs_fileop_setinfo  *fo_setinfo;
    ramfs_fileop_getinfo  *fo_getinfo;
    ramfs_fileop_fstat    *fo_fstat;
    ramfs_fileop_close    *fo_close;
    ramfs_fileop_fsync    *fo_fsync;
    ramfs_fileop_select   *fo_select;
    ramfs_fileop_ioctl    *fo_ioctl;
    ramfs_fileop_lseek    *fo_lseek;
    ramfs_fileop_write    *fo_write;
    ramfs_fileop_read     *fo_read;
};

struct RAMFS_FILE_TAG
{
    ramfs_mtab_entry         *f_mte;         /* mount table entry            */
    UINT32                    f_xops;        /* extra type specific ops      */
    AARCHPTR                  f_data;        /* file or socket               */
    off_t                     f_offset;      /* current offset               */
    struct RAMFS_FILEOPS_TAG *f_ops;         /* file operations              */
    UINT32                    f_syncmode;    /* synchronization protocol     */
    UINT16                    f_type;        /* descriptor type              */
    UINT16                    f_ucount;      /* use count                    */
    UINT32                    f_flag;        /* file state                   */
};

/** File flags */

/** Allocation here is that bits 0..15 are copies of bits from the open
    flags, bits 16..23 are extra bits that are visible to filesystems but
    are not derived from the open call, and bits 24..31 are reserved for
    the fileio infrastructure. */

#ifdef LOSCFG_NET_LWIP_SACK
#define RAMFS_FNONBLOCK   O_NONBLOCK
#else
#define RAMFS_FNONBLOCK   0x00000004
#endif
#define RAMFS_FWRITE      O_WRONLY
#define RAMFS_FREAD       O_RDONLY
#define RAMFS_FAPPEND     O_APPEND
#define RAMFS_FDIR        0x00020000
#define RAMFS_FASYNC      0x00010000

#define RAMFS_FALLOC      0x80000000  /** File is "busy", i.e. allocated */
#define RAMFS_FLOCK       0x02000000  /** Lock during file ops */
#define RAMFS_FLOCKED     0x01000000  /** Set if file is locked */

/** Mask for open mode bits stored in file object */
#define RAMFS_FILE_MODE_MASK (RAMFS_FREAD|RAMFS_FWRITE|RAMFS_FNONBLOCK|RAMFS_FAPPEND)

/** Synchronization modes */
/** These values are filled into the syncmode fields of the above structures
    and define the synchronization protocol used when accessing the object in
    question. */

#define RAMFS_SYNCMODE_SOCK_SHIFT         (8)     /** shift for sock to file bits */
#define RAMFS_SYNCMODE_IO_SHIFT           (4)     /** shift for IO to file bits */
#define RAMFS_SYNCMODE_SOCK_NETSTACK      0x0800  /** lock netstack during socket ops */
#define RAMFS_SYNCMODE_SOCK_FILE          0x0100  /** lock socket during socket ops */
#define RAMFS_SYNCMODE_IO_MOUNTPOINT      0x0040  /** lock mte during io ops */
#define RAMFS_SYNCMODE_IO_FILESYSTEM      0x0020  /** lock fs during io ops */
#define RAMFS_SYNCMODE_IO_FILE             0x0010  /** lock file during io ops */
#define RAMFS_SYNCMODE_FILE_MOUNTPOINT    0x0004  /** lock mte during file ops */
#define RAMFS_SYNCMODE_FILE_FILESYSTEM    0x0002  /** lock fs during file ops */
#define RAMFS_SYNCMODE_NONE               (0)      /** no locking required */


/** Get/set info buffer structures */

/** This is used for pathconf() and fpathconf() */
struct ramfs_pathconf_info
{
    int         name;           /** POSIX defined variable name */
    long        value;          /** Returned variable value */
};

/** Modes for fsync() */

#define RAMFS_FDATASYNC          2
#define RAMFS_FSYNC              1

/** Keys for getinfo() and setinfo() */

#define FILE_INFO_CONF          1       /* fpathconf() */

/** Type of file */

#define RAMFS_FILE_TYPE_DEVICE  3   /* device */
#define RAMFS_FILE_TYPE_SOCKET  2   /* communications endpoint */
#define RAMFS_FILE_TYPE_FILE    1   /* file */

int ramfs_fs_fssync(const char *path);

int ramfs_fs_set_attrib( const char *fname, const UINT32 new_attrib );
int ramfs_fs_get_attrib( const char *fname, UINT32 * const file_attrib );
void ramfs_fs_lock( ramfs_mtab_entry *mte, UINT32 syncmode );

void ramfs_fs_unlock( ramfs_mtab_entry *mte, UINT32 syncmode );

ramfs_mtab_entry * ramfs_fs_root_lookup( UINT32 *root );

int ramfs_fs_getinfo( const char *path, int key, void *buf, int len );
int ramfs_fs_setinfo( const char *path, int key, void *buf, int len );
int ramfs_fs_fgetinfo( int fd, int key, void *buf, int len );
int ramfs_fs_fsetinfo( int fd, int key, void *buf, int len );

int ramfs_fileio_enotdir(void);
int ramfs_fileio_enoerr(void);
int ramfs_fileio_erofs(void);
int ramfs_fileio_enosys(void);

ramfs_fileop_select ramfs_fileio_seltrue;
#ifdef RAMFSFUN_IO_FILEIO_SELECT

struct RAMFS_SELINFO_TAG
{
    UINT32        si_info;                /** info passed through from fo_select() */
    ramfs_flag_value_t    si_waitFlag;    /** select wait flags */
};

void ramfs_selinit( struct RAMFS_SELINFO_TAG *sip );
void ramfs_selrecord( UINT32 info, struct RAMFS_SELINFO_TAG *sip );
void ramfs_selwakeup( struct RAMFS_SELINFO_TAG *sip );
#endif

#endif /* CONFIG_FS_RAMFS */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* ifndef _RAMFS_H_ */
