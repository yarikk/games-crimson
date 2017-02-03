// Crimson Fields -- a game of tactical warfare
// Copyright (C) 2000-2009 Jens Granseuer
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

///////////////////////////////////////////////////////////////
// globals.h - global variable declarations
// This file can be included by both C and C++ code.
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_GLOBALS_H
#define _INCLUDE_GLOBALS_H

/* data paths and files
 * the symbol VERSION should be defined externally (usually via autoconf)
 * the symbol CF_DATADIR may be defined externally to define the location
 *        of data files. If it is not defined the program will try to
 *        locate the files at runtime (default for Win32).
 */

#ifndef VERSION
# define VERSION "0.5.3"
#endif

#define CF_SHORTNAME	"crimson"
#define CF_DATFILE	"cf.dat"

#define CF_FONT		"Bepa-Roman.ttf"
#define CF_FONT_SMALL	12
#define CF_FONT_LARGE	16

/* font sizes for small resolutions */
#define CF_FONT_LOWRES_SMALL	9
#define CF_FONT_LOWRES_LARGE	11

#define CF_MUSIC_THEME		"theme"
#define CF_MUSIC_DEFAULT	"default"
#define CF_MUSIC_FADE_TIME	2000

#define FILE_VERSION  13
#define FID_MISSION   MakeID('M','S','S','N')  /* mission file identifier */

#define DISPLAY_BPP	16	/* display depth */

#define MIN_XRES	240
#define MIN_YRES	240

#if defined PSP || defined V43
# define DEFAULT_RESOLUTION	480,272
#elif defined _WIN32_WCE
# define DEFAULT_RESOLUTION	240,320
#else
# define DEFAULT_RESOLUTION	800,600
#endif

#if defined _WIN32_WCE || defined PSP
# define strcasecmp _stricmp
# define strncasecmp _strnicmp
#else
# ifndef HAVE_STRCASECMP
#  define strcasecmp stricmp
# endif
# ifndef HAVE_STRNCASECMP
#  define strncasecmp strnicmp
# endif
#endif

#endif	/* _INCLUDE_GLOBALS_H */

