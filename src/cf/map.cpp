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
// map.cpp
////////////////////////////////////////////////////////////////////////

#include "map.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Map::Map
// DESCRIPTION: Create a new map instance and initialize data.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Map::Map( void ) {
  m_data = NULL;
  m_objects = NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::~Map
// DESCRIPTION: Free memory allocated by the map structure.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Map::~Map( void ) {
  delete [] m_data;
  delete [] m_objects;
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
// NAME       : Map::GetNeighbors
// DESCRIPTION: Get the hex coordinates of all hexes surrounding a hex.
// PARAMETERS : hex    - hex to get neighbors for
//              parray - pointer to an array of at least six Points to
//                       hold the neighbors' coordinates. Each neighbor
//                       will be put in the slot corresponding to the
//                       direction the hex is in (e.g. the hex north of
//                       the source hex will reside in parray[NORTH]).
//                       If there is no valid neighbor (because the map
//                       ends here), the respective Points will contain
//                       {-1, -1}
// RETURNS    : number of adjacent hexes found
////////////////////////////////////////////////////////////////////////

short Map::GetNeighbors( const Point &hex, Point *parray ) const {
  short num = 0;
  for ( int i = NORTH; i <= NORTHWEST; ++i ) {
    if ( !Dir2Hex( hex, (Direction)i, parray[i] ) ) ++num;
    else parray[i].x = parray[i].y = -1;
  }
  return num;
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
// RETURNS    : 1 if a building was conquered, 0 otherwise
////////////////////////////////////////////////////////////////////////

short Map::SetUnit( Unit *u, const Point &pos ) {
  short conquer = 0;
  short x = pos.x, y = pos.y;

  if ( (x >= 0) && (y >= 0) ) {
    if ( u ) {
      u->SetPosition( x, y );

      Transport *local = static_cast<Transport *>(GetUnit( pos ));
      if ( local ) {					// move into transport
        if ( u->IsDummy() ) u->SetFlags( U_SHELTERED );
        else local->InsertUnit( u );
      } else {
        Building *b = GetBuilding( pos );
        if ( b ) {					// move unit into building
          if ( u->IsDummy() ) u->SetFlags( U_SHELTERED );
          else {
            conquer = b->InsertUnit( u );
            if ( conquer == 1 ) {				// building conquered
              // exchange the building entrance
              short hextype = HexTypeID(pos), newhex;
              if ( b->Owner() ) newhex = hextype + u->Owner()->ID() - b->Owner()->ID();
              else newhex = hextype + u->Owner()->ID() - PLAYER_NONE;
              SetHexType( x, y, newhex );
              b->SetOwner( u->Owner() );
            }
          }
        } else {
          if ( u->IsGround() ) {
            if ( IsWater( pos ) ) u->SetFlags( U_FLOATING );
            else u->UnsetFlags( U_FLOATING );
          }
          m_objects[Hex2Index(pos)] = u;
        }
      }
    } else m_objects[Hex2Index(pos)] = u;
  }
  return conquer;
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
  if ( o && o->IsShop() ) return static_cast<Building *>(o);
  return NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::HexColor
// DESCRIPTION: Get the color of a hex for the overview map.
// PARAMETERS : xy - hex index (y * map_height + x)
// RETURNS    : hex color 
////////////////////////////////////////////////////////////////////////

unsigned long Map::HexColor( unsigned short xy ) const {
  return( tset->GetTerrainInfo(m_data[xy])->tt_color );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::HexType
// DESCRIPTION: Get some info about a hex.
// PARAMETERS : hex - hex position
// RETURNS    : terrain type information
////////////////////////////////////////////////////////////////////////

const TerrainType *Map::HexType( const Point &hex ) const {
  return tset->GetTerrainInfo(HexTypeID(hex));
}

////////////////////////////////////////////////////////////////////////
// NAME       : Map::MoveCost
// DESCRIPTION: Calculate the number of movement points a certain unit
//              requires to go to a given adjacent hex.
// PARAMETERS : u     - unit to calculate cost for
//              src   - from position; this field must be a neighbour
//                      of the destination hex, otherwise the return
//                      value is undefined.
//              dst   - to position
//              state - indicator for special move attributes;
//                      normally 0; 1 if move ends in building or
//                      transport or on hex blocked by enemy unit(s)
// RETURNS    : movement cost in points; -1 means unit cannot enter hex
//              at all
////////////////////////////////////////////////////////////////////////

signed char Map::MoveCost( const Unit *u, const Point &src,
                           const Point &dst, unsigned short &state ) const {
  signed char cost;
  const TerrainType *type = HexType( dst );
  bool hexok = (type->tt_type & u->Terrain()) != 0;

  state = 0;

  Unit *block = GetUnit( dst );
  if ( block ) {
    if ( block->IsTransport() && static_cast<Transport *>(block)->Allow(u) ) {
      if ( !u->IsSheltered() ) {
        cost = MCOST_MIN;
        state = 1;
      } else cost = MCOST_UNIT;
    } else cost = (hexok ? MCOST_UNIT : -1);

  } else {
    Building *bld;

    if ( u->IsAircraft() || u->IsMine() ) cost = MCOST_MIN;
    else cost = type->tt_move;

    if ( !hexok ) cost = -1;
    else if ( (bld = GetBuilding( dst )) ) {
      if ( !bld->Allow(u) ) cost = -1;
      else if ( u->IsSheltered() ) cost = MCOST_UNIT;
      else state = 1;
    } else {
      // Is the path blocked? This is the case if the unit slips through
      // between two enemy units, or one enemy unit and impassable terrain.
      // This results in movement cost equal to the unit's remaining points.
      Direction dir = Hex2Dir( src, dst );
      short narrow = 0, foenear = 0;
      Point p;

      if ( !Dir2Hex( src, TurnLeft(dir), p ) ) {
        block = GetUnit( p );

        if ( block && (block->Owner() != u->Owner()) ) ++foenear;
        else if ( !(TerrainTypes(p) & u->Terrain()) ) ++narrow;
      } else ++narrow;

      if ( narrow || foenear ) {
        if ( !Dir2Hex( src, TurnRight(dir), p ) ) {
          block = GetUnit( p );

          if ( block && (block->Owner() != u->Owner()) ) ++foenear;
          else if ( !(TerrainTypes(p) & u->Terrain()) ) ++narrow;
        } else ++narrow;

        if ( (foenear > 1) || ((foenear == 1) && (narrow > 0)) ) state = 1;  // blocked
      }
    }
  }

  return cost;
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

