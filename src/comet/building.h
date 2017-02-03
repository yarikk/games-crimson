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
// building.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_ED_BUILDING_H
#define _INCLUDE_ED_BUILDING_H

#include "unit.h"

class Building : public Node, public MapObject {
public:
  Building( void ) : MapObject(MO_BUILDING), b_name(0) {}
  Building( const Point &pos, unsigned short id, unsigned char pid );
  int Load( MemBuffer &file );
  int Save( MemBuffer &file ) const;
  int Export( ofstream &file, const UnitSet *uset ) const;

  bool Allow( const Unit *unit ) const;

  bool IsWorkshop( void ) const { return (b_flags & BLD_WORKSHOP) != 0; }
  bool IsFactory( void ) const { return (b_flags & BLD_FACTORY) != 0; }
  bool IsMine( void ) const { return CrystalProduction() > 0; }

  unsigned char CrystalProduction( void ) const { return b_cprod; }
  unsigned short Crystals( void ) const { return b_crystals; }
  unsigned short MaxCrystals( void ) const { return b_maxcrystals; }
  unsigned short ID( void ) const { return b_id; }
  signed char NameID( void ) const { return b_name_id; }
  const char *Name( void ) const { return b_name; }
  unsigned char Owner( void ) const { return b_pid; }
  const Point &Position( void ) const { return b_pos; }

  unsigned short MinWeight( void ) const { return b_minweight; }
  unsigned short MaxWeight( void ) const { return b_maxweight; }
  void SetMinWeight( unsigned short w ) { b_minweight = w; }
  void SetMaxWeight( unsigned short w ) { b_maxweight = w; }

  unsigned long UnitProduction( void ) const { return b_uprod; }
  bool CanProduce( unsigned short utid ) const
                 { return (UnitProduction() & (1 << utid)) != 0; }

  void SetID( unsigned short id ) { b_id = id; }
  void SetNameID( signed char name ) { b_name_id = name; }
  void SetName( const char *name ) { b_name = name; }
  void SetPosition( const Point &pos ) { b_pos = pos; }
  void SetOwner( unsigned char player ) { b_pid = player; }
  void SetCrystalProduction( unsigned char crystals ) { b_cprod = crystals; }
  void SetCrystals( unsigned short crystals ) { b_crystals = crystals; }
  void SetMaxCrystals( unsigned short crystals ) { b_maxcrystals = crystals; }
  void SetUnitProduction( unsigned long unitmask ) { b_uprod = unitmask; }

  void SetFlags( unsigned short flags ) { b_flags |= flags; }
  void UnsetFlags( unsigned short flags ) { b_flags &= ~(flags); }

private:
  Point b_pos;

  unsigned short b_id;

  unsigned short b_flags;
  unsigned short b_crystals;      // amount of crystals on hold
  unsigned short b_maxcrystals;   // maximum amount of crystals that can be stored

  unsigned long b_uprod;          // unit types that can be built for factories
  unsigned char b_cprod;          // crystal production per turn for mines

  unsigned char b_minweight;      // minimum unit weight allowed
  unsigned char b_maxweight;      // maximum unit weight allowed

  unsigned char b_pid;
  signed char b_name_id;
  const char *b_name;
};

#endif  // _INCLUDE_ED_BUILDING_H

