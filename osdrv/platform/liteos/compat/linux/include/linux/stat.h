#ifndef __LINUX_STAT_H__
#define __LINUX_STAT_H__


#include <sys/stat.h>


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef S_IRUGO
#define S_IRUGO (S_IRUSR|S_IRGRP|S_IROTH)
#endif
#ifndef S_IWUGO
#define S_IWUGO (S_IWUSR|S_IWGRP|S_IWOTH)
#endif
#ifndef S_IXUGO
#define S_IXUGO (S_IXUSR|S_IXGRP|S_IXOTH)
#endif
#ifndef S_IRWXUGO
#define S_IRWXUGO (S_IRWXU|S_IRWXG|S_IRWXO)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __LINUX_STAT_H__ */
