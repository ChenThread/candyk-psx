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

#define SPU_CHANNEL_COUNT 24

#define SPU_CTRL_MASTER_ENABLE 0x8000
#define SPU_CTRL_MASTER_UNMUTE 0x4000
#define SPU_CTRL_NOISE_SHIFT(n) (((n)&0xF)<<10)
#define SPU_CTRL_NOISE_STEP(n) (((n)&0x3)<<8)
#define SPU_CTRL_REVERB_ENABLE 0x0080
#define SPU_CTRL_IRQ9_ENABLE 0x0040
#define SPU_CTRL_TRANSFER_MODE(n) (((n)&0x3)<<4)
#define SPU_CTRL_TRANSFER_MODE_MASK (((0x3)&0x3)<<4)
#define SPU_CTRL_EXTERNAL_REVERB 0x0008
#define SPU_CTRL_CDROM_REVERB 0x0004
#define SPU_CTRL_EXTERNAL_ENABLE 0x0002
#define SPU_CTRL_CDROM_ENABLE 0x0001

#define SPU_STAT_TRANSFER_MODE(n) (((n)&0x3)<<4)
#define SPU_STAT_TRANSFER_MODE_MASK (((0x3)&0x3)<<4)
#define SPU_STAT_EXTERNAL_REVERB 0x0008
#define SPU_STAT_CDROM_REVERB 0x0004
#define SPU_STAT_EXTERNAL_ENABLE 0x0002
#define SPU_STAT_CDROM_ENABLE 0x0001

#define SPU_TRANSFER_MODE_STOP 0x0
#define SPU_TRANSFER_MODE_FIFO_WRITE 0x1
#define SPU_TRANSFER_MODE_DMA_WRITE 0x2
#define SPU_TRANSFER_MODE_DMA_READ 0x3

