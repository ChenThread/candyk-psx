/*
libpsxav: MDEC video + SPU/XA-ADPCM audio library
Copyright (c) 2019, 2020 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell
*/

#include "libpsxav.h"

static uint32_t psx_cdrom_calculate_edc(uint8_t *sector, uint32_t offset, uint32_t size)
{
	uint32_t edc = 0;
	for (int i = offset; i < offset+size; i++) {
		edc ^= 0xFF&(uint32_t)sector[i];
		for (int ibit = 0; ibit < 8; ibit++) {
			edc = (edc>>1)^(0xD8018001*(edc&0x1));
		}
	}
	return edc;
}

void psx_cdrom_calculate_checksums(uint8_t *sector, psx_cdrom_sector_type_t type)
{
	switch (type) {
		case PSX_CDROM_SECTOR_TYPE_MODE2_FORM1: {
			uint32_t edc = psx_cdrom_calculate_edc(sector, 0x10, 0x808);
			sector[0x818] = (uint8_t)(edc);
			sector[0x819] = (uint8_t)(edc >> 8);
			sector[0x81A] = (uint8_t)(edc >> 16);
			sector[0x81B] = (uint8_t)(edc >> 24);

			// TODO: ECC
		} break;
		case PSX_CDROM_SECTOR_TYPE_MODE2_FORM2: {
			uint32_t edc = psx_cdrom_calculate_edc(sector, 0x10, 0x91C);
			sector[0x92C] = (uint8_t)(edc);
			sector[0x92D] = (uint8_t)(edc >> 8);
			sector[0x92E] = (uint8_t)(edc >> 16);
			sector[0x92F] = (uint8_t)(edc >> 24);
		} break;
	}
}