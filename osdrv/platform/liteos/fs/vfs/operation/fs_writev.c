/*----------------------------------------------------------------------------
* Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
* All rights reserved.
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice, this list of
* conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list
* of conditions and the following disclaimer in the documentation and/or other materials
* provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used
* to endorse or promote products derived from this software without specific prior written
* permission.
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
* Notice of Export Control Law
* ===============================================
* Huawei LiteOS may be subject to applicable export control laws and regulations, which might
* include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
* Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
* applicable export control laws and regulations.
*---------------------------------------------------------------------------*/

#include "sys/types.h"
#include "sys/uio.h"
#include "unistd.h"
#include "string.h"
#include "stdlib.h"

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    int i;
    char *buf;
    char *cur_buf;
    char *write_buf;
    size_t buf_len = 0;
    size_t bytes_to_write;
    ssize_t total_bytes_written = 0;

    if (iov == NULL) {
        return -1;
    }

    for (i = 0; i < iovcnt; ++i) {
        buf_len += iov[i].iov_len;
    }

    buf = (char *)malloc(buf_len * sizeof(char));

    if (NULL == buf) {
        return -1;
    }
    cur_buf = buf;
    for (i = 0; i < iovcnt; ++i) {
        write_buf = (char *)iov[i].iov_base;
        bytes_to_write = iov[i].iov_len;
        memcpy(cur_buf, write_buf, bytes_to_write);
        cur_buf += bytes_to_write;
    }

    total_bytes_written = write(fd, buf, buf_len);
    free(buf);

    return total_bytes_written;
}
