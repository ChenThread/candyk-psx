/*
orelei: PS SPU sound driver
Copyright (C) GreaseMonkey, 2017, licensed under Creative Commons Zero:
https://creativecommons.org/publicdomain/zero/1.0/
*/

// MIDI component
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <psxdefs/intc.h>
#include <psxdefs/spu.h>
#include <psxregs.h>

#include <orelei.h>

#define ASSERT(x) if (!(x)) { for(;;) {} }

static struct midi_master midi_masters[ORELEI_MIDI_MASTER_COUNT];
static struct midi_slave midi_slaves[SPU_CHANNEL_COUNT];
static struct midi_track midi_tracks[ORELEI_MIDI_MAX_TRACKS];
static uint32_t midi_slaves_on = 0x000000;
static int midi_track_count = 0;
static int midi_divisions = 0;
static uint32_t midi_touch_counter = 0;
static uint32_t midi_tempo = 500000; // 120 BPM

static uint32_t midi_read_u8(uint8_t const** ptr)
{
	uint32_t v = 0;

	v |= ((uint32_t)(*((*ptr)++)));

	return v;
}

static uint32_t midi_read_u16be(uint8_t const** ptr)
{
	uint32_t v = 0;

	v |= ((uint32_t)(*((*ptr)++)))<<8;
	v |= ((uint32_t)(*((*ptr)++)));

	return v;
}

static uint32_t midi_read_u32be(uint8_t const** ptr)
{
	uint32_t v = 0;

	v |= ((uint32_t)(*((*ptr)++)))<<24;
	v |= ((uint32_t)(*((*ptr)++)))<<16;
	v |= ((uint32_t)(*((*ptr)++)))<<8;
	v |= ((uint32_t)(*((*ptr)++)));

	return v;
}

static uint32_t midi_read_delta(uint8_t const** ptr)
{
	uint32_t v = 0;
	int ctr = 0;

	for(;;) {
		uint32_t iv = *((*ptr)++);
		v <<= 7;
		v |= (iv&0x7F);
		if((iv & 0x80) == 0) {
			break;
		}
		ctr++;
		ASSERT(ctr < 4);
	}

	return v;
}

void orelei_midi_reset(void)
{
	midi_touch_counter = 0;
	for(int i = 0; i < SPU_CHANNEL_COUNT; i++) {
		struct midi_slave *S = &midi_slaves[i];
		S->last_touched = 0;
		S->ch = 0xFF;
		S->note = 0;
		orelei_stop_note(i);
	}

	for(int i = 0; i < ORELEI_MIDI_MASTER_COUNT; i++) {
		struct midi_master *M = &midi_masters[i];
		M->prg = 0;
		M->rpn_lo = 0x7F;
		M->rpn_hi = 0x7F;
		M->real_wheel = 0x2000;
		M->wheel = 0x0000;
		M->wheel_depth = 0x0200;
	}

	for(int i = 0; i < midi_track_count; i++) {
		struct midi_track *T = &midi_tracks[i];
		T->mptr = T->mptr_beg;
		T->ticks_waiting = midi_read_delta(&T->mptr);
		T->last_v0 = 0x00;
	}
}

static void midi_note_on(int ch, int note, int vel, void (*f_play_note)(int hwch, int ch, int prg, int note, int vel, int wheel))
{
	int slave_idx = -1;

	// Do we already have this as a slave?
	if(slave_idx == -1) {
		for(int i = 0; i < SPU_CHANNEL_COUNT; i++) {
			struct midi_slave *S = &midi_slaves[i];
			if(S->ch == ch && S->note == note) {
				orelei_stop_note(i);
				S->ch = 0xFF;
				slave_idx = i;
				break;
			}
		}
	}

	// Find the longest-untouched unused slave
	if(slave_idx == -1) {
		for(int i = 0; i < SPU_CHANNEL_COUNT; i++) {
			struct midi_slave *S = &midi_slaves[i];
			if(S->ch == 0xFF) {
				if(slave_idx == -1 || S->last_touched < midi_slaves[slave_idx].last_touched) {
					slave_idx = i;
				}
			}
		}
	}

	// Find the longest-untouched slave
	if(slave_idx == -1) {
		for(int i = 0; i < SPU_CHANNEL_COUNT; i++) {
			struct midi_slave *S = &midi_slaves[i];
			if(slave_idx == -1 || S->last_touched < midi_slaves[slave_idx].last_touched) {
				slave_idx = i;
			}
		}
	}

	ASSERT(slave_idx >= 0 && slave_idx < SPU_CHANNEL_COUNT);

	struct midi_slave *S = &midi_slaves[slave_idx];
	S->ch = ch;
	S->note = note;
	S->last_touched = ++midi_touch_counter;

	// Play note
	struct midi_master *M = &midi_masters[ch];
	f_play_note(slave_idx, ch, M->prg, note, vel, M->wheel);
}

static void midi_note_off(int ch, int note, int vel)
{
	for(int i = 0; i < SPU_CHANNEL_COUNT; i++) {
		struct midi_slave *S = &midi_slaves[i];
		if(S->ch == ch && S->note == note) {
			orelei_stop_note(i);
			S->ch = 0xFF;
			break;
		}
	}
}

static int32_t midi_tick_accum = 0;
void orelei_midi_update(int32_t time_advanced_us, void (*f_play_note)(int hwch, int ch, int prg, int note, int vel, int wheel))
{
	/*
	Divisions = ticks per 1/4-note
	Tempo = whole-notes per second
	*/
	int64_t tickbase = midi_divisions;
	tickbase *= time_advanced_us;
	//tickbase += midi_tempo/2;
	midi_tick_accum += (tickbase % midi_tempo);
	tickbase /= midi_tempo;
	tickbase += midi_tick_accum / midi_tempo;
	midi_tick_accum %= midi_tempo;
	int32_t ticks_advanced = tickbase;

	for(int i = 0; i < midi_track_count; i++) {
		struct midi_track *T = &midi_tracks[i];
		uint8_t const** mpp = &T->mptr;

		T->ticks_waiting -= ticks_advanced;
		while(T->ticks_waiting <= 0) {
			if(T->mptr >= T->mptr_end || T->mptr < T->mptr_beg) {
				break;
			}
			uint8_t v0 = midi_read_u8(mpp);
			uint8_t v1;
			if((v0 & 0x80) != 0) {
				v1 = midi_read_u8(mpp);
				T->last_v0 = v0;
			} else {
				v1 = v0;
				v0 = T->last_v0;
			}
			//*(volatile uint32_t *)0x801FFFF0 = v0;

			struct midi_master *M = &midi_masters[v0&0xF];

			uint8_t v2, v3;
			switch(v0>>4) {
				case 0x8: // Note off
					v2 = midi_read_u8(mpp);
					midi_note_off(v0&0xF, v1, v2);
					break;
				case 0x9: // Note on
					v2 = midi_read_u8(mpp);
					if(v2 == 0x00) {
						// DON'T ASK ME WHY THIS IS A THING.
						// I DIDN'T MAKE THIS STUPID THING.
						midi_note_off(v0&0xF, v1, 0x00);
					} else {
						midi_note_on(v0&0xF, v1, v2, f_play_note);
					}
					break;
				case 0xA: // Key aftertouch
					v2 = midi_read_u8(mpp);
					break;
				case 0xB: // Controller
					v2 = midi_read_u8(mpp);
					switch(v1) {
						case 0x06:
							if(M->rpn_lo == 0x00 && M->rpn_hi == 0x00) {
								// Pitch wheel depth
								M->wheel_depth = ((int)v2)<<7;
							}
							break;
						case 0x64:
							M->rpn_lo = v2;
							break;
						case 0x65:
							M->rpn_hi = v2;
							break;
					}
					break;
				case 0xC: // Program change
					M->prg = v1 & 0x7F;
					break;
				case 0xD: // Instrument aftertouch
					break;
				case 0xE: // Pitch bend
					v2 = midi_read_u8(mpp);
					// ...is LSB first. Good job, MIDI. Slow clap.
					M->real_wheel = (((int32_t)v2)<<7)|((int32_t)v1);
					M->wheel = ((M->real_wheel-0x2000)*M->wheel_depth + (1<<12))>>13;
					for(int i = 0; i < 24; i++) {
						struct midi_slave *S = &midi_slaves[i];
						if(S->ch == (v0&0xF)) {
							f_play_note(i, -1, M->prg, 0, 0, M->wheel);
						}
					}
					break;
				case 0xF: switch(v0) {
					case 0xFF:
						v2 = midi_read_u8(mpp);
						if(v1 == 0x51) {
							ASSERT(v2 == 3);
							midi_tempo = 0;
							midi_tempo |= ((uint32_t)((*mpp)[0]))<<16;
							midi_tempo |= ((uint32_t)((*mpp)[0]))<<8;
							midi_tempo |= ((uint32_t)((*mpp)[0]));
						}
						(*mpp) += v2; // SKIP
						break;
					default:
						//*(volatile uint32_t *)0x801FFFF0 = 'K';
						//*(volatile uint32_t *)0x801FFFF4 = v0;
						//ASSERT(false);
						(*mpp) += v1; // SKIP
						break;
				} break;
			}

			// Read next delta
			int delta = midi_read_delta(mpp);
			ASSERT(delta >= 0);
			T->ticks_waiting += delta;
		}
	}

	orelei_commit_key_changes();
}

void orelei_midi_load_from_data(const uint8_t *midi_data)
{
	// Read header
	ASSERT(!memcmp(midi_data, "MThd", 4));
	midi_data += 4;
	uint32_t mthd_len = midi_read_u32be(&midi_data);
	ASSERT(mthd_len == 6);
	uint32_t midi_format = midi_read_u16be(&midi_data);
	ASSERT(midi_format == 0 || midi_format == 1 || midi_format == 2);
	midi_track_count = midi_read_u16be(&midi_data);
	ASSERT(midi_track_count >= 0 && midi_track_count <= ORELEI_MIDI_MAX_TRACKS);
	midi_divisions = midi_read_u16be(&midi_data);

	// Get each track pointer
	for(int i = 0; i < midi_track_count; i++) {
		struct midi_track *T = &midi_tracks[i];
		ASSERT(!memcmp(midi_data, "MTrk", 4));
		midi_data += 4;
		uint32_t mtrk_len = midi_read_u32be(&midi_data);
		//ASSERT(mtrk_len < 0x20000);
		T->mptr_beg = midi_data;
		midi_data += mtrk_len;
		T->mptr_end = midi_data;
		T->mptr = T->mptr_beg;
	}

	orelei_midi_reset();
}
