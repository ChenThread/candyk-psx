/*
psxavenc: MDEC video + SPU/XA-ADPCM audio encoder
Copyright (c) 2019 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell
*/

#include "common.h"

#define MAKE_HUFFMAN_PAIR(zeroes, value) (((zeroes)<<10)|((+(value))&0x3FF)),(((zeroes)<<10)|((-(value))&0x3FF))
const struct {
	int c_bits;
	uint32_t c_value;
	uint16_t u_hword_pos;
	uint16_t u_hword_neg;
} huffman_lookup[] = {
	// Fuck this Huffman tree in particular --GM
	2,0x3,MAKE_HUFFMAN_PAIR(0,1),
	3,0x3,MAKE_HUFFMAN_PAIR(1,1),
	4,0x4,MAKE_HUFFMAN_PAIR(0,2),
	4,0x5,MAKE_HUFFMAN_PAIR(2,1),
	5,0x05,MAKE_HUFFMAN_PAIR(0,3),
	5,0x06,MAKE_HUFFMAN_PAIR(4,1),
	5,0x07,MAKE_HUFFMAN_PAIR(3,1),
	6,0x04,MAKE_HUFFMAN_PAIR(7,1),
	6,0x05,MAKE_HUFFMAN_PAIR(6,1),
	6,0x06,MAKE_HUFFMAN_PAIR(1,2),
	6,0x07,MAKE_HUFFMAN_PAIR(5,1),
	7,0x04,MAKE_HUFFMAN_PAIR(2,2),
	7,0x05,MAKE_HUFFMAN_PAIR(9,1),
	7,0x06,MAKE_HUFFMAN_PAIR(0,4),
	7,0x07,MAKE_HUFFMAN_PAIR(8,1),
	8,0x20,MAKE_HUFFMAN_PAIR(13,1),
	8,0x21,MAKE_HUFFMAN_PAIR(0,6),
	8,0x22,MAKE_HUFFMAN_PAIR(12,1),
	8,0x23,MAKE_HUFFMAN_PAIR(11,1),
	8,0x24,MAKE_HUFFMAN_PAIR(3,2),
	8,0x25,MAKE_HUFFMAN_PAIR(1,3),
	8,0x26,MAKE_HUFFMAN_PAIR(0,5),
	8,0x27,MAKE_HUFFMAN_PAIR(10,1),
	10,0x008,MAKE_HUFFMAN_PAIR(16,1),
	10,0x009,MAKE_HUFFMAN_PAIR(5,2),
	10,0x00A,MAKE_HUFFMAN_PAIR(0,7),
	10,0x00B,MAKE_HUFFMAN_PAIR(2,3),
	10,0x00C,MAKE_HUFFMAN_PAIR(1,4),
	10,0x00D,MAKE_HUFFMAN_PAIR(15,1),
	10,0x00E,MAKE_HUFFMAN_PAIR(14,1),
	10,0x00F,MAKE_HUFFMAN_PAIR(4,2),
	12,0x010,MAKE_HUFFMAN_PAIR(0,11),
	12,0x011,MAKE_HUFFMAN_PAIR(8,2),
	12,0x012,MAKE_HUFFMAN_PAIR(4,3),
	12,0x013,MAKE_HUFFMAN_PAIR(0,10),
	12,0x014,MAKE_HUFFMAN_PAIR(2,4),
	12,0x015,MAKE_HUFFMAN_PAIR(7,2),
	12,0x016,MAKE_HUFFMAN_PAIR(21,1),
	12,0x017,MAKE_HUFFMAN_PAIR(20,1),
	12,0x018,MAKE_HUFFMAN_PAIR(0,9),
	12,0x019,MAKE_HUFFMAN_PAIR(19,1),
	12,0x01A,MAKE_HUFFMAN_PAIR(18,1),
	12,0x01B,MAKE_HUFFMAN_PAIR(1,5),
	12,0x01C,MAKE_HUFFMAN_PAIR(3,3),
	12,0x01D,MAKE_HUFFMAN_PAIR(0,8),
	12,0x01E,MAKE_HUFFMAN_PAIR(6,2),
	12,0x01F,MAKE_HUFFMAN_PAIR(17,1),
	13,0x0010,MAKE_HUFFMAN_PAIR(10,2),
	13,0x0011,MAKE_HUFFMAN_PAIR(9,2),
	13,0x0012,MAKE_HUFFMAN_PAIR(5,3),
	13,0x0013,MAKE_HUFFMAN_PAIR(3,4),
	13,0x0014,MAKE_HUFFMAN_PAIR(2,5),
	13,0x0015,MAKE_HUFFMAN_PAIR(1,7),
	13,0x0016,MAKE_HUFFMAN_PAIR(1,6),
	13,0x0017,MAKE_HUFFMAN_PAIR(0,15),
	13,0x0018,MAKE_HUFFMAN_PAIR(0,14),
	13,0x0019,MAKE_HUFFMAN_PAIR(0,13),
	13,0x001A,MAKE_HUFFMAN_PAIR(0,12),
	13,0x001B,MAKE_HUFFMAN_PAIR(26,1),
	13,0x001C,MAKE_HUFFMAN_PAIR(25,1),
	13,0x001D,MAKE_HUFFMAN_PAIR(24,1),
	13,0x001E,MAKE_HUFFMAN_PAIR(23,1),
	13,0x001F,MAKE_HUFFMAN_PAIR(22,1),
	14,0x0010,MAKE_HUFFMAN_PAIR(0,31),
	14,0x0011,MAKE_HUFFMAN_PAIR(0,30),
	14,0x0012,MAKE_HUFFMAN_PAIR(0,29),
	14,0x0013,MAKE_HUFFMAN_PAIR(0,28),
	14,0x0014,MAKE_HUFFMAN_PAIR(0,27),
	14,0x0015,MAKE_HUFFMAN_PAIR(0,26),
	14,0x0016,MAKE_HUFFMAN_PAIR(0,25),
	14,0x0017,MAKE_HUFFMAN_PAIR(0,24),
	14,0x0018,MAKE_HUFFMAN_PAIR(0,23),
	14,0x0019,MAKE_HUFFMAN_PAIR(0,22),
	14,0x001A,MAKE_HUFFMAN_PAIR(0,21),
	14,0x001B,MAKE_HUFFMAN_PAIR(0,20),
	14,0x001C,MAKE_HUFFMAN_PAIR(0,19),
	14,0x001D,MAKE_HUFFMAN_PAIR(0,18),
	14,0x001E,MAKE_HUFFMAN_PAIR(0,17),
	14,0x001F,MAKE_HUFFMAN_PAIR(0,16),
	15,0x0010,MAKE_HUFFMAN_PAIR(0,40),
	15,0x0011,MAKE_HUFFMAN_PAIR(0,39),
	15,0x0012,MAKE_HUFFMAN_PAIR(0,38),
	15,0x0013,MAKE_HUFFMAN_PAIR(0,37),
	15,0x0014,MAKE_HUFFMAN_PAIR(0,36),
	15,0x0015,MAKE_HUFFMAN_PAIR(0,35),
	15,0x0016,MAKE_HUFFMAN_PAIR(0,34),
	15,0x0017,MAKE_HUFFMAN_PAIR(0,33),
	15,0x0018,MAKE_HUFFMAN_PAIR(0,32),
	15,0x0019,MAKE_HUFFMAN_PAIR(1,14),
	15,0x001A,MAKE_HUFFMAN_PAIR(1,13),
	15,0x001B,MAKE_HUFFMAN_PAIR(1,12),
	15,0x001C,MAKE_HUFFMAN_PAIR(1,11),
	15,0x001D,MAKE_HUFFMAN_PAIR(1,10),
	15,0x001E,MAKE_HUFFMAN_PAIR(1,9),
	15,0x001F,MAKE_HUFFMAN_PAIR(1,8),
	16,0x0010,MAKE_HUFFMAN_PAIR(1,18),
	16,0x0011,MAKE_HUFFMAN_PAIR(1,17),
	16,0x0012,MAKE_HUFFMAN_PAIR(1,16),
	16,0x0013,MAKE_HUFFMAN_PAIR(1,15),
	16,0x0014,MAKE_HUFFMAN_PAIR(6,3),
	16,0x0015,MAKE_HUFFMAN_PAIR(16,2),
	16,0x0016,MAKE_HUFFMAN_PAIR(15,2),
	16,0x0017,MAKE_HUFFMAN_PAIR(14,2),
	16,0x0018,MAKE_HUFFMAN_PAIR(13,2),
	16,0x0019,MAKE_HUFFMAN_PAIR(12,2),
	16,0x001A,MAKE_HUFFMAN_PAIR(11,2),
	16,0x001B,MAKE_HUFFMAN_PAIR(31,1),
	16,0x001C,MAKE_HUFFMAN_PAIR(30,1),
	16,0x001D,MAKE_HUFFMAN_PAIR(29,1),
	16,0x001E,MAKE_HUFFMAN_PAIR(28,1),
	16,0x001F,MAKE_HUFFMAN_PAIR(27,1),
};
#undef MAKE_HUFFMAN_PAIR

const uint8_t quant_dec[8*8] = {
	 2, 16, 19, 22, 26, 27, 29, 34,
	16, 16, 22, 24, 27, 29, 34, 37,
	19, 22, 26, 27, 29, 34, 34, 38,
	22, 22, 26, 27, 29, 34, 37, 40,
	22, 26, 27, 29, 32, 35, 40, 48,
	26, 27, 29, 32, 35, 40, 48, 58,
	26, 27, 29, 34, 38, 46, 56, 69,
	27, 29, 35, 38, 46, 56, 69, 83,
};

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
		val &= (1<<bits)-1;
		flush_bits(state);
	}

	state->bits_value |= (uint16_t)((val<<state->bits_left)>>bits);
	state->bits_left -= bits;
}

static void encode_ac_value(vid_encoder_state_t *state, uint16_t value)
{
	assert(0 <= value && value <= 0xFFFF);

	// FIXME: this is busted
	if(false) for(int i = 0; i < sizeof(huffman_lookup)/sizeof(huffman_lookup[0]); i++) {
		if(value == huffman_lookup[i].u_hword_pos) {
			printf("unescape + %04X %2d %04X\n", value, huffman_lookup[i].c_bits, huffman_lookup[i].c_value);
			encode_bits(state, huffman_lookup[i].c_bits, huffman_lookup[i].c_value);
			encode_bits(state, 1, 0);
			return;
		}
		else if(value == huffman_lookup[i].u_hword_neg) {
			printf("unescape - %04X %2d %04X\n", value, huffman_lookup[i].c_bits, huffman_lookup[i].c_value);
			encode_bits(state, huffman_lookup[i].c_bits, huffman_lookup[i].c_value);
			encode_bits(state, 1, 1);
		}
	}

	// Use an escape
	//printf("escape %04X\n", value);
	encode_bits(state, 6, 0x01);
	encode_bits(state, 16, value);
}

static void transform_dct_block(vid_encoder_state_t *state, int32_t *block)
{
	// Apply DCT to block
	int32_t midblock[8*8];

	for (int reps = 0; reps < 2; reps++) {
		for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			int32_t v = 0;
			for(int k = 0; k < 8; k++) {
				v += block[8*j+k]*dct_scale_table[8*i+k];
			}
			midblock[8*i+j] = (v + (1<<((14)-1)))>>(14);
		}
		}
		memcpy(block, midblock, sizeof(midblock));
	}

	// FIXME: Work out why the math has to go this way
	block[0] /= 4;
	for (int i = 0; i < 64; i++) {
		// Finish reducing it
		block[i] /= 8;

		// If it's below the quantisation threshold, zero it
		if(abs(block[i]) < quant_dec[i]) {
			block[i] = 0;
		}
	}

}

static void encode_dct_block(vid_encoder_state_t *state, int32_t *block)
{
	int dc_value = 0;

	for (int i = 0; i < 64; i++) {
		// Quantise it
		block[i] = (block[i])/quant_dec[i];

		// Clamp it
		if (block[i] < -0x200) { block[i] = -0x200; }
		if (block[i] > +0x1FF) { block[i] = +0x1FF; }
	}

	// Get DC value
	dc_value = block[0];
	//dc_value = 0;
	encode_bits(state, 10, dc_value&0x3FF);

	// Build RLE output
	uint16_t zero_rle_data[8*8];
	int zero_rle_words = 0;
	for (int i = 1, zeroes = 0; i < 64; i++) {
		int ri = dct_zagzig_table[i];
		//int ri = dct_zigzag_table[i];
		if (block[ri] == 0) {
			zeroes++;
		} else {
			zero_rle_data[zero_rle_words++] = (zeroes<<10)|(block[ri]&0x3FF);
			zeroes = 0;
			state->uncomp_hwords_used += 1;
		}
	}

	// Now Huffman-code the data
	for (int i = 0; i < zero_rle_words; i++) {
		encode_ac_value(state, zero_rle_data[i]);
	}

	//printf("dc %08X rles %2d\n", dc_value, zero_rle_words);
	//assert(dc_value >= -0x200); assert(dc_value <  +0x200);

	// Store end of block
	encode_bits(state, 2, 0x2);
	state->uncomp_hwords_used += 2;

	state->uncomp_hwords_used = (state->uncomp_hwords_used+0xF)&~0xF;
}

static int reduce_dct_block(vid_encoder_state_t *state, int32_t *block, int32_t min_val, int *values_to_shed)
{
	// Reduce so it can all fit
	int nonzeroes = 0;

	for (int i = 1; i < 64; i++) {
		//int ri = dct_zigzag_table[i];
		if (block[i] != 0) {
			//if (abs(block[i])+(ri>>3) < min_val+(64>>3)) {
			if ((*values_to_shed) > 0 && abs(block[i]) < min_val*1) {
				block[i] = 0;
				(*values_to_shed)--;
			} else {
				nonzeroes++;
			}
		}
	}

	// Factor in DC + EOF values
	return nonzeroes+2;
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

	if (settings->state_vid.dct_block_lists[0] == NULL) {
		int dct_block_count_x = (settings->video_width+15)/16;
		int dct_block_count_y = (settings->video_height+15)/16;
		int dct_block_size = dct_block_count_x*dct_block_count_y*sizeof(int32_t)*8*8;
		for (int i = 0; i < 6; i++) {
			settings->state_vid.dct_block_lists[i] = malloc(dct_block_size);
		}
	}

	memset(settings->state_vid.unmuxed, 0, sizeof(settings->state_vid.unmuxed));
	memset(header, 0, sizeof(header));

	settings->state_vid.quant_scale = 1;
	settings->state_vid.uncomp_hwords_used = 0;
	settings->state_vid.bytes_used = 8;
	settings->state_vid.blocks_used = 0;

	// TODO: non-16x16-aligned videos
	assert((settings->video_width % 16) == 0);
	assert((settings->video_height % 16) == 0);

	// Do the initial transform
	for(int fx = 0; fx < settings->video_width; fx += 16) {
	for(int fy = 0; fy < settings->video_height; fy += 16) {
		// Order: Cr Cb [Y1|Y2\nY3|Y4]
		int block_offs = 8*8*((fy>>4)*((settings->video_width+15)/16)+(fx>>4));
		int32_t *blocks[6] = {
			settings->state_vid.dct_block_lists[0] + block_offs,
			settings->state_vid.dct_block_lists[1] + block_offs,
			settings->state_vid.dct_block_lists[2] + block_offs,
			settings->state_vid.dct_block_lists[3] + block_offs,
			settings->state_vid.dct_block_lists[4] + block_offs,
			settings->state_vid.dct_block_lists[5] + block_offs,
		};

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
#if 1
			blocks[0][k] = ((cr<<2) - cluma + (1<<(4-1)))>>4;
			blocks[1][k] = ((cb<<2) - cluma + (1<<(4-1)))>>4;
#else
			blocks[0][k] = 0;
			blocks[1][k] = 0;
#endif

			for(int ly = 0; ly < 2; ly++) {
			for(int lx = 0; lx < 2; lx++) {
				int loffs = pitch*(fy+ly*8+y) + 4*(fx+lx*8+x);
				int lr = video_frame[loffs+0];
				int lg = video_frame[loffs+1];
				int lb = video_frame[loffs+2];

				// TODO: Get the real math for this
				int lluma = (lr+lg*2+lb+2)-0x200;
				if(lluma < -0x200) { lluma = -0x200; }
				if(lluma > +0x1FF) { lluma = +0x1FF; }
				lluma >>= 1;
				blocks[2+2*ly+lx][k] = lluma;
			}
			}
		}
		}
		for(int i = 0; i < 6; i++) {
			transform_dct_block(&(settings->state_vid), blocks[i]);
		}
	}
	}

	// Now reduce all the blocks
	const int accum_threshold = 6500;
	int values_to_shed = 0;
	for(int min_val = 0;; min_val += 1) {
		int accum = 0;
		for(int fx = 0; fx < settings->video_width; fx += 16) {
		for(int fy = 0; fy < settings->video_height; fy += 16) {
			// Order: Cr Cb [Y1|Y2\nY3|Y4]
			int block_offs = 8*8*((fy>>4)*((settings->video_width+15)/16)+(fx>>4));
			int32_t *blocks[6] = {
				settings->state_vid.dct_block_lists[0] + block_offs,
				settings->state_vid.dct_block_lists[1] + block_offs,
				settings->state_vid.dct_block_lists[2] + block_offs,
				settings->state_vid.dct_block_lists[3] + block_offs,
				settings->state_vid.dct_block_lists[4] + block_offs,
				settings->state_vid.dct_block_lists[5] + block_offs,
			};
			const int luma_reduce_mul = 4;
			const int chroma_reduce_mul = 4;
			for(int i = 6-1; i >= 0; i--) {
				accum += reduce_dct_block(&(settings->state_vid), blocks[i], (i < 2 ? min_val*luma_reduce_mul+1 : min_val*chroma_reduce_mul+1), &values_to_shed);
			}
		}
		}

		if(accum <= accum_threshold) {
			break;
		}

		values_to_shed = accum - accum_threshold;
	}

	// Now encode all the blocks
	for(int fx = 0; fx < settings->video_width; fx += 16) {
	for(int fy = 0; fy < settings->video_height; fy += 16) {
		// Order: Cr Cb [Y1|Y2\nY3|Y4]
		int block_offs = 8*8*((fy>>4)*((settings->video_width+15)/16)+(fx>>4));
		int32_t *blocks[6] = {
			settings->state_vid.dct_block_lists[0] + block_offs,
			settings->state_vid.dct_block_lists[1] + block_offs,
			settings->state_vid.dct_block_lists[2] + block_offs,
			settings->state_vid.dct_block_lists[3] + block_offs,
			settings->state_vid.dct_block_lists[4] + block_offs,
			settings->state_vid.dct_block_lists[5] + block_offs,
		};
		for(int i = 0; i < 6; i++) {
			encode_dct_block(&(settings->state_vid), blocks[i]);
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
