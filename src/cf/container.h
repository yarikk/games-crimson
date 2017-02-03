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
// container.h - UnitContainer and Transport classes
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_CONTAINER_H
#define _INCLUDE_CONTAINER_H

#include "list.h"
#include "unit.h"

#define UC_MAX_SLOTS	30000  // number of unit slots for buildings (!= units)

// node class used for keeping track of units
class UCNode : public Node {
public:
  UCNode( Unit *unit ) { uc_unit = unit; }

  Unit *uc_unit;
};


class UnitContainer {
public:
  UnitContainer( void );
  virtual ~UnitContainer( void ) {}

  virtual unsigned short Crystals( void ) const = 0;
  virtual void SetCrystals( unsigned short crystals ) = 0;
  virtual unsigned short MaxCrystals( void ) const = 0;
  void SetWeightLimits( unsigned char min, unsigned char max )
       { uc_min_weight = min; uc_max_weight = max; }

  short InsertUnit( Unit *unit );
  void RemoveUnit( Unit *unit );

  Unit *GetUnit( short slot ) const;
  virtual bool Allow( const Unit *unit ) const;
  bool Allow( const UnitType *type ) const;

  unsigned char UnitCount( void ) const { return uc_units.CountNodes(); }
  unsigned short Slots( void ) const { return uc_slots; }
  unsigned short FullSlots( void ) const { return uc_slots_full; }
  unsigned char MinWeight( void ) const { return uc_min_weight; }
  unsigned char MaxWeight( void ) const { return uc_max_weight; }

protected:
  unsigned short uc_slots;
  unsigned short uc_slots_full;
  unsigned char uc_min_weight;
  unsigned char uc_max_weight;

  List uc_units;
};


class Transport : public Unit, public UnitContainer {
public:
  Transport( void ) : t_crystals(0) {}
  Transport( const UnitType *type, Player *player, unsigned short id,
             const Point &pos );

  int Load( MemBuffer &file, const UnitType *type, Player *player );
  int Save( MemBuffer &file ) const;

  void SetCrystals( unsigned short crystals );
  unsigned short Crystals( void ) const { return t_crystals; }
  unsigned short MaxCrystals( void ) const { return Slots() * 10; }
  void SetPosition( short x, short y );
  bool Hit( unsigned short damage );
  bool Allow( const Unit *unit ) const;
  unsigned short Weight( void ) const;

private:
  unsigned short t_crystals;
};

#endif	/* _INCLUDE_CONTAINER_H */

