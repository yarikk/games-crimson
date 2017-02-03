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
// building.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_BUILDING_H
#define _INCLUDE_BUILDING_H

#include "container.h"
#include "player.h"

class Building : public UnitContainer, public Node, public MapObject {
public:
  Building( void ) :
    MapObject(MO_BUILDING), b_name(0), b_player(0) {}
  short Load( MemBuffer &file );
  int Save( MemBuffer &file ) const;

  bool Allow( const Unit *unit ) const;

  bool IsWorkshop( void ) const { return (b_flags & BLD_WORKSHOP) != 0; }
  bool IsFactory( void ) const { return (b_flags & BLD_FACTORY) != 0; }
  bool IsMine( void ) const { return CrystalProduction() > 0; }

  unsigned char CrystalProduction( void ) const { return b_cprod; }
  unsigned short Crystals( void ) const { return b_crystals; }
  unsigned short MaxCrystals( void ) const { return b_crystalstore; }
  unsigned short ID( void ) const { return b_id; }
  const char *Name( void ) const { return b_name; }
  signed char NameID( void ) const { return b_name_id; }
  Player *Owner( void ) const { return b_player; }
  const Point &Position( void ) const { return b_pos; }
  unsigned long UnitProduction( void ) const { return b_uprod; }

  void SetOwner( Player *player, bool recurse = true );
  void SetCrystalProduction( unsigned char crystals ) { b_cprod = crystals; }
  void ModifyCrystalProduction( short crystals );
  void SetCrystals( unsigned short crystals );
  void ModifyCrystals( short crystals );
  void SetUnitProduction( unsigned long unitmask ) { b_uprod |= unitmask; }
  void UnsetUnitProduction( unsigned long unitmask ) { b_uprod &= ~unitmask; }
  void SetName( const char *name ) { b_name = name; }

private:
  Point b_pos;

  unsigned short b_id;

  unsigned short b_flags;
  unsigned short b_crystals;      // amount of crystals on hold
  unsigned short b_crystalstore;  // maximum amount of crystals that can be stored

  unsigned char b_cprod;          // crystal production per turn for mines
  unsigned long b_uprod;          // unit types that can be built for factories

  signed char b_name_id;
  const char *b_name;

  Player *b_player;
};

#endif	/* _INCLUDE_BUILDING_H */

