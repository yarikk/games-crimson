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
// hexsup.h - miscellaneous classless hex support functions
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_HEXSUP_H
#define _INCLUDE_HEXSUP_H

#include "misc.h"

enum Direction { NORTH, NORTHEAST, SOUTHEAST, SOUTH, SOUTHWEST, NORTHWEST, WEST, EAST };

#define ReverseDir(dir)  ((Direction)((dir + 3) % 6))
#define TurnLeft(dir)    ((Direction)((dir + 5) % 6))
#define TurnRight(dir)   ((Direction)((dir + 1) % 6))

int Distance( short sx, short sy, short tx, short ty );
int Distance( const Point &s, const Point &t );
Direction Hex2Dir( const Point &src, const Point &dest );
bool NextTo( const Point &p1, const Point &p2 );

#endif	/* _INCLUDE_HEXSUP_H */

