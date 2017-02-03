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
// map.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_MAP_H
#define _INCLUDE_MAP_H

#include "hexsup.h"
#include "rect.h"
#include "unit.h"
#include "building.h"
#include "list.h"
#include "misc.h"
#include "lset.h"

#define MCOST_UNIT	20	// theoretical cost to cross a hex occupied by another unit
				// must be higher than the maximum unit speed

class Map {
public:
  Map( void );
  ~Map( void );

  void SetUnitSet( UnitSet *set ) { uset = set; }
  UnitSet *GetUnitSet( void ) const { return uset; }
  void SetTerrainSet( TerrainSet *set ) { tset = set; }
  TerrainSet *GetTerrainSet( void ) const { return tset; }

  int Load( MemBuffer &file );
  int Save( MemBuffer &file ) const;

  unsigned short Width( void ) const { return m_w; }
  unsigned short Height( void ) const { return m_h; }

  Unit *GetUnit( const Point &pos ) const;
  short SetUnit( Unit *u, const Point &pos );
  Building *GetBuilding( const Point &pos ) const;
  void SetBuilding( Building *b, const Point &pos ) { m_objects[Hex2Index(pos)] = b; }
  MapObject *GetMapObject( const Point &hex ) const { return m_objects[Hex2Index(hex)]; }

  short AttackMod( const Point &hex ) const { return( HexType(hex)->tt_att_mod ); }
  short DefenceMod( const Point &hex ) const { return( HexType(hex)->tt_def_mod ); }
  const TerrainType *HexType( const Point &hex ) const;
  short HexTypeID( const Point &hex ) const { return m_data[Hex2Index(hex)]; }
  short HexImage( const Point &hex ) const { return( HexType(hex)->tt_image ); }
  signed char MoveCost( const Point &hex ) const { return( HexType(hex)->tt_move ); }
  signed char MoveCost( const Unit *u, const Point &src, const Point &dst, unsigned short &state ) const;
  unsigned short TerrainTypes( const Point &hex ) const { return HexType(hex)->tt_type; }
  bool IsWater( const Point &hex ) const
        { return (TerrainTypes(hex) & (TT_WATER|TT_WATER_SHALLOW|TT_WATER_DEEP)) != 0; }
  bool IsShop( const Point &hex ) const { return (TerrainTypes(hex) & TT_ENTRANCE) != 0; }

  unsigned long HexColor( unsigned short xy ) const;
  void SetHexType( short x, short y, short type )
               { m_data[y * m_w + x] = type; }

  short GetNeighbors( const Point &hex, Point *parray ) const;
  int Hex2Index( const Point &hex ) const { return hex.y * m_w + hex.x; }
  Point Index2Hex( int index ) const;
  int Dir2Hex( const Point &hex, Direction dir, Point &dest ) const;
  bool Contains( const Point &hex ) const;

private:
  unsigned short m_w;
  unsigned short m_h;

  short *m_data;
  MapObject **m_objects;
  UnitSet *uset;
  TerrainSet *tset;
};

#endif	/* _INCLUDE_MAP_H */

