/*
sawpads: Actually Tested* Joypad Code
Copyright (C) Chen Thread, 2017, licensed under Creative Commons Zero:
https://creativecommons.org/publicdomain/zero/1.0/

Code by asie and GreaseMonkey,
the former of whom actually made this code work properly
*/

void sawpads_unlock_dualshock(void);
void sawpads_do_read(void);

extern volatile uint8_t sawpads_id;
extern volatile uint8_t sawpads_hid;
extern volatile uint16_t sawpads_buttons;
extern volatile uint8_t sawpads_axes[4];
extern volatile uint32_t sawpads_read_counter;
extern volatile uint8_t sawpads_rumble[2];

