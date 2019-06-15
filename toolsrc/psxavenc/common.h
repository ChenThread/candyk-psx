/*
psxavenc: MDEC video + SPU/XA-ADPCM audio encoder
Copyright (c) 2019 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell
*/

#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

#define FREQ_SINGLE 18900
#define FREQ_DOUBLE 37800
#define FORMAT_XA 0
#define FORMAT_XACD 1
#define FORMAT_SPU 2

#define ADPCM_FILTER_COUNT 5
#define XA_ADPCM_FILTER_COUNT 4
#define SPU_ADPCM_FILTER_COUNT 5

typedef struct {
	int qerr; // quanitisation error
	uint64_t mse; // mean square error
	int prev1, prev2;
} encoder_state_t;

typedef struct {
	int format; // FORMAT_XA, FORMAT_XACD or FORMAT_SPU
	bool stereo; // false or true
	int frequency; // 18900 or 37800 Hz
	int bits_per_sample; // 4 or 8
	int file_number; // 00-FF
	int channel_number; // 00-1F

	encoder_state_t state_left;
	encoder_state_t state_right;
} settings_t;

// adpcm.c
uint8_t encode_nibbles(encoder_state_t *state, int16_t *samples, int pitch, uint8_t *data, int data_shift, int data_pitch, int filter_count);

// cdrom.c
void init_sector_buffer(uint8_t *buffer, settings_t *settings);
void calculate_edc_xa(uint8_t *buffer);

// filefmt.c
void encode_file_spu(int16_t *samples, int sample_count, settings_t *settings, FILE *output);
void encode_file_xa(int16_t *samples, int sample_count, settings_t *settings, FILE *output);