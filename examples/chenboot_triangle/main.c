/*
chenboot_triangle: Hello, triangle! (Bare minimum setup w/ chenboot)
Copyright (C) GreaseMonkey, 2017, licensed under Creative Commons Zero:
https://creativecommons.org/publicdomain/zero/1.0/

... yeah, this is way easier than the PS2.
*/
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <psxregs.h>

void gpu_write_gp0_command(uint32_t v) {
	int i;

	for(i = 0; i < 1000; i+=1) {
		if((PSXREG_GPU_GPUSTAT & (1<<26)) != 0) {
			break;
		}
	}

	PSXREG_GPU_GP0 = v;
}

void gpu_write_gp0_data(uint32_t v) {
	int i;

	for(i = 0; i < 1000; i+=1) {
		if((PSXREG_GPU_GPUSTAT & (1<<28)) != 0) {
			break;
		}
	}

	PSXREG_GPU_GP0 = v;
}

void gpu_write_gp1(uint32_t v) {
	PSXREG_GPU_GP1 = v;
}

int main(int argc, char *argv[])
{
	// Reset GPU
	gpu_write_gp1(0x00000000);

	// Set video mode
	gpu_write_gp1(0x08000009); // PAL, 320x240 base, 15bpp, progressive
	gpu_write_gp1(0x06000000 | (0x260) | ((0x260+320*8)<<12)); // Xrange
	gpu_write_gp1(0x07000000 | (0xA3-200/2) | ((0xA3+200/2)<<10)); // Yrange

	// Set rendering attribs
	gpu_write_gp0_command(0xE1000600); // Dither, allow draw to display, 4bpp tex
	gpu_write_gp0_command(0xE2000000); // Tex window setting (don't care)
	gpu_write_gp0_command(0xE3000000 | (0) | ((0)<<12)); // Min X/Y
	gpu_write_gp0_command(0xE4000000 | (320-1) | ((200-1)<<12)); // Max X/Y
	gpu_write_gp0_command(0xE5000000 | (320/2) | ((200/2)<<11)); // Drawing offset
	gpu_write_gp0_command(0xE6000000); // Mask bit (draw always)

	// Clear screen to colour
	gpu_write_gp0_command(0x02402000);
	gpu_write_gp0_data((0) | ((0)<<16));
	gpu_write_gp0_data((320) | ((200)<<16));

	// Draw a triangle
	gpu_write_gp0_command(0x200080FF);
	gpu_write_gp0_data(((60)&0xFFFF) | ((-50)<<16));
	gpu_write_gp0_data(((0)&0xFFFF) | ((70)<<16));
	gpu_write_gp0_data(((-60)&0xFFFF) | ((-50)<<16));

	// Enable display
	gpu_write_gp1(0x03000000); // display on

	// Spin!
	for(;;) {
	}
	return 0;
}
