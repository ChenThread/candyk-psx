/*
seedy: CD-ROM driver
Copyright (C) GreaseMonkey, 2017, licensed under Creative Commons Zero:
https://creativecommons.org/publicdomain/zero/1.0/
*/
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <psxdefs/intc.h>
#include <psxdefs/cdrom.h>
#include <psxregs.h>
#include <seedy.h>

//
// Maths!
//

static inline int tobcd8(int v)
{
	return (v%10)+((v/10)<<4);
}

//
// Raw hardware interface
//

void seedy_wait_until_ready(void)
{
	while((PSXREG_CDROM_In_IDXSR & 0x80) != 0) {
		// wait for busy flag to clear
	}
}

int seedy_poll_interrupt_blocking(void)
{
	PSXREG_CDROM_In_IDXSR = 0x01;
	PSXREG_CDROM_I1_INTFLG = 0x1F;
	while((PSXREG_CDROM_I1_INTFLG & 0x07) == 0x00) {
		// wait for interrupt
	}
	int result = (PSXREG_CDROM_I1_INTFLG & 0x07);
	PSXREG_CDROM_I1_INTFLG = 0x47;
	PSXREG_CDROM_In_IDXSR = 0x00;
	return result;
}

void seedy_ack_main_interrupt(void)
{
	PSXREG_CDROM_In_IDXSR = 0x01;
	PSXREG_CDROM_I1_INTFLG = 0x1F;
	PSXREG_CDROM_In_IDXSR = 0x00;
}

void seedy_write_command(int b)
{
	//seedy_wait_until_ready();
	PSXREG_CDROM_In_IDXSR = 0x00;
	PSXREG_CDROM_I0_CMD = b;
}

void seedy_write_param(int b)
{
	//seedy_wait_until_ready();
	PSXREG_CDROM_In_IDXSR = 0x00;
	PSXREG_CDROM_I0_PARAMS = b;
	//seedy_wait_until_ready();
}

int seedy_read_response(void)
{
	PSXREG_CDROM_In_IDXSR = 0x01;
	return PSXREG_CDROM_I1_RESP_R;
}

void seedy_start_command(void)
{
	seedy_wait_until_ready();
	while((PSXREG_CDROM_In_IDXSR & 0x40) != 0) { volatile int k = PSXREG_CDROM_In_DATA_R; }
	while((PSXREG_CDROM_In_IDXSR & 0x20) != 0) { volatile int k = seedy_read_response(); }
	PSXREG_CDROM_In_IDXSR = 0x01;
	PSXREG_CDROM_I1_INTFLG = 0x1F;
	PSXREG_CDROM_In_IDXSR = 0x00;
}

//
// Raw commands
//

void seedy_send_cmd_getstat(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_GETSTAT);
}

void seedy_send_cmd_setloc(int min, int sec, int sub)
{
	seedy_start_command();
	seedy_write_param(min);
	seedy_write_param(sec);
	seedy_write_param(sub);
	seedy_write_command(CD_CMD_SETLOC);
}

void seedy_send_cmd_setloc_lba(int sector) // Helper
{
	sector += 75*2;
	seedy_send_cmd_setloc(tobcd8(sector/(60*75)), tobcd8((sector/75)%60), tobcd8(sector%75));
}

void seedy_send_cmd_play_notrack()
{
	seedy_start_command();
	seedy_write_command(CD_CMD_PLAY);
}

void seedy_send_cmd_play(int track)
{
	seedy_start_command();
	seedy_write_param(track);
	seedy_write_command(CD_CMD_PLAY);
}

void seedy_send_cmd_forward(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_FORWARD);
}

void seedy_send_cmd_backward(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_BACKWARD);
}

void seedy_send_cmd_readn(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_READN);
}

void seedy_send_cmd_motoron(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_MOTORON);
}

void seedy_send_cmd_stop(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_STOP);
}

void seedy_send_cmd_pause(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_PAUSE);
}

void seedy_send_cmd_init(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_INIT);
}

void seedy_send_cmd_mute(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_MUTE);
}

void seedy_send_cmd_demute(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_DEMUTE);
}

void seedy_send_cmd_setfilter(int file, int channel)
{
	seedy_start_command();
	seedy_write_param(file);
	seedy_write_param(channel);
	seedy_write_command(CD_CMD_SETFILTER);
}

void seedy_send_cmd_setmode(int mode)
{
	seedy_start_command();
	seedy_write_param(mode);
	seedy_write_command(CD_CMD_SETMODE);
}

void seedy_send_cmd_getparam(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_GETPARAM);
}

void seedy_send_cmd_getlocl(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_GETLOCL);
}

void seedy_send_cmd_getlocp(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_GETLOCP);
}

void seedy_send_cmd_setsession(int session)
{
	seedy_start_command();
	seedy_write_param(session);
	seedy_write_command(CD_CMD_SETSESSION);
}

void seedy_send_cmd_gettn(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_GETTN);
}

void seedy_send_cmd_gettd(int track_bcd)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_GETTD);
}

void seedy_send_cmd_getid(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_GETID);
}

void seedy_send_cmd_reads(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_READS);
}

void seedy_send_cmd_reset(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_RESET);
}

void seedy_send_cmd_getq(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_GETQ);
}

void seedy_send_cmd_readtoc(void)
{
	seedy_start_command();
	seedy_write_command(CD_CMD_READTOC);
}

//
// ISR handlers
//

void seedy_isr_cdrom(void)
{
	// TODO!
}

//
// Helpers
//

static int seedy_ack(void)
{
	int init_isr1 = seedy_poll_interrupt_blocking();
	int init_val1 = seedy_read_response();
	seedy_ack_main_interrupt();
	return init_val1;
}

static void seedy_setloc_lba(int lba)
{
	{
		seedy_send_cmd_setsession(0x01);
		int _isr1 = seedy_poll_interrupt_blocking();
		int _val1 = seedy_read_response();
		if(_isr1 == 0x05) {
			seedy_read_response();
			seedy_ack_main_interrupt();
		} else {
			seedy_ack_main_interrupt();
			int _isr2 = seedy_poll_interrupt_blocking();
			int _val2 = seedy_read_response();
			seedy_ack_main_interrupt();
		}
	}

	{
		seedy_send_cmd_setloc_lba(lba);
		int setloc_isr1 = seedy_poll_interrupt_blocking();
		int setloc_val1 = seedy_read_response();
		if(setloc_isr1 == 0x05) {
			seedy_read_response();
		}
		seedy_ack_main_interrupt();
	}
}

void seedy_drive_start(void)
{
	{
		seedy_send_cmd_getstat();
		int stat = seedy_ack();
		if ((stat & 0x2) != 0) return;
	}

	{
		seedy_send_cmd_init();
	}
}

void seedy_drive_stop(void)
{
	seedy_send_cmd_stop();
}

int seedy_is_xa_playing(void)
{
	return ((PSXREG_CDROM_I1_INTFLG >> 2) & 0x01);
}

void seedy_stop_xa(void)
{
	PSXREG_CDROM_In_IDXSR = 0x02;
	PSXREG_CDROM_I2_VOL_LL = 0x00;
	PSXREG_CDROM_I2_VOL_LR = 0x00;
	PSXREG_CDROM_In_IDXSR = 0x03;
	PSXREG_CDROM_I3_VOL_RL = 0x00;
	PSXREG_CDROM_I3_VOL_RR = 0x00;
	PSXREG_CDROM_I3_VOLCTL = 1;
	PSXREG_CDROM_In_IDXSR = 0x00;

	seedy_send_cmd_pause();
	seedy_ack();
}

void seedy_read_xa(int lba, int flags, int file, int channel)
{
	PSXREG_CDROM_In_IDXSR = 0x03;
	PSXREG_CDROM_I3_VOLCTL = 1;
	PSXREG_CDROM_I3_VOL_RL = 0x00;
	PSXREG_CDROM_I3_VOL_RR = 0x80;
	PSXREG_CDROM_In_IDXSR = 0x02;
	PSXREG_CDROM_I2_SMAP_W = (
		((flags & SEEDY_PLAY_XA_STEREO) ? 0x01 : 0)
		| ((flags & SEEDY_PLAY_XA_18900) ? 0x04 : 0)
		| ((flags & SEEDY_PLAY_XA_8BIT) ? 0x10 : 0)
		| ((flags & SEEDY_PLAY_XA_EMPHASIS) ? 0x40 : 0)
	);
	PSXREG_CDROM_I2_VOL_LL = 0x80;
	PSXREG_CDROM_I2_VOL_LR = 0x00;
	PSXREG_CDROM_In_IDXSR = 0x00;

	seedy_send_cmd_setfilter(file, channel);
	seedy_ack();
	seedy_send_cmd_setmode(
		(!(flags & SEEDY_READ_SINGLE_SPEED) ? 0x80 : 0)
		| 0x48
	);
	seedy_ack();
	seedy_setloc_lba(lba);
	seedy_send_cmd_reads();
	seedy_ack();

	PSXREG_CDROM_In_IDXSR = 0x03;
	PSXREG_CDROM_I3_VOLCTL = 0;
	PSXREG_CDROM_In_IDXSR = 0x00;
}

int seedy_read_data_sync(int lba, int flags, uint8_t *buffer, int size)
{
	int bufpos = 0;
	int secsize = (flags & SEEDY_READ_WHOLE_SECTORS) ? 0x924 : 0x800;

	seedy_send_cmd_setmode(
		(!(flags & SEEDY_READ_SINGLE_SPEED) ? 0x80 : 0)
		| ((flags & SEEDY_READ_WHOLE_SECTORS) ? 0x20 : 0)
	);
	seedy_ack();
	seedy_setloc_lba(lba);

	{
		seedy_send_cmd_readn();
		int readn_isr1 = seedy_poll_interrupt_blocking();
		int readn_val1 = seedy_read_response();
		if(readn_isr1 == 0x03) {
			seedy_ack_main_interrupt();
			int readn_isr2 = seedy_poll_interrupt_blocking();
			int readn_val2 = seedy_read_response();
			seedy_ack_main_interrupt();
			if(readn_isr2 == 0x01) {
				PSXREG_CDROM_In_IDXSR = 0x00;
				PSXREG_CDROM_I0_RQST_W = 0x00;
				PSXREG_CDROM_I0_RQST_W = 0x80;
				while((PSXREG_CDROM_In_IDXSR & 0x40) == 0) {
					// wait
				}

				// Read as much of the buffer as we can
				for(int reps = ((size + secsize - 1) / secsize); reps > 0; reps--) {
					PSXREG_CDROM_In_IDXSR = 0x00;
					PSXREG_CDROM_I0_RQST_W = 0x80;
					for(int i = 0; i < secsize; i++) {
						while((PSXREG_CDROM_In_IDXSR & 0x40) == 0) {
							// wait
						}
						buffer[bufpos++] = PSXREG_CDROM_In_DATA_R;
						if (bufpos >= size) break;
					}
					while((PSXREG_CDROM_In_IDXSR & 0x40) != 0) {
						volatile int k = PSXREG_CDROM_In_DATA_R;
					}
					while((PSXREG_CDROM_In_IDXSR & 0x20) != 0) {
						volatile int k = seedy_read_response();
					}

					if(reps > 1) {
						int readn_isr2 = seedy_poll_interrupt_blocking();
						int readn_val2 = seedy_read_response();
						seedy_ack_main_interrupt();
					}
				}

				// Pause the damn thing
				seedy_send_cmd_pause();
				int pause_isr1 = seedy_poll_interrupt_blocking();
				int pause_val1 = seedy_read_response();
				seedy_ack_main_interrupt();
				int pause_isr2 = seedy_poll_interrupt_blocking();
				int pause_val2 = seedy_read_response();
				seedy_ack_main_interrupt();
			}
		} else {
			seedy_read_response();
			seedy_ack_main_interrupt();
		}
	}
}

//
// Initialisation
//

void seedy_init_cdrom(void)
{
	PSXREG_CDROM_In_IDXSR = 0x01;
	PSXREG_CDROM_I1_INTE_W = 0x1F;
	PSXREG_CDROM_I1_INTFLG = 0x1F;
	while((PSXREG_CDROM_In_IDXSR & 0x20) != 0) { volatile int k = seedy_read_response(); }
	while((PSXREG_CDROM_In_IDXSR & 0x40) != 0) { volatile int k = PSXREG_CDROM_In_DATA_R; }

	PSXREG_CDROM_In_IDXSR = 0x00;
	PSXREG_CDROM_I0_RQST_W = 0x00;
	PSXREG_MEM_COM_DELAY = 0x1325;

	for(int i = 0; i < 2; i++) {
		seedy_send_cmd_getstat();
		int getstat_isr1 = seedy_poll_interrupt_blocking();
		int getstat_val1 = seedy_read_response();
		seedy_ack_main_interrupt();
	}

	{
		seedy_send_cmd_init();
		int init_isr1 = seedy_poll_interrupt_blocking();
		int init_val1 = seedy_read_response();
		seedy_ack_main_interrupt();
		int init_isr2 = seedy_poll_interrupt_blocking();
		int init_val2 = seedy_read_response();
		seedy_ack_main_interrupt();
	}

	{
		seedy_send_cmd_demute();
		int demute_isr1 = seedy_poll_interrupt_blocking();
		int demute_val1 = seedy_read_response();
		seedy_ack_main_interrupt();
	}
}

