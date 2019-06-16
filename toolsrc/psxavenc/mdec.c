/*
psxavenc: MDEC video + SPU/XA-ADPCM audio encoder
Copyright (c) 2019 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell
*/

#include "common.h"

const uint8_t dct_zigzag_table[8*8] = {
	0x00,0x01,0x05,0x06,0x0E,0x0F,0x1B,0x1C,
	0x02,0x04,0x07,0x0D,0x10,0x1A,0x1D,0x2A,
	0x03,0x08,0x0C,0x11,0x19,0x1E,0x29,0x2B,
	0x09,0x0B,0x12,0x18,0x1F,0x28,0x2C,0x35,
	0x0A,0x13,0x17,0x20,0x27,0x2D,0x34,0x36,
	0x14,0x16,0x21,0x26,0x2E,0x33,0x37,0x3C,
	0x15,0x22,0x25,0x2F,0x32,0x38,0x3B,0x3D,
	0x23,0x24,0x30,0x31,0x39,0x3A,0x3E,0x3F,
};

const uint8_t dct_zagzig_table[8*8] = {
	0x00,0x01,0x08,0x10,0x09,0x02,0x03,0x0A,
	0x11,0x18,0x20,0x19,0x12,0x0B,0x04,0x05,
	0x0C,0x13,0x1A,0x21,0x28,0x30,0x29,0x22,
	0x1B,0x14,0x0D,0x06,0x07,0x0E,0x15,0x1C,
	0x23,0x2A,0x31,0x38,0x39,0x32,0x2B,0x24,
	0x1D,0x16,0x0F,0x17,0x1E,0x25,0x2C,0x33,
	0x3A,0x3B,0x34,0x2D,0x26,0x1F,0x27,0x2E,
	0x35,0x3C,0x3D,0x36,0x2F,0x37,0x3E,0x3F,
};

const int16_t dct_scale_table[8*8] = {
	+0x5A82, +0x5A82, +0x5A82, +0x5A82, +0x5A82, +0x5A82, +0x5A82, +0x5A82,
	+0x7D8A, +0x6A6D, +0x471C, +0x18F8, -0x18F9, -0x471D, -0x6A6E, -0x7D8B,
	+0x7641, +0x30FB, -0x30FC, -0x7642, -0x7642, -0x30FC, +0x30FB, +0x7641,
	+0x6A6D, -0x18F9, -0x7D8B, -0x471D, +0x471C, +0x7D8A, +0x18F8, -0x6A6E,
	+0x5A82, -0x5A83, -0x5A83, +0x5A82, +0x5A82, -0x5A83, -0x5A83, +0x5A82,
	+0x471C, -0x7D8B, +0x18F8, +0x6A6D, -0x6A6E, -0x18F9, +0x7D8A, -0x471D,
	+0x30FB, -0x7642, +0x7641, -0x30FC, -0x30FC, +0x7641, -0x7642, +0x30FB,
	+0x18F8, -0x471D, +0x6A6D, -0x7D8B, +0x7D8A, -0x6A6E, +0x471C, -0x18F9,
};

static void flush_bits(vid_encoder_state_t *state)
{
	if(state->bits_left < 16) {
		assert(state->bytes_used < sizeof(state->unmuxed));
		state->unmuxed[state->bytes_used++] = (uint8_t)state->bits_value;
		assert(state->bytes_used < sizeof(state->unmuxed));
		state->unmuxed[state->bytes_used++] = (uint8_t)(state->bits_value>>8);
	}
	state->bits_left = 16;
	state->bits_value = 0;
}

static void encode_bits(vid_encoder_state_t *state, int bits, uint32_t val)
{
	assert(val < (1<<bits));

	while(bits > state->bits_left) {
		state->bits_value |= (uint16_t)((val<<state->bits_left)>>bits);
		bits -= state->bits_left;
		//val >>= state->bits_left;
		flush_bits(state);
	}

	state->bits_value |= (uint16_t)((val<<state->bits_left)>>bits);
	state->bits_left -= bits;
	//val >>= bits;
}

static void encode_dct_block(vid_encoder_state_t *state, int32_t *block, bool is_luma)
{
	int dc_value = 0;

	// Apply DCT to block
	int32_t midblock[8*8];

	for (int reps = 0; reps < 2; reps++) {
		for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			int32_t v = 0;
			for(int k = 0; k < 8; k++) {
				v += block[8*i+k]*dct_scale_table[8*i+k];
			}
			midblock[8*j+i] = (v + (1<<((14+2)-1)))>>(14+2);
		}
		}
		memcpy(block, midblock, sizeof(midblock));
	}

	for (int i = 0; i < 64; i++) {
		if (block[i] < -0x200) { block[i] = -0x200; }
		if (block[i] > +0x1FF) { block[i] = +0x1FF; }
	}

	dc_value = block[0];
	//printf("dc %08X\n", dc_value);
	//assert(dc_value >= -0x200); assert(dc_value <  +0x200);

	encode_bits(state, 10, dc_value&0x3FF);
	encode_bits(state, 2, 0x2);
	state->uncomp_hwords_used += 2;

	state->uncomp_hwords_used = (state->uncomp_hwords_used+0xF)&~0xF;
}

void encode_block_str(uint8_t *video_frames, int video_frame_count, uint8_t *output, settings_t *settings)
{
	uint8_t header[32];
	int pitch = settings->video_width*4;
	int real_index = (settings->state_vid.frame_index-1);
	if (real_index > video_frame_count-1) {
		real_index = video_frame_count-1;
	}
	uint8_t *video_frame = video_frames + settings->video_width*settings->video_height*4*real_index;

	memset(settings->state_vid.unmuxed, 0, sizeof(settings->state_vid.unmuxed));
	memset(header, 0, sizeof(header));

	settings->state_vid.quant_scale = 1;
	settings->state_vid.uncomp_hwords_used = 0;
	settings->state_vid.bytes_used = 8;
	settings->state_vid.blocks_used = 0;

	// TODO: non-16x16-aligned videos
	assert((settings->video_width % 16) == 0);
	assert((settings->video_height % 16) == 0);

	// TEST: Blank frames.
	for(int fx = 0; fx < settings->video_width; fx += 16) {
	for(int fy = 0; fy < settings->video_height; fy += 16) {
		// Order: Cr Cb [Y1|Y2\nY3|Y4]
		int32_t blocks[6][8*8];
		for(int y = 0; y < 8; y++) {
		for(int x = 0; x < 8; x++) {
			int k = y*8+x;

			int cr = 0;
			int cg = 0;
			int cb = 0;
			for(int cy = 0; cy < 2; cy++) {
			for(int cx = 0; cx < 2; cx++) {
				int coffs = pitch*(fy+y*2+cy) + 4*(fx+x*2+cx);
				cr += video_frame[coffs+0];
				cg += video_frame[coffs+1];
				cb += video_frame[coffs+2];
			}
			}

			// TODO: Get the real math for this
			int cluma = cr+cg*2+cb;
			blocks[0][k] = ((cr<<2) - cluma)>>2;
			blocks[1][k] = ((cb<<2) - cluma)>>2;

			for(int ly = 0; ly < 2; ly++) {
			for(int lx = 0; lx < 2; lx++) {
				int loffs = pitch*(fy+ly*8+y) + 4*(fx+lx*8+x);
				int lr = video_frame[loffs+0];
				int lg = video_frame[loffs+1];
				int lb = video_frame[loffs+2];

				// TODO: Get the real math for this
				int lluma = (lr+lg*2+lb+2)-0x200;
				blocks[2+2*ly+lx][k] = lluma;
			}
			}
		}
		}
		for(int i = 0; i < 6; i++) {
			encode_dct_block(&(settings->state_vid), blocks[i], (i >= 2));
		}
	}
	}
	encode_bits(&(settings->state_vid), 10, 0x1FF);
	encode_bits(&(settings->state_vid), 2, 0x2);
	settings->state_vid.uncomp_hwords_used += 2;
	settings->state_vid.uncomp_hwords_used = (settings->state_vid.uncomp_hwords_used+0xF)&~0xF;

	flush_bits(&(settings->state_vid));

	settings->state_vid.blocks_used = ((settings->state_vid.uncomp_hwords_used+0xF)&~0xF)>>4;

	// We need a multiple of 4
	settings->state_vid.bytes_used = (settings->state_vid.bytes_used+0x3)&~0x3;

	// Build the demuxed header
	settings->state_vid.unmuxed[0x000] = (uint8_t)settings->state_vid.blocks_used;
	settings->state_vid.unmuxed[0x001] = (uint8_t)(settings->state_vid.blocks_used>>8);
	settings->state_vid.unmuxed[0x002] = (uint8_t)0x00;
	settings->state_vid.unmuxed[0x003] = (uint8_t)0x38;
	settings->state_vid.unmuxed[0x004] = (uint8_t)settings->state_vid.quant_scale;
	settings->state_vid.unmuxed[0x005] = (uint8_t)(settings->state_vid.quant_scale>>8);
	settings->state_vid.unmuxed[0x006] = 0x02; // Version 2
	settings->state_vid.unmuxed[0x007] = 0x00;

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
		header[0x014] = (uint8_t)settings->state_vid.blocks_used;
		header[0x015] = (uint8_t)(settings->state_vid.blocks_used>>8);

		// Some weird thing
		header[0x016] = 0x00;
		header[0x017] = 0x38;

		// Quantization scale
		header[0x018] = (uint8_t)settings->state_vid.quant_scale;
		header[0x019] = (uint8_t)(settings->state_vid.quant_scale>>8);

		// Version
		header[0x01A] = 0x02; // Version 2
		header[0x01B] = 0x00;

		// Demuxed bytes used as a multiple of 4
		header[0x00C] = (uint8_t)settings->state_vid.bytes_used;
		header[0x00D] = (uint8_t)(settings->state_vid.bytes_used>>8);
		header[0x00E] = (uint8_t)(settings->state_vid.bytes_used>>16);
		header[0x00F] = (uint8_t)(settings->state_vid.bytes_used>>24);

		memcpy(output + 2352*i + 0x018, header, sizeof(header));
		memcpy(output + 2352*i + 0x018 + 0x020, settings->state_vid.unmuxed + 2016*i, 2016);
	}

	settings->state_vid.frame_index++;
}
