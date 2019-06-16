/*
psxavenc: MDEC video + SPU/XA-ADPCM audio encoder
Copyright (c) 2019 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell
*/

#include "common.h"

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

bool load_av_data(const char *filename, settings_t *settings, int *audio_sample_count, int16_t **audio_buffer, int *video_frame_count, uint8_t **video_buffer) {
	int i, stream_index, frame_size, frame_sample_count, sample_count_mul;
	AVFormatContext* format;
	AVStream* stream;
	AVCodecContext* codec_context;
	AVCodec* codec;
	struct SwrContext* resampler;
	AVPacket packet;
	AVFrame* frame;
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
		if (format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			if (stream_index >= 0) {
				fprintf(stderr, "load_av_data: found multiple audio tracks?\n");
				return NULL;
			}
			stream_index = i;
		}
	}
	if (stream_index == -1) {
		return NULL;
	}

	stream = format->streams[stream_index];
	codec = avcodec_find_decoder(stream->codecpar->codec_id);
	codec_context = avcodec_alloc_context3(codec);
	if(codec_context == NULL) {
		return NULL;
	}
	if (avcodec_parameters_to_context(codec_context, stream->codecpar) < 0) {
		return NULL;
	}
	if (avcodec_open2(codec_context, codec, NULL) < 0) {
		return NULL;
	}

	resampler = swr_alloc();
	av_opt_set_int(resampler, "in_channel_count", codec_context->channels, 0);
	av_opt_set_int(resampler, "in_channel_layout", codec_context->channel_layout, 0);
	av_opt_set_int(resampler, "in_sample_rate", codec_context->sample_rate, 0);
	av_opt_set_sample_fmt(resampler, "in_sample_fmt", codec_context->sample_fmt, 0);

	sample_count_mul = settings->stereo ? 2 : 1;
	av_opt_set_int(resampler, "out_channel_count", settings->stereo ? 2 : 1, 0);
	av_opt_set_int(resampler, "out_channel_layout", settings->stereo ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO, 0);
	av_opt_set_int(resampler, "out_sample_rate", settings->frequency, 0);
	av_opt_set_sample_fmt(resampler, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

	if (swr_init(resampler) < 0) {
		avcodec_free_context(&codec_context);
		return NULL;
	}

	av_init_packet(&packet);
	frame = av_frame_alloc();
	if (!frame) {
		avcodec_free_context(&codec_context);
		return NULL;
	}

	*audio_buffer = NULL;
	*audio_sample_count = 0;
	*video_buffer = NULL;
	*video_frame_count = 0;

	while (av_read_frame(format, &packet) >= 0) {
		if (decode_frame(codec_context, frame, &frame_size, &packet)) {
			size_t buffer_size = sizeof(int16_t) * sample_count_mul * swr_get_out_samples(resampler, frame->nb_samples);
			buffer[0] = malloc(buffer_size);
			memset(buffer[0], 0, buffer_size);
			frame_sample_count = swr_convert(resampler, buffer, frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
			*audio_buffer = realloc(*audio_buffer, (*audio_sample_count + ((frame_sample_count + 4032) * sample_count_mul)) * sizeof(int16_t));
			memmove(&((*audio_buffer)[*audio_sample_count]), buffer[0], sizeof(int16_t) * frame_sample_count * sample_count_mul);
			*audio_sample_count += frame_sample_count * sample_count_mul;
			free(buffer[0]);
		}
	}

	// out is always padded out with 4032 "0" samples, this makes calculations elsewhere easier
	// and is mostly irrelevant as we load the whole thing into memory anyway at this time
	memset((*audio_buffer) + (*audio_sample_count), 0, 4032 * sample_count_mul * sizeof(int16_t));

	av_frame_free(&frame);
	swr_free(&resampler);
	avcodec_close(codec_context);
	avcodec_free_context(&codec_context);
	avformat_free_context(format);

	return true;
}

void print_help() {
	printf("Usage: psxavenc [-f freq] [-b bitdepth] [-c channels] [-F num] [-C num] [-t xa|spu] <in> <out>\n\n");
	printf("    -f freq          Use specified frequency\n");
	printf("    -t format        Use specified output type:\n");
	printf("                       xa     [A.] .xa 2336-byte sectors\n");
	printf("                       xacd   [A.] .xa 2352-byte sectors\n");
	printf("                       spu    [A.] raw SPU-ADPCM data\n");
	printf("                       str2   [AV] v2 .str video 2352-byte sectors\n");
	printf("    -b bitdepth      Use specified bit depth (only 4 bits supported)\n");
	printf("    -c channels      Use specified channel count (1 or 2)\n");
	printf("    -F num           [.xa] Set the file number to num (0-255)\n");
	printf("    -C num           [.xa] Set the channel number to num (0-31)\n");
}

int parse_args(settings_t* settings, int argc, char** argv) {
	int c;
	while ((c = getopt(argc, argv, "t:f:b:c:F:C:")) != -1) {
		switch (c) {
			case 't': {
				if (strcmp(optarg, "xa") == 0) {
					settings->format = FORMAT_XA;
				} else if (strcmp(optarg, "xacd") == 0) {
					settings->format = FORMAT_XACD;
				} else if (strcmp(optarg, "spu") == 0) {
					settings->format = FORMAT_SPU;
				} else if (strcmp(optarg, "str") == 0) {
					settings->format = FORMAT_STR2;
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
				settings->stereo = (ch == 2 ? 1 : 0);
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

	if (settings->format == FORMAT_XA || settings->format == FORMAT_XACD) {
		if (settings->frequency != FREQ_SINGLE && settings->frequency != FREQ_DOUBLE) {
			fprintf(stderr, "Invalid frequency: %d Hz\n", settings->frequency);
			return -1;
		}
	}

	if (settings->format == FORMAT_SPU) {
		settings->stereo = false;
	}

	return optind;
}

int main(int argc, char **argv) {
	settings_t settings;
	int arg_offset;
	int audio_sample_count = 0;
	int16_t *audio_buffer = NULL;
	int video_frame_count = 0;
	uint8_t *video_buffer = NULL;
	FILE* output;

	memset(&settings,0,sizeof(settings_t));

	settings.file_number = 0;
	settings.channel_number = 0;
	settings.stereo = true;
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

	bool did_load_data = load_av_data(argv[arg_offset + 0], &settings, &audio_sample_count, &audio_buffer, &video_frame_count, &video_buffer);
	if (audio_buffer == NULL) {
		fprintf(stderr, "Could not open input file!\n");
		return 1;
	}

	printf("Loaded %d samples.\n", audio_sample_count);

	output = fopen(argv[arg_offset + 1], "wb");
	if (output == NULL) {
		fprintf(stderr, "Could not open output file!\n");
		return 1;
	}

	switch (settings.format) {
		case FORMAT_XA:
		case FORMAT_XACD:
			encode_file_xa(audio_buffer, audio_sample_count, &settings, output);
			break;
		case FORMAT_SPU:
			encode_file_spu(audio_buffer, audio_sample_count, &settings, output);
			break;
		case FORMAT_STR2:
			encode_file_str(audio_buffer, audio_sample_count, &settings, output);
			break;
	}

	fclose(output);
	if(audio_buffer != NULL) {
		free(audio_buffer);
		audio_buffer = NULL;
	}
	if(video_buffer != NULL) {
		free(video_buffer);
		video_buffer = NULL;
	}
	return 0;
}
