// CoMET - The Crimson Fields Map Editing Tool
// Copyright (C) 2002-2007 Jens Granseuer
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
// mapgen.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_MAPGEN_H
#define _INCLUDE_MAPGEN_H

#include "map.h"

class MapGenerator {
public:
  MapGenerator( void ) {}
  void Generate( Map &map, unsigned char water,
                 unsigned char roughness ) const;
};

class HeightMap {
public:
  HeightMap( unsigned short width, unsigned short height, unsigned char scale );
  ~HeightMap( void );

  unsigned char Height( unsigned short x, unsigned short y ) const;

private:
  void Pass( int xstep, int ystep, int step );

  unsigned short size;
  unsigned short scale;
  unsigned char *data;
};

#endif	/* _INCLUDE_MAPGEN_H */

