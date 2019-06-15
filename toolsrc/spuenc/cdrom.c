/*
spuenc: SPU/XA-ADPCM audio encoder
Copyright (c) 2019 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell
*/

#include "common.h"

void init_sector_buffer(uint8_t *buffer, settings_t *settings) {
	memset(buffer,0,2352);
	memset(buffer+0x001,0xFF,10);
	buffer[0x00F] = 0x02;
	buffer[0x010] = settings->file_number;
	buffer[0x011] = settings->channel_number & 0x1F;
	buffer[0x012] = 0x24 | 0x40;
	buffer[0x013] =
		(settings->stereo ? 1 : 0)
		| (settings->frequency >= FREQ_DOUBLE ? 0 : 4)
		| (settings->bits_per_sample >= 8 ? 16 : 0);
	memcpy(buffer + 0x014, buffer + 0x010, 4);
}

void calculate_edc_xa(uint8_t *buffer)
{
	uint32_t edc = 0;
	for (int i = 0x010; i < 0x92C; i++) {
		edc ^= 0xFF&(uint32_t)buffer[i];
		for (int ibit = 0; ibit < 8; ibit++) {
			edc = (edc>>1)^(0xD8018001*(edc&0x1));
		}
	}
	buffer[0x92C] = (uint8_t)(edc);
	buffer[0x92D] = (uint8_t)(edc >> 8);
	buffer[0x92E] = (uint8_t)(edc >> 16);
	buffer[0x92F] = (uint8_t)(edc >> 24);
}
