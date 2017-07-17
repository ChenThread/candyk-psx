/*
orelei: PS SPU sound driver
Copyright (C) GreaseMonkey, 2017, licensed under Creative Commons Zero:
https://creativecommons.org/publicdomain/zero/1.0/
*/

// orelei.c
int orelei_note_to_pitch(int note, int cxoctave, unsigned int cxspeed);
void orelei_commit_key_changes(void);
void orelei_play_note(int ch, int sram_addr, int adsr, int voll, int volr, int pitch);
void orelei_stop_note(int ch);

void orelei_sram_write_blocking(int sram_addr, void const* data, size_t len);
void orelei_pack_spu(uint8_t *outbuf, const int16_t *inbuf, int16_t *pred1, int16_t *pred2, int blocks, int loopbeg, int loopend, bool fade_on_loop);

void orelei_init_spu(void);

// midi.c
#define ORELEI_MIDI_MASTER_COUNT 16
#define ORELEI_MIDI_MAX_TRACKS 32
struct midi_master {
	uint8_t rpn_lo, rpn_hi;
	uint8_t prg;
	uint16_t wheel_depth;
	uint16_t real_wheel;
	int16_t wheel;
};
struct midi_slave {
	uint32_t last_touched;
	uint8_t ch;
	uint8_t note;
};
struct midi_track {
	int32_t ticks_waiting;
	uint8_t last_v0;
	const uint8_t *mptr;
	const uint8_t *mptr_beg;
	const uint8_t *mptr_end;
};

void orelei_midi_reset(void);
void orelei_midi_update(int32_t time_advanced_us, void (*f_play_note)(int hwch, int ch, int prg, int note, int vel, int wheel));
void orelei_midi_load_from_data(const uint8_t *midi_data);

