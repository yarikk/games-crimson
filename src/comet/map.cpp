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
// map.cpp
////////////////////////////////////////////////////////////////////////

#include "map.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Map::Map
// DESCRIPTION: Create a new map instance.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Map::Map( void ) : m_data(0), m_objects(0) {}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::~Map
// DESCRIPTION: Free memory allocated by the map structure so we can.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Map::~Map( void ) {
  delete [] m_data;
  delete [] m_objects;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::SetSize
// DESCRIPTION: Set a new map size. Terrain and unit sets must be
//              non-NULL when calling this method.
// PARAMETERS : size - new map size
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Map::SetSize( const Point &size ) {
  delete [] m_data;
  delete [] m_objects;

  m_w = size.x;
  m_h = size.y;

  unsigned short imax = m_w * m_h;

  m_data = new short [imax];
  m_objects = new MapObject * [imax];

  for ( int i = 0; i < imax; ++i ) {
    m_data[i] = m_tset->GetTerrainClassID(0);
    m_objects[i] = NULL;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::Load
// DESCRIPTION: Initialize the map structure.
// PARAMETERS : file - descriptor of an opened data file from which to
//                     read the map data
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int Map::Load( MemBuffer &file ) {
  m_w = file.Read16();
  m_h = file.Read16();

  unsigned short size = m_w * m_h;
  m_data = new short [size];
  m_objects = new MapObject * [size];

  // terrain types are loaded from a level set 

  for ( int i = 0; i < size; ++i ) {
    m_data[i] = file.Read16();
    m_objects[i] = NULL;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::Save
// DESCRIPTION: Save the current map status to a file. Only the terrain
//              type information is saved for each hex. Other data
//              (units, buildings...) must be stored separately.
// PARAMETERS : file - descriptor of the save file
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int Map::Save( MemBuffer &file ) const {
  int rc;
  file.Write16( m_w );
  rc = file.Write16( m_h );

  unsigned short size = m_w * m_h;
  for ( int i = 0; i < size && !rc; ++i )
    rc = file.Write16( m_data[i] );
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::Export
// DESCRIPTION: Save the map to a plain text file.
// PARAMETERS : file - descriptor of the save file
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int Map::Export( ofstream &file ) const {
  file << "[map-raw]\n";

  Point p;
  for ( p.y = 0; p.y < m_h; ++p.y ) {
    for ( p.x = 0; p.x < m_w; ++p.x ) {
      file << HexTypeID( p );
      if ( p.x != m_w - 1 ) file << ',';
    }
    file << '\n';
  }
  file << '\n';
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::Dir2Hex
// DESCRIPTION: Get the coordinates of the hex we'll be on when we move
//              in a given direction from another hex.
// PARAMETERS : hex  - source hex
//              dir  - direction to move in
//              dest - pointer to a Point to hold the coordinates of the
//                     destination hex
// RETURNS    : 0 on success, -1 on error (the contents of the Point can
//              not be relied on in this case)
////////////////////////////////////////////////////////////////////////

int Map::Dir2Hex( const Point &hex, Direction dir, Point &dest ) const {
  short x = hex.x, y = hex.y;

  switch ( dir ) {
    case NORTH:
      --y;
      break;
    case SOUTH:
      ++y;
      break;
    case NORTHEAST:
      if ( !(x & 1) ) --y;    // fall through...
    case EAST:
      ++x;
      break;
    case SOUTHEAST:
      if ( x & 1 ) ++y;
      ++x;
      break;
    case SOUTHWEST:
      if ( x & 1 ) ++y; // fall through...
    case WEST:
      --x;
      break;
    case NORTHWEST:
      if ( !(x & 1) ) --y;
      --x;
  }

  dest = Point( x, y );
  if ( !Contains( dest ) ) return -1;
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::Contains
// DESCRIPTION: Check whether a hex with the given coordinates exists
//              on the map.
// PARAMETERS : hex - hex position
// RETURNS    : true if the coordinates denote a valid hex, false
//              otherwise
////////////////////////////////////////////////////////////////////////

bool Map::Contains( const Point &hex ) const {
  return( (hex.x >= 0) && (hex.x < Width()) &&
          (hex.y >= 0) && (hex.y < Height()) );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::GetUnit
// DESCRIPTION: Get a unit from the map.
// PARAMETERS : hex - hex position
// RETURNS    : the unit at the given coordinates, or NULL if no unit
//              was found there
////////////////////////////////////////////////////////////////////////

Unit *Map::GetUnit( const Point &hex ) const {
  MapObject *o = GetMapObject( hex );
  if ( o && o->IsUnit() ) return static_cast<Unit *>(o);
  return NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::SetUnit
// DESCRIPTION: Put a unit on the map. Also sets the unit parameters
//              according to the new position.
// PARAMETERS : u   - unit (may be NULL)
//              pos - hex position
// RETURNS    : 0
////////////////////////////////////////////////////////////////////////

short Map::SetUnit( Unit *u, const Point &pos ) {

  if ( u ) {
    u->SetPosition( pos );

    if ( u->IsGround() ) {
      if ( IsWater( pos ) ) u->SetFlags( U_FLOATING );
      else u->UnsetFlags( U_FLOATING );
    }
  }
  m_objects[Hex2Index(pos)] = u;

  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::GetBuilding
// DESCRIPTION: Get a building from the map.
// PARAMETERS : pos - hex position
// RETURNS    : the building at the given coordinates, or NULL if no
//              building was found there
////////////////////////////////////////////////////////////////////////

Building *Map::GetBuilding( const Point &pos ) const {
  MapObject *o = GetMapObject( pos );
  if ( o && o->IsBuilding() ) return static_cast<Building *>(o);
  return NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::HexColor
// DESCRIPTION: Get the color of a hex for the overview map.
// PARAMETERS : xy - hex index (y * map_height + x)
// RETURNS    : hex color 
////////////////////////////////////////////////////////////////////////

unsigned long Map::HexColor( unsigned short xy ) const {
  return( m_tset->GetTerrainInfo(m_data[xy])->tt_color );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::HexType
// DESCRIPTION: Get some info about a hex.
// PARAMETERS : hex - hex position
// RETURNS    : terrain type information
////////////////////////////////////////////////////////////////////////

const TerrainType *Map::HexType( const Point &hex ) const {
  return m_tset->GetTerrainInfo(HexTypeID(hex));
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::Index2Hex
// DESCRIPTION: Turn a hex index back into a Point.
// PARAMETERS : index - hex index
// RETURNS    : hex coordinates (-1/-1 on invalid index)
////////////////////////////////////////////////////////////////////////

Point Map::Index2Hex( int index ) const {
  Point p(-1, -1);

  if ( index >= 0 ) {
    p.x = index % Width();
    p.y = index / Width();

    if ( (p.x >= Width()) || (p.y >= Height()) ) return Point(-1, -1);
  }
  return p;
}

