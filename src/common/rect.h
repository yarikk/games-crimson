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
// rect.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_RECT_H
#define _INCLUDE_RECT_H

class Rect {
public:
  Rect( void ) {};
  Rect( short x, short y, unsigned short w, unsigned short h )
      { this->x = x; this->y = y; this->w = w; this->h = h; }

  short LeftEdge( void ) const { return x; }
  short TopEdge( void ) const { return y; }
  unsigned short Width( void ) const { return w; }
  unsigned short Height( void ) const { return h; }

  void Align( const Rect &parent );
  void Center( const Rect &anchor );
  void Clip( const Rect &clip );
  void ClipBlit( Rect &src, const Rect &clip );
  bool Contains( short x, short y ) const;

  bool operator>=( const Rect &r ) const
                 { return (w >= r.w) && (h >= r.h); }

  short x;
  short y;
  unsigned short w;
  unsigned short h;
};

#endif	/* _INCLUDE_RECT_H */

