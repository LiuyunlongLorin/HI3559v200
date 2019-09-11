#ifndef JFFS2ONCE_FILEIO_H
#define JFFS2ONCE_FILEIO_H
//=============================================================================
//
//      fileio.h
//
//      Fileio header
//
//=============================================================================
//*   Copyright (c) <2014-2017>, <Huawei Technologies Co., Ltd>
//*   All rights reserved.
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later
// version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with eCos; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// As a special exception, if other files instantiate templates or use
// macros or inline functions from this file, or you compile this file
// and link it with other works to produce a work based on this file,
// this file does not by itself cause the resulting work to be covered by
// the GNU General Public License. However the source code for this file
// must still be made available in accordance with section (3) of the GNU
// General Public License v2.
//
// This exception does not invalidate any other reasons why a work based
// on this file might be covered by the GNU General Public License.
// -------------------------------------------
// ####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     nickg
// Contributors:  nickg
// Date:          2000-05-25
// Purpose:       Fileio header
// Description:   This header contains the external definitions of the general file
//                IO subsystem for POSIX and EL/IX compatability.
//
// Usage:
//              #include <fileio.h>
//              ...
//
//
//####DESCRIPTIONEND####
/****************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations,
 * which might include those applicable to Huawei LiteOS of U.S. and the country in
 * which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in
 * compliance with such applicable export control laws and regulations.
 ****************************************************************************/
//=============================================================================

#include "jffs2_type.h"
#include <stddef.h>             // NULL, size_t
#include <limits.h>
#include <sys/types.h>
#include "fcntl.h"
#include <zconf.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

//sdd
typedef unsigned int jffs2_bool;
//=============================================================================
// forward definitions

struct jffs2_mtab_entry;
typedef struct jffs2_mtab_entry jffs2_mtab_entry;

struct  jffs2_fstab_entry;
typedef struct  jffs2_fstab_entry  jffs2_fstab_entry;

struct JFFS2_FILEOPS_TAG;
typedef struct JFFS2_FILEOPS_TAG jffs2_s_fileops;

struct JFFS2_FILE_TAG;
typedef struct JFFS2_FILE_TAG jffs2_sfile;

struct JFFS2_IOVEC_TAG;
typedef struct JFFS2_IOVEC_TAG jffs2_iovec;

struct JFFS2_UIO_TAG;
typedef struct JFFS2_UIO_TAG jffs2_uio;

struct JFFS2_SELINFO_TAG;
typedef struct JFFS2_SELINFO_TAG jffs2_selinfo;

//=============================================================================
// Directory pointer

typedef JFFS2_ADDRWORD jffs2_dir;

#define JFFS2_DIR_NULL 0

//=============================================================================
// Filesystem table entry

struct jffs2_stat;
typedef int     jffs2_fsop_mount    ( jffs2_fstab_entry *fste, jffs2_mtab_entry *mte ,int parttiton_num);
typedef int     jffs2_fsop_umount   ( jffs2_mtab_entry *mte );
typedef int     jffs2_fsop_open     ( jffs2_mtab_entry *mte, jffs2_dir dir, const char *name,
                                    int jmode, int mode, jffs2_sfile *fte );
typedef int     jffs2_fsop_unlink   ( jffs2_mtab_entry *mte, jffs2_dir dir, const char *name );
typedef int     jffs2_fsop_mkdir    (  jffs2_mtab_entry *mte,jffs2_dir dir, const char *name ,int mode);
typedef int     jffs2_fsop_rmdir    ( jffs2_mtab_entry *mte, jffs2_dir dir, const char *name );
typedef int     jffs2_fsop_rename   ( jffs2_mtab_entry *mte, jffs2_dir dir1, const char *name1,
                                    jffs2_dir dir2, const char *name2 );
typedef int     jffs2_fsop_link     ( jffs2_mtab_entry *mte, jffs2_dir dir1, const char *name1,
                                    jffs2_dir dir2, const char *name2, int type );
typedef int     jffs2_fsop_opendir  ( jffs2_mtab_entry *mte, jffs2_dir dir, const char *name,
                                    jffs2_sfile *fte );
typedef int     jffs2_fsop_chdir    ( jffs2_mtab_entry *mte, jffs2_dir dir, const char *name,
                                    jffs2_dir *dir_out );
typedef int     jffs2_fsop_stat     ( jffs2_mtab_entry *mte, jffs2_dir dir, const char *name,
                                    struct jffs2_stat *buf);
typedef int     jffs2_fsop_getinfo  ( jffs2_mtab_entry *mte, jffs2_dir dir, const char *name,
                                    int key, void *buf, int len );
typedef int     jffs2_fsop_setinfo  ( jffs2_mtab_entry *mte, jffs2_dir dir, const char *name,
                                    int key, void *buf, int len );


struct jffs2_fstab_entry
{
    const char          *name;          // filesystem name
    JFFS2_ADDRWORD        data;           // private data value
//#ifdef JFFS2OPT_FS_JFFS2_GCTHREAD
//    struct rt_mutex     syncmode;       // synchronization mode
//#endif
    jffs2_uint32          syncmode;       // synchronization mode
    jffs2_fsop_mount      *mount;
    jffs2_fsop_umount     *umount;
    jffs2_fsop_open       *open;
    jffs2_fsop_unlink     *unlink;
    jffs2_fsop_mkdir      *mkdir;
    jffs2_fsop_rmdir      *rmdir;
    jffs2_fsop_rename     *rename;
    jffs2_fsop_link       *link;
    jffs2_fsop_opendir    *opendir;
    jffs2_fsop_chdir      *chdir;
    jffs2_fsop_stat       *stat;
    jffs2_fsop_getinfo    *getinfo;
    jffs2_fsop_setinfo    *setinfo;
} ;// JFFS2_HAL_TABLE_TYPE;   //prife

//-----------------------------------------------------------------------------
// Keys for getinfo() and setinfo()

#define FS_INFO_CONF            1       /* pathconf() */
#define FS_INFO_ACCESS          2       /* access() */
#define FS_INFO_GETCWD          3       /* getcwd() */
#define FS_INFO_SYNC            4       /* jffs2_fs_fssync() */
#define FS_INFO_ATTRIB          5       /* jffs2_fs_(get|set)_attrib() */
#ifdef  JFFS2SEM_FILEIO_INFO_DISK_USAGE
#define FS_INFO_DISK_USAGE      6       /* get_disk_usage()    */
#endif
#define FS_INFO_CHMOD           7       /* chmod() */

//-----------------------------------------------------------------------------
// Types for link()

#define JFFS2_FSLINK_HARD         1       /* form a hard link */
#define JFFS2_FSLINK_SOFT         2       /* form a soft link */

//-----------------------------------------------------------------------------
// getinfo() and setinfo() buffers structures.

struct jffs2_getcwd_info
{
    char        *buf;           /* buffer for cwd string */
    size_t      size;           /* size of buffer */
};

struct jffs2_fs_disk_usage{
  jffs2_uint64 total_blocks;
  jffs2_uint64 free_blocks;
  jffs2_uint32 block_size;
};

typedef jffs2_uint32 jffs2_fs_attrib_t;

//-----------------------------------------------------------------------------
// Macro to define an initialized fstab entry

#define FSTAB_ENTRY( _l, _name, _data, _syncmode, _mount, _umount,      \
                     _open, _unlink,  _mkdir, _rmdir, _rename, _link,   \
                     _opendir, _chdir, _stat, _getinfo, _setinfo)       \
struct jffs2_fstab_entry _l /*JFFS2_HAL_TABLE_ENTRY(fstab) prife*/ =                  \
{                                                                       \
    _name,                                                              \
    _data,                                                              \
    _syncmode,                                                          \
    _mount,                                                             \
    _umount,                                                            \
    _open,                                                              \
    _unlink,                                                            \
    _mkdir,                                                             \
    _rmdir,                                                             \
    _rename,                                                            \
    _link,                                                              \
    _opendir,                                                           \
    _chdir,                                                             \
    _stat,                                                              \
    _getinfo,                                                           \
    _setinfo                                                            \
};

//=============================================================================
// Mount table entry

struct jffs2_mtab_entry
{
    const char          *name;          // name of mount point
    const char          *fsname;        // name of implementing filesystem
    const char          *devname;       // name of hardware device
    JFFS2_ADDRWORD        data;           // private data value

    // The following are filled in after a successful mount operation
    int            valid;          // Valid entry?
    jffs2_fstab_entry     *fs;            // pointer to fstab entry
    jffs2_dir             root;           // root directory pointer
} ; // JFFS2_HAL_TABLE_TYPE;  // prife


// This macro defines an initialized mtab entry

#define MTAB_ENTRY( _l, _name, _fsname, _devname, _data )       \
struct jffs2_mtab_entry _l /*JFFS2_HAL_TABLE_ENTRY(mtab)  prife */ =            \
{                                                               \
    (const char*)_name,                                                      \
    (const char*)_fsname,                                                    \
    (const char*)_devname,                                                   \
    (JFFS2_ADDRWORD)_data,                                                      \
    false,                                                      \
    (jffs2_fstab_entry*)NULL,                                                       \
    (jffs2_dir)JFFS2_DIR_NULL                                                \
};

//=============================================================================
// IO vector descriptors

struct JFFS2_IOVEC_TAG
{
    void           *iov_base;           /* Base address. */
    ssize_t        iov_len;             /* Length. */
};

enum	jffs2_uio_rw { UIO_READ, UIO_WRITE };

/* Segment flag values. */
enum jffs2_uio_seg
{
    UIO_USERSPACE,		        /* from user data space */
    UIO_SYSSPACE		        /* from system space */
};

struct JFFS2_UIO_TAG
{
    struct JFFS2_IOVEC_TAG *uio_iov;	/* pointer to array of iovecs */
    int	                 uio_iovcnt;	/* number of iovecs in array */
    off_t       	 uio_offset;	/* offset into file this uio corresponds to */
    ssize_t     	 uio_resid;	/* residual i/o count */
    enum jffs2_uio_seg     uio_segflg;    /* see above */
    enum jffs2_uio_rw      uio_rw;        /* see above */
};

struct JFFS2_FILE_TAG
{
    jffs2_uint32	                f_flag;		/* file state                   */
    jffs2_uint16                  f_ucount;       /* use count                    */
    jffs2_uint16                  f_type;		/* descriptor type              */
    jffs2_uint32                  f_syncmode;     /* synchronization protocol     */
    struct JFFS2_FILEOPS_TAG      *f_ops;         /* file operations              */
    off_t       	        f_offset;       /* current offset               */
    JFFS2_ADDRWORD	        f_data;		/* file or socket               */
    JFFS2_ADDRWORD                f_xops;         /* extra type specific ops      */
    jffs2_mtab_entry              *f_mte;         /* mount table entry            */
};

// Limits
#define UIO_SMALLIOV	8		/* 8 on stack, else malloc */

//=============================================================================
// Description of open file

typedef int jffs2_fileop_readwrite (struct JFFS2_FILE_TAG *fp, struct JFFS2_UIO_TAG *uio);
typedef jffs2_fileop_readwrite jffs2_fileop_read;
typedef jffs2_fileop_readwrite jffs2_fileop_write;
typedef int jffs2_fileop_lseek   (struct JFFS2_FILE_TAG *fp, off_t *pos, int whence );
typedef int jffs2_fileop_ioctl   (struct JFFS2_FILE_TAG *fp, JFFS2_ADDRWORD com,
                                JFFS2_ADDRWORD data);
typedef int jffs2_fileop_select  (struct JFFS2_FILE_TAG *fp, int which, JFFS2_ADDRWORD info);
typedef int jffs2_fileop_fsync   (struct JFFS2_FILE_TAG *fp, int mode );
typedef int jffs2_fileop_close   (struct JFFS2_FILE_TAG *fp);
typedef int jffs2_fileop_fstat   (struct JFFS2_FILE_TAG *fp, struct jffs2_stat *buf );
typedef int jffs2_fileop_getinfo (struct JFFS2_FILE_TAG *fp, int key, void *buf, int len );
typedef int jffs2_fileop_setinfo (struct JFFS2_FILE_TAG *fp, int key, void *buf, int len );

struct JFFS2_FILEOPS_TAG
{
    jffs2_fileop_read     *fo_read;
    jffs2_fileop_write    *fo_write;
    jffs2_fileop_lseek    *fo_lseek;
    jffs2_fileop_ioctl    *fo_ioctl;
    jffs2_fileop_select   *fo_select;
    jffs2_fileop_fsync    *fo_fsync;
    jffs2_fileop_close    *fo_close;
    jffs2_fileop_fstat    *fo_fstat;
    jffs2_fileop_getinfo  *fo_getinfo;
    jffs2_fileop_setinfo  *fo_setinfo;
};

//-----------------------------------------------------------------------------
// File flags

// Allocation here is that bits 0..15 are copies of bits from the open
// flags, bits 16..23 are extra bits that are visible to filesystems but
// are not derived from the open call, and bits 24..31 are reserved for
// the fileio infrastructure.
#define JFFS2_FREAD       JFFS2_O_RDONLY
#define JFFS2_FWRITE      JFFS2_O_WRONLY
#define JFFS2_FNONBLOCK   JFFS2_O_NONBLOCK
#define JFFS2_FAPPEND     JFFS2_O_APPEND
#define JFFS2_FASYNC      0x00010000
#define JFFS2_FDIR        0x00020000

#define JFFS2_FLOCKED     0x01000000  // Set if file is locked
#define JFFS2_FLOCK       0x02000000  // Lock during file ops
#define JFFS2_FALLOC      0x80000000  // File is "busy", i.e. allocated

// Mask for open mode bits stored in file object
#define JFFS2_FILE_MODE_MASK (JFFS2_FREAD|JFFS2_FWRITE|JFFS2_FNONBLOCK|JFFS2_FAPPEND)
//-----------------------------------------------------------------------------
// Type of file

#define	JFFS2_FILE_TYPE_FILE      1	/* file */
#define	JFFS2_FILE_TYPE_SOCKET	2	/* communications endpoint */
#define	JFFS2_FILE_TYPE_DEVICE	3	/* device */

//-----------------------------------------------------------------------------
// Keys for getinfo() and setinfo()

#define FILE_INFO_CONF          1       /* fpathconf() */

//-----------------------------------------------------------------------------
// Modes for fsync()

#define JFFS2_FSYNC              1
#define JFFS2_FDATASYNC          2

//-----------------------------------------------------------------------------
// Get/set info buffer structures

// This is used for pathconf() and fpathconf()
struct jffs2_pathconf_info
{
    int         name;           // POSIX defined variable name
    long        value;          // Returned variable value
};

//=============================================================================
// Synchronization modes
// These values are filled into the syncmode fields of the above structures
// and define the synchronization protocol used when accessing the object in
// question.

#define JFFS2_SYNCMODE_NONE               (0)     // no locking required

#define JFFS2_SYNCMODE_FILE_FILESYSTEM    0x0002  // lock fs during file ops
#define JFFS2_SYNCMODE_FILE_MOUNTPOINT    0x0004  // lock mte during file ops
#define JFFS2_SYNCMODE_IO_FILE            0x0010  // lock file during io ops
#define JFFS2_SYNCMODE_IO_FILESYSTEM      0x0020  // lock fs during io ops
#define JFFS2_SYNCMODE_IO_MOUNTPOINT      0x0040  // lock mte during io ops
#define JFFS2_SYNCMODE_SOCK_FILE          0x0100  // lock socket during socket ops
#define JFFS2_SYNCMODE_SOCK_NETSTACK      0x0800  // lock netstack during socket ops

#define JFFS2_SYNCMODE_IO_SHIFT           (4)     // shift for IO to file bits
#define JFFS2_SYNCMODE_SOCK_SHIFT         (8)     // shift for sock to file bits

//=============================================================================
// Mount and umount functions

//extern int mount( const char *devname,
//                     const char *dir,
//                     const char *fsname);

//extern int umount( const char *name);
extern int mount(const char   *device_name,
              const char   *path,
              const char   *filesystemtype,
              unsigned long rwflag,
              const void   *data);
extern int umount(const char *specialfile);
//=============================================================================
// Get/Set info functions

__externC int jffs2_fs_getinfo( const char *path, int key, void *buf, int len );
__externC int jffs2_fs_setinfo( const char *path, int key, void *buf, int len );
__externC int jffs2_fs_fgetinfo( int fd, int key, void *buf, int len );
__externC int jffs2_fs_fsetinfo( int fd, int key, void *buf, int len );

#ifdef JFFS2FUN_IO_FILEIO_SELECT
//=============================================================================
// Select support

//-----------------------------------------------------------------------------
// Data structure for embedding in client data structures. A pointer to this
// must be passed to jffs2_selrecord() and jffs2_selwakeup().

struct JFFS2_SELINFO_TAG
{
    JFFS2_ADDRWORD        si_info;        // info passed through from fo_select()
    jffs2_uint32    si_waitFlag;    // select wait flags
};

//-----------------------------------------------------------------------------
// Select support functions.

// jffs2_selinit() is used to initialize a selinfo structure.
__externC void jffs2_selinit( struct JFFS2_SELINFO_TAG *sip );

// jffs2_selrecord() is called when a client device needs to register
// the current thread for selection.
__externC void jffs2_selrecord( JFFS2_ADDRWORD info, struct JFFS2_SELINFO_TAG *sip );

// jffs2_selwakeup() is called when the client device matches the select
// criterion, and needs to wake up a selector.
__externC void jffs2_selwakeup( struct JFFS2_SELINFO_TAG *sip );
#endif
//=============================================================================
// Timestamp support

// Provides the current time as a time_t timestamp for use in filesystem
// data strucures.
__externC time_t jffs2_get_timestamp(void);

//=============================================================================
// Miscellaneous functions.

// Provide a function to synchronize an individual file system. (ie write
// file and directory information to disk)
__externC int jffs2_fs_fssync(const char *path);

// Functions to set and get attributes of a file, eg FAT attributes
// like hidden and system.
__externC int jffs2_fs_set_attrib( const char *fname,
                                 const jffs2_fs_attrib_t new_attrib );
__externC int jffs2_fs_get_attrib( const char *fname,
                                 jffs2_fs_attrib_t * const file_attrib );

// Functions to lock and unlock a filesystem. These are normally used
// internally by the fileio layer, but the file system might need to
// use them when it needs to lock itself, eg when performing garbage
// collect.
__externC void jffs2_fs_lock( jffs2_mtab_entry *mte, jffs2_uint32 syncmode );

__externC void jffs2_fs_unlock( jffs2_mtab_entry *mte, jffs2_uint32 syncmode );

// To be able to lock the filesystem you need the mte. This function
// allows the table of mounted filesystems to be searched to find an
// mte which uses the give filesystem root.

__externC jffs2_mtab_entry * jffs2_fs_root_lookup( jffs2_dir *root );

//=============================================================================
// Default functions.
// Cast to the appropriate type, these functions can be put into any of
// the operation table slots to provide the defined error code.

__externC int jffs2_fileio_enosys(void);
__externC int jffs2_fileio_erofs(void);
__externC int jffs2_fileio_enoerr(void);
__externC int jffs2_fileio_enotdir(void);
__externC jffs2_fileop_select jffs2_fileio_seltrue;

//-----------------------------------------------------------------------------

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif // ifndef JFFS2ONCE_FILEIO_H
// End of fileio.h
