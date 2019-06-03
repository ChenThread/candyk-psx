/*
seedy: CD-ROM driver
Copyright (C) GreaseMonkey, 2017, licensed under Creative Commons Zero:
https://creativecommons.org/publicdomain/zero/1.0/
*/

#define SEEDY_READ_WHOLE_SECTORS 1
#define SEEDY_READ_SINGLE_SPEED 2

int seedy_read_data_sync(int lba, int flags, uint8_t *buffer, int size);

void seedy_isr_cdrom(void);
void seedy_init_cdrom(void);

