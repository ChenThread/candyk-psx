/*
psxavenc: MDEC video + SPU/XA-ADPCM audio encoder
Copyright (c) 2019 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell
*/

#include "common.h"

static void flush_bits(uint8_t *buffer, int buffer_size, int *bytes_used, vid_encoder_state_t *state)
{
	if(state->bits_left < 16) {
		assert(*bytes_used < buffer_size);
		buffer[(*bytes_used)++] = (uint8_t)state->bits_value;
		assert(*bytes_used < buffer_size);
		buffer[(*bytes_used)++] = (uint8_t)(state->bits_value>>8);
	}
	state->bits_left = 16;
	state->bits_value = 0;
}

static void encode_bits(uint8_t *buffer, int buffer_size, int *bytes_used, vid_encoder_state_t *state, int bits, uint32_t val)
{
	assert(val < (1<<bits));

	while(bits > state->bits_left) {
		state->bits_value |= (uint16_t)((val<<state->bits_left)>>bits);
		bits -= state->bits_left;
		//val >>= state->bits_left;
		flush_bits(buffer, buffer_size, bytes_used, state);
	}

	state->bits_value |= (uint16_t)((val<<state->bits_left)>>bits);
	state->bits_left -= bits;
	//val >>= bits;
}

static int encode_dct_block(uint8_t *buffer, int buffer_size, int *bytes_used, vid_encoder_state_t *state, bool is_luma)
{
	int dc_value = 0;

	if(is_luma) {
		// -0x200 == black, 0x1FF = white
		dc_value -= 0x200;
	}

	encode_bits(buffer, buffer_size, bytes_used, state, 10, dc_value&0x3FF);
	encode_bits(buffer, buffer_size, bytes_used, state, 2, 0x2);
	return 2;
}

void encode_block_str(uint8_t *video_frames, uint8_t *output, settings_t *settings)
{
	uint8_t buffer[2016*7];
	uint8_t header[32];

	memset(buffer, 0, sizeof(buffer));
	memset(header, 0, sizeof(header));

	uint32_t bytes_used = 8;
	uint32_t blocks_used = 0;
	uint32_t uncomp_hwords_used = 0;
	uint32_t quant_scale = 1;

	// TODO: non-16x16-aligned videos
	assert((settings->video_width % 16) == 0);
	assert((settings->video_height % 16) == 0);

	// TEST: Blank frames.
	for(int fx = 0; fx < settings->video_width; fx += 16) {
	for(int fy = 0; fy < settings->video_height; fy += 16) {
		// Order: Cr Cb [Y1|Y2\nY3|Y4]
		for(int i = 0; i < 6; i++) {
			uncomp_hwords_used += encode_dct_block(buffer, sizeof(buffer), &bytes_used, &(settings->state_vid), (i >= 2));
			uncomp_hwords_used = (uncomp_hwords_used+0xF)&~0xF;
		}
	}
	}
	encode_bits(buffer, sizeof(buffer), &bytes_used, &(settings->state_vid), 10, 0x1FF);
	encode_bits(buffer, sizeof(buffer), &bytes_used, &(settings->state_vid), 2, 0x2);
	uncomp_hwords_used += 2;
	uncomp_hwords_used = (uncomp_hwords_used+0xF)&~0xF;

	flush_bits(buffer, sizeof(buffer), &bytes_used, &(settings->state_vid));

	blocks_used = ((uncomp_hwords_used+0xF)&~0xF)>>4;

	// We need a multiple of 4
	bytes_used = (bytes_used+0x3)&~0x3;

	// Build the demuxed header
	buffer[0x000] = (uint8_t)blocks_used;
	buffer[0x001] = (uint8_t)(blocks_used>>8);
	buffer[0x002] = (uint8_t)0x00;
	buffer[0x003] = (uint8_t)0x38;
	buffer[0x004] = (uint8_t)quant_scale;
	buffer[0x005] = (uint8_t)(quant_scale>>8);
	buffer[0x006] = 0x02; // Version 2
	buffer[0x007] = 0x00;

	for(int i = 0; i < 7; i++) {
		// Header: MDEC0 register
		header[0x000] = 0x60;
		header[0x001] = 0x01;
		header[0x002] = 0x01;
		header[0x003] = 0x80;

		// Muxed chunk index/count
		int chunk_index = i;
		int chunk_count = 7;
		header[0x004] = (uint8_t)chunk_index;
		header[0x005] = (uint8_t)(chunk_index>>8);
		header[0x006] = (uint8_t)chunk_count;
		header[0x007] = (uint8_t)(chunk_count>>8);

		// Frame index
		header[0x008] = (uint8_t)settings->state_vid.frame_index;
		header[0x009] = (uint8_t)(settings->state_vid.frame_index>>8);
		header[0x00A] = (uint8_t)(settings->state_vid.frame_index>>16);
		header[0x00B] = (uint8_t)(settings->state_vid.frame_index>>24);

		// Video frame size
		header[0x010] = (uint8_t)settings->video_width;
		header[0x011] = (uint8_t)(settings->video_width>>8);
		header[0x012] = (uint8_t)settings->video_height;
		header[0x013] = (uint8_t)(settings->video_height>>8);

		// 32-byte blocks required for MDEC data
		header[0x014] = (uint8_t)blocks_used;
		header[0x015] = (uint8_t)(blocks_used>>8);

		// Some weird thing
		header[0x016] = 0x00;
		header[0x017] = 0x38;

		// Quantization scale
		header[0x018] = (uint8_t)quant_scale;
		header[0x019] = (uint8_t)(quant_scale>>8);

		// Version
		header[0x01A] = 0x02; // Version 2
		header[0x01B] = 0x00;

		// Demuxed bytes used as a multiple of 4
		header[0x00C] = (uint8_t)bytes_used;
		header[0x00D] = (uint8_t)(bytes_used>>8);
		header[0x00E] = (uint8_t)(bytes_used>>16);
		header[0x00F] = (uint8_t)(bytes_used>>24);

		memcpy(output + 2352*i + 0x018, header, sizeof(header));
		memcpy(output + 2352*i + 0x018 + 0x020, buffer + 2016*i, 2016);
	}

	settings->state_vid.frame_index++;
}
