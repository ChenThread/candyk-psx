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

void print_help(void) {
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "    psxavenc -t <xa|xacd>     [-f 18900|37800] [-c 1|2] [-b 4|8] [-F 0-255] [-C 0-31] <in> <out.xa>\n");
	fprintf(stderr, "    psxavenc -t <str2|str2cd> [-f 18900|37800] [-c 1|2] [-b 4|8] [-W width] [-H height] <in> <out.str>\n");
	fprintf(stderr, "    psxavenc -t <spu|vag>     [-f freq] [-L] <in> <out.vag>\n");
	fprintf(stderr, "    psxavenc -t vagi          [-f freq] [-c 1-24] [-I size] [-L] <in> <out.vag>\n");
	fprintf(stderr, "\nOptions:\n");
	fprintf(stderr, "    -f freq          Use specified frequency\n");
	fprintf(stderr, "    -t format        Use specified output type:\n");
	fprintf(stderr, "                       xa     [A.] .xa 2336-byte sectors\n");
	fprintf(stderr, "                       xacd   [A.] .xa 2352-byte sectors\n");
	fprintf(stderr, "                       spu    [A.] raw SPU-ADPCM mono data\n");
	fprintf(stderr, "                       spui   [A.] raw SPU-ADPCM interleaved data\n");
	fprintf(stderr, "                       vag    [A.] .vag SPU-ADPCM mono\n");
	fprintf(stderr, "                       vagi   [A.] .vag SPU-ADPCM interleaved\n");
	fprintf(stderr, "                       str2   [AV] v2 .str video 2336-byte sectors\n");
	fprintf(stderr, "                       str2cd [AV] v2 .str video 2352-byte sectors\n");
	fprintf(stderr, "    -b bitdepth      Use specified bit depth for .xa/.str (only 4 bits supported)\n");
	fprintf(stderr, "    -c channels      Use specified channel count (1-2 for .xa/.str, any for interleaved .vag)\n");
	fprintf(stderr, "    -F num           Set the XA file number to num (0-255)\n");
	fprintf(stderr, "    -C num           Set the XA channel number to num (0-31)\n");
	fprintf(stderr, "    -L               Add a loop marker at the end of SPU-ADPCM data\n");
	fprintf(stderr, "    -W width         [str2] Rescale input file to the specified width (default 320)\n");
	fprintf(stderr, "    -H height        [str2] Rescale input file to the specified height (default 240)\n");
	fprintf(stderr, "    -I size          [spui/vagi] Use specified interleave\n");
	fprintf(stderr, "    -A size          [spui/vagi] Pad header and each interleaved chunk to specified size\n");
}

int parse_args(settings_t* settings, int argc, char** argv) {
	int c;
	while ((c = getopt(argc, argv, "t:f:b:c:F:C:W:H:I:A:Lh?")) != -1) {
		switch (c) {
			case 't': {
				if (strcmp(optarg, "xa") == 0) {
					settings->format = FORMAT_XA;
				} else if (strcmp(optarg, "xacd") == 0) {
					settings->format = FORMAT_XACD;
				} else if (strcmp(optarg, "spu") == 0) {
					settings->format = FORMAT_SPU;
				} else if (strcmp(optarg, "spui") == 0) {
					settings->format = FORMAT_SPUI;
				} else if (strcmp(optarg, "vag") == 0) {
					settings->format = FORMAT_VAG;
				} else if (strcmp(optarg, "vagi") == 0) {
					settings->format = FORMAT_VAGI;
				} else if (strcmp(optarg, "str2") == 0) {
					settings->format = FORMAT_STR2;
				} else if (strcmp(optarg, "str2cd") == 0) {
					settings->format = FORMAT_STR2CD;
				} else {
					fprintf(stderr, "Invalid format: %s\n", optarg);
					return -1;
				}
			} break;
			case 'f': {
				settings->frequency = strtol(optarg, NULL, 0);
			} break;
			case 'b': {
				settings->bits_per_sample = strtol(optarg, NULL, 0);
				//if (settings->bits_per_sample != 4 && settings->bits_per_sample != 8) {
				if (settings->bits_per_sample != 4) {
					fprintf(stderr, "Invalid bit depth: %d\n", settings->frequency);
					return -1;
				}
			} break;
			case 'c': {
				settings->channels = strtol(optarg, NULL, 0);
				if (settings->channels < 1 || settings->channels > 24) {
					fprintf(stderr, "Invalid channel count: %d\n", settings->channels);
					return -1;
				}
			} break;
			case 'F': {
				settings->file_number = strtol(optarg, NULL, 0);
				if (settings->file_number < 0 || settings->file_number > 255) {
					fprintf(stderr, "Invalid file number: %d\n", settings->file_number);
					return -1;
				}
			} break;
			case 'C': {
				settings->channel_number = strtol(optarg, NULL, 0);
				if (settings->channel_number < 0 || settings->channel_number > 31) {
					fprintf(stderr, "Invalid channel number: %d\n", settings->channel_number);
					return -1;
				}
			} break;
			case 'L': {
				settings->loop = true;
			} break;
			case 'W': {
				settings->video_width = (strtol(optarg, NULL, 0) + 15) & ~15;
				if (settings->video_width < 16 || settings->video_width > 640) {
					fprintf(stderr, "Invalid video width: %d\n", settings->video_width);
					return -1;
				}
			} break;
			case 'H': {
				settings->video_height = (strtol(optarg, NULL, 0) + 15) & ~15;
				if (settings->video_height < 16 || settings->video_height > 480) {
					fprintf(stderr, "Invalid video height: %d\n", settings->video_height);
					return -1;
				}
			} break;
			case 'I': {
				settings->interleave = (strtol(optarg, NULL, 0) + 15) & ~15;
				if (settings->interleave < 16) {
					fprintf(stderr, "Invalid interleave: %d\n", settings->interleave);
					return -1;
				}
			} break;
			case 'A': {
				settings->alignment = strtol(optarg, NULL, 0);
				if (settings->alignment < 1) {
					fprintf(stderr, "Invalid alignment: %d\n", settings->alignment);
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

	// Validate settings
	switch (settings->format) {
		case FORMAT_XA:
		case FORMAT_XACD:
		case FORMAT_STR2:
		case FORMAT_STR2CD:
			if (settings->frequency != PSX_AUDIO_XA_FREQ_SINGLE && settings->frequency != PSX_AUDIO_XA_FREQ_DOUBLE) {
				fprintf(
					stderr, "Invalid XA-ADPCM frequency: %d Hz (must be %d or %d Hz)\n", settings->frequency,
					PSX_AUDIO_XA_FREQ_SINGLE, PSX_AUDIO_XA_FREQ_DOUBLE
				);
				return -1;
			}
			if (settings->channels > 2) {
				fprintf(stderr, "Invalid XA-ADPCM channel count: %d (must be 1 or 2)\n", settings->channels);
				return -1;
			}
			if (settings->loop) {
				fprintf(stderr, "XA-ADPCM does not support loop markers\n");
				return -1;
			}
			break;
		case FORMAT_SPU:
		case FORMAT_VAG:
			if (settings->bits_per_sample != 4) {
				fprintf(stderr, "Invalid SPU-ADPCM bit depth: %d (must be 4)\n", settings->bits_per_sample);
				return -1;
			}
			if (settings->channels != 1) {
				fprintf(stderr, "Invalid SPU-ADPCM channel count: %d (must be 1)\n", settings->channels);
				return -1;
			}
			if (settings->interleave) {
				fprintf(stderr, "Interleave cannot be specified for mono SPU-ADPCM\n");
				return -1;
			}
			break;
		case FORMAT_SPUI:
		case FORMAT_VAGI:
			if (settings->bits_per_sample != 4) {
				fprintf(stderr, "Invalid SPU-ADPCM bit depth: %d (must be 4)\n", settings->bits_per_sample);
				return -1;
			}
			if (!settings->interleave) {
				fprintf(stderr, "Interleave must be specified for interleaved SPU-ADPCM\n");
				return -1;
			}
			break;
		default:
			fprintf(stderr, "Output format must be specified\n");
			return -1;
	}

	return optind;
}

int main(int argc, char **argv) {
	settings_t settings;
	int arg_offset;
	FILE* output;

	memset(&settings,0,sizeof(settings_t));

	settings.format = -1;
	settings.file_number = 0;
	settings.channel_number = 0;
	settings.channels = 1;
	settings.frequency = PSX_AUDIO_XA_FREQ_DOUBLE;
	settings.bits_per_sample = 4;
	settings.interleave = 0;
	settings.alignment = 2048;
	settings.loop = false;

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

	if (settings.channels == 1)
		fprintf(stderr, "Using settings: %d Hz @ %d bit depth, mono. F=%d C=%d\n",
			settings.frequency, settings.bits_per_sample,
			settings.file_number, settings.channel_number
		);
	else
		fprintf(stderr, "Using settings: %d Hz @ %d bit depth, %d channels. F=%d C=%d\n",
			settings.frequency, settings.bits_per_sample, settings.channels,
			settings.file_number, settings.channel_number
		);

	bool did_open_data = open_av_data(argv[arg_offset + 0], &settings);
	if (!did_open_data) {
		fprintf(stderr, "Could not open input file!\n");
		return 1;
	}

	output = fopen(argv[arg_offset + 1], "wb");
	if (output == NULL) {
		fprintf(stderr, "Could not open output file!\n");
		return 1;
	}

	switch (settings.format) {
		case FORMAT_XA:
		case FORMAT_XACD:
			pull_all_av_data(&settings);
			encode_file_xa(settings.audio_samples, settings.audio_sample_count / settings.channels, &settings, output);
			break;
		case FORMAT_SPU:
		case FORMAT_VAG:
			pull_all_av_data(&settings);
			encode_file_spu(settings.audio_samples, settings.audio_sample_count / settings.channels, &settings, output);
			break;
		case FORMAT_SPUI:
		case FORMAT_VAGI:
			pull_all_av_data(&settings);
			encode_file_spu_interleaved(settings.audio_samples, settings.audio_sample_count / settings.channels, &settings, output);
			break;
		case FORMAT_STR2:
		case FORMAT_STR2CD:
			encode_file_str(&settings, output);
			break;
	}

	fclose(output);
	close_av_data(&settings);
	return 0;
}
