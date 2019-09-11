#!/bin/sh
#/*----------------------------------------------------------------------------
# * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
# * All rights reserved.
# * Redistribution and use in source and binary forms, with or without modification,
# * are permitted provided that the following conditions are met:
# * 1. Redistributions of source code must retain the above copyright notice, this list of
# * conditions and the following disclaimer.
# * 2. Redistributions in binary form must reproduce the above copyright notice, this list
# * of conditions and the following disclaimer in the documentation and/or other materials
# * provided with the distribution.
# * 3. Neither the name of the copyright holder nor the names of its contributors may be used
# * to endorse or promote products derived from this software without specific prior written
# * permission.
# * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# *---------------------------------------------------------------------------*/
#/*----------------------------------------------------------------------------
# * Notice of Export Control Law
# * ===============================================
# * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
# * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
# * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
# * applicable export control laws and regulations.
# *---------------------------------------------------------------------------*/

#useage:
# choose default: . env_setup.sh    or source env_setup.sh
# choose second : . env_setup.sh  2
# choose second : . env_setup.sh  hi3518ev200
# choose second :
# . env_setup.sh
#	1. hi3516a
#	2. hi3518ev200
#	3. hi3911
#	4. him5v100
#	Select your platform [1]:2


export LITEOSTOPDIR=`pwd`

declare -i count=1
declare -i select_count=1
declare -i selected_flag=1;

#find the all platform depend on board.mk file
PLATFORM_ARRAY=$(ls -lh platform/bsp/board| awk -F " " '{print$9}' |sort)
PLATFORM_ARRAY_LENGTH=$(ls -lh platform/bsp/board | awk -F " " '{print$9}' |sort |wc -l)

#list the platform
for p in $PLATFORM_ARRAY
do
   echo "$count. $p"
   let count++
done

#read select platform
if [ "$1" == "" ];then
read -p "Select your platform [1]:$1" select_count
else
echo "Select your platform [1]:$1"
fi

#default select 1
if [ $select_count -eq 0 ];then
	select_count=1
fi

#if $1 is number  set to select_count
if [[ "$1" =~ ^[1-9]+$ ]];then
	select_count=$1
fi

count=1
#support . env_setup.sh plaform
for p in $PLATFORM_ARRAY
	do
	if [ "$p" == "$1" ];then
		select_count=$count
		break
	fi
	let count++
done

#input must be number and little than platform array length
if [[ "$select_count" =~ ^[1-9]+$ ]];then
	if [ $select_count -gt $PLATFORM_ARRAY_LENGTH ];then
		selected_flag=0
		echo "[ERROR]Please select the right platform!"
	fi
else
	selected_flag=0
	echo "[ERROR]Please select the platform number!"
fi

#set select platform to LITEOS_PLATFORM
if [ $selected_flag -eq 1 ];then
platform_stringss=$(echo ${PLATFORM_ARRAY[*]})
export LITEOS_PLATFORM=`echo $platform_stringss |awk -F " " -v select_count="$select_count" '{print $select_count}'`
fi

