/*
psxdefs: PS register/constants definitions

Copyright (c) 2017 Ben "GreaseMonkey" Russell
Copyright (c) 2019 Adrian "asie" Siekierka

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include <stdint.h>

#ifndef PSX_IOBASE
#define PSX_IOBASE 0x1F800000
#endif

// most names are based on nocash's psx-spx

//
// Memory control
//
// invented a few names for this one
#define PSXREG_MEM_EXP1_BASE  (*(volatile uint32_t *)(PSX_IOBASE + 0x1000))
#define PSXREG_MEM_EXP2_BASE  (*(volatile uint32_t *)(PSX_IOBASE + 0x1004))
#define PSXREG_MEM_EXP1_CONF  (*(volatile uint32_t *)(PSX_IOBASE + 0x1008))
#define PSXREG_MEM_EXP3_CONF  (*(volatile uint32_t *)(PSX_IOBASE + 0x100C))
#define PSXREG_MEM_BIOS_CONF  (*(volatile uint32_t *)(PSX_IOBASE + 0x1010))
#define PSXREG_MEM_SPU_CONF   (*(volatile uint32_t *)(PSX_IOBASE + 0x1014))
#define PSXREG_MEM_CDROM_CONF (*(volatile uint32_t *)(PSX_IOBASE + 0x1018))
#define PSXREG_MEM_EXP2_CONF  (*(volatile uint32_t *)(PSX_IOBASE + 0x101C))
#define PSXREG_MEM_COM_DELAY  (*(volatile uint32_t *)(PSX_IOBASE + 0x1020))

#define PSXREG_MEM_RAM_SIZE   (*(volatile uint32_t *)(PSX_IOBASE + 0x1060))

//
// Serial/Joypad/Memcard
//
// DATA1 is a 1-byte alias, apparently this can be used?
#define PSXREG_JOY_DATA  (*(volatile uint32_t *)(PSX_IOBASE + 0x1040))
#define PSXREG_JOY_DATA1 (*(volatile uint8_t  *)(PSX_IOBASE + 0x1040))
#define PSXREG_JOY_STAT  (*(volatile uint32_t *)(PSX_IOBASE + 0x1044))
#define PSXREG_JOY_MODE  (*(volatile uint16_t *)(PSX_IOBASE + 0x1048))
#define PSXREG_JOY_CTRL  (*(volatile uint16_t *)(PSX_IOBASE + 0x104A))
#define PSXREG_JOY_BAUD  (*(volatile uint16_t *)(PSX_IOBASE + 0x104E))
#define PSXREG_SIO_DATA  (*(volatile uint32_t *)(PSX_IOBASE + 0x1050))
#define PSXREG_SIO_DATA1 (*(volatile uint8_t  *)(PSX_IOBASE + 0x1050))
#define PSXREG_SIO_STAT  (*(volatile uint32_t *)(PSX_IOBASE + 0x1054))
#define PSXREG_SIO_MODE  (*(volatile uint16_t *)(PSX_IOBASE + 0x1058))
#define PSXREG_SIO_CTRL  (*(volatile uint16_t *)(PSX_IOBASE + 0x105A))
#define PSXREG_SIO_MISC  (*(volatile uint16_t *)(PSX_IOBASE + 0x105C))
#define PSXREG_SIO_BAUD  (*(volatile uint16_t *)(PSX_IOBASE + 0x105E))

//
// Interrupt Controller
//
#define PSXREG_I_STAT (*(volatile uint16_t *)(PSX_IOBASE + 0x1070))
#define PSXREG_I_MASK (*(volatile uint16_t *)(PSX_IOBASE + 0x1074))

//
// DMA Controller
//
#define PSXREG_Dn_MADR(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1080 + ((n)*0x10)))
#define PSXREG_Dn_BCR(n)  (*(volatile uint32_t *)(PSX_IOBASE + 0x1084 + ((n)*0x10)))
#define PSXREG_Dn_CHCR(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1088 + ((n)*0x10)))

#define PSXREG_DPCR (*(volatile uint32_t *)(PSX_IOBASE + 0x10F0))
#define PSXREG_DICR (*(volatile uint32_t *)(PSX_IOBASE + 0x10F4))

//
// Timers
//
// these names are from the PS2 EE User's Manual
// the register purposes match the PS1's
// only the bottom 16 bits make sense but they're still 32-bit accesses
#define PSXREG_Tn_COUNT(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1100 + ((n)*0x10)))
#define PSXREG_Tn_MODE(n)  (*(volatile uint32_t *)(PSX_IOBASE + 0x1104 + ((n)*0x10)))
#define PSXREG_Tn_COMP(n)  (*(volatile uint32_t *)(PSX_IOBASE + 0x1108 + ((n)*0x10)))

//
// CD-ROM
//
// had to make up names for these as well
// by the way, check the docs for this - it will make no sense if you don't
#define PSXREG_CDROM_In_IDXSR  (*(volatile uint8_t *)(PSX_IOBASE + 0x1800))
#define PSXREG_CDROM_I0_CMD    (*(volatile uint8_t *)(PSX_IOBASE + 0x1801))
#define PSXREG_CDROM_I0_PARAMS (*(volatile uint8_t *)(PSX_IOBASE + 0x1802))
#define PSXREG_CDROM_I0_RQST_W (*(volatile uint8_t *)(PSX_IOBASE + 0x1803))
#define PSXREG_CDROM_In_DATA_R (*(volatile uint8_t *)(PSX_IOBASE + 0x1802))
#define PSXREG_CDROM_I1_RESP_R (*(volatile uint8_t *)(PSX_IOBASE + 0x1801))
#define PSXREG_CDROM_I1_INTE_W (*(volatile uint8_t *)(PSX_IOBASE + 0x1802))
#define PSXREG_CDROM_I0_INTE_R (*(volatile uint8_t *)(PSX_IOBASE + 0x1803))
#define PSXREG_CDROM_I1_INTFLG (*(volatile uint8_t *)(PSX_IOBASE + 0x1803))
#define PSXREG_CDROM_I2_VOL_LL (*(volatile uint8_t *)(PSX_IOBASE + 0x1802))
#define PSXREG_CDROM_I2_VOL_LR (*(volatile uint8_t *)(PSX_IOBASE + 0x1803))
#define PSXREG_CDROM_I3_VOL_RR (*(volatile uint8_t *)(PSX_IOBASE + 0x1801))
#define PSXREG_CDROM_I3_VOL_RL (*(volatile uint8_t *)(PSX_IOBASE + 0x1802))
#define PSXREG_CDROM_I3_VOLCTL (*(volatile uint8_t *)(PSX_IOBASE + 0x1803))
#define PSXREG_CDROM_I2_SMAP_W (*(volatile uint8_t *)(PSX_IOBASE + 0x1801))

//
// GPU
//
#define PSXREG_GPU_GP0 (*(volatile uint32_t *)(PSX_IOBASE + 0x1810))
#define PSXREG_GPU_GP1 (*(volatile uint32_t *)(PSX_IOBASE + 0x1814))
#define PSXREG_GPU_GPUREAD (*(volatile uint32_t *)(PSX_IOBASE + 0x1810))
#define PSXREG_GPU_GPUSTAT (*(volatile uint32_t *)(PSX_IOBASE + 0x1814))

//
// MDEC
//
#define PSXREG_MDEC_MDEC0 (*(volatile uint32_t *)(PSX_IOBASE + 0x1820))
#define PSXREG_MDEC_MDEC1 (*(volatile uint32_t *)(PSX_IOBASE + 0x1824))

//
// SPU
//
// many of these names are from the PS2 SPU2 Overview Manual
// there are some 16-bit aliases
//
// CDVOL and XVOL are called AVOL and BVOL
// but I don't know which way around they go
#define PSXREG_SPU_n_VOL(n)   (*(volatile uint32_t *)(PSX_IOBASE + 0x1C00 + (n)*0x10))
#define PSXREG_SPU_n_VOLL(n)  (*(volatile uint16_t *)(PSX_IOBASE + 0x1C00 + (n)*0x10))
#define PSXREG_SPU_n_VOLR(n)  (*(volatile uint16_t *)(PSX_IOBASE + 0x1C02 + (n)*0x10))
#define PSXREG_SPU_n_PITCH(n) (*(volatile uint16_t *)(PSX_IOBASE + 0x1C04 + (n)*0x10))
#define PSXREG_SPU_n_SSAH(n)  (*(volatile uint16_t *)(PSX_IOBASE + 0x1C06 + (n)*0x10))
#define PSXREG_SPU_n_ADSR(n)  (*(volatile uint32_t *)(PSX_IOBASE + 0x1C08 + (n)*0x10))
#define PSXREG_SPU_n_ADSR1(n) (*(volatile uint16_t *)(PSX_IOBASE + 0x1C08 + (n)*0x10))
#define PSXREG_SPU_n_ADSR2(n) (*(volatile uint16_t *)(PSX_IOBASE + 0x1C0A + (n)*0x10))
#define PSXREG_SPU_n_ENVX(n)  (*(volatile uint16_t *)(PSX_IOBASE + 0x1C0C + (n)*0x10))
#define PSXREG_SPU_n_LSAX(n)  (*(volatile uint16_t *)(PSX_IOBASE + 0x1C0E + (n)*0x10))

#define PSXREG_SPU_MVOL   (*(volatile uint32_t *)(PSX_IOBASE + 0x1D80))
#define PSXREG_SPU_MVOLL  (*(volatile uint16_t *)(PSX_IOBASE + 0x1D80))
#define PSXREG_SPU_MVOLR  (*(volatile uint16_t *)(PSX_IOBASE + 0x1D82))
#define PSXREG_SPU_EVOL   (*(volatile uint32_t *)(PSX_IOBASE + 0x1D84))
#define PSXREG_SPU_EVOLL  (*(volatile uint16_t *)(PSX_IOBASE + 0x1D84))
#define PSXREG_SPU_EVOLR  (*(volatile uint16_t *)(PSX_IOBASE + 0x1D86))
#define PSXREG_SPU_KON    (*(volatile uint32_t *)(PSX_IOBASE + 0x1D88))
#define PSXREG_SPU_KON0   (*(volatile uint16_t *)(PSX_IOBASE + 0x1D88))
#define PSXREG_SPU_KON1   (*(volatile uint16_t *)(PSX_IOBASE + 0x1D8A))
#define PSXREG_SPU_KOFF   (*(volatile uint32_t *)(PSX_IOBASE + 0x1D8C))
#define PSXREG_SPU_KOFF0  (*(volatile uint16_t *)(PSX_IOBASE + 0x1D8C))
#define PSXREG_SPU_KOFF1  (*(volatile uint16_t *)(PSX_IOBASE + 0x1D8E))
#define PSXREG_SPU_PMON   (*(volatile uint32_t *)(PSX_IOBASE + 0x1D90))
#define PSXREG_SPU_PMON0  (*(volatile uint16_t *)(PSX_IOBASE + 0x1D90))
#define PSXREG_SPU_PMON1  (*(volatile uint16_t *)(PSX_IOBASE + 0x1D92))
#define PSXREG_SPU_NON    (*(volatile uint32_t *)(PSX_IOBASE + 0x1D94))
#define PSXREG_SPU_NON0   (*(volatile uint16_t *)(PSX_IOBASE + 0x1D94))
#define PSXREG_SPU_NON1   (*(volatile uint16_t *)(PSX_IOBASE + 0x1D96))
#define PSXREG_SPU_EON    (*(volatile uint32_t *)(PSX_IOBASE + 0x1D98))
#define PSXREG_SPU_EON0   (*(volatile uint16_t *)(PSX_IOBASE + 0x1D98))
#define PSXREG_SPU_EON1   (*(volatile uint16_t *)(PSX_IOBASE + 0x1D9A))
#define PSXREG_SPU_ENDX   (*(volatile uint32_t *)(PSX_IOBASE + 0x1D9C))
#define PSXREG_SPU_ENDX0  (*(volatile uint16_t *)(PSX_IOBASE + 0x1D9C))
#define PSXREG_SPU_ENDX1  (*(volatile uint16_t *)(PSX_IOBASE + 0x1D9E))
// 0x1DA0 16-bit unknown
#define PSXREG_SPU_ESA    (*(volatile uint16_t *)(PSX_IOBASE + 0x1DA2))
#define PSXREG_SPU_IRQA   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DA4))
#define PSXREG_SPU_TSA    (*(volatile uint16_t *)(PSX_IOBASE + 0x1DA6))
#define PSXREG_SPU_FIFO   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DA8))
#define PSXREG_SPU_CNT    (*(volatile uint16_t *)(PSX_IOBASE + 0x1DAA))
#define PSXREG_SPU_TRNCTL (*(volatile uint16_t *)(PSX_IOBASE + 0x1DAC))
#define PSXREG_SPU_STAT   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DAE))
#define PSXREG_SPU_CDVOL  (*(volatile uint32_t *)(PSX_IOBASE + 0x1DB0))
#define PSXREG_SPU_CDVOLL (*(volatile uint16_t *)(PSX_IOBASE + 0x1DB0))
#define PSXREG_SPU_CDVOLR (*(volatile uint16_t *)(PSX_IOBASE + 0x1DB2))
#define PSXREG_SPU_XVOL   (*(volatile uint32_t *)(PSX_IOBASE + 0x1DB4))
#define PSXREG_SPU_XVOLL  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DB4))
#define PSXREG_SPU_XVOLR  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DB6))
#define PSXREG_SPU_MVOLX  (*(volatile uint32_t *)(PSX_IOBASE + 0x1DB8))
#define PSXREG_SPU_MVOLXL (*(volatile uint16_t *)(PSX_IOBASE + 0x1DB8))
#define PSXREG_SPU_MVOLXR (*(volatile uint16_t *)(PSX_IOBASE + 0x1DBA))
// 0x1DBC 32-bit unknown

#define PSXREG_SPU_EFFECT_dAPF1   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DC0))
#define PSXREG_SPU_EFFECT_dAPF2   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DC2))
#define PSXREG_SPU_EFFECT_vIIR    (*(volatile uint16_t *)(PSX_IOBASE + 0x1DC4))
#define PSXREG_SPU_EFFECT_vCOMB1  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DC6))
#define PSXREG_SPU_EFFECT_vCOMB2  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DC8))
#define PSXREG_SPU_EFFECT_vCOMB3  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DCA))
#define PSXREG_SPU_EFFECT_vCOMB4  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DCC))
#define PSXREG_SPU_EFFECT_vWALL   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DCE))
#define PSXREG_SPU_EFFECT_vAPF1   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DD0))
#define PSXREG_SPU_EFFECT_vAPF2   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DD2))

#define PSXREG_SPU_EFFECT_mSAME   (*(volatile uint32_t *)(PSX_IOBASE + 0x1DD4))
#define PSXREG_SPU_EFFECT_mCOMB1  (*(volatile uint32_t *)(PSX_IOBASE + 0x1DD8))
#define PSXREG_SPU_EFFECT_mCOMB2  (*(volatile uint32_t *)(PSX_IOBASE + 0x1DDC))
#define PSXREG_SPU_EFFECT_dSAME   (*(volatile uint32_t *)(PSX_IOBASE + 0x1DE0))
#define PSXREG_SPU_EFFECT_mDIFF   (*(volatile uint32_t *)(PSX_IOBASE + 0x1DE4))
#define PSXREG_SPU_EFFECT_mCOMB3  (*(volatile uint32_t *)(PSX_IOBASE + 0x1DE8))
#define PSXREG_SPU_EFFECT_mCOMB4  (*(volatile uint32_t *)(PSX_IOBASE + 0x1DEC))
#define PSXREG_SPU_EFFECT_dDIFF   (*(volatile uint32_t *)(PSX_IOBASE + 0x1DF0))
#define PSXREG_SPU_EFFECT_mAPF1   (*(volatile uint32_t *)(PSX_IOBASE + 0x1DF4))
#define PSXREG_SPU_EFFECT_mAPF2   (*(volatile uint32_t *)(PSX_IOBASE + 0x1DF8))
#define PSXREG_SPU_EFFECT_vIN     (*(volatile uint32_t *)(PSX_IOBASE + 0x1DFC))

#define PSXREG_SPU_EFFECT_mLSAME   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DD4))
#define PSXREG_SPU_EFFECT_mLCOMB1  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DD8))
#define PSXREG_SPU_EFFECT_mLCOMB2  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DDC))
#define PSXREG_SPU_EFFECT_dLSAME   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DE0))
#define PSXREG_SPU_EFFECT_mLDIFF   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DE4))
#define PSXREG_SPU_EFFECT_mLCOMB3  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DE8))
#define PSXREG_SPU_EFFECT_mLCOMB4  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DEC))
#define PSXREG_SPU_EFFECT_dLDIFF   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DF0))
#define PSXREG_SPU_EFFECT_mLAPF1   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DF4))
#define PSXREG_SPU_EFFECT_mLAPF2   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DF8))
#define PSXREG_SPU_EFFECT_vLIN     (*(volatile uint16_t *)(PSX_IOBASE + 0x1DFC))
#define PSXREG_SPU_EFFECT_mRSAME   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DD6))
#define PSXREG_SPU_EFFECT_mRCOMB1  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DDA))
#define PSXREG_SPU_EFFECT_mRCOMB2  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DDE))
#define PSXREG_SPU_EFFECT_dRSAME   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DE2))
#define PSXREG_SPU_EFFECT_mRDIFF   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DE6))
#define PSXREG_SPU_EFFECT_mRCOMB3  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DEA))
#define PSXREG_SPU_EFFECT_mRCOMB4  (*(volatile uint16_t *)(PSX_IOBASE + 0x1DEE))
#define PSXREG_SPU_EFFECT_dRDIFF   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DF2))
#define PSXREG_SPU_EFFECT_mRAPF1   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DF6))
#define PSXREG_SPU_EFFECT_mRAPF2   (*(volatile uint16_t *)(PSX_IOBASE + 0x1DFA))
#define PSXREG_SPU_EFFECT_vRIN     (*(volatile uint16_t *)(PSX_IOBASE + 0x1DFE))


#define PSXREG_SPU_n_VOLX(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1E00 + (n)*0x04))

