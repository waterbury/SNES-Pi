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
#ifndef _apu_play_h__
#define _apu_play_h__


int LoadAPU(FILE *fptr);
int LoadAPU_embedded(FILE *fptr);

/* offset in a .spc file */ 
#define OFFSET_SPCDATA	0x100
#define OFFSET_DSPDATA	0x10100
#define OFFSET_SPCRAM	0x101c0

/* Some SPC registers */
#define SPC_PORT0	0xf4
#define SPC_PORT1	0xf5
#define SPC_PORT2	0xf6
#define SPC_PORT3	0xf7
#define SPC_TIMER0	0xfa
#define SPC_TIMER1	0xfb
#define SPC_TIMER2	0xfc
#define SPC_CONTROL	0xf1
#define SPC_REGADD	0xf2

/* some Dsp registers address and bits */
#define DSP_FLG			0x6C
	#define DSP_FLG_RES 	0x80
	#define DSP_FLG_MUTE	0x40
	#define DSP_FLG_ECEN	0x20
#define DSP_ESA 0x6D
#define DSP_EDL	0x7D
#define DSP_KON	0x4C

#define BOOT_SPC_PORT0	0x19
#define BOOT_SPC_PORT1	0x1f
#define BOOT_SPC_PORT2	0x25
#define BOOT_SPC_PORT3	0x2b
#define BOOT_SPC_TIMER2	0x07
#define BOOT_SPC_TIMER1	0x0a
#define BOOT_SPC_TIMER0	0x0d
#define BOOT_SPC_CONTROL	0x10
#define BOOT_DSP_FLG	0x38
#define BOOT_DSP_KON	0x3e
#define BOOT_SPC_REGADD	0x41
#define BOOT_A	0x47
#define BOOT_Y	0x49
#define BOOT_X	0x4b
#define BOOT_SP	0x44

#endif // _apu_play_h__

