#ifndef _STDBOOL_H
#define _STDBOOL_H

#ifndef __cplusplus
#define true  1
#define false 0
#define bool  _Bool
#else /* __cplusplus */
#define false   false
#define true    true
#define _Bool   bool
#define bool    bool
#endif /* __cplusplus */

#endif  /* stdbool.h */
