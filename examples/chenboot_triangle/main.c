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

#include <chenboot.h>
#include <psxdefs/gpu.h>
#include <psxdefs/intc.h>
#include <psxregs.h>

volatile uint32_t vblank_counter = 0;
volatile int out_vidy = 0;

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
	if((iflags & INTC_VBLANK) != 0) {
		vblank_counter += 1;
		gpu_write_gp1(GP1_DISPLAY_START(0, out_vidy));
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
	PSXREG_I_MASK = (0
		| INTC_VBLANK
	);

	// Reset GPU
	gpu_write_gp1(GP1_RESET_GPU());

	// Set video mode
	gpu_write_gp1(GP1_DISPLAY_MODE(
		GPU_DISPMODE_W_320,
		GPU_DISPMODE_H_240P,
		GPU_DISPMODE_BPP_15,
		GPU_DISPMODE_STD_PAL));
	gpu_write_gp1(GP1_DISPLAY_RANGE_X(0x260, 0x260 + 320*8));
	gpu_write_gp1(GP1_DISPLAY_RANGE_Y(0xA3 - 200/2, 0xA3 + 200/2));
	gpu_write_gp1(GP1_DISPLAY_START(0, vidy));

	// Clear screen to colour
	gpu_write_gp0_command(GP0_MEM_FILL(GPU_RGB8(0x00, 0x20, 0x40)));
	gpu_write_gp0_data(GPU_BOX_OFFS(  0,   0+vidy));
	gpu_write_gp0_data(GPU_BOX_SIZE(320, 200));

	// Enable display
	gpu_write_gp1(GP1_DISPLAY_ENABLE());

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
		gpu_write_gp0_command(GP0_ATTR_TEXPAGE(
			0,
			0,
			GPU_TEXPAGE_BLEND_HALF,
			GPU_TEXPAGE_TEXBPP_4,
			GPU_TEXPAGE_DITHER | GPU_TEXPAGE_DRAWTODISPLAY));
		gpu_write_gp0_command(GP0_ATTR_TEXWINDOW(0,0,0,0));
		gpu_write_gp0_command(GP0_ATTR_DRAW_RANGE_MIN(0, 0+vidy));
		gpu_write_gp0_command(GP0_ATTR_DRAW_RANGE_MAX(320-1, (200-1)+vidy));
		gpu_write_gp0_command(GP0_ATTR_DRAW_OFFSET(320/2, (200/2)+vidy));
		gpu_write_gp0_command(GP0_ATTR_MASKBIT(0));

		// Clear screen to colour
		gpu_write_gp0_command(GP0_MEM_FILL(GPU_RGB8(0x00, 0x20, 0x40)));
		gpu_write_gp0_data(GPU_BOX_OFFS(  0,   0+vidy));
		gpu_write_gp0_data(GPU_BOX_SIZE(320, 200));

		// Draw a triangle
		gpu_write_gp0_command(GP0_TRI_FLAT(GPU_RGB8(0xFF, 0x80, 0x00)));
		gpu_write_gp0_data(GPU_VERTEX(( 60)+x, (-50)+y));
		gpu_write_gp0_data(GPU_VERTEX((  0)+x, ( 70)+y));
		gpu_write_gp0_data(GPU_VERTEX((-60)+x, (-50)+y));

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

		out_vidy = vidy;

		// Wait for at least one vblank
		do {
			vblanks = vblank_counter - expected_vblank_counter;
		} while(vblanks == 0);
		expected_vblank_counter += vblanks;
	}

	return 0;
}
