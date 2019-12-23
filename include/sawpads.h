/*
sawpads: Actually Tested* Joypad Code
Copyright (C) Chen Thread, 2017, 2019, licensed under Creative Commons Zero:
https://creativecommons.org/publicdomain/zero/1.0/

Code by asie and GreaseMonkey,
the former of whom actually made this code work properly
*/

typedef struct sawpads_controller {
	uint8_t id, hid;
	uint16_t buttons;
	uint16_t analogs;
	uint8_t axes[4];
	uint8_t rumble[2];
} sawpads_controller_t;

int32_t sawpads_read_card_sector(uint16_t address, uint8_t *buffer);
int32_t sawpads_write_card_sector(uint16_t address, uint8_t *buffer);

void sawpads_isr_joy(void);
void sawpads_isr_vblank(void);

void sawpads_do_read_controller(uint8_t port);
void sawpads_do_read(void);
void sawpads_unlock_dualshock(uint8_t port);

extern volatile sawpads_controller_t sawpads_controller[2];
