/* Crimson Fields - a game of tactical warfare
   Copyright (C) 2000-2007 Jens Granseuer

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* mksurface.cpp */

#include <iostream>
using namespace std;
#include "mksurface.h"

////////////////////////////////////////////////////////////////////////
// NAME       : MkSurface::SaveImageData
// DESCRIPTION: Load a bitmap file and save the data into another file.
// PARAMETERS : from - name of the bitmap to load
//              out  - file to save to
//              tp   - whether to save color key information
//                     (transparent color). If true we look at the upper
//                     left pixel and assume that should be the color
//                     key (should work for hex images at least).
// RETURNS    : 0 on success, non-0 on error
////////////////////////////////////////////////////////////////////////

int MkSurface::SaveImageData( const string &from, MemBuffer &out, bool tp ) {
  int rc = LoadBMP( from.c_str() );

  if ( rc == 0 ) {
    if ( s_surface->format->BitsPerPixel == 8 ) {
      SDL_Palette *palette = s_surface->format->palette;
      int i;

      out.Write16( Width() );
      out.Write16( Height() );
      out.Write8( s_surface->format->BitsPerPixel );
      out.Write8( tp ? RAW_DATA_TRANSPARENT : 0 );
      out.Write16( palette->ncolors ); // only for <= 256 colors (bpp = 8)

      if ( tp ) {
        int ck = GuessColorKey();
        // transparent color first
        out.Write8( palette->colors[ck].r );
        out.Write8( palette->colors[ck].g );
        out.Write8( palette->colors[ck].b );
      }

      for ( i = 0; i < palette->ncolors; ++i ) {
        out.Write8( palette->colors[i].r );
        out.Write8( palette->colors[i].g );
        out.Write8( palette->colors[i].b );
      }

      // save surface lines to file
      for ( i = 0; i < Height(); ++i ) {
        Uint8 *pix = (Uint8 *)s_surface->pixels + i * s_surface->pitch;
        out.Write( pix, Width() );
      }
    } else {
      cerr << "Error: Couldn't find palette in image " << from << endl
           << "Make sure it is 8 bit (<= 256 colors) only" << endl;
      rc = -1;
    }
  } else
    cerr << "Error: Couldn't load image " << from << endl;

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : MkSurface::GuessColorKey
// DESCRIPTION: Look at the pixel in the upper left corner and assume it
//              should be transparent (should be true for hex images).
// PARAMETERS : -
// RETURNS    : palette index of the transparent color
////////////////////////////////////////////////////////////////////////

int MkSurface::GuessColorKey( void ) const {
  // only implemented for 8 bit surfaces
  return *((Uint8 *)s_surface->pixels);
}

