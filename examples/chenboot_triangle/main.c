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
#include <chenboot.h>

#define PAIR16(x, y) (((x)&0xFFFF) | ((y)<<16))

volatile uint32_t vblank_counter = 0;

void gpu_write_gp0_command(uint32_t v)
{
	int i;

	for(i = 0; i < 1000; i+=1) {
		if((PSXREG_GPU_GPUSTAT & (1<<26)) != 0) {
			break;
		}
	}

	PSXREG_GPU_GP0 = v;
}

void gpu_write_gp0_data(uint32_t v)
{
	int i;

	for(i = 0; i < 1000; i+=1) {
		if((PSXREG_GPU_GPUSTAT & (1<<28)) != 0) {
			break;
		}
	}

	PSXREG_GPU_GP0 = v;
}

void gpu_write_gp1(uint32_t v)
{
	PSXREG_GPU_GP1 = v;
}

chenboot_exception_frame_t *isr_main(chenboot_exception_frame_t *sp)
{
	// If this isn't an interrupt, spin
	if((sp->cause&0xFC) != 0) {
		chenboot_isr_disable();
		for(;;) {
		}
	}

	// Get interrupt flags
	int iflags = PSXREG_I_STAT;

	// vblank handler
	if((iflags & (1<<0)) != 0) {
		vblank_counter += 1;
	}

	// Acknowledge all interrupts
	PSXREG_I_STAT = ~iflags;

	// Return
	return sp;
}

int main(int argc, char *argv[])
{
	int vidy = 0;
	int x = 0;
	int y = -45;
	int vx = 4;
	int vy = 0;

	// Install ISR handler
	chenboot_isr_disable();
	PSXREG_I_MASK = 0x0000;
	PSXREG_I_STAT = 0x0000;
	chenboot_isr_install(isr_main);
	chenboot_isr_enable();
	PSXREG_I_MASK = 0x0001;

	// Reset GPU
	gpu_write_gp1(0x00000000);

	// Set video mode
	gpu_write_gp1(0x08000009); // PAL, 320x240 base, 15bpp, progressive
	gpu_write_gp1(0x06000000 | (0x260) | ((0x260+320*8)<<12)); // Xrange
	gpu_write_gp1(0x07000000 | (0xA3-200/2) | ((0xA3+200/2)<<10)); // Yrange
	gpu_write_gp1(0x05000000 | (0) | ((vidy)<<10)); // Output coordinates

	// Clear screen to colour
	gpu_write_gp0_command(0x02402000);
	gpu_write_gp0_data(PAIR16(  0,   0+vidy));
	gpu_write_gp0_data(PAIR16(320, 200+vidy));

	// Enable display
	gpu_write_gp1(0x03000000); // display on

	// Reset vblank counter
	vblank_counter = 0;
	uint32_t expected_vblank_counter = 0;
	uint32_t vblanks = 0;

	// Main loop
	for(;;) {
		//
		// DRAW
		//

		// Swap vidy
		vidy = 200-vidy;

		// Set rendering attribs
		gpu_write_gp0_command(0xE1000600); // Dither, allow draw to display, 4bpp tex
		gpu_write_gp0_command(0xE2000000); // Tex window setting (don't care)
		gpu_write_gp0_command(0xE3000000 | (0) | ((0+vidy)<<10)); // Min X/Y
		gpu_write_gp0_command(0xE4000000 | (320-1) | (((200-1)+vidy)<<10)); // Max X/Y
		gpu_write_gp0_command(0xE5000000 | (320/2) | (((200/2)+vidy)<<11)); // Drawing offset
		gpu_write_gp0_command(0xE6000000); // Mask bit (draw always)

		// Clear screen to colour
		gpu_write_gp0_command(0x02402000);
		gpu_write_gp0_data(PAIR16(  0,   0+vidy));
		gpu_write_gp0_data(PAIR16(320, 200));

		// Draw a triangle
		gpu_write_gp0_command(0x200080FF);
		gpu_write_gp0_data(PAIR16(( 60)+x, (-50)+y));
		gpu_write_gp0_data(PAIR16((  0)+x, ( 70)+y));
		gpu_write_gp0_data(PAIR16((-60)+x, (-50)+y));

		//
		// PHYSICS
		//

		// Move
		for(int i = 0; i < vblanks; i++) {
			vy += 1;
			x += vx;
			y += vy;
			if(y >= 30) {
				y = 30*2-y;
				vy = -vy;
			}
			if(x >= 100 && vx > 0) {
				x = 100*2-x;
				vx = -vx;
			} else if(x <= -100 && vx < 0) {
				x = -100*2-x;
				vx = -vx;
			}
		}

		// Wait for vblank
		do {
			vblanks = vblank_counter - expected_vblank_counter;
		} while(vblanks == 0);
		expected_vblank_counter += vblanks;

		// Swap buffers
		gpu_write_gp1(0x05000000 | (0) | ((vidy)<<10)); // Output coordinates
	}

	return 0;
}
