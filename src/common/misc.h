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
// misc.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_MISC_H
#define _INCLUDE_MISC_H

#define MIN(a,b)	((a)<=(b)?(a):(b))
#define MAX(a,b)        ((a)>=(b)?(a):(b))
#define ABS(a)		((a)<0?-(a):(a))
#define SIGN(a)		((a)<0?(-1):(1))

#define MakeID(a,b,c,d)         ((a)|((b)<<8)|((c)<<16)|((d)<<24))

int random( int min, int max );
unsigned int rand_range( unsigned int range);
char *itoa( int n, char *buf );

struct Point {
  short x;
  short y;

  Point( void ) {}
  Point( short x, short y ) { this->x = x; this->y = y; }
  bool operator==( const Point &p ) const { return( (x == p.x) && (y == p.y) ); }
  bool operator!=( const Point &p ) const { return !(*this == p); }
};

#endif	/* _INCLUDE_MISC_H */

