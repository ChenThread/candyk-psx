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
	PSXREG_CDROM_I1_INTFLG = 0x5F;
	while((PSXREG_CDROM_I1_INTFLG & 0x07) == 0x00) {
		// wait for interrupt
	}
	int result = (PSXREG_CDROM_I1_INTFLG & 0x07);
	return result;
}

void seedy_ack_main_interrupt(void)
{
	PSXREG_CDROM_In_IDXSR = 0x01;
	PSXREG_CDROM_I1_INTFLG = 0x1F;
}

void seedy_write_command(int b)
{
	//seedy_wait_until_ready();
	//PSXREG_CDROM_In_IDXSR = 0x00;
	PSXREG_CDROM_I0_CMD = b;
}

void seedy_write_param(int b)
{
	//seedy_wait_until_ready();
	//PSXREG_CDROM_In_IDXSR = 0x00;
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
	//while((PSXREG_CDROM_In_IDXSR & 0x40) != 0) { volatile int k = PSXREG_CDROM_In_DATA_R; }
	//while((PSXREG_CDROM_In_IDXSR & 0x20) != 0) { volatile int k = seedy_read_response(); }
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
	sector += 75*60*2;
	seedy_send_cmd_setloc(sector/(60*75), (sector/75)%60, sector%75);
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
	seedy_write_command(CD_CMD_SETFILTER);
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
		seedy_send_cmd_setloc_lba(16);
		int setloc_isr1 = seedy_poll_interrupt_blocking();
		int setloc_val1 = seedy_read_response();
		if(setloc_isr1 == 0x05) {
			seedy_read_response();
		}
		seedy_ack_main_interrupt();
	}

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
				uint8_t *ptr = (uint8_t *)0x1FE000;
				for(int reps = 1; reps > 0; reps--) {
					PSXREG_CDROM_In_IDXSR = 0x00;
					PSXREG_CDROM_I0_RQST_W = 0x80;
					for(int i = 0; i < 0x800; i++) {
						while((PSXREG_CDROM_In_IDXSR & 0x40) == 0) {
							// wait
						}
						ptr[i] = PSXREG_CDROM_In_DATA_R;
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

