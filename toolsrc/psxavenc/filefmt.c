/*
psxavenc: MDEC video + SPU/XA-ADPCM audio encoder
Copyright (c) 2019 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell
*/

#include "common.h"

void encode_file_spu(int16_t *audio_samples, int audio_sample_count, settings_t *settings, FILE *output) {
	uint8_t prebuf[28];
	uint8_t buffer[16];
	uint8_t *data;

	for (int i = 0; i < audio_sample_count; i += 28) {
		buffer[0] = encode_nibbles(&(settings->state_left), audio_samples + i, 1, prebuf, 0, 1, SPU_ADPCM_FILTER_COUNT);
		for (int j = 0; j < 28; j+=2) {
			buffer[2 + (j>>1)] = (prebuf[j] & 0x0F) | (prebuf[j+1] << 4);
		}

		buffer[1] = 0;
		buffer[1] |= (i == 0) ? 4 : 0;
		buffer[1] |= ((i + 28) >= audio_sample_count) ? 1 : 0;

		fwrite(buffer, 16, 1, output);
	}
}

static void encode_block_xa(int16_t *audio_samples, uint8_t *data, settings_t *settings) {
	if (settings->bits_per_sample == 4) {
		if (settings->stereo) {
			data[0]  = encode_nibbles(&(settings->state_left), audio_samples,            2, data + 0x10, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[1]  = encode_nibbles(&(settings->state_right), audio_samples + 1,        2, data + 0x10, 4, 4, XA_ADPCM_FILTER_COUNT);
			data[2]  = encode_nibbles(&(settings->state_left), audio_samples + 56,       2, data + 0x11, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[3]  = encode_nibbles(&(settings->state_right), audio_samples + 56 + 1,   2, data + 0x11, 4, 4, XA_ADPCM_FILTER_COUNT);
			data[8]  = encode_nibbles(&(settings->state_left), audio_samples + 56*2,     2, data + 0x12, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[9]  = encode_nibbles(&(settings->state_right), audio_samples + 56*2 + 1, 2, data + 0x12, 4, 4, XA_ADPCM_FILTER_COUNT);
			data[10] = encode_nibbles(&(settings->state_left), audio_samples + 56*3,     2, data + 0x13, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[11] = encode_nibbles(&(settings->state_right), audio_samples + 56*3 + 1, 2, data + 0x13, 4, 4, XA_ADPCM_FILTER_COUNT);
		} else {
			data[0]  = encode_nibbles(&(settings->state_left), audio_samples,            1, data + 0x10, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[1]  = encode_nibbles(&(settings->state_right), audio_samples + 28,       1, data + 0x10, 4, 4, XA_ADPCM_FILTER_COUNT);
			data[2]  = encode_nibbles(&(settings->state_left), audio_samples + 28*2,     1, data + 0x11, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[3]  = encode_nibbles(&(settings->state_right), audio_samples + 28*3,     1, data + 0x11, 4, 4, XA_ADPCM_FILTER_COUNT);
			data[8]  = encode_nibbles(&(settings->state_left), audio_samples + 28*4,     1, data + 0x12, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[9]  = encode_nibbles(&(settings->state_right), audio_samples + 28*5,     1, data + 0x12, 4, 4, XA_ADPCM_FILTER_COUNT);
			data[10] = encode_nibbles(&(settings->state_left), audio_samples + 28*6,     1, data + 0x13, 0, 4, XA_ADPCM_FILTER_COUNT);
			data[11] = encode_nibbles(&(settings->state_right), audio_samples + 28*7,     1, data + 0x13, 4, 4, XA_ADPCM_FILTER_COUNT);
		}
	} else {
/*		if (settings->stereo) {
			data[0]  = encode_bytes(audio_samples,            2, data + 0x10);
			data[1]  = encode_bytes(audio_samples + 1,        2, data + 0x11);
			data[2]  = encode_bytes(audio_samples + 56,       2, data + 0x12);
			data[3]  = encode_bytes(audio_samples + 57,       2, data + 0x13);
		} else {
			data[0]  = encode_bytes(audio_samples,            1, data + 0x10);
			data[1]  = encode_bytes(audio_samples + 28,       1, data + 0x11);
			data[2]  = encode_bytes(audio_samples + 56,       1, data + 0x12);
			data[3]  = encode_bytes(audio_samples + 84,       1, data + 0x13);
		} */
	}
}

#define WRITE_BUFFER() \
	if (settings->format == FORMAT_XA) { \
		fwrite(buffer + 0x010, 2336, 1, output); \
	} else { \
		fwrite(buffer, 2352, 1, output); \
	}

void encode_file_xa(int16_t *audio_samples, int audio_sample_count, settings_t *settings, FILE *output) {
	uint8_t buffer[2352];
	uint8_t *data;
	int i, j = 0;
	int sample_jump = (settings->bits_per_sample == 8) ? 112 : 224;

	init_sector_buffer(buffer, settings);

	for (i = 0; i < audio_sample_count; i += sample_jump) {
		data = buffer + 0x18 + (j * 0x80);

		encode_block_xa(audio_samples + i, data, settings);

		memcpy(data + 4, data, 4);
		memcpy(data + 12, data + 8, 4);

		if ((i + sample_jump) >= audio_sample_count) {
			// next written buffer is final
			buffer[0x12] |= 0x80;
			buffer[0x16] |= 0x80;
		}

		if ((++j) == 18) {
			calculate_edc_xa(buffer);
			WRITE_BUFFER();
			j = 0;
		}
	}

	// write final sector
	if (j > 0) {
		calculate_edc_xa(buffer);
		WRITE_BUFFER();
	}
}

void encode_file_str(int16_t *audio_samples, int audio_sample_count, settings_t *settings, FILE *output) {
	fprintf(stderr, "encode_file_str: Not supported yet!\n");
	abort();
}
