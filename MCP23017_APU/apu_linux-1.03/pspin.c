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
#include "pspin.h"

/* progress spinner */

static int pspin_step=0;


void pspin_update(void)
{
	switch(pspin_step)
	{
		default:
		case 4:
			pspin_step = 0;
		case 0:
			printf("|\b"); break;
		case 1:
			printf("/\b"); break;
		case 2:
			printf("-\b"); break;
		case 3:
			printf("\\\b"); break;
	}
	fflush(stdout);
	pspin_step++;
}


