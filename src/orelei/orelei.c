/*
orelei: PS SPU sound driver
Copyright (C) GreaseMonkey, 2017, licensed under Creative Commons Zero:
https://creativecommons.org/publicdomain/zero/1.0/
*/
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <psxdefs/intc.h>
#include <psxdefs/spu.h>
#include <psxregs.h>

#include <orelei.h>

// Required due to SPU writes being a bit unstable
#define TWICE(x) x;x

static const unsigned int notetab[12] = {
	0x8000, 0x879C, 0x8FAD, 0x9838, 0xA145, 0xAADC,
	0xB505, 0xBFC9, 0xCB30, 0xD745, 0xE412, 0xF1A2,
};

static uint16_t spu_cnt_shadow;

static uint32_t spu_new_key_off = 0x000000;
static uint32_t spu_new_key_on = 0x000000;

int orelei_note_to_pitch(int note, int cxoctave, unsigned int cxspeed)
{
	int octave = (note < 0 ? -((12-note)/12) : note/12);
	int subnote = note - octave*12;
	unsigned int shift_amt = (15+cxoctave-octave);
	unsigned int pitch = (notetab[subnote]*cxspeed + (1<<(shift_amt-1)))>>shift_amt;
	return pitch;
}

void orelei_commit_key_changes(void)
{
	TWICE(PSXREG_SPU_KOFF = spu_new_key_off);
	TWICE(PSXREG_SPU_KON = spu_new_key_on);
	spu_new_key_off = 0x000000;
	spu_new_key_on = 0x000000;
}

void orelei_play_note(int ch, int sram_addr, int adsr, int voll, int volr, int pitch)
{
	int ssah = (sram_addr >> 4) << 1;
	int volboth = (voll & 0xFFFF) | (volr<<16);
	TWICE(PSXREG_SPU_n_VOL(ch) = volboth);
	TWICE(PSXREG_SPU_n_PITCH(ch) = pitch);
	TWICE(PSXREG_SPU_n_SSAH(ch) = ssah);
	TWICE(PSXREG_SPU_n_ADSR(ch) = adsr);
	spu_new_key_on |= (0x1<<ch);
}

void orelei_stop_note(int ch)
{
	spu_new_key_off |= (0x1<<ch);
}

void orelei_set_transfer_mode(int mode)
{
	spu_cnt_shadow &= ~SPU_CNT_TRANSFER_MODE_MASK;
	spu_cnt_shadow |= SPU_CNT_TRANSFER_MODE(mode);
	TWICE(PSXREG_SPU_CNT = spu_cnt_shadow);
	while((PSXREG_SPU_STAT & 0x3F) != (spu_cnt_shadow & 0x3F)) {
	}
}

void orelei_sram_write_blocking(int sram_addr, void const* data, size_t len)
{
	sram_addr >>= 4;
	len >>= 4;

	TWICE(PSXREG_SPU_TRNCTL = 0x0004);
	orelei_set_transfer_mode(SPU_TRANSFER_MODE_STOP);
	TWICE(PSXREG_SPU_TSA = (sram_addr<<1));
	orelei_set_transfer_mode(SPU_TRANSFER_MODE_DMA_WRITE);

	PSXREG_Dn_CHCR(4) = 0x00000201;
	PSXREG_Dn_MADR(4) = (uint32_t)data;
	PSXREG_Dn_BCR(4) = (len<<16)|0x0010;
	PSXREG_Dn_CHCR(4) = 0x01000201;
	PSXREG_DPCR |= (0x8<<(4<<2));
	while((PSXREG_Dn_CHCR(4) & (0x01<<24)) != 0) {
	}
	PSXREG_DPCR &= ~(0x8<<(4<<2));
	orelei_set_transfer_mode(SPU_TRANSFER_MODE_STOP);
}

void orelei_init_spu(void)
{
	for(int i = 0; i < SPU_CHANNEL_COUNT; i++) {
		TWICE(PSXREG_SPU_n_VOL(i) = 0x00000000);
		TWICE(PSXREG_SPU_n_PITCH(i) = 0x0000);
		TWICE(PSXREG_SPU_n_SSAH(i) = 0x0000);
		TWICE(PSXREG_SPU_n_ADSR(i) = 0x00000000);
	}

	TWICE(PSXREG_SPU_MVOL = 0x3FFF3FFF);
	TWICE(PSXREG_SPU_EVOL = 0x00000000);
	TWICE(PSXREG_SPU_KOFF = 0x00FFFFFF);
	TWICE(PSXREG_SPU_PMON = 0x00000000);
	TWICE(PSXREG_SPU_NON = 0x00000000);
	TWICE(PSXREG_SPU_EON = 0x00000000);

	TWICE(PSXREG_SPU_ESA = 0xFFFE);
	TWICE(PSXREG_SPU_IRQA = 0xFFFD);
	spu_cnt_shadow = (0
		| SPU_CNT_MASTER_ENABLE
		| SPU_CNT_MASTER_UNMUTE
		| SPU_CNT_REVERB_ENABLE
		| SPU_CNT_TRANSFER_MODE(SPU_TRANSFER_MODE_STOP)
	);
	TWICE(PSXREG_SPU_CNT = spu_cnt_shadow);
	TWICE(PSXREG_SPU_TRNCTL = 0x0004);
	TWICE(PSXREG_SPU_CDVOL = 0x00000000);
	TWICE(PSXREG_SPU_XVOL = 0x00000000);

	// Disable reverb fully
	TWICE(PSXREG_SPU_EFFECT_dAPF1 = 0x0000);
	TWICE(PSXREG_SPU_EFFECT_dAPF2 = 0x0000);
	TWICE(PSXREG_SPU_EFFECT_vIIR = 0x0000);
	TWICE(PSXREG_SPU_EFFECT_vCOMB1 = 0x0000);
	TWICE(PSXREG_SPU_EFFECT_vCOMB2 = 0x0000);
	TWICE(PSXREG_SPU_EFFECT_vCOMB3 = 0x0000);
	TWICE(PSXREG_SPU_EFFECT_vCOMB4 = 0x0000);
	TWICE(PSXREG_SPU_EFFECT_vWALL = 0x0000);
	TWICE(PSXREG_SPU_EFFECT_vAPF1 = 0x0000);
	TWICE(PSXREG_SPU_EFFECT_vAPF2 = 0x0000);
	TWICE(PSXREG_SPU_EFFECT_mSAME = 0x00010001);
	TWICE(PSXREG_SPU_EFFECT_mCOMB1 = 0x00010001);
	TWICE(PSXREG_SPU_EFFECT_mCOMB2 = 0x00010001);
	TWICE(PSXREG_SPU_EFFECT_dSAME = 0x00000000);
	TWICE(PSXREG_SPU_EFFECT_mDIFF = 0x00010001);
	TWICE(PSXREG_SPU_EFFECT_mCOMB3 = 0x00010001);
	TWICE(PSXREG_SPU_EFFECT_mCOMB4 = 0x00010001);
	TWICE(PSXREG_SPU_EFFECT_dDIFF = 0x00000000);
	TWICE(PSXREG_SPU_EFFECT_mAPF1 = 0x00010001);
	TWICE(PSXREG_SPU_EFFECT_mAPF2 = 0x00010001);
	TWICE(PSXREG_SPU_EFFECT_vIN = 0x00000000);
}

