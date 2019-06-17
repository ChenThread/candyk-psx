/*
psxavenc: MDEC video + SPU/XA-ADPCM audio encoder
Copyright (c) 2019 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell
*/

#include "common.h"

void print_help(void) {
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

	bool did_open_data = open_av_data(argv[arg_offset + 0], &settings);
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
	close_av_data(&settings);
	return 0;
}
