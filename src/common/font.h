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
// font.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_FONT_H
#define _INCLUDE_FONT_H

#include "SDL_ttf.h"
#include "surface.h"

class Font {
public:
  Font( void ) { f = 0; }
  ~Font( void );

  int Load( const char *file, int size );

  // returns max. character width for proportional fonts
  unsigned char Width( void ) const { return width; }
  unsigned char Height( void ) const { return height; }
  unsigned char CharWidth( char c ) const;
  unsigned short TextWidth( const char *str ) const;
  unsigned short TextHeight( const char *str, unsigned short linew, unsigned short spacing ) const;
  unsigned short FitText( const char *str, unsigned short width, bool word ) const;

  void SetColor( const Color &fcol );
  Color GetColor( void ) const { return col; }
  int Write( const char *str, Surface *dest, short x, short y ) const;
  int Write( char c, Surface *dest, short x, short y ) const;
  int Write( const char *str, Surface *dest, short x, short y, const Color &color );
  int Write( char c, Surface *dest, short x, short y, const Color &color );
  int Write( const char *str, Surface *dest, short x, short y, const Rect &clip ) const;
  int WriteEllipsis( const char *str, Surface *dest, short x, short y, const Rect &clip ) const;

private:
  TTF_Font *f;
  unsigned char width;
  unsigned char height;

  Color col;
};

#endif	/* _INCLUDE_FONT_H */

