//-*- C++ -*-
/*
 * lingot, a musical instrument tuner.
 *
 * Copyright (C) 2004-2010  Ibán Cereijo Graña, Jairo Chapela Martínez.
 *
 * This file is part of lingot.
 *
 * lingot is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * lingot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with lingot; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __LINGOT_DEFS_H__
#define __LINGOT_DEFS_H__

#include <sys/types.h>
#include "../config.h"

// floating point precission.
#define FLT                  double

#define SAMPLE_TYPE          int16_t
#define SAMPLE_FORMAT        AFMT_S16_LE

#define CONFIG_DIR_NAME           ".lingot/"
#define DEFAULT_CONFIG_FILE_NAME  "lingot.conf"
extern char CONFIG_FILE_NAME[];

#define QUICK_MESSAGES

#define GTK_EVENTS_RATE      20.0
#define GAUGE_RATE           60.0

/* object forward declaration */
typedef struct _LingotMainFrame LingotMainFrame;

// optionally we can use the following libraries
//#define LIB_FFTW
//#define LIBSNDOBJ

#endif
