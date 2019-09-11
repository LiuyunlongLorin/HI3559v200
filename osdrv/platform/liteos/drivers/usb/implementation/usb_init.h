/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2017>, <Huawei Technologies Co., Ltd>
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

#ifndef __USB_INIT_H__
#define  __USB_INIT_H__

#ifdef USB_GLOBAL_INCLUDE_FILE
#include USB_GLOBAL_INCLUDE_FILE
#include "core/usb_process.h"
#include "los_memory.h"
#endif

//Controller type
typedef enum  controller_type{
    HOST = 0,
    DEVICE
}controller_type;

//device type
typedef enum device_type{
    DEV_SERIAL = 0,  // used for serial
    DEV_ETHERNET, //used for ethernet
    DEV_SER_ETH,    //used for serial and ethernet
    DEV_DFU,             //used for DFU
    DEV_MASS,          //used for mass
    DEV_UVC,            //used for USB video
    DEV_UAC,            //used for  USB audio
    DEV_CAMERA       //used for  USB camera
}device_type;

/*****************************************
Function:usb_init
Description: initialize the controller of usb,attach the usb protocol
Input: ctype ------the type of usb's controller, the legal value is one of  'controller_type';
                              if it is HOST , 'dtype' can be any value of 'device_type'; if it is DEVICE, 'dtype' must be one of 'device_type' that user needs
          dtype------the type of device, the legal value is one of 'device_type', if 'ctype' is DEVICE, it is determined by the use requirements
Output: None
Return: success is LOS_OK, failure is LOS_NOK
******************************************/
UINT32 usb_init(controller_type ctype, device_type dtype);

 #endif