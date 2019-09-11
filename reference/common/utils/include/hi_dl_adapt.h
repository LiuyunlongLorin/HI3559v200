#ifndef __HuaweiLite__
#include <dlfcn.h>
#else
#include <los_ld_elflib.h>
#endif

#ifndef __HuaweiLite__
/*please set LD_LIBRARY_PATH before evecv*/
#else
#define Hi_dlpath(so_path)  LOS_PathAdd(so_path)
#endif

#ifndef __HuaweiLite__
#define Hi_dlopen(filename,flag)  dlopen(filename,flag)
#else
#define Hi_dlopen(filename,flag)  LOS_SoLoad(filename)
#endif

#ifndef __HuaweiLite__
#define Hi_dlsym(handle,symbol)  dlsym(handle,symbol)
#else
#define Hi_dlsym(handle,symbol)  LOS_FindSymByName(handle,symbol)
#endif

#ifndef __HuaweiLite__
#define Hi_dlclose(handle)  dlclose(handle)
#else
#define Hi_dlclose(handle)  LOS_ModuleUnload(handle)
#endif
