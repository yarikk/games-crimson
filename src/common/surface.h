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

//////////////////////////////////////////////////////////////
// surface.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_SURFACE_H
#define _INCLUDE_SURFACE_H

#include "SDL.h"

#include "rect.h"
#include "color.h"
#include "fileio.h"

// color definitions
#define CF_COLOR_BACKGROUND	CF_COLOR_DARKGRAY
#define CF_COLOR_HIGHLIGHT	CF_COLOR_WHITE
#define CF_COLOR_SHADOW		CF_COLOR_BLACK
#define CF_COLOR_GHOSTED	CF_COLOR_LIGHTGRAY

// flags for DrawBox()
#define BOX_RAISED    0x0001  // |
#define BOX_RECESSED  0x0002  // |- only one of these at a time
#define BOX_CARVED    0x0004  // |
#define BOX_SOLID     0x0010  // fill box with background color

class Surface : public Rect {
public:
  Surface( void ) : Rect( 0, 0, 0, 0 ) { s_surface = 0; }
  ~Surface( void );

  int Create( short w, short h, int bpp, unsigned long flags );
  int DisplayFormat( void );

  int SetAlpha( unsigned char alpha, unsigned long flags )
                { return SDL_SetAlpha( s_surface, flags, alpha ); }
  int SetColorKey( const Color &col );
  Color GetColorKey( void ) const;
  Color GetPixel( unsigned short x, unsigned short y ) const;
  unsigned long MapRGB( const Color &col ) const
                { return SDL_MapRGB( s_surface->format, col.r, col.g, col.b ); }

  int Blit( Surface *dest, const Rect &from, short dx, short dy ) const;
  int LowerBlit( Surface *dest, const Rect &from, short dx, short dy ) const;
  void DrawBox( const Rect &box, unsigned short type );

  void Flood( const Color &col ) const { FillRect( *this, col ); }
  int FillRect( short x, short y, unsigned short w, unsigned short h,
                unsigned long col ) const;
  int FillRect( const Rect &rect, unsigned long col ) const
              { return FillRect( rect.x, rect.y, rect.w, rect.h, col ); }
  int FillRect( short x, short y, unsigned short w, unsigned short h,
                const Color &col ) const
              { return FillRect( x, y, w, h, MapRGB( col ) ); }
  int FillRect( const Rect &rect, const Color &col ) const
              { return FillRect( rect.x, rect.y, rect.w, rect.h, col ); }
  int FillRectAlpha( short x, short y, unsigned short w, unsigned short h,
                const Color &col, unsigned char alpha = 128 ) const;
  int FillRectAlpha( const Rect &rect, const Color &col, unsigned char alpha = 128 ) const
              { return FillRectAlpha( rect.x, rect.y, rect.w, rect.h, col, alpha ); }
  void FillPattern( short x, short y, unsigned short w, unsigned short h,
                    const class Image &pattern, short dx, short dy );
  void FillPattern( const Rect &rect, const class Image &pattern, short dx, short dy )
                  { FillPattern( rect.x, rect.y, rect.w, rect.h, pattern, dx, dy ); }

  void SetClipRect( const Rect &clip ) const;
  void GetClipRect( Rect &clip ) const;

  int LoadImageData( MemBuffer &file, bool hwsurface = false );
  int LoadPalette( MemBuffer &file, unsigned short cols );
  int LoadBMP( const char *file );

  SDL_Surface *s_surface;

protected:
  enum {
    RAW_DATA_TRANSPARENT = 0x01
  };

  void DrawPixel( short const x, short const y, const Color &col ) const;
};

class SurfaceLock {
public:
  SurfaceLock( const Surface *surface ) : s(surface->s_surface) {
    locked = ( SDL_MUSTLOCK( s ) && (SDL_LockSurface( s ) == 0) );
  }

  ~SurfaceLock( void ) { if ( locked ) SDL_UnlockSurface( s ); }

private:
  SDL_Surface *s;
  bool locked;
};


class Image : public Rect {
public:
  Image( void ) {}
  Image( Surface *surface, short x, short y, unsigned short w, unsigned short h );

  void Draw( Surface *dest, short x, short y ) const;

private:
  Surface *surface;
};

#endif	/* _INCLUDE_SURFACE_H */

