/*
spuenc: SPU/XA-ADPCM audio encoder
Copyright (c) 2019 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell
*/

#include "common.h"

static const int16_t filter_k1[ADPCM_FILTER_COUNT] = {0, 60, 115, 98, 122};
static const int16_t filter_k2[ADPCM_FILTER_COUNT] = {0, 0, -52, -55, -60};

static int find_min_shift(const encoder_state_t *state, int16_t *samples, int pitch, int filter) {
	// Assumption made:
	//
	// There is value in shifting right one step further to allow the nibbles to clip.
	// However, given a possible shift value, there is no value in shifting one step less.
	//
	// Having said that, this is not a completely accurate model of the encoder,
	// so maybe we will need to shift one step less.
	//
	int prev1 = state->prev1;
	int prev2 = state->prev2;
	int k1 = filter_k1[filter];
	int k2 = filter_k2[filter];

	int right_shift = 0;

	int32_t s_min = 0;
	int32_t s_max = 0;
	for (int i = 0; i < 28; i++) {
		int32_t raw_sample = samples[i * pitch];
		int32_t previous_values = (k1*prev1 + k2*prev2 + (1<<5))>>6;
		int32_t sample = raw_sample - previous_values;
		if (sample < s_min) { s_min = sample; }
		if (sample > s_max) { s_max = sample; }
		prev2 = prev1;
		prev1 = raw_sample;
	}
	while(right_shift < 12 && (s_max>>right_shift) > +0x7) { right_shift += 1; };
	while(right_shift < 12 && (s_min>>right_shift) < -0x8) { right_shift += 1; };

	int min_shift = 12 - right_shift;
	assert(0 <= min_shift && min_shift <= 12);
	return min_shift;
}

static uint8_t attempt_to_encode_nibbles(encoder_state_t *outstate, const encoder_state_t *instate, int16_t *samples, int pitch, uint8_t *data, int data_shift, int data_pitch, int filter, int sample_shift) {
	uint8_t nondata_mask = ~(0x0F << data_shift);
	int min_shift = sample_shift;
	int k1 = filter_k1[filter];
	int k2 = filter_k2[filter];

	uint8_t hdr = (min_shift & 0x0F) | (filter << 4);

	if (outstate != instate) {
		memcpy(outstate, instate, sizeof(encoder_state_t));
	}

	outstate->mse = 0;

	for (int i = 0; i < 28; i++) {
		int32_t sample = samples[i * pitch] + outstate->qerr;
		int32_t previous_values = (k1*outstate->prev1 + k2*outstate->prev2 + (1<<5))>>6;
		int32_t sample_enc = sample - previous_values;
		sample_enc <<= min_shift;
		sample_enc += (1<<(12-1));
		sample_enc >>= 12;
		if(sample_enc < -8) { sample_enc = -8; }
		if(sample_enc > +7) { sample_enc = +7; }
		sample_enc &= 0xF;

		int32_t sample_dec = (int16_t) ((sample_enc&0xF) << 12);
		sample_dec >>= min_shift;
		sample_dec += previous_values;
		if (sample_dec > +0x7FFF) { sample_dec = +0x7FFF; }
		if (sample_dec < -0x8000) { sample_dec = -0x8000; }
		int64_t sample_error = sample_dec - sample;

		assert(sample_error < (1<<30));
		assert(sample_error > -(1<<30));

		data[i * data_pitch] = (data[i * data_pitch] & nondata_mask) | (sample_enc << data_shift);
		// FIXME: dithering is hard to predict
		//outstate->qerr += sample_error;
		outstate->mse += ((uint64_t)sample_error) * (uint64_t)sample_error;

		outstate->prev2 = outstate->prev1;
		outstate->prev1 = sample_dec;
	}

	return hdr;
}

uint8_t encode_nibbles(encoder_state_t *state, int16_t *samples, int pitch, uint8_t *data, int data_shift, int data_pitch, int filter_count) {
	encoder_state_t proposed;
	int64_t best_mse = ((int64_t)1<<(int64_t)50);
	int best_filter = 0;
	int best_sample_shift = 0;

	for (int filter = 0; filter < filter_count; filter++) {
		int true_min_shift = find_min_shift(state, samples, pitch, filter);

		// Testing has shown that the optimal shift can be off the true minimum shift
		// by 1 in *either* direction.
		// This is NOT the case when dither is used.
		int min_shift = true_min_shift - 1;
		int max_shift = true_min_shift + 1;
		if (min_shift < 0) { min_shift = 0; }
		if (max_shift > 12) { max_shift = 12; }

		for (int sample_shift = min_shift; sample_shift <= max_shift; sample_shift++) {
			// ignore header here
			attempt_to_encode_nibbles(
				&proposed, state,
				samples, pitch,
				data, data_shift, data_pitch,
				filter, sample_shift);

			if (best_mse > proposed.mse) {
				best_mse = proposed.mse;
				best_filter = filter;
				best_sample_shift = sample_shift;
			}
		}
	}

	// now go with the encoder
	return attempt_to_encode_nibbles(
		state, state,
		samples, pitch,
		data, data_shift, data_pitch,
		best_filter, best_sample_shift);
}
