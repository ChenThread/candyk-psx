/*
seedy: CD-ROM driver
Copyright (C) GreaseMonkey, 2017, licensed under Creative Commons Zero:
https://creativecommons.org/publicdomain/zero/1.0/
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

void seedy_isr_cdrom(void);
void seedy_init_cdrom(void);

