/*
psxavenc: MDEC video + SPU/XA-ADPCM audio encoder frontend
Copyright (c) 2019, 2020 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell
*/

#include "common.h"

void init_sector_buffer_video(uint8_t *buffer, settings_t *settings) {
	memset(buffer,0,2352);
	memset(buffer+0x001,0xFF,10);

	buffer[0x00F] = 0x02;
	buffer[0x010] = settings->file_number;
	buffer[0x011] = settings->channel_number & 0x1F;
	buffer[0x012] = 0x08 | 0x40;
	buffer[0x013] = 0x00;
	memcpy(buffer + 0x014, buffer + 0x010, 4);
}

void calculate_edc_data(uint8_t *buffer)
{
	uint32_t edc = 0;
	for (int i = 0x010; i < 0x818; i++) {
		edc ^= 0xFF&(uint32_t)buffer[i];
		for (int ibit = 0; ibit < 8; ibit++) {
			edc = (edc>>1)^(0xD8018001*(edc&0x1));
		}
	}
	buffer[0x818] = (uint8_t)(edc);
	buffer[0x819] = (uint8_t)(edc >> 8);
	buffer[0x81A] = (uint8_t)(edc >> 16);
	buffer[0x81B] = (uint8_t)(edc >> 24);

	// TODO: ECC
}
