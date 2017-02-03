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

////////////////////////////////////////////////////////////////////////
// hexsup.cpp - classless hex functions
////////////////////////////////////////////////////////////////////////

#include "hexsup.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Distance
// DESCRIPTION: Calculate the distance between to hexes in hexes.
// PARAMETERS : sx - first hex horizontal position
//              sy - first hex vertical position
//              tx - second hex horizontal position
//              ty - second hex vertical position
// RETURNS    : number of hexes between s and t
////////////////////////////////////////////////////////////////////////

int Distance( short sx, short sy, short tx, short ty ) {
  int x1 = sy - (sx / 2);
  int y1 = sy + ((sx + 1) / 2);
  int x2 = ty - (tx / 2);
  int y2 = ty + ((tx + 1) / 2);
  int dx = x2 - x1;
  int dy = y2 - y1;
  int dist;
  if ( SIGN(dx) == SIGN(dy) ) dist = MAX( ABS(dx), ABS(dy) );
  else dist = ABS(dx) + ABS(dy);

  return dist;
}

int Distance( const Point &s, const Point &t ) {
  return Distance( s.x, s.y, t.x, t.y );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Hex2Dir
// DESCRIPTION: Get the direction to move in to get to the destination
//              hex. The destination hex must be adjacent to the source
//              hex for this function to work properly.
// PARAMETERS : src  - source hex
//              dest - destination hex
// RETURNS    : direction to go
////////////////////////////////////////////////////////////////////////

Direction Hex2Dir( const Point &src, const Point &dest ) {
  if ( dest.x < src.x ) {
    if ( (dest.y < src.y) || ((dest.y == src.y) && !(dest.x & 1)) ) return NORTHWEST;
    else return SOUTHWEST;
  } else if ( dest.x > src.x ) {
    if ( (dest.y < src.y) || ((dest.y == src.y) && !(dest.x & 1)) ) return NORTHEAST;
    else return SOUTHEAST;
  } else {
    if ( dest.y < src.y ) return NORTH;
    else return SOUTH;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : NextTo
// DESCRIPTION: Find out whether two hexes are adjacent.
// PARAMETERS : p1 - first hex
//              p2 - second hex
// RETURNS    : TRUE if hexes are adjacent, FALSE if not
////////////////////////////////////////////////////////////////////////

bool NextTo( const Point &p1, const Point &p2 ) {
 return (Distance(p1,p2) == 1);
}

