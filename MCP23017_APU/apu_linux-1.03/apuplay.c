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
#include <stdlib.h>
#include <unistd.h>
#include "apu.h"
#ifdef PROGRESS_SPINNER
#include "pspin.h"
#endif
#include "bootcode.h"
#include "dsploader.h"
#include "apuplay.h"

/*
 * Ported and comented by Raphael Assenat, 
 * the VB code and DLL by Alpha II, used in APUPLAY.
 * 
 * http://www.caitsith2.com/snes/apu.htm
 */

extern int g_verbose; // from main.c
extern int g_debug; // from main.c
extern int g_progress; // from main.c
extern int g_exit_now; // from main.c
extern int g_playing; // from main.c

int LoadAPU(FILE *fptr)
{
	int i=0, j=0, count=0;
	
	unsigned char spc_pcl;
	unsigned char spc_pch;
	unsigned char spc_a;
	unsigned char spc_x;
	unsigned char spc_y;
	unsigned char spc_sw;
	unsigned char spc_sp;

	unsigned char spcdata[65536];
	unsigned char spcram[64];
	unsigned char dspdata[128];

	int echosize, echoregion, bootptr, echoclear=2, readcount;
	
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


	
	fseek(fptr, 0x100, SEEK_SET);
	fread(spcdata, 65536, 1, fptr);

	fseek(fptr, 0x10100, SEEK_SET);
	fread(dspdata, 128, 1, fptr);
	
	fseek(fptr, 0x101c0, SEEK_SET);
	fread(spcram, 64, 1, fptr);

	/* save a bunch of registers to be restored
	 * later by the "bootcode" */
	bootcode[BOOT_SPC_PORT0] = spcdata[SPC_PORT0];
	bootcode[BOOT_SPC_PORT1] = spcdata[SPC_PORT1];
	bootcode[BOOT_SPC_PORT2] = spcdata[SPC_PORT2];
	bootcode[BOOT_SPC_PORT3] = spcdata[SPC_PORT3];
	bootcode[0x01] = spcdata[0x00];
	bootcode[0x04] = spcdata[0x01];
	bootcode[BOOT_SPC_TIMER2] = spcdata[SPC_TIMER2];
	bootcode[BOOT_SPC_TIMER1] = spcdata[SPC_TIMER1];
	bootcode[BOOT_SPC_TIMER0] = spcdata[SPC_TIMER0];
	bootcode[BOOT_SPC_CONTROL] = spcdata[SPC_CONTROL];
	bootcode[BOOT_DSP_FLG] = dspdata[DSP_FLG];
	bootcode[BOOT_DSP_KON] = dspdata[DSP_KON];
	bootcode[BOOT_SPC_REGADD] = spcdata[SPC_REGADD];
	bootcode[BOOT_A] = spc_a;
	bootcode[BOOT_Y] = spc_y;
	bootcode[BOOT_X] = spc_x;
	
	/* push program counter and status ward on stack */
	spcdata[0x100 + spc_sp - 0] = spc_pch;
	spcdata[0x100 + spc_sp - 1] = spc_pcl;
	spcdata[0x100 + spc_sp - 2] = spc_sw;
	bootcode[BOOT_SP] = spc_sp - 3; // save new stack pointer

	/* mute all voices */
	dspdata[DSP_FLG] = DSP_FLG_MUTE|DSP_FLG_ECEN;
	dspdata[DSP_KON] = 0x00; // Voice 0-7 off 

	/* to produce an echo effect, the dsp uses a memory region.
	 * ESA: Esa * 100h becomes the lead-off address of the echo
	 * region. Calculate this address... */
	echoregion = dspdata[DSP_ESA] * 256;

	/* echo delay. The bigger the delay is, more memory is needed.
	 * calculate how much memory used... */
	echosize = dspdata[DSP_EDL] * 2048;
	if (echosize==0) { echosize = 4; }

	/* we need to find a place to install our boot code, 
	 * which is 77 byte big.
	 * 	
	 * we attempt to find 77 consecutive and identical bytes anywhere
	 * in the memory, minus the bootrom area, minus the
	 * page 0 (registers) and page 1 (stack), minus the
	 * echo region. */
	count = 0;
	for (j=255; j>=0; j--)
	{
		for (bootptr = 65471; bootptr >= 0x100; bootptr--)
		{
			if (	(bootptr > echoregion + echosize) ||
					(bootptr < echoregion) )
			{
				if (spcdata[bootptr] == j)
				{
					count++;
				}
				else
				{
					count = 0;
				}
				if (count == 77) { break; }
			}
			else
			{
				count = 0;
			}
		}
		if (count == 77) { break; }
	}

	/* we did not find an area of 77 consecutive identical byte values. */
	if (bootptr == 0xff)
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

	if (g_debug) { printf("Count: %d\n", count); }

	/* Copy our bootcode into the area we found */
	for (i=bootptr; i<=bootptr+count-1; i++) {
		spcdata[i] = bootcode[i - bootptr];
	}
	
	apu_reset();
	apu_initTransfer(0x0002);

	if (g_verbose) 
		printf("Restoring dsp...\n");

	if (g_exit_now || !g_playing) { apu_reset(); return 0; }
	
	/* first, we send a small program called the dsploader which we will
	 * use to restore the DSP registers (with our modified KON and FLG to
	 * keep it silent) */
	if (apu_writeBytes(dsploader, 16)) {
		fprintf(stderr, "Timeout sending dsploader\n");
		return -1;
	}

	if (g_exit_now || !g_playing) { apu_reset(); return 0; }
	
	apu_endTransfer(0x0002);
	
	/* restore the 128 dsp registers one by one with the help of the dsp loader.
	 * (with our modified KON and FLG)
	 */
	for (i=0; i<128; i++)
	{
		if (g_exit_now || !g_playing) { apu_reset(); return 0; }
#ifdef PROGRESS_SPINNER
		if (g_progress) {
			pspin_update();
		}
#endif
		apu_write(1, dspdata[i]);
		apu_write(0, i);
		if (!apu_waitInport(0, i, 500)) {
			if (apu_read(0)==0xaa)
			{
	//			fprintf(stderr, "ingored\n");
			}
			else
			{
				fprintf(stderr, "timeout 3\n"); return -1; 
			}
		}
	}

	if (apu_initTransfer(0x0002)<0) {
		fprintf(stderr, "timeout 4\n"); return -1; 
	}

	if (g_verbose) 
		printf("Restoring spc memory...\n");

	/* send the first part of the memory (0x02 to 0xef)
	 * After 0xef comes spc700 registers (0xf0 to 0xff). Those
	 * are taken care of by the bootcode */
	for (i=2; i<=0xef; i++)
	{
		apu_write(1, spcdata[i]);
		apu_write(0, i-2);
		if (!apu_waitInport(0, i-2, 500)) {
			fprintf(stderr, "timeout 5\n"); return -1; 
		}
#ifdef PROGRESS_SPINNER
		if (g_progress) 
			pspin_update();
#endif
		if (g_exit_now || !g_playing) { apu_reset(); return 0; }
	}

	apu_newTransfer(0x0100);

	/* upload the external memory region data (0x100 (page 1) to 0xffc0 (rom) */	
	for (i=0x100; i <= 65471; i+= 16)
	{
		// echoclear = 0 : Clear echo region with 0's if enable in DSP_FLG
		// echoclear = 1 : Clear echo region with 0's, regardless of DSP_FLG
		// echoclear = 2 : Dont touch echo region
#ifndef NO_ECHOCLEAR_STUFF
		if (
			(i >= echoregion) && 
			(i<=(echoregion + echosize)) &&
			(bootptr != echoregion)
			)
		{
			for (j=0; j<16; j++)
			{
				if ( ((bootcode[BOOT_DSP_FLG] & DSP_FLG_ECEN) == 0) && 
					(echoclear == 0)) {
					spcdata[i+j]=0;
				} else if (echoclear == 1) {
					spcdata[i+j]=0;
				}
			}
		}
#endif

		if (apu_writeBytes(&spcdata[i], 16))
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

	/* upload the external memory area overlapping with the rom... I guess
	 * if we write to those address from the SPC it really writes to this
	 * memory area, but if you read you'll probably get the ROM code. Maybe
	 * it's really Read/Write from the DSP point of view... TODO: Check this 
	 *
	 * Maybe also setting SPC_CONTROL msb bit enables this region? It's not
	 * documented my manual...
	 * */
	for (i=65472; i<= 65535; i++)
	{
		if (spcdata[SPC_CONTROL] & 0x80) {
			#ifndef NO_ECHOCLEAR_STUFF
			if (echosize + echoregion > 65472)
			{
				// echoclear = 0 : Clear echo region with 0's if enable in DSP_FLG
				// echoclear = 1 : Clear echo region with 0's, regardless of DSP_FLG
				// echoclear = 2 : Dont touch echo region
				if ( ((bootcode[BOOT_DSP_FLG] & DSP_FLG_ECEN)==0) &&
					echoclear == 0)
				{
					spcram[i-65472] = 0;					
				} else if (echoclear == 1) {
					spcram[i-65472] = 0;
				}
			}
			#endif
				
			if (apu_writeHandshake(1, spcram[i-65472])==1) {
				fprintf(stderr, "some error\n");
				return -1;
			}
		}
		else {
			#ifndef NO_ECHOCLEAR_STUFF
			if (echosize + echoregion > 65472) {
				// echoclear = 0 : Clear echo region with 0's if enable in DSP_FLG
				// echoclear = 1 : Clear echo region with 0's, regardless of DSP_FLG
				// echoclear = 2 : Dont touch echo region
				if ( ((bootcode[BOOT_DSP_FLG] & DSP_FLG_ECEN)==0) &&
						echoclear ==0) {
					spcdata[i]=0;
				} else if (echoclear == 1) {
					spcdata[i]=0;
				}
			}
			#endif

			if (apu_writeHandshake(1, spcram[i-65472])==1) {
				fprintf(stderr, "some error AGAIN\n");
				return -1;
			}
			
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

	/* Tell the APU where it should jump to start the program
	 * will just uploaded. It will enter our bootcode, and jump
	 * back to the original PC from the .spc with registers in
	 * the same state as in the .spc */
	apu_endTransfer(bootptr);

/*	if (!apu_waitInport(0, 0x53, 500)) {
		fprintf(stderr, "timeout 7\n");
		return -1;

	}
*/

	/* Restore the ports to the value they
	 * had in the .spc */
	apu_write(0, spcdata[SPC_PORT0]);
	apu_write(1, spcdata[SPC_PORT1]);
	apu_write(2, spcdata[SPC_PORT2]);
	apu_write(3, spcdata[SPC_PORT3]);

	if (g_debug) {
		printf("Boot ptr: %04X\n", bootptr);
		printf("Echo pointer: %04X\n", echoregion);
		printf("Echo size: %04X\n", echosize);
	}
	
	if (g_exit_now || !g_playing) { apu_reset(); return 0; }
	return 0;
}


