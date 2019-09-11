/*    $NetBSD: bus.h,v 1.12 1997/10/01 08:25:15 fvdl Exp $    */

/*-
 * Copyright (c) 1996, 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe of the Numerical Aerospace Simulation Facility,
 * NASA Ames Research Center.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*-
 * Copyright (c) 1996 Charles M. Hannum.  All rights reserved.
 * Copyright (c) 1996 Christopher G. Demetriou.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Christopher G. Demetriou
 *    for the NetBSD Project.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* $FreeBSD$ */

#ifndef _BUSDMA_REF_H
#define _BUSDMA_REF_H

#define BUS_SPACE_MAXADDR_24BIT    0xFFFFFF
#define BUS_SPACE_MAXADDR_32BIT 0xFFFFFFFF
#define BUS_SPACE_MAXADDR     0xFFFFFFFF
#define BUS_SPACE_MAXSIZE_24BIT    0xFFFFFF
#define BUS_SPACE_MAXSIZE_32BIT    0xFFFFFFFF
#define BUS_SPACE_MAXSIZE     0xFFFFFFFF
#define    BUS_SPACE_BARRIER_READ    0x01
#define    BUS_SPACE_BARRIER_WRITE    0x02


#define    BUS_DMA_WAITOK        0x00    /* safe to sleep (pseudo-flag) */
#define    BUS_DMA_NOWAIT        0x01    /* not safe to sleep */
#define    BUS_DMA_ALLOCNOW    0x02    /* perform resource allocation now */
#define    BUS_DMA_COHERENT    0x04    /* hint: map memory in a coherent way */
#define    BUS_DMA_ZERO        0x08    /* allocate zero'ed memory */
#define    BUS_DMA_BUS1        0x10    /* placeholders for bus functions... */
#define    BUS_DMA_BUS2        0x20
#define    BUS_DMA_BUS3        0x40
#define    BUS_DMA_BUS4        0x80

#define    BUS_DMASYNC_PREREAD    1
#define    BUS_DMASYNC_POSTREAD    2
#define    BUS_DMASYNC_PREWRITE    4
#define    BUS_DMASYNC_POSTWRITE    8


#define bus_dma_tag_create(...)        (1)
#define bus_dma_tag_destroy(...)    (1)
#define bus_dmamap_create(...)    (1)
#define bus_dmamap_destroy(...)    (1)
#define bus_dmamem_alloc(...)    (1)
#define bus_dmamem_free(...)    (1)
#define bus_dmamap_load(...)    (1)
#define bus_dmamap_unload(...)    (1)
#define bus_dmamap_sync(...)    (1)

#endif
