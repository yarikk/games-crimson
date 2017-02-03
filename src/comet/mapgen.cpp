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

////////////////////////////////////////////////////////////////////////
// mapgen.cpp
////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "mapgen.h"
#include "misc.h"

#define TILE_PLAINS 30
#define TILE_PLAINS_PEBBLES_1 34
#define TILE_PLAINS_PEBBLES_2 35
#define TILE_PLAINS_RUGGED_1 31
#define TILE_PLAINS_HOUSE 53
#define TILE_PLAINS_COTTAGE 54
#define TILE_PLAINS_SHED 55
#define TILE_PLAINS_HILLS 38
#define TILE_HILLS_1 65
#define TILE_HILLS_2 66
#define TILE_MOUNTAINS 117
#define TILE_WATER_SHALLOW 360
#define TILE_WATER_CLIFFS_1 363
#define TILE_WATER_CLIFFS_2 364
#define TILE_WATER_CLIFFS_3 365
#define TILE_WATER 361
#define TILE_WATER_DEEP 362

#define TILE_FOREST_1 77
#define TILE_FOREST_2 78
#define TILE_FOREST_3 79
#define TILE_FOREST_4 80
#define TILE_FOREST_5 81
#define TILE_FOREST_6 82

////////////////////////////////////////////////////////////////////////
// NAME       : MapGenerator::Generate
// DESCRIPTION: Generate a map with the given parameters. The generator
//              contains a lot of hard-coded knowledge about the tile
//              set used so it only works with the default set.
// PARAMETERS : map       - map structure that will be populated
//              water     - water level (0-10)
//              roughness - map roughness (1-20)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapGenerator::Generate( Map &map, unsigned char water,
                                       unsigned char roughness ) const {
  water *= 20;

  HeightMap terrain( map.Width(), map.Height(), roughness );
  HeightMap forest( map.Width(), map.Height(), 15 );

  // interval for a single terrain type
  unsigned short interval_land = (255 - water) / 5; // create 5 zones
  unsigned short interval_sea = water / 3;

  for ( int y = 0; y < map.Height(); ++y ) {
    for ( int x = 0; x < map.Width(); ++x ) {

      unsigned char level = terrain.Height( x, y );
      unsigned short tile;

      if ( level < water ) { // water
        if ( level < interval_sea ) tile = TILE_WATER_DEEP;
        else if ( level < 2 * interval_sea ) tile = TILE_WATER;
        else {
          switch ( rand()%300 ) {
          case 0: tile = TILE_WATER_CLIFFS_1; break;
          case 1: tile = TILE_WATER_CLIFFS_2; break;
          case 2: tile = TILE_WATER_CLIFFS_3; break;
          default: tile = TILE_WATER_SHALLOW;
          }
        }
      } else { // land
        level -= water;

        if ( level < interval_land * 3 ) { // plains
          if ( level < interval_land * 2 ) {
            unsigned char flevel = forest.Height( x, y );
            if ( flevel > 165 ) {
              switch ( (flevel - 165) / 15 ) {
              case 0: tile = TILE_FOREST_1; break;
              case 1: tile = TILE_FOREST_2; break;
              case 2: tile = TILE_FOREST_3; break;
              case 3: tile = TILE_FOREST_4; break;
              case 4: tile = TILE_FOREST_5; break;
              default: tile = TILE_FOREST_6;
              }
            } else {
              switch ( rand()%300 ) {
              case 0:
              case 1:
              case 2: tile = TILE_PLAINS_RUGGED_1; break;
              case 3:
              case 4: tile = TILE_PLAINS_PEBBLES_1; break;
              case 5:
              case 6: tile = TILE_PLAINS_PEBBLES_2; break;
              case 7: tile = TILE_PLAINS_HOUSE; break;
              case 8: tile = TILE_PLAINS_COTTAGE; break;
              case 9: tile = TILE_PLAINS_SHED; break;
              default: tile = TILE_PLAINS;
              }
            }
          } else tile = TILE_PLAINS_HILLS;
        }
        else if ( level < 4 * interval_land ) {
          if ( rand()%3 <= 1 ) tile = TILE_HILLS_1;
          else tile = TILE_HILLS_2;
        }
        else tile = TILE_MOUNTAINS;
      }

      map.SetHexType( x, y, tile );
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : HeightMap::HeightMap
// DESCRIPTION: Create a fractal height map. This code is based on a
//              terrain generator by Randy J. Relander
//              <rjrelander@users.sourceforge.net>.
// PARAMETERS : width  - map width
//              height - map height
//              scale  - map "roughness" (1-20)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

HeightMap::HeightMap( unsigned short width, unsigned short height, unsigned char scale ) :
           size(1), scale(scale) {

  // we always create a square height map with the size being a power of 2
  // because that's way easier
  unsigned short base = MAX( width, height ) - 1;

  while ( base ) {
    base /= 2;
    size *= 2;
  }

  data = new unsigned char[(size+1) * (size+1)];

  // seed the corners
  data[ 0 ] = rand()%256;
  data[ size ] = rand()%256;
  data[ (size+1) * size ] = rand()%256;
  data[ size + (size+1) * size ] = rand()%256;

  // generate fractal
  for ( int step = size; step > 1; step /= 2 ) {
    Pass( 1, 0, step );
    Pass( 0, 1, step );
    Pass( 1, 1, step );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : HeightMap::~HeightMap
// DESCRIPTION: Destroy the height map.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

HeightMap::~HeightMap( void ) {
  delete [] data;
}

////////////////////////////////////////////////////////////////////////
// NAME       : HeightMap::Pass
// DESCRIPTION: Make one pass across the array.
// PARAMETERS : xstep -
//              ystep -
//              step  -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void HeightMap::Pass( int xstep, int ystep, int step ) {
  int x, y, z;
  int z1, z2;

  int range = step * scale;
  int shift = (range/2);

  int dx = xstep * (step/2);
  int dy = ystep * (step/2);

  int delta1 = (size+1) * dy;
  int delta2 = (size+1) * step;

  unsigned char *p = data + delta1;

  for ( y = dy; y <= size; y += step ) {
    for ( x = dx; x <= size; x += step ) {
      // select two points
      if ( rand()%2 ) {
        z1 = p[x - dx - delta1];
        z2 = p[x + dx + delta1];
      } else {
        z1 = p[x + dx - delta1];
        z2 = p[x - dx + delta1];
      }

      // average and randomize
      z = (z1+z2)/2 + (rand()%range) - shift;

      if (z < 0) z = 0;
      if (z > 255) z = 255;

      p[x] = z;
    }

    p += delta2;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : HeightMap::Height
// DESCRIPTION: Get the height of a specified point.
// PARAMETERS : x - horizontal coordinate
//              y - vertical coordinate
// RETURNS    : height at the requested point
////////////////////////////////////////////////////////////////////////

inline unsigned char HeightMap::Height( unsigned short x, unsigned short y ) const {
  return data[ x + (size+1) * y ];
}

