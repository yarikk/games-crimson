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
// color.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_COLOR_H
#define _INCLUDE_COLOR_H

class Color {
public:
  Color() {}
  Color( unsigned char r, unsigned char g, unsigned char b )
       { this->r = r; this->g = g; this->b = b; }
  Color( unsigned long rgb )
       { r = (unsigned char)((rgb & 0x00FF0000) >> 16);
         g = (unsigned char)((rgb & 0x0000FF00) >> 8);
         b = (unsigned char)(rgb & 0x000000FF); }
  bool operator==( const Color &col ) const
       { return col.r == r && col.g == g && col.b == b; }
  bool operator!=( const Color &col ) const
       { return col.r != r || col.g != g || col.b != b; }

  unsigned char r, g, b;
};

#define CF_COLOR_BLACK		0x00,0x00,0x00
#define CF_COLOR_WHITE		0xFF,0xFF,0xFF
#define CF_COLOR_DARKGRAY	0x50,0x50,0x50
#define CF_COLOR_LIGHTGRAY	0x8B,0x8B,0x8B

#endif	/* _INCLUDE_COLOR_H */

