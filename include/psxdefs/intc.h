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

#define INTC_VBLANK 0x0001
#define INTC_GPU    0x0002
#define INTC_CDROM  0x0004
#define INTC_DMA    0x0008
#define INTC_TMR0   0x0010
#define INTC_TMR1   0x0020
#define INTC_TMR2   0x0040
#define INTC_JOY    0x0080 /* also memcard */
#define INTC_SIO    0x0100
#define INTC_SPU    0x0200
#define INTC_JOYAUX 0x0400

#define INTC_VBLANK_IDX 0
#define INTC_GPU_IDX    1
#define INTC_CDROM_IDX  2
#define INTC_DMA_IDX    3
#define INTC_TMR0_IDX   4
#define INTC_TMR1_IDX   5
#define INTC_TMR2_IDX   6
#define INTC_JOY_IDX    7 /* also memcard */
#define INTC_SIO_IDX    8
#define INTC_SPU_IDX    9
#define INTC_JOYAUX_IDX 10

