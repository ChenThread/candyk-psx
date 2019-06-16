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
	int sample_jump = (settings->bits_per_sample == 8) ? 112 : 224;

	init_sector_buffer(buffer, settings, false);

	for (int i = 0, j = 0; i < audio_sample_count; i += sample_jump, j++) {
		uint8_t *data = buffer + 0x18 + ((j%18) * 0x80);

		encode_block_xa(audio_samples + i, data, settings);

		memcpy(data + 4, data, 4);
		memcpy(data + 12, data + 8, 4);

		if ((i + sample_jump) >= audio_sample_count) {
			// next written buffer is final
			buffer[0x12] |= 0x80;
			buffer[0x16] |= 0x80;
		}

		if ((j+1)%18 == 0 || i + sample_jump >= audio_sample_count) {
			calculate_edc_xa(buffer);
			WRITE_BUFFER();
		}
	}
}

void encode_file_str(int16_t *audio_samples, int audio_sample_count, uint8_t *video_frames, int video_frame_count, settings_t *settings, FILE *output) {
	uint8_t buffer[2352*8];
	int sample_jump = (settings->bits_per_sample == 8) ? 112 : 224;

	settings->state_vid.frame_index = 0;
	settings->state_vid.bits_value = 0;
	settings->state_vid.bits_left = 16;
	settings->state_vid.frame_block_index = 0;
	settings->state_vid.frame_block_count = 0;
	// 8.75
	settings->state_vid.frame_block_base_overflow = 8*4 + 3;
	settings->state_vid.frame_block_overflow_den = 4;
	settings->state_vid.frame_block_overflow_num = 0;

	init_sector_buffer(buffer + 2352*7, settings, false);
	for (int i = 0, j = 0; i < audio_sample_count; i += sample_jump, j++) {

		uint8_t *data = buffer + 2352*7 + 0x18 + ((j%18) * 0x80);

		encode_block_xa(audio_samples + i, data, settings);

		memcpy(data + 4, data, 4);
		memcpy(data + 12, data + 8, 4);

		if ((i + sample_jump) >= audio_sample_count) {
			// next written buffer is final
			buffer[2352*7 + 0x12] |= 0x80;
			buffer[2352*7 + 0x16] |= 0x80;
		}

		if ((j+1)%18 == 0 || i + sample_jump >= audio_sample_count) {
			for(int k = 0; k < 7; k++) {
				init_sector_buffer(buffer + 2352*k, settings, true);
			}
			encode_block_str(video_frames, video_frame_count, buffer, settings);
			for(int k = 0; k < 8; k++) {
				int t = k + (j/18)*8 + 75*2;

				// Put the time in
				buffer[0x00C + 2352*k] = ((t/75/60)%10)|(((t/75/60)/10)<<4);
				buffer[0x00D + 2352*k] = (((t/75)%60)%10)|((((t/75)%60)/10)<<4);
				buffer[0x00E + 2352*k] = ((t%75)%10)|(((t%75)/10)<<4);

				if(k != 7) {
					calculate_edc_data(buffer + 2352*k);
				}
			}
			calculate_edc_xa(buffer + 2352*7);
			fwrite(buffer, 2352*8, 1, output);
			init_sector_buffer(buffer + 2352*7, settings, false);
		}
	}
}
