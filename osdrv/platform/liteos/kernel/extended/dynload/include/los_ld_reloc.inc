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

/** @defgroup dynload dynamic loading
 *@ingroup kernel
 */

#ifndef _KERNEL_LOS_LD_RELOC_INC
#define _KERNEL_LOS_LD_RELOC_INC


#ifdef LOSCFG_AARCH64
UINT32 AARCH64_ELF_Call26Reloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 AARCH64_ELF_Abs64Reloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 AARCH64_ELF_AdrPrelPgHi21Reloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 AARCH64_ELF_Ldst32AbsLo12NcReloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 AARCH64_ELF_AddAbsLo12NcReloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 AARCH64_ELF_Ldst8AbsLo12NcReloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 AARCH64_ELF_Ldst64AbsLo12NcReloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 AARCH64_ELF_Ldst16AbsLo12NcReloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 AARCH64_ELF_GlobDatReloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 AARCH64_ELF_JumpSlotReloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 AARCH64_ELF_RelativeReloc(LD_RELOC_PARAM * pstRelocParam);
#else
UINT32 MCPAT_ELFArmPc24Reloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFArmAbs32Reloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFArmRel32Reloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFThumbPc22Reloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFArmCallReloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFThumbPc9Reloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFThumbPc11Reloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFArmAbs8Reloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFArmAbs16Reloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFArmAbs12Reloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFThumbAbs5Reloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFThumbMovwOrMovtReloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFArmMovwOrMovtReloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFGlobDatReloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFJumpSlotReloc(LD_RELOC_PARAM * pstRelocParam);
UINT32 MCPAT_ELFArmRelativeReloc(LD_RELOC_PARAM * pstRelocParam);
#endif
VOID LD_MoveBitsArea16(AARCHUINT DataFrom, UINT16 *DataTo, UINT8 BitFrom, UINT8 BitTo, UINT8 BitWide);
VOID LD_MoveBitsArea32(AARCHUINT DataFrom, UINT32 *DataTo, UINT8 BitFrom, UINT8 BitTo, UINT8 BitWide);
VOID LD_MoveBitsArea64(AARCHUINT DataFrom, UINT64 *DataTo, UINT8 BitFrom, UINT8 BitTo, UINT8 BitWide);
#endif /* _KERNEL_LOS_LD_RELOC_INC */
