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
// unit.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_ED_UNIT_H
#define _INCLUDE_ED_UNIT_H

#include <fstream>
using namespace std;

#include "list.h"
#include "misc.h"
#include "lset.h"

#define MO_UNIT      1
#define MO_BUILDING  2

class MapObject {
public:
  MapObject( unsigned short type ) : mo_type(type) {}
  virtual ~MapObject( void ) {}
  unsigned short Type( void ) const { return mo_type; }

  bool IsUnit( void ) const { return mo_type == MO_UNIT; }
  bool IsBuilding( void ) const { return mo_type == MO_BUILDING; }
  virtual const char *Name( void ) const = 0;
  virtual unsigned char Owner( void ) const = 0;
  virtual const Point &Position( void ) const = 0;

  virtual unsigned short MinWeight( void ) const = 0;
  virtual unsigned short MaxWeight( void ) const = 0;

private:
  unsigned short mo_type;
};


class Unit : public Node, public MapObject {
public:
  Unit( void ) : MapObject(MO_UNIT) {}
  Unit( const UnitType *type, unsigned char pid, unsigned short id, const Point &pos );
  Unit( MemBuffer &file, const UnitType *type );

  int Save( MemBuffer &file ) const;
  int Export( ofstream &file ) const;

  unsigned short BaseImage( void ) const { return u_type->Image() + u_pid * 6; }
  unsigned short BuildCost( void ) const { return u_type->Cost(); }
  unsigned short Crystals( void ) const { return u_crystals; }
  void SetCrystals( unsigned short crystals ) { u_crystals = crystals; }
  unsigned char GroupSize( void ) const { return u_group; }
  void SetGroupSize( unsigned char size ) { u_group = size; }
  unsigned short ID( void ) const { return u_id; }
  void SetID( unsigned short id ) { u_id = id; }
  unsigned short Image( void ) const { return BaseImage() + u_facing; }
  const char *Name( void ) const { return u_type->Name(); }
  unsigned char Owner( void ) const { return u_pid; }
  void SetOwner( unsigned char pid ) { u_pid = pid; }
  const Point &Position( void ) const { return u_pos; }
  void SetPosition( const Point &pos ) { u_pos = pos; }
  unsigned short Terrain( void ) const { return u_type->Terrain(); }
  const UnitType *Type( void ) const { return u_type; }
  void SetType( const UnitType *type ) { u_type = type; }
  unsigned char XPLevel( void ) const { return u_xp/XP_PER_LEVEL; }
  void SetXP( unsigned char xp ) { u_xp = xp; }

  void SetDirection( unsigned char dir ) { u_facing = dir; }
  unsigned char GetDirection( void ) const { return u_facing; }

  unsigned long Flags( void ) const { return u_flags; }
  void SetFlags( unsigned long f ) { u_flags |= (f); }
  void UnsetFlags( unsigned long f ) { u_flags &= (~f); }

  bool IsGround( void ) const { return (u_flags & U_GROUND) != 0; }
  bool IsShip( void ) const { return (u_flags & U_SHIP) != 0; }
  bool IsAircraft( void ) const { return (u_flags & U_AIR) != 0; }
  bool IsMine( void ) const { return (u_flags & U_MINE) != 0; }
  bool IsTransport( void ) const { return (u_flags & U_TRANSPORT) != 0; }
  bool IsMedic( void ) const { return (u_flags & U_MEDIC) != 0; }
  bool IsMinesweeper( void ) const { return (u_flags & U_MINESWEEPER) != 0; }

  bool IsSlow( void ) const { return (u_flags & U_SLOW) != 0; }
  bool IsSheltered( void ) const { return (u_flags & U_SHELTERED) != 0; }
  bool IsFloating( void ) const { return (u_flags & U_FLOATING) != 0; }
  bool IsReady( void ) const { return (u_flags & U_DONE) == 0; }

  unsigned short Weight( void ) const { return u_type->Weight(); }
  unsigned short MinWeight( void ) const { return u_type->MinWeight(); }
  unsigned short MaxWeight( void ) const { return u_type->MaxWeight(); }

protected:
  Point u_pos;                  // position on map
  unsigned long u_flags;
  unsigned short u_id;
  
  unsigned char u_facing;       // direction
  unsigned char u_group;        // group size
  unsigned char u_xp;           // experience


  unsigned char u_pid;          // player id

  const UnitType *u_type;

  unsigned short u_crystals;    // only for transports
};

#endif  // _INCLUDE_ED_UNIT_H

