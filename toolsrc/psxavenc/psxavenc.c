/*
psxavenc: MDEC video + SPU/XA-ADPCM audio encoder
Copyright (c) 2019 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell
*/

#include "common.h"

int decode_audio_frame(AVCodecContext *codec, AVFrame *frame, int *frame_size, AVPacket *packet) {
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

int decode_video_frame(AVCodecContext *codec, AVFrame *frame, int *frame_size, AVPacket *packet) {
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

bool load_av_data(const char *filename, settings_t *settings) {
	int frame_size, frame_sample_count, sample_count_mul;
	AVPacket packet;
	AVFrame* frame;
	uint8_t *buffer[1];
	double video_next_pts = 0.0;

	av_decoder_state_t* av = &(settings->decoder_state_av);
	av->video_frame_src_size = 0;
	av->video_frame_dst_size = 0;
	av->audio_stream_index = -1;
	av->video_stream_index = -1;
	av->format = NULL;
	av->audio_stream = NULL;
	av->video_stream = NULL;
	av->audio_codec_context = NULL;
	av->video_codec_context = NULL;
	av->audio_codec = NULL;
	av->video_codec = NULL;
	av->resampler = NULL;
	av->scaler = NULL;

	av->format = avformat_alloc_context();
	if (avformat_open_input(&(av->format), filename, NULL, NULL)) {
		return false;
	}
	if (avformat_find_stream_info(av->format, NULL) < 0) {
		return false;
	}

	for (int i = 0; i < av->format->nb_streams; i++) {
		if (av->format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			if (av->audio_stream_index >= 0) {
				fprintf(stderr, "load_av_data: found multiple audio tracks?\n");
				return false;
			}
			av->audio_stream_index = i;
		}
	}
	if (av->audio_stream_index == -1) {
		return false;
	}

	for (int i = 0; i < av->format->nb_streams; i++) {
		if (av->format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			if (av->video_stream_index >= 0) {
				fprintf(stderr, "load_av_data: found multiple video tracks?\n");
				return false;
			}
			av->video_stream_index = i;
		}
	}

	av->audio_stream = av->format->streams[av->audio_stream_index];
	av->video_stream = (av->video_stream_index != -1 ? av->format->streams[av->video_stream_index] : NULL);
	av->audio_codec = avcodec_find_decoder(av->audio_stream->codecpar->codec_id);
	av->audio_codec_context = avcodec_alloc_context3(av->audio_codec);
	if (av->audio_codec_context == NULL) {
		return false;
	}
	if (avcodec_parameters_to_context(av->audio_codec_context, av->audio_stream->codecpar) < 0) {
		return false;
	}
	if (avcodec_open2(av->audio_codec_context, av->audio_codec, NULL) < 0) {
		return false;
	}

	av->resampler = swr_alloc();
	av_opt_set_int(av->resampler, "in_channel_count", av->audio_codec_context->channels, 0);
	av_opt_set_int(av->resampler, "in_channel_layout", av->audio_codec_context->channel_layout, 0);
	av_opt_set_int(av->resampler, "in_sample_rate", av->audio_codec_context->sample_rate, 0);
	av_opt_set_sample_fmt(av->resampler, "in_sample_fmt", av->audio_codec_context->sample_fmt, 0);

	sample_count_mul = settings->stereo ? 2 : 1;
	av_opt_set_int(av->resampler, "out_channel_count", settings->stereo ? 2 : 1, 0);
	av_opt_set_int(av->resampler, "out_channel_layout", settings->stereo ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO, 0);
	av_opt_set_int(av->resampler, "out_sample_rate", settings->frequency, 0);
	av_opt_set_sample_fmt(av->resampler, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

	if (swr_init(av->resampler) < 0) {
		return false;
	}

	if (av->video_stream != NULL) {
		av->video_codec = avcodec_find_decoder(av->video_stream->codecpar->codec_id);
		av->video_codec_context = avcodec_alloc_context3(av->video_codec);
		if(av->video_codec_context == NULL) {
			return false;
		}
		if (avcodec_parameters_to_context(av->video_codec_context, av->video_stream->codecpar) < 0) {
			return false;
		}
		if (avcodec_open2(av->video_codec_context, av->video_codec, NULL) < 0) {
			return false;
		}

		av->scaler = sws_getContext(
			av->video_codec_context->width,
			av->video_codec_context->height,
			av->video_codec_context->pix_fmt,
			settings->video_width,
			settings->video_height,
			AV_PIX_FMT_RGBA,
			SWS_BICUBIC,
			NULL,
			NULL,
			NULL);
		
		av->video_frame_src_size = 4*av->video_codec_context->width*av->video_codec_context->height;
		av->video_frame_dst_size = 4*settings->video_width*settings->video_height;
	}

	av_init_packet(&packet);
	frame = av_frame_alloc();
	if (!frame) {
		return false;
	}

	settings->audio_samples = NULL;
	settings->audio_sample_count = 0;
	settings->video_frames = NULL;
	settings->video_frame_count = 0;

	while (av_read_frame(av->format, &packet) >= 0) {
		if (packet.stream_index == av->audio_stream_index) {
			if (decode_audio_frame(av->audio_codec_context, frame, &frame_size, &packet)) {
				size_t buffer_size = sizeof(int16_t) * sample_count_mul * swr_get_out_samples(av->resampler, frame->nb_samples);
				buffer[0] = malloc(buffer_size);
				memset(buffer[0], 0, buffer_size);
				frame_sample_count = swr_convert(av->resampler, buffer, frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
				settings->audio_samples = realloc(settings->audio_samples, (settings->audio_sample_count + ((frame_sample_count + 4032) * sample_count_mul)) * sizeof(int16_t));
				memmove(&(settings->audio_samples[settings->audio_sample_count]), buffer[0], sizeof(int16_t) * frame_sample_count * sample_count_mul);
				settings->audio_sample_count += frame_sample_count * sample_count_mul;
				free(buffer[0]);
			}

		}
		else if (packet.stream_index == av->video_stream_index) {
			if (decode_video_frame(av->video_codec_context, frame, &frame_size, &packet)) {
				// FIXME: increasing framerate doesn't fill it in with duplicate frames!
				double pts = (((double)frame->pts)*(double)av->video_stream->time_base.num)/av->video_stream->time_base.den;
				//printf("%f\n", pts);
				// Drop frames with negative PTS values
				if(pts < 0.0) {
					// do nothing
					continue;
				}
				if((settings->video_frame_count) >= 1 && pts < video_next_pts) {
					// do nothing
					continue;
				}
				if((settings->video_frame_count) < 1) {
					video_next_pts = pts;
				}

				//printf("%d %f %f %f\n", (*video_frame_count), pts, video_next_pts, ((double)1.0*settings->video_fps_den)/settings->video_fps_num);
				video_next_pts += ((double)1.0*(double)settings->video_fps_den)/(double)settings->video_fps_num;
				//size_t buffer_size = frame_count_mul;
				//buffer[0] = malloc(buffer_size);
				//memset(buffer[0], 0, buffer_size);
				//frame_sample_count = swr_convert(av->resampler, buffer, frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
				settings->video_frames = realloc(settings->video_frames, (settings->video_frame_count + 1) * av->video_frame_dst_size);
				int dst_strides[1] = {
					settings->video_width*4,
				};
				uint8_t *dst_pointers[1] = {
					(settings->video_frames) + av->video_frame_dst_size*(settings->video_frame_count),
				};
				sws_scale(av->scaler, frame->data, frame->linesize, 0, frame->height, dst_pointers, dst_strides);

				settings->video_frame_count += 1;
				//free(buffer[0]);
			}

		}
	}

	// out is always padded out with 4032 "0" samples, this makes calculations elsewhere easier
	// and is mostly irrelevant as we load the whole thing into memory anyway at this time
	memset((settings->audio_samples) + (settings->audio_sample_count), 0, 4032 * sample_count_mul * sizeof(int16_t));

	av_frame_free(&(frame));
	swr_free(&(av->resampler));
	avcodec_close(av->audio_codec_context);
	avcodec_free_context(&(av->audio_codec_context));
	avformat_free_context(av->format);

	return true;
}

void print_help() {
	printf("Usage: psxavenc [-f freq] [-b bitdepth] [-c channels] [-F num] [-C num] [-t xa|xacd|spu|str2] <in> <out>\n\n");
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
				} else if (strcmp(optarg, "str2") == 0) {
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
	FILE* output;

	memset(&settings,0,sizeof(settings_t));

	settings.file_number = 0;
	settings.channel_number = 0;
	settings.stereo = true;
	settings.frequency = FREQ_DOUBLE;
	settings.bits_per_sample = 4;

	settings.video_width = 320;
	settings.video_height = 240;

	settings.audio_samples = NULL;
	settings.audio_sample_count = 0;
	settings.video_frames = NULL;
	settings.video_frame_count = 0;

	// TODO: make this adjustable
	// also for some reason ffmpeg seems to hard-code the framerate to 15fps
	settings.video_fps_num = 15;
	settings.video_fps_den = 1;
	for(int i = 0; i < 6; i++) {
		settings.state_vid.dct_block_lists[i] = NULL;
	}

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

	bool did_load_data = load_av_data(argv[arg_offset + 0], &settings);
	if (settings.audio_samples == NULL) {
		fprintf(stderr, "Could not open input file!\n");
		return 1;
	}

	printf("Loaded %d samples.\n", settings.audio_sample_count);
	printf("Loaded %d frames.\n", settings.video_frame_count);

	output = fopen(argv[arg_offset + 1], "wb");
	if (output == NULL) {
		fprintf(stderr, "Could not open output file!\n");
		return 1;
	}

	switch (settings.format) {
		case FORMAT_XA:
		case FORMAT_XACD:
			encode_file_xa(settings.audio_samples, settings.audio_sample_count, &settings, output);
			break;
		case FORMAT_SPU:
			encode_file_spu(settings.audio_samples, settings.audio_sample_count, &settings, output);
			break;
		case FORMAT_STR2:
			encode_file_str(settings.audio_samples, settings.audio_sample_count, settings.video_frames, settings.video_frame_count, &settings, output);
			break;
	}

	fclose(output);
	if(settings.audio_samples != NULL) {
		free(settings.audio_samples);
		settings.audio_samples = NULL;
	}
	if(settings.video_frames != NULL) {
		free(settings.video_frames);
		settings.video_frames = NULL;
	}
	return 0;
}
