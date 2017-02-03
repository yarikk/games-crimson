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
// map.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_ED_MAP_H
#define _INCLUDE_ED_MAP_H

#include "hexsup.h"
#include "rect.h"
#include "unit.h"
#include "building.h"
#include "list.h"
#include "misc.h"
#include "lset.h"

class Map {
public:
  Map( void );
  ~Map( void );

  void SetUnitSet( UnitSet *set ) { m_uset = set; }
  UnitSet *GetUnitSet( void ) const { return m_uset; }
  void SetTerrainSet( TerrainSet *set ) { m_tset = set; }
  TerrainSet *GetTerrainSet( void ) const { return m_tset; }

  int SetSize( const Point &size );
  int Load( MemBuffer &file );
  int Save( MemBuffer &file ) const;
  int Export( ofstream &file ) const;

  unsigned short Width( void ) const { return m_w; }
  unsigned short Height( void ) const { return m_h; }

  Unit *GetUnit( const Point &pos ) const;
  short SetUnit( Unit *u, const Point &pos );
  Building *GetBuilding( const Point &pos ) const;
  void SetBuilding( Building *b, const Point &pos ) { m_objects[Hex2Index(pos)] = b; }
  MapObject *GetMapObject( const Point &hex ) const { return m_objects[Hex2Index(hex)]; }
  const TerrainType *HexType( const Point &hex ) const;
  short HexTypeID( const Point &hex ) const { return m_data[Hex2Index(hex)]; }
  short HexImage( const Point &hex ) const { return( HexType(hex)->tt_image ); }
  unsigned short TerrainTypes( const Point &hex ) const { return HexType(hex)->tt_type; }
  bool IsWater( const Point &hex ) const
        { return (TerrainTypes(hex) & (TT_WATER|TT_WATER_SHALLOW|TT_WATER_DEEP)) != 0; }
  bool IsBuilding( const Point &hex ) const
        { return (TerrainTypes(hex) & TT_ENTRANCE) != 0; }

  unsigned long HexColor( unsigned short xy ) const;
  void SetHexType( const Point &pos, unsigned short type ) { SetHexType( pos.x,pos.y,type); }
  void SetHexType( short x, short y, unsigned short type ) { m_data[y * m_w + x] = type; }

  int Hex2Index( const Point &hex ) const { return hex.y * m_w + hex.x; }
  Point Index2Hex( int index ) const;
  int Dir2Hex( const Point &hex, Direction dir, Point &dest ) const;
  bool Contains( const Point &hex ) const;

private:
  unsigned short m_w;
  unsigned short m_h;

  short *m_data;
  MapObject **m_objects;
  UnitSet *m_uset;
  TerrainSet *m_tset;
};

#endif  // _INCLUDE_ED_MAP_H

