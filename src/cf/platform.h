// Crimson Fields -- a game of tactical warfare
// Copyright (C) 2000-2007 Jens Granseuer
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
// platform.h -- platform-specific functions
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_PLATFORM_H
#define _INCLUDE_PLATFORM_H

#include "view.h"

struct GUIOptions {
  short px_width;
  short px_height;
  short bpp;
  bool sfx;
  bool music;
  unsigned char sfx_vol;
  unsigned char music_vol;
  unsigned long sdl_flags;
  const char *level;
};

// init environment
bool platform_init( GUIOptions &opts );
// setup display
bool platform_setup( View *view );
// prepare for closing the display
void platform_dispose( void );
// clean up after resources have been released
void platform_shutdown( void );

#endif	/* _INCLUDE_PLATFORM_H */

