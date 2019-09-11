#ifndef JFFS2ONCE_FCNTL_H
#define JFFS2ONCE_FCNTL_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define __CC_ARM
/* File access modes used for open() and fnctl() */
/* for vfs_jffs2.c                                                             */
/*------------------------------------------------------------------------*/
/* File access modes used for open() and fnctl() */
#define JFFS2_O_RDONLY     (1<<0)   /* Open for reading only */
#define JFFS2_O_WRONLY     (1<<1)   /* Open for writing only */
#define JFFS2_O_RDWR       (JFFS2_O_RDONLY|JFFS2_O_WRONLY) /* Open for reading and writing */

/* File access mode mask */
#define JFFS2_O_ACCMODE    (JFFS2_O_RDONLY|JFFS2_O_RDWR|JFFS2_O_WRONLY)

/* open() mode flags */

#define JFFS2_O_CREAT      (1<<3)    /* Create file it it does not exist */
#define JFFS2_O_EXCL       (1<<4)    /* Exclusive use */
#define JFFS2_O_NOCTTY     (1<<5)    /* Do not assign a controlling terminal */
#define JFFS2_O_TRUNC      (1<<6)    /* Truncate */

/* File status flags used for open() and fcntl() */
#define JFFS2_O_APPEND     (1<<7)    /* Set append mode */
#define JFFS2_O_DSYNC      (1<<8)    /* Synchronized I/O data integrity writes */
#define JFFS2_O_NONBLOCK   (1<<9)    /* No delay */
#define JFFS2_O_RSYNC      (1<<10)   /* Synchronized read I/O */
#define JFFS2_O_SYNC       (1<<11)   /* Synchronized I/O file integrity writes */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
/* EOF fcntl.h */
