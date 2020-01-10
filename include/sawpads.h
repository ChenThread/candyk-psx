/*
sawpads: Actually Tested* Joypad Code

Copyright (c) 2017, 2019 Ben "GreaseMonkey" Russell
Copyright (c) 2019, 2020 Adrian "asie" Siekierka

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

typedef struct sawpads_controller {
	uint8_t id, hid;
	uint16_t buttons;
	uint16_t analogs;
	uint8_t axes[4];
	uint8_t rumble[2];
} sawpads_controller_t;

int32_t sawpads_read_card_sector(uint8_t port, uint16_t address, uint8_t *buffer);
int32_t sawpads_write_card_sector(uint8_t port, uint16_t address, uint8_t *buffer);

void sawpads_isr_joy(void);
void sawpads_isr_vblank(void);

void sawpads_do_read_controller(uint8_t port);
void sawpads_do_read(void);
void sawpads_unlock_dualshock(uint8_t port);

extern volatile sawpads_controller_t sawpads_controller[2];
