/*
psxavenc: MDEC video + SPU/XA-ADPCM audio encoder frontend

Copyright (c) 2019, 2020 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell

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

#include "common.h"
#include "libpsxav.h"

static psx_audio_xa_settings_t settings_to_libpsxav_xa_audio(settings_t *settings) {
	psx_audio_xa_settings_t new_settings;
	new_settings.bits_per_sample = settings->bits_per_sample;
	new_settings.frequency = settings->frequency;
	new_settings.stereo = settings->channels == 2;
	new_settings.file_number = settings->file_number;
	new_settings.channel_number = settings->channel_number;

	switch (settings->format) {
		case FORMAT_XA:
		case FORMAT_STR2:
			new_settings.format = PSX_AUDIO_XA_FORMAT_XA;
			break;
		default:
			new_settings.format = PSX_AUDIO_XA_FORMAT_XACD;
			break;
	}

	return new_settings;
};

void write_vag_header(int size_per_channel, uint8_t *header, settings_t *settings) {
	// Magic
	header[0x00] = 'V';
	header[0x01] = 'A';
	header[0x02] = 'G';
	header[0x03] = settings->interleave ? 'i' : 'p';

	// Version (big-endian)
	header[0x04] = 0x00;
	header[0x05] = 0x00;
	header[0x06] = 0x00;
	header[0x07] = 0x20;

	// Interleave (little-endian)
	header[0x08] = (uint8_t)settings->interleave;
	header[0x09] = (uint8_t)(settings->interleave>>8);
	header[0x0a] = (uint8_t)(settings->interleave>>16);
	header[0x0b] = (uint8_t)(settings->interleave>>24);

	// Length of data for each channel (big-endian)
	header[0x0c] = (uint8_t)(size_per_channel>>24);
	header[0x0d] = (uint8_t)(size_per_channel>>16);
	header[0x0e] = (uint8_t)(size_per_channel>>8);
	header[0x0f] = (uint8_t)size_per_channel;

	// Sample rate (big-endian)
	header[0x10] = (uint8_t)(settings->frequency>>24);
	header[0x11] = (uint8_t)(settings->frequency>>16);
	header[0x12] = (uint8_t)(settings->frequency>>8);
	header[0x13] = (uint8_t)settings->frequency;

	// Number of channels (little-endian)
	header[0x1e] = settings->channels;
	header[0x1f] = 0x00;

	// Filename
	//strncpy(header + 0x20, "psxavenc", 16);
}

void encode_file_spu(int16_t *audio_samples, int audio_sample_count, settings_t *settings, FILE *output) {
	psx_audio_encoder_channel_state_t audio_state;	
	int audio_samples_per_block = psx_audio_spu_get_samples_per_block();
	int block_count = (audio_sample_count + audio_samples_per_block - 1) / audio_samples_per_block;
	uint8_t buffer[16];

	memset(&audio_state, 0, sizeof(psx_audio_encoder_channel_state_t));

	if (settings->format == FORMAT_VAG) {
		uint8_t header[48];
		memset(header, 0, 48);
		write_vag_header(block_count * 16, header, settings);
		fwrite(header, 48, 1, output);
	}

	for (int i = 0; i < audio_sample_count; i += audio_samples_per_block) {
		int samples_length = audio_sample_count - i;
		if (samples_length > audio_samples_per_block) samples_length = audio_samples_per_block;

		int length = psx_audio_spu_encode(&audio_state, audio_samples + i, samples_length, 1, buffer);
		if (i == 0) {
			// The SPU already resets the loop address when starting playback of a sample
			//buffer[1] = PSX_AUDIO_SPU_LOOP_START;
		} else if ((i + audio_samples_per_block) >= audio_sample_count) {
			buffer[1] = settings->loop ? PSX_AUDIO_SPU_LOOP_REPEAT : PSX_AUDIO_SPU_LOOP_END;
		}
		fwrite(buffer, length, 1, output);
	}
}

void encode_file_spu_interleaved(int16_t *audio_samples, int audio_sample_count, settings_t *settings, FILE *output) {
	int audio_state_size = sizeof(psx_audio_encoder_channel_state_t) * settings->channels;
	int buffer_size = (settings->interleave + 2047) & ~2047;
	psx_audio_encoder_channel_state_t *audio_state = malloc(audio_state_size);
	uint8_t *buffer = malloc(buffer_size);
	int audio_samples_per_block = psx_audio_spu_get_samples_per_block();
	int block_count = (audio_sample_count + audio_samples_per_block - 1) / audio_samples_per_block;
	int audio_samples_per_chunk = (settings->interleave + 15) / 16 * audio_samples_per_block;

	memset(audio_state, 0, audio_state_size);

	if (settings->format == FORMAT_VAGI) {
		uint8_t header[2048];
		memset(header, 0, 2048);
		write_vag_header(block_count * 16, header, settings);
		fwrite(header, 2048, 1, output);
	}

	for (int i = 0; i < audio_sample_count; i += audio_samples_per_chunk) {
		int samples_length = audio_sample_count - i;
		if (samples_length > audio_samples_per_chunk) samples_length = audio_samples_per_chunk;

		for (int ch = 0; ch < settings->channels; ch++) {
			memset(buffer, 0, buffer_size);
			int length = psx_audio_spu_encode(audio_state + ch, audio_samples + i * settings->channels + ch, samples_length, settings->channels, buffer);
			// The SPU already resets the loop address when starting playback of a sample
			//buffer[1] = PSX_AUDIO_SPU_LOOP_START;
			if (settings->loop) {
				buffer[length - 16 + 1] = PSX_AUDIO_SPU_LOOP_REPEAT;
			} else if ((i + audio_samples_per_chunk) >= audio_sample_count) {
				buffer[length - 16 + 1] = PSX_AUDIO_SPU_LOOP_END;
			}
			fwrite(buffer, buffer_size, 1, output);
		}
	}

	free(audio_state);
	free(buffer);
}

void encode_file_xa(int16_t *audio_samples, int audio_sample_count, settings_t *settings, FILE *output) {
	psx_audio_xa_settings_t xa_settings = settings_to_libpsxav_xa_audio(settings);
	psx_audio_encoder_state_t audio_state;	
	int audio_samples_per_sector = psx_audio_xa_get_samples_per_sector(xa_settings);
	uint8_t buffer[2352];

	memset(&audio_state, 0, sizeof(psx_audio_encoder_state_t));

	for (int i = 0; i < audio_sample_count; i += audio_samples_per_sector) {
		int samples_length = audio_sample_count - i;
		if (samples_length > audio_samples_per_sector) samples_length = audio_samples_per_sector;
		int length = psx_audio_xa_encode(xa_settings, &audio_state, audio_samples + (i * settings->channels), samples_length, buffer);
		if ((i + audio_samples_per_sector) >= audio_sample_count) {
			psx_audio_xa_encode_finalize(xa_settings, buffer, length);
		}
		fwrite(buffer, length, 1, output);
	}
}

void encode_file_str(settings_t *settings, FILE *output) {
	uint8_t buffer[2352*8];
	psx_audio_xa_settings_t xa_settings = settings_to_libpsxav_xa_audio(settings);
	psx_audio_encoder_state_t audio_state;
	int sector_size = psx_audio_xa_get_buffer_size_per_sector(xa_settings);
	int audio_samples_per_sector = psx_audio_xa_get_samples_per_sector(xa_settings);

	memset(&audio_state, 0, sizeof(psx_audio_encoder_state_t));

	settings->state_vid.frame_index = 0;
	settings->state_vid.bits_value = 0;
	settings->state_vid.bits_left = 16;
	settings->state_vid.frame_block_index = 0;
	settings->state_vid.frame_block_count = 0;

	settings->state_vid.frame_block_overflow_num = 0;

	// Number of total sectors per second: 150
	// Proportion of sectors for video due to A/V interleave: 7/8
	// 15FPS = (150*7/8/15) = 8.75 blocks per frame
	settings->state_vid.frame_block_base_overflow = 150*7*settings->video_fps_den;
	settings->state_vid.frame_block_overflow_den = 8*settings->video_fps_num;
	//fprintf(stderr, "%f\n", ((double)settings->state_vid.frame_block_base_overflow)/((double)settings->state_vid.frame_block_overflow_den)); abort();

	// FIXME: this needs an extra frame to prevent A/V desync
	const int frames_needed = 2;
	for (int j = 0; ensure_av_data(settings, audio_samples_per_sector*settings->channels*frames_needed, 1*frames_needed); j+=18) {
		psx_audio_xa_encode(xa_settings, &audio_state, settings->audio_samples, audio_samples_per_sector, buffer + sector_size * 7);
		
		// TODO: the final buffer
		for(int k = 0; k < 7; k++) {
			init_sector_buffer_video(buffer + sector_size*k, settings);
		}
		encode_block_str(settings->video_frames, settings->video_frame_count, buffer, settings);

		if (settings->format == FORMAT_STR2CD) {
			for(int k = 0; k < 8; k++) {
				int t = k + (j/18)*8 + 75*2;

				// Put the time in
				buffer[0x00C + 2352*k] = ((t/75/60)%10)|(((t/75/60)/10)<<4);
				buffer[0x00D + 2352*k] = (((t/75)%60)%10)|((((t/75)%60)/10)<<4);
				buffer[0x00E + 2352*k] = ((t%75)%10)|(((t%75)/10)<<4);

				// FIXME: EDC is not calculated in 2336-byte sector mode
				// (shouldn't matter anyway, any CD image builder will
				// have to recalculate it due to the sector's MSF changing)
				if(k != 7) {
					calculate_edc_data(buffer + 2352*k);
				}
			}
		}
		retire_av_data(settings, audio_samples_per_sector*settings->channels, 0);
		fwrite(buffer, sector_size*8, 1, output);
	}
}
