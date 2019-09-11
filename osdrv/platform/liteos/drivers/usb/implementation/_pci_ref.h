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

#ifndef _PCI_REF_H
#define _PCI_REF_H


#define pci_get_devid(...)    (1)
#define pci_get_class(...)    (1)
#define pci_get_subclass(...)    (1)
#define pci_get_progif(...)    (1)
#define pci_get_revid(...)    (1)
#define pci_get_vendor(...)    (1)
#define pci_get_device(...)    (1)
#define pci_get_powerstate(...)    (1)
#define pci_set_powerstate(...)    (1)
#define pci_get_intpin(...)    (1)
#define pci_set_intpin(...)    (1)


#define pci_find_device(...)    (NULL)
#define pci_read_config(...)    (1)
#define pci_write_config(...)    (1)
#define pci_enable_busmaster(...)    (1)
#define pci_disable_busmaster(...)    (1)

#define pci_msi_count(...)    (1)
#define pci_alloc_msi(...)    (1)
#define pci_release_msi(...)    (1)

#define rman_get_bustag(...)    (NULL)
#define rman_get_bushandle(...)    (0)
#define rman_get_size(...)    (1)

#define bus_alloc_resource_any(...)    (NULL)
#define bus_release_resource(...)    (1)

#define PCIC_SERIALBUS    (1)
#define PCIS_SERIALBUS_USB    (1)
#define PCI_POWERSTATE_D0    (1)
#define PCIP_SERIALBUS_USB_XHCI    (1)

#endif
