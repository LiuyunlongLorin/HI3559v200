#ifndef __FSTOOL_IO_H__
#define __FSTOOL_IO_H__
#include <sys/types.h>


int FSTOOL_IO_Open(const char* path);
void FSTOOL_IO_Close(int fd);
int FSTOOL_IO_Read(int fd, unsigned long long sector, u_int count, void *data);
int FSTOOL_IO_Write(int fd, unsigned long long sector, u_int count, void *data);
#endif
