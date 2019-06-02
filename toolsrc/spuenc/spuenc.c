/*
spuenc: SPU/XA-ADPCM audio encoder
Copyright (c) 2019 Adrian "asie" Siekierka

does not support filters, which is a real shame
*/
#include <getopt.h>
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
#define FORMAT_SPU 1

typedef struct {
	int qerr;
	int16_t prev, prev2;
} encoder_state_t;

typedef struct {
	int format; // FORMAT_XA or FORMAT_SPU
	int stereo; // 0 or 1
	int frequency; // 18900 or 37800 Hz
	int bits_per_sample; // 4 or 8
	int file_number; // 00-FF
	int channel_number; // 00-1F

	encoder_state_t state_left;
	encoder_state_t state_right;
} settings_t;

int decode_frame(AVCodecContext *codec, AVFrame *frame, int *frame_size, AVPacket *packet) {
	int ret;

	if (packet != NULL) {
		ret = avcodec_send_packet(codec, packet);
		if (ret != 0) {
			return 0;
		}
	}

	ret = avcodec_receive_frame(codec, frame);
	if (ret >= 0) {
		*frame_size = ret;
		return 1;
	} else {
		return ret == AVERROR(EAGAIN) ? 1 : 0;
	}
}

int16_t *load_samples(const char *filename, int *sample_count, settings_t *settings) {
	int i, stream_index, frame_size, frame_sample_count, sample_count_mul;
	AVFormatContext* format;
	AVStream* stream;
	AVCodecContext* codec;
	struct SwrContext* resampler;
	AVPacket packet;
	AVFrame* frame;
	int16_t *out, *out_loc;
	uint8_t *buffer[1];

	format = avformat_alloc_context();
	if (avformat_open_input(&format, filename, NULL, NULL)) {
		return NULL;
	}
	if (avformat_find_stream_info(format, NULL) < 0) {
		return NULL;
	}

	stream_index = -1;
	for (i = 0; i < format->nb_streams; i++) {
		if (format->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			if (stream_index >= 0) {
				fprintf(stderr, "load_samples: found multiple audio tracks?\n");
				return NULL;
			}
			stream_index = i;
		}
	}
	if (stream_index == -1) {
		return NULL;
	}

	stream = format->streams[stream_index];
	codec = stream->codec;
	if (avcodec_open2(codec, avcodec_find_decoder(codec->codec_id), NULL) < 0) {
		return NULL;
	}

	resampler = swr_alloc();
	av_opt_set_int(resampler, "in_channel_count", codec->channels, 0);
	av_opt_set_int(resampler, "in_channel_layout", codec->channel_layout, 0);
	av_opt_set_int(resampler, "in_sample_rate", codec->sample_rate, 0);
	av_opt_set_sample_fmt(resampler, "in_sample_fmt", codec->sample_fmt, 0);

	sample_count_mul = settings->stereo ? 2 : 1;
	av_opt_set_int(resampler, "out_channel_count", settings->stereo ? 2 : 1, 0);
	av_opt_set_int(resampler, "out_channel_layout", settings->stereo ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO, 0);
	av_opt_set_int(resampler, "out_sample_rate", settings->frequency, 0);
	av_opt_set_sample_fmt(resampler, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

	if (swr_init(resampler) < 0) {
		return NULL;
	}

	av_init_packet(&packet);
	frame = av_frame_alloc();
	if (!frame) {
		return NULL;
	}

	out = NULL;
	*sample_count = 0;

	while (av_read_frame(format, &packet) >= 0) {
		if (decode_frame(codec, frame, &frame_size, &packet)) {
			buffer[0] = malloc(sizeof(int16_t) * sample_count_mul * swr_get_out_samples(resampler, frame->nb_samples));
			frame_sample_count = swr_convert(resampler, buffer, frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
			out = realloc(out, (*sample_count + ((frame_sample_count + 4032) * sample_count_mul)) * sizeof(int16_t));
			memmove(&out[*sample_count], buffer[0], sizeof(int16_t) * frame_sample_count * sample_count_mul);
			*sample_count += frame_sample_count * sample_count_mul;
			free(buffer[0]);
		}
	}

	// out is always padded out with 4032 "0" samples, this makes calculations elsewhere easier
	// and is mostly irrelevant as we load the whole thing into memory anyway at this time
	memset(out + (*sample_count), 0, 4032 * sample_count_mul * sizeof(int16_t));

	av_frame_free(&frame);
	swr_free(&resampler);
	avcodec_close(codec);
	avformat_free_context(format);

	return out;
}

static void init_sector_buffer(uint8_t *buffer, settings_t *settings) {
	memset(buffer,0,2352);
	buffer[0x0F] = 0x02;
	buffer[0x10] = settings->file_number;
	buffer[0x11] = settings->channel_number & 0x1F;
	buffer[0x12] = 0x24 | 0x40;
	buffer[0x13] =
		(settings->stereo ? 1 : 0)
		| (settings->frequency >= FREQ_DOUBLE ? 0 : 4)
		| (settings->bits_per_sample >= 8 ? 16 : 0);
	memcpy(buffer + 0x14, buffer + 0x10, 4);
}

int16_t filter_pos[4] = {0, 60, 115, 98};
int16_t filter_neg[4] = {0, 0, -52, -55};

uint8_t encode_nibbles(encoder_state_t *state, int16_t *samples, int pitch, uint8_t *data, int data_shift, int data_pitch) {
	// TODO: implement a real encoder
	uint8_t nondata_mask = ~(0x0F << data_shift);
	int min_shift = 12, curr_shift;
	int i;
	int sample;
	uint8_t sample_enc;
	uint8_t hdr = 0;
	int16_t sample_dec;
	int s_min, s_max;

	for (i = 0; i < 28; i++) {
		sample = samples[i * pitch];
		if (sample == 0 || sample == -1) continue;
		if (sample < 0) sample = (~sample); // works for clz

		curr_shift = __builtin_clz(sample) - 17;
		if (min_shift > curr_shift) min_shift = curr_shift;
	}

	hdr = min_shift & 0x0F;

	s_min = ((-8) << 12) >> min_shift;
	s_max = ((7) << 12) >> min_shift;

	for (i = 0; i < 28; i++) {
		sample = samples[i * pitch] + (state->qerr);
		if (sample < s_min) sample = s_min;
		else if (sample > s_max) sample = s_max;

		if (min_shift < 12) sample += (1 << (11 - min_shift));
		sample_enc = (sample >> (12 - min_shift)) & 0x0F;
		data[i * data_pitch] = (data[i * data_pitch] & nondata_mask) | sample_enc << data_shift;
		sample_dec = ((int16_t) (sample_enc << 12)) >> min_shift;
		state->qerr += samples[i * pitch] - sample_dec;

		state->prev = sample_dec;
		state->prev2 = state->prev;
	}

	return hdr;
}

void encode_file_spu(int16_t *samples, int sample_count, settings_t *settings, FILE *output) {
	uint8_t prebuf[28];
	uint8_t buffer[16];
	uint8_t *data;
	int i, j = 0;

	for (i = 0; i < sample_count; i += 28) {
		buffer[0] = encode_nibbles(&(settings->state_left), samples + i, 1, prebuf, 0, 1);
		for (j = 0; j < 28; j+=2) {
			buffer[2 + (j>>1)] = prebuf[j] | (prebuf[j+1] << 4);
		}

		buffer[1] = 0;
		buffer[1] |= (i == 0) ? 4 : 0;
		buffer[1] |= ((i + 28) >= sample_count) ? 1 : 0;

		fwrite(buffer, 16, 1, output);
	}
}

void encode_block_xa(int16_t *samples, uint8_t *data, settings_t *settings) {
	if (settings->bits_per_sample == 4) {
		if (settings->stereo) {
			data[0]  = encode_nibbles(&(settings->state_left), samples,            2, data + 0x10, 0, 4);
			data[1]  = encode_nibbles(&(settings->state_right), samples + 1,        2, data + 0x10, 4, 4);
			data[2]  = encode_nibbles(&(settings->state_left), samples + 56,       2, data + 0x11, 0, 4);
			data[3]  = encode_nibbles(&(settings->state_right), samples + 56 + 1,   2, data + 0x11, 4, 4);
			data[8]  = encode_nibbles(&(settings->state_left), samples + 56*2,     2, data + 0x12, 0, 4);
			data[9]  = encode_nibbles(&(settings->state_right), samples + 56*2 + 1, 2, data + 0x12, 4, 4);
			data[10] = encode_nibbles(&(settings->state_left), samples + 56*3,     2, data + 0x13, 0, 4);
			data[11] = encode_nibbles(&(settings->state_right), samples + 56*3 + 1, 2, data + 0x13, 4, 4);
		} else {
			data[0]  = encode_nibbles(&(settings->state_left), samples,            1, data + 0x10, 0, 4);
			data[1]  = encode_nibbles(&(settings->state_right), samples + 28,       1, data + 0x10, 4, 4);
			data[2]  = encode_nibbles(&(settings->state_left), samples + 28*2,     1, data + 0x11, 0, 4);
			data[3]  = encode_nibbles(&(settings->state_right), samples + 28*3,     1, data + 0x11, 4, 4);
			data[8]  = encode_nibbles(&(settings->state_left), samples + 28*4,     1, data + 0x12, 0, 4);
			data[9]  = encode_nibbles(&(settings->state_right), samples + 28*5,     1, data + 0x12, 4, 4);
			data[10] = encode_nibbles(&(settings->state_left), samples + 28*6,     1, data + 0x13, 0, 4);
			data[11] = encode_nibbles(&(settings->state_right), samples + 28*7,     1, data + 0x13, 4, 4);
		}
	} else {
/*		if (settings->stereo) {
			data[0]  = encode_bytes(samples,            2, data + 0x10);
			data[1]  = encode_bytes(samples + 1,        2, data + 0x11);
			data[2]  = encode_bytes(samples + 56,       2, data + 0x12);
			data[3]  = encode_bytes(samples + 57,       2, data + 0x13);
		} else {
			data[0]  = encode_bytes(samples,            1, data + 0x10);
			data[1]  = encode_bytes(samples + 28,       1, data + 0x11);
			data[2]  = encode_bytes(samples + 56,       1, data + 0x12);
			data[3]  = encode_bytes(samples + 84,       1, data + 0x13);
		} */
	}
}

//#define WRITE_BUFFER() fwrite(buffer, 2352, 1, output)
#define WRITE_BUFFER() fwrite(buffer + 0x10, 2336, 1, output)

void encode_file_xa(int16_t *samples, int sample_count, settings_t *settings, FILE *output) {
	uint8_t buffer[2352];
	uint8_t *data;
	int i, j = 0;
	int sample_jump = (settings->bits_per_sample == 8) ? 112 : 224;

	init_sector_buffer(buffer, settings);

	for (i = 0; i < sample_count; i += sample_jump) {
		data = buffer + 0x18 + (j * 0x80);

		encode_block_xa(samples + i, data, settings);

		memcpy(data + 4, data, 4);
		memcpy(data + 12, data + 8, 4);

		if ((i + sample_jump) >= sample_count) {
			// next written buffer is final
			buffer[0x12] |= 0x80;
			buffer[0x16] |= 0x80;
		}

		if ((++j) == 18) {
			WRITE_BUFFER();
			j = 0;
		}
	}

	// write final sector
	if (j > 0) {
		WRITE_BUFFER();
	}
}

void print_help() {
	printf("Usage: spuenc [-f freq] [-b bitdepth] [-c channels] [-F num] [-C num] [-t xa|spu] <in> <out>\n\n");
	printf("    -f freq        Use specified frequency\n");
	printf("    -t xa|spu      Use specified output type:\n");
	printf("                     xa    .xa 2336-byte sectors\n");
	printf("                     spu   raw SPU-ADPCM data\n");
	printf("    -b bitdepth    Use specified bit depth (only 4 bits supported)\n");
	printf("    -c channels    Use specified channel count (1 or 2)\n");
	printf("    -F num         [.xa] Set the file number to num (0-255)\n");
	printf("    -C num         [.xa] Set the channel number to num (0-31)\n");
}

int parse_args(settings_t* settings, int argc, char** argv) {
	int c;
	while ((c = getopt(argc, argv, "t:f:b:c:F:C:")) != -1) {
		switch (c) {
			case 't': {
				if (strcmp(optarg, "xa") == 0) {
					settings->format = FORMAT_XA;
				} else if (strcmp(optarg, "spu") == 0) {
					settings->format = FORMAT_SPU;
				} else {
					fprintf(stderr, "Invalid format: %s\n", optarg);
					return -1;
				}
			} break;
			case 'f': {
				settings->frequency = atoi(optarg);
			} break;
			case 'b': {
				settings->bits_per_sample = atoi(optarg);
				if (settings->bits_per_sample != 4) {
					fprintf(stderr, "Invalid bit depth: %d\n", settings->frequency);
					return -1;
				}
			} break;
			case 'c': {
				int ch = atoi(optarg);
				if (ch <= 0 || ch > 2) {
					fprintf(stderr, "Invalid channel count: %d\n", ch);
					return -1;
				}
				settings->stereo = ch == 2 ? 1 : 0;
			} break;
			case 'F': {
				settings->file_number = atoi(optarg);
				if (settings->file_number < 0 || settings->file_number > 255) {
					fprintf(stderr, "Invalid file number: %d\n", settings->file_number);
					return -1;
				}
			} break;
			case 'C': {
				settings->channel_number = atoi(optarg);
				if (settings->channel_number < 0 || settings->channel_number > 31) {
					fprintf(stderr, "Invalid channel number: %d\n", settings->channel_number);
					return -1;
				}
			} break;
			case '?':
			case 'h': {
				print_help();
				return -1;
			} break;
		}
	}

	if (settings->format == FORMAT_XA) {
		if (settings->frequency != FREQ_SINGLE && settings->frequency != FREQ_DOUBLE) {
			fprintf(stderr, "Invalid frequency: %d Hz\n", settings->frequency);
			return -1;
		}
	}

	if (settings->format == FORMAT_SPU) {
		settings->stereo = 0;
	}

	return optind;
}

int main(int argc, char **argv) {
	settings_t settings;
	int sample_count, arg_offset;
	int16_t *buffer;
	FILE* output;

	memset(&settings,0,sizeof(settings_t));

	settings.file_number = 0;
	settings.channel_number = 0;
	settings.stereo = 1;
	settings.frequency = FREQ_DOUBLE;
	settings.bits_per_sample = 4;

	arg_offset = parse_args(&settings, argc, argv);
	if (arg_offset < 0) {
		return 1;
	} else if (argc < arg_offset + 2) {
		print_help();
		return 1;
	}

	printf("Using settings: %d Hz @ %d bit depth, %s. F%d C%d\n",
		settings.frequency, settings.bits_per_sample,
		settings.stereo ? "stereo" : "mono",
		settings.file_number, settings.channel_number
	);

	buffer = load_samples(argv[arg_offset + 0], &sample_count, &settings);
	if (buffer == NULL) {
		fprintf(stderr, "Could not open input file!\n");
		return 1;
	}

	printf("Loaded %d samples.\n", sample_count);

	output = fopen(argv[arg_offset + 1], "wb");
	if (output == NULL) {
		fprintf(stderr, "Could not open output file!\n");
		return 1;
	}

	switch (settings.format) {
		case FORMAT_XA:
			encode_file_xa(buffer, sample_count, &settings, output);
			break;
		case FORMAT_SPU:
			encode_file_spu(buffer, sample_count, &settings, output);
			break;
	}

	fclose(output);
	free(buffer);
	return 0;
}
