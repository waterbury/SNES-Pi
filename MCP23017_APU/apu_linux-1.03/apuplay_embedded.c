/* hwapu - SPC music playback tools for real snes apu
 * Copyright (C) 2004-2005  Raphael Assenat <raph@raphnet.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <stdio.h>
#include "apuplay.h"
#include "apu.h"
#include "dsploader.h"
#include "bootcode.h"
#ifdef PROGRESS_SPINNER
#include "pspin.h"
#endif

extern int g_debug;
extern int g_verbose;
extern int g_exit_now;
extern int g_playing;
extern int g_progress;

int LoadAPU_embedded(FILE *fptr)
{
	int i=0, j=0, count=0, val=0;
	
	unsigned char spc_pcl;
	unsigned char spc_pch;
	unsigned char spc_a;
	unsigned char spc_x;
	unsigned char spc_y;
	unsigned char spc_sw;
	unsigned char spc_sp;

//	unsigned char spcdata[65536];
//	unsigned char spcram[64];

	unsigned char dsp_kon=0;
	unsigned char dsp_flg=0;
	unsigned char dsp_esa=0;
	unsigned char dsp_edl=0;
	
	unsigned char workbuf[64];
	
	int echosize, echoregion, bootptr, readcount;
	
	fseek(fptr, 0x25, SEEK_SET);

	fread(&spc_pcl, 1, 1, fptr);
	fread(&spc_pch, 1, 1, fptr);
	fread(&spc_a, 1, 1, fptr);
	fread(&spc_x, 1, 1, fptr);
	fread(&spc_y, 1, 1, fptr);
	fread(&spc_sw, 1, 1, fptr);
	fread(&spc_sp, 1, 1, fptr);

	if (g_debug) {
		printf("PC: %02x%02x\n", spc_pch, spc_pcl);
		printf("A: %02X\n", spc_a);
		printf("X: %02X\n", spc_x);
		printf("Y: %02X\n", spc_y);
		printf("SW: %02X\n", spc_sw);
		printf("SP: %02X\n", spc_sp);
	}

	apu_reset();
	apu_initTransfer(0x0002);

	if (g_verbose) 
		printf("Restoring dsp registers...\n");

	if (g_exit_now || !g_playing) { apu_reset(); return 0; }
	
	/* first, we send a small program called the dsploader which we will
	 * use to restore the DSP registers (with our modified KON and FLG to
	 * keep it silent) */
	if (apu_writeBytes(dsploader, 16)) {
		fprintf(stderr, "Timeout sending dsploader\n");
		return -1;
	}
	apu_endTransfer(0x0002);	
	
	if (g_exit_now || !g_playing) { apu_reset(); return 0; }

	/* restore the 128 dsp registers one by one with the help of the dsp loader. */
	fseek(fptr, OFFSET_DSPDATA, SEEK_SET);
	for (i=0; i<128; i+=64)
	{
		fread(workbuf, 64, 1, fptr);
		for (j=0; j<64; j++)
		{
			/* mute all voices and stop all notes */
			if (i+j == DSP_FLG) {
				dsp_flg = workbuf[j]; // save it for later
				workbuf[j] = DSP_FLG_MUTE|DSP_FLG_ECEN;
			}
			if (i+j == DSP_KON) {
				dsp_kon = workbuf[j]; // save it for later
				workbuf[j] = 0x00;
			}

			// take note of some values while we upload...
			if (i+j == DSP_ESA) { dsp_esa = workbuf[j]; }
			if (i+j == DSP_EDL) { dsp_edl = workbuf[j]; }
			
			apu_write(1, workbuf[j]);
			apu_write(0, i+j);
			if (!apu_waitInport(0, i+j, 500)) {
				if (apu_read(0)==0xaa) {
//				fprintf(stderr, "ingored\n");
				} else {
					fprintf(stderr, "timeout 3\n"); return -1; 
				}
			}

			if (g_exit_now || !g_playing) { apu_reset(); return 0; }
#ifdef PROGRESS_SPINNER
			if (g_progress) {
				pspin_update();
			}
#endif
		}

	}
//	if (g_verbose) 
//		printf("\n");

	/* after receiving 128 registers, the dsp loaded will jump
	 * inside the rom at address $ffc9. Once 0xAA appears in 
	 * port0, the apu is ready for a new transfer. */
	if (!apu_waitInport(0, 0xaa, 500)) {
		fprintf(stderr, "timeout 4\n"); return -1; 
	}

//	fseek(fptr, OFFSET_SPCRAM, SEEK_SET);
//	fread(spcram, 64, 1, fptr);

	/* save a bunch of registers to be restored
	 * later by the "bootcode" */
	bootcode[BOOT_DSP_FLG] = dsp_flg;
	bootcode[BOOT_DSP_KON] = dsp_kon;
	bootcode[BOOT_A] = spc_a;
	bootcode[BOOT_Y] = spc_y;
	bootcode[BOOT_X] = spc_x;
	bootcode[BOOT_SP] = spc_sp - 3; // save new stack pointer

	/* save address $0000 and $0001 to be restored by "bootcode" */
	fseek(fptr, OFFSET_SPCDATA, SEEK_SET);
	fread(workbuf, 2, 1, fptr);
	bootcode[0x01] = workbuf[0];
	bootcode[0x04] = workbuf[1];
	
	/* save most spc registers (0xf0 to 0xff) into bootcode to be restored
	 * later */
	fseek(fptr, OFFSET_SPCDATA+0xf0, SEEK_SET);
	fread(workbuf, 0x10, 1, fptr);
	for (i=0xf0; i<=0xff; i++) {
		switch (i)
		{
			case SPC_PORT0: bootcode[BOOT_SPC_PORT0] = workbuf[i-0xf0]; break;
			case SPC_PORT1: bootcode[BOOT_SPC_PORT1] = workbuf[i-0xf0]; break;
			case SPC_PORT2: bootcode[BOOT_SPC_PORT2] = workbuf[i-0xf0]; break;
			case SPC_PORT3: bootcode[BOOT_SPC_PORT3] = workbuf[i-0xf0]; break;
			case SPC_TIMER0: bootcode[BOOT_SPC_TIMER0] = workbuf[i-0xf0]; break;
			case SPC_TIMER1: bootcode[BOOT_SPC_TIMER1] = workbuf[i-0xf0]; break;
			case SPC_TIMER2: bootcode[BOOT_SPC_TIMER2] = workbuf[i-0xf0]; break;
			case SPC_CONTROL: bootcode[BOOT_SPC_CONTROL] = workbuf[i-0xf0]; break;
			case SPC_REGADD: bootcode[BOOT_SPC_REGADD] = workbuf[i-0xf0]; break;
		}
	}
	


	/* to produce an echo effect, the dsp uses a memory region.
	 * ESA: Esa * 100h becomes the lead-off address of the echo
	 * region. Calculate this address... */
	echoregion = dsp_esa * 256;

	/* echo delay. The bigger the delay is, more memory is needed.
	 * calculate how much memory used... */
	echosize = dsp_edl * 2048;
	if (echosize==0) { echosize = 4; }

	if (g_debug) { 
		printf("debug: echoregion: $%04x, size %d\n", echoregion, echosize);
	}

	apu_initTransfer(0x0002);
	if (g_verbose) 
		printf("Restoring spc700 memory...\n");
	
	if (g_debug) { 
		printf("debug: Sending spc memory from 0x02 to 0xef\n");
	}
	/* send the first part of the memory (0x02 to 0xef)
	 * After 0xef comes spc700 registers (0xf0 to 0xff). Those
	 * are taken care of by the bootcode. 0x00 and 0x01 are
	 * retored by the bootcode too. */
	fseek(fptr, OFFSET_SPCDATA, SEEK_SET);
	for (j=0; j<256; j+=64)
	{
		fread(workbuf, 64, 1, fptr);
		for (i=0; i<0x40; i++) {
			if (j+i>=0xf0) { break; }
			if (j==0 && i<2) { continue; } // skip $0000 and $0001
			apu_write(1, workbuf[i]);
			apu_write(0, j+i-2);
			if (!apu_waitInport(0, j+i-2, 500)) {
				fprintf(stderr, "timeout 5\n"); return -1; 
			}
#ifdef PROGRESS_SPINNER
			if (g_progress) 
				pspin_update();
#endif
			if (g_exit_now || !g_playing) { apu_reset(); return 0; }
		}
		if (j+i>=0xf0) { break; }
	}

//	if (g_verbose) 
//		printf("\n");

	if (apu_newTransfer(0x100)) { apu_reset(); return -1; }
	
	if (g_debug) { 
		printf("debug: Sending spc memory from 0x100 to 0xffc0\n");
	}
	/* upload the external memory region data (0x100 (page 1) to 0xffbf (rom),
	 * and look for an area with the same consecutive value repeated 77 times */	
	fseek(fptr, OFFSET_SPCDATA+0x100, SEEK_SET);
	bootptr = -1;
	for (i=0x100; i <= 65471; i+= 16)
	{		
		fread(workbuf, 16, 1, fptr);
		
		for (j=0; j<16; j++) {
			/* push program counter and status ward on stack */
			if ((i+j) == (0x100 +spc_sp - 0)) {
				workbuf[j] = spc_pch;
			}
			if ((i+j) == (0x100 +spc_sp - 1)) {
				workbuf[j] = spc_pcl;
			}
			if ((i+j) == (0x100 +spc_sp - 2)) {
				workbuf[j] = spc_sw;
			}
			
			if ((i > echoregion + echosize) || (i < echoregion) )
			{
				if (val==workbuf[j]) {
					count++;
					if (count>=77) {
						bootptr = i+j-77;
//						printf("nbptr: %d\n", i+j-77);
					}
				}
				else {
					val = workbuf[j];
					count = 0;
				}
			}
			else
			{
				count = 0;
			}
		}
		
		if (apu_writeBytes(workbuf, 16))
		{
			fprintf(stderr, "Transfer error\n");
			return -1;
		}

		if (i % 256 == 0) {
			readcount += 256;
#ifdef PROGRESS_SPINNER
			if (g_progress) {
				pspin_update();
			}
#endif
		}
		if (g_exit_now || !g_playing) { apu_reset(); return 0; }
	}
	
//	bootptr = 0x2e47;
	if (g_debug) { 	
		printf("debug: area for bootcode: $%04x (%02X)\n", bootptr, val);
	}
	
	/* we did not find an area of 77 consecutive identical byte values. */
	if (bootptr == -1) 
	{
		/* We will have to use the echo region. The region will need to be
		 * at least 77 bytes... */
		if (echosize < 77) {
			fprintf(stderr, "This spc file does not have sufficient ram to be loaded");
			return -1;
		}
		else {
			/* we will use the echo region */
			bootptr = echoregion;
		}
	}

	if (g_debug) { 
		printf("debug: Sending spc memory from 0xffc0 to 0xffff\n");
	}
	/* upload the external memory area overlapping with the rom... I guess
	 * if we write to those address from the SPC it really writes to this
	 * memory area, but if you read you'll probably get the ROM code. Maybe
	 * it's really Read/Write from the DSP point of view... TODO: Check this 
	 *
	 * Maybe also setting SPC_CONTROL msb bit enables this region? It's not
	 * documented my manual...
	 * */
	if (bootcode[BOOT_SPC_CONTROL] & 0x80) {
		fseek(fptr, OFFSET_SPCRAM, SEEK_SET);
		fread(workbuf, 64, 1, fptr);
	}
	else {
		fseek(fptr, OFFSET_SPCDATA + 65472, SEEK_SET);
		fread(workbuf, 64, 1, fptr);
	}
	
	if (apu_writeBytes(workbuf, 64)) {
		return -1;
	}

//	if (g_verbose) 
//		printf("\n");

	if (apu_newTransfer(bootptr)) { apu_reset(); return -1; }

	/* Copy our bootcode into the area we found */
	if (apu_writeBytes(bootcode, 77)) {
		fprintf(stderr, "Bootcode transfer error\n");
		return -1;
	}

	apu_endTransfer(bootptr);
	
	//i = 0;
	if (!apu_waitInport(0, 0x53, 500)) {
		fprintf(stderr, "timeout 7\n");
		return -1;
	}

	if (g_debug) {
		printf("Setting final port values $%02X $%02X $%02X $%02X\n",
				bootcode[BOOT_SPC_PORT0], bootcode[BOOT_SPC_PORT1], 
				bootcode[BOOT_SPC_PORT2], bootcode[BOOT_SPC_PORT3]);
	}

	/* Restore the ports to the value they
	 * had in the .spc  (this is not done by the bootcode because
	 * Port0-3 have 2 different values (The value set internally is
	 * seen externaly and the value seen internally is set externally) */
	apu_write(0, bootcode[BOOT_SPC_PORT0]);
	apu_write(1, bootcode[BOOT_SPC_PORT1]);
	apu_write(2, bootcode[BOOT_SPC_PORT2]);
	apu_write(3, bootcode[BOOT_SPC_PORT3]);

	
	if (g_exit_now || !g_playing) { apu_reset(); return 0; }
	return 0;
}


