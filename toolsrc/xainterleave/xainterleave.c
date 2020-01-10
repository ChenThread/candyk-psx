/*
xainterleave: simple sector interleaving tool

Copyright (c) 2019 Adrian "asie" Siekierka

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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENTRY_MAX 64

#define TYPE_NULL 0
#define TYPE_RAW 1
#define TYPE_XA 2
#define TYPE_XACD 3

typedef struct {
	int sectors, type;

	FILE *file;

	int xa_file;
	int xa_channel;
} entry_t;

static entry_t entries[ENTRY_MAX];

int parse(char *filename) {
	entry_t e;
	char type_str[65];
	char fn_str[257];
	int entry_count = 0;
	FILE *file = fopen(filename, "r");
	if (file == NULL) return 0;

	while (fscanf(file, " %d %64s", &(e.sectors), type_str) > 0) {
		if (strcmp(type_str, "null") == 0) e.type = TYPE_NULL;
		else if (strcmp(type_str, "raw") == 0) e.type = TYPE_RAW;
		else if (strcmp(type_str, "xacd") == 0) e.type = TYPE_XACD;
		else if (strcmp(type_str, "xa") == 0) e.type = TYPE_XA;
		else { fprintf(stderr, "Unknown type: %s\n", type_str); continue; }

		switch (e.type) {
			case TYPE_RAW:
			case TYPE_XA:
			case TYPE_XACD:
				if (fscanf(file, " %256s", fn_str) > 0) {
					if ((e.file = fopen(fn_str, "rb")) == NULL) return 0;
				} else return 0;
				break;
		}

		switch (e.type) {
			case TYPE_XA:
			case TYPE_XACD:
				if (fscanf(file, " %d %d", &(e.xa_file), &(e.xa_channel)) > 0) {
					// nop
				} else {
					e.xa_file = 0;
					e.xa_channel = 0;
				}
				break;
		}

		entries[entry_count] = e;
		entry_count++;
	}

	fclose(file);
	return entry_count;
}

int main(int argc, char** argv) {
	uint8_t buffer[2352];

	if (argc < 3) {
		fprintf(stderr, "Usage: xainterleave <in.txt> <out.raw>\n");
		return 1;
	}

	int entry_count = parse(argv[1]);
	if (entry_count <= 0) {
		fprintf(stderr, "Empty manifest?\n");
		return 1;
	}

	int sector_div = 0;
	for (int i = 0; i < entry_count; i++) {
		sector_div += entries[i].sectors;
	}
	printf("Interleaving into %d-sector chunks\n", sector_div);

	FILE *output = fopen(argv[2], "wb");

	while (1) {
		int can_read = 0;
		for (int i = 0; i < entry_count; i++) {
			entry_t *e = &entries[i];
			if (e->file != NULL) {
				if (!feof(e->file)) can_read++;
			}
		}
		if (can_read <= 0) break;

		for (int i = 0; i < entry_count; i++) {
			entry_t *e = &entries[i];
			for (int is = 0; is < e->sectors; is++) {
				int write_null = 0;
				switch (e->type) {
					case TYPE_NULL:
						write_null = 1;
						break;
					case TYPE_RAW:
						if (!fread(buffer, 2352, 1, e->file)) { write_null = 1; break; }
						fwrite(buffer, 2352, 1, output);
						break;
					case TYPE_XA:
					case TYPE_XACD:
						if (e->type == TYPE_XACD) {
							if (!fread(buffer, 2352, 1, e->file)) { write_null = 1; break; }
						} else {
							if (!fread(buffer + 0x10, 2336, 1, e->file)) { write_null = 1; break; }
							memset(buffer, 0, 15);
							buffer[15] = 0x02;
						}
						if (e->xa_file >= 0) buffer[0x010] = buffer[0x014] = e->xa_file;
						if (e->xa_channel >= 0) buffer[0x011] = buffer[0x015] = e->xa_channel & 0x1F;
						buffer[0x92F] = 0xFF; // make pscd-new generate EDC
						fwrite(buffer, 2352, 1, output); break;
				}

				if (write_null) {
					for (int j = 0; j < 2352; j++) fputc(0, output);
				}
			}
		}
	}

	fclose(output);
	return 0;
}
