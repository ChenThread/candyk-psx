/*
seedy: CD-ROM driver

Copyright (c) 2017, 2019 Ben "GreaseMonkey" Russell
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

#define SEEDY_READ_WHOLE_SECTORS 1
#define SEEDY_READ_SINGLE_SPEED 2
#define SEEDY_PLAY_XA_18900 4
#define SEEDY_PLAY_XA_STEREO 8
#define SEEDY_PLAY_XA_8BIT 16
#define SEEDY_PLAY_XA_EMPHASIS 32

// flag-less variants of the above
#define SEEDY_READ_DOUBLE_SPEED 0
#define SEEDY_PLAY_XA_37800 0
#define SEEDY_PLAY_XA_MONO 0
#define SEEDY_PLAY_XA_4BIT 0

int seedy_is_xa_playing(void);
void seedy_read_xa(int lba, int flags, int file, int channel);
void seedy_stop_xa(void);

int seedy_read_data_sync(int lba, int flags, uint8_t *buffer, int size);

void seedy_drive_start(void);
void seedy_drive_stop(void);

void seedy_isr_cdrom(void);
void seedy_init_cdrom(void);

