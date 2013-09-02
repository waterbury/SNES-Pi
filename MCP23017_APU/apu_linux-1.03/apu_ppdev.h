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
#ifndef _apu_ppdev_h__
#define _apu_ppdev_h__

#include "config.h"

#if defined(HAVE_LINUX_PARPORT_H) && defined(HAVE_LINUX_PPDEV_H)
#define PPDEV_SUPPORTED

#include "apu.h"

APU_ops *apu_ppdev_getOps(void);

#endif

#endif // _apu_ppdev_h__

