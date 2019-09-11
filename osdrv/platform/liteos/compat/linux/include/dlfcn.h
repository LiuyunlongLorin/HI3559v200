#ifndef _DLFCN_H
#define _DLFCN_H


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


extern VOID *LOS_SoLoad(CHAR *pscElfFileName);
extern VOID *LOS_FindSymByName(VOID *pHandle, CHAR *pscName);
extern UINT32 LOS_ModuleUnload(VOID *pstHandle);

#define dlopen(pathname, mode) LOS_SoLoad(pathname)
#define dlsym(handle, symbol) LOS_FindSymByName(handle, symbol)
#define dlclose(handle) LOS_ModuleUnload(handle)


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _DLFCN_H */
