/*
sawpads: Actually Tested* Joypad Code
Copyright (C) Chen Thread, 2017, 2019, licensed under Creative Commons Zero:
https://creativecommons.org/publicdomain/zero/1.0/

Code by asie and GreaseMonkey,
the former of whom actually made this code work properly
*/
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <psxdefs/intc.h>
#include <psxdefs/joy.h>
#include <psxregs.h>
#include <sawpads.h>

volatile sawpads_controller_t sawpads_controller[2];
volatile uint32_t sawpads_read_counter = 0;
volatile uint8_t sawpads_has_ack = 0;
volatile uint16_t sawpads_buffer[16];

static void sawpads_stop_read(void)
{
	PSXREG_JOY_CTRL = 0x0010;
	sawpads_read_counter++;

	// Also kill time
	for(uint32_t i = 0; i < 1000; i++) {
		asm volatile ("");
	}
	sawpads_has_ack = 0;
}

static uint8_t sawpads_recv_solo(void)
{
	while((PSXREG_JOY_STAT & (1<<1)) == 0) {}
	return PSXREG_JOY_DATA;
}

static uint8_t sawpads_send(uint8_t data, bool wait_ack)
{
	sawpads_has_ack = 0;
	if(!wait_ack) {
		while((PSXREG_JOY_STAT & (1<<1)) != 0) {
			asm volatile (""
				:
				: "r"(PSXREG_JOY_DATA)
				:);
		}
	}
	PSXREG_JOY_DATA = data;

	if(wait_ack) {
		for(uint32_t i = 0; i < 0x44*100; i++) {
			asm volatile ("");
			if((sawpads_has_ack>>1) != 0) {
				sawpads_has_ack -= 2;
				sawpads_has_ack &= ~0x01;
				return PSXREG_JOY_DATA;
			}
		}

		return 0xFF; // no data
	} else {
		while((PSXREG_JOY_STAT & (1<<1)) == 0) {}
		// But wait anyway
		for(uint32_t i = 0; i < 0x44*10; i++) { asm volatile (""); }
		return PSXREG_JOY_DATA;
	}
}

static void sawpads_start_read(uint8_t port)
{
	PSXREG_JOY_CTRL = 0x1013 | ((port & 1) << 13);

	// Kill time (more than 2000 cycles is overkill according to nocash)
	for(uint32_t i = 0; i < 1500; i++) {
		asm volatile ("");
	}
}

static uint8_t sawpads_read_words(uint8_t cmd, uint8_t port, uint8_t* response, int response_len)
{
	uint8_t sawpads_hwords = 0;
	volatile sawpads_controller_t *c = &sawpads_controller[port];

	sawpads_start_read(port);
	sawpads_send(0x01, true);
	c->id = sawpads_send(cmd, true);
	sawpads_hwords = (uint8_t) c->id;
	sawpads_hwords -= 1;
	sawpads_hwords &= 0x0F;
	sawpads_hwords += 1;
	c->hid = sawpads_send(0x00, true);
	if (c->hid == 0xFF) {
		// no controller connected
		return 0;
	}
	int j = 0;
	for (int i = 0; i < sawpads_hwords; i++) {
		sawpads_buffer[i] = sawpads_send(j < response_len ? response[j++] : 0xFF, true);
		sawpads_buffer[i] |= sawpads_send(j < response_len ? response[j++] : 0xFF, i != (sawpads_hwords - 1)) << 8;
	}
	sawpads_stop_read();

	return sawpads_hwords;
}

int32_t sawpads_read_card_sector(uint16_t address, uint8_t *buffer)
{
	uint8_t flag;
	uint16_t id;
	int8_t attempts = 3;
	uint8_t checksum = 0;

	while ((--attempts) >= 0) {
		uint16_t conf_addr;

		sawpads_start_read(0);
		sawpads_send(0x81, true);
		flag = sawpads_send(0x52, true);
		id = sawpads_send(0x00, true) << 8;
		id |= sawpads_send(0x00, true);

		// TODO: is this the right way to sense? guessing
		if ((id & 0xFF00) != 0x5A00) {
			sawpads_stop_read();
			continue;
		}

		sawpads_send(address >> 8, true);
		sawpads_send(address & 0xFF, true);

		sawpads_send(0x00, true); // ACK
		sawpads_send(0x00, true); // ACK

		conf_addr = sawpads_send(0x00, true) << 8;
		conf_addr |= sawpads_send(0x00, true);
		if ((conf_addr & 0x3FF) != (address & 0x3FF)) {
			// try again, loop back
			sawpads_stop_read();
			continue;
		}

		checksum = (address >> 8) ^ (address & 0xFF);

		for (int i = 0; i < 128; i++) {
			uint8_t val = sawpads_send(0x00, true);
			buffer[i] = val;
			checksum ^= val;
		}

		if (sawpads_send(0x00, true) != checksum) {
			sawpads_stop_read();
			continue;
		}

		uint8_t result = sawpads_send(0x00, false);
		sawpads_stop_read();
		switch (result) {
			case 0x47:
				return 128;
			default:
				continue;
		}
	}

	return 0;
}

int32_t sawpads_write_card_sector(uint16_t address, uint8_t *buffer)
{
	uint8_t flag;
	uint16_t id;
	int8_t attempts = 3;
	uint8_t checksum = 0;

	while ((--attempts) >= 0) {
		uint16_t conf_addr;

		sawpads_start_read(0);
		sawpads_send(0x81, true);
		flag = sawpads_send(0x57, true);
		id = sawpads_send(0x00, true) << 8;
		id |= sawpads_send(0x00, true);

		// TODO: is this the right way to sense? guessing
		if ((id & 0xFF00) != 0x5A00) {
			sawpads_stop_read();
			continue;
		}

		sawpads_send(address >> 8, true);
		sawpads_send(address & 0xFF, true);

		checksum = (address >> 8) ^ (address & 0xFF);
		for (int i = 0; i < 128; i++) {
			uint8_t val = buffer[i];
			checksum ^= val;
			sawpads_send(val, true);
		}

		sawpads_send(checksum, true);

		sawpads_send(0x00, true); // ACK
		sawpads_send(0x00, true); // ACK

		uint8_t result = sawpads_send(0x00, false);
		sawpads_stop_read();
		switch (result) {
			case 0x47:
				return 128;
			case 0x4E:
				continue;
			case 0xFF:
			default:
				continue;
		}
	}

	return 0;
}

void sawpads_do_read_controller(uint8_t port)
{
	volatile sawpads_controller_t *c = &sawpads_controller[port];
	uint8_t rumble_buf[2];
	rumble_buf[0] = c->rumble[0];
	rumble_buf[1] = c->rumble[1];
	uint8_t sawpads_words = sawpads_read_words(0x42, port, rumble_buf, 2);

	if(sawpads_words >= 1) {
		c->buttons = sawpads_buffer[0];
		if (sawpads_words >= 2) {
			c->analogs = 2;
			c->axes[0] = sawpads_buffer[1] & 0xFF;
			c->axes[1] = sawpads_buffer[1] >> 8;
			if (sawpads_words >= 3) {
				c->analogs = 4;
				c->axes[2] = sawpads_buffer[2] & 0xFF;
				c->axes[3] = sawpads_buffer[2] >> 8;
			}
		}
	} else {
		c->buttons = 0xFFFF;
	}

	if (!(c->id == 0x12 && c->hid == 0x5A) /* mouse */) {
		for (int i = 0; i < c->analogs; i++) {
			if (c->axes[i] > 0x60 && c->axes[i] < 0xA0)
				c->axes[i] = 0x00;
			else
				c->axes[i] ^= 0x80;
		}
	}
	for (int i = c->analogs; i < 4; i++) {
		c->axes[i] = 0x00;
	}
}

void sawpads_do_read(void)
{
	sawpads_do_read_controller(0);
	sawpads_do_read_controller(1);
}

void sawpads_unlock_dualshock(uint8_t port)
{
	static uint8_t response[6];

	// Kick joypad into config mode
	response[0] = 0x01;
	response[1] = 0x00;
	sawpads_read_words(0x43, port, response, 2);

	// Turn on analog mode
	response[0] = 0x01;
	response[1] = 0x03;
	sawpads_read_words(0x44, port, response, 2);

	// Enable rumble
	response[0] = 0x00;
	response[1] = 0x01;
	response[2] = 0xFF;
	response[3] = 0xFF;
	response[4] = 0xFF;
	response[5] = 0xFF;
	sawpads_read_words(0x4D, port, response, 6);

	// Enable pressure
	response[0] = 0xFF;
	response[1] = 0xFF;
	response[2] = 0x03;
	response[3] = 0x00;
	response[4] = 0x00;
	response[5] = 0x00;
	sawpads_read_words(0x4F, port, response, 6);

	// Revert to normal mode
	response[0] = 0x00;
	response[1] = 0x5A;
	response[2] = 0x5A;
	response[3] = 0x5A;
	response[4] = 0x5A;
	response[5] = 0x5A;
	sawpads_read_words(0x43, port, response, 6);
}

void sawpads_isr_vblank(void)
{
	sawpads_has_ack += 1;
}

void sawpads_isr_joy(void)
{
	PSXREG_JOY_CTRL |= 0x0010; // ACK
	sawpads_has_ack += 2;
}

