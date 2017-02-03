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
// container.cpp
////////////////////////////////////////////////////////////////////////

#include "container.h"
#include "map.h"

////////////////////////////////////////////////////////////////////////
// NAME       : UnitContainer::UnitContainer
// DESCRIPTION: Initialize a unit container.
// PARAMETERS : type - either MO_UNIT or MO_BUILDING
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

UnitContainer::UnitContainer( void ) :
  uc_slots(UC_MAX_SLOTS), uc_slots_full(0),
  uc_min_weight(0), uc_max_weight(99) {}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitContainer::InsertUnit
// DESCRIPTION: Put a unit into the container. No checks are performed.
// PARAMETERS : unit - unit to insert
// RETURNS    : 1 if the owner of the building changed because of the
//              insertion (an enemy unit with the U_CONQUER flag came
//              in), -1 on error, 0 otherwise
////////////////////////////////////////////////////////////////////////

short UnitContainer::InsertUnit( Unit *unit ) {
  short rc = 0;

  UCNode *n = new UCNode( unit );
  if ( n ) {
    unit->SetFlags( U_SHELTERED );
    uc_units.AddTail( n );

    if ( dynamic_cast<MapObject *>(this)->Owner() != unit->Owner() ) rc = 1;

    // if a transport is coming in, we must unload it
    if ( unit->IsTransport() ) {
      Transport *t = static_cast<Transport *>(unit);

      SetCrystals( Crystals() + t->Crystals() );
      t->SetCrystals( 0 );

      while ( Unit *u = t->GetUnit(0) ) {
        t->RemoveUnit( u );
        InsertUnit( u );
      }
    }

    uc_slots_full += unit->Weight();
  } else rc = -1;
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitContainer::RemoveUnit
// DESCRIPTION: Take a unit out of the container.
// PARAMETERS : unit - unit to remove from container
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void UnitContainer::RemoveUnit( Unit *unit ) {
  if ( unit->IsDummy() ) unit->UnsetFlags( U_SHELTERED );
  else {

    UCNode *n = static_cast<UCNode *>( uc_units.Head() );

    while ( n ) {
      if ( n->uc_unit == unit ) {
        n->Remove();
        delete n;

        unit->UnsetFlags( U_SHELTERED );
        // only subtract unit's own weight even for transports
        // since carried units are removed separately
        uc_slots_full -= unit->Unit::Weight();
        break;
      }
      n = static_cast<UCNode *>( n->Next() );
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitContainer::GetUnit
// DESCRIPTION: Get the unit in the specified slot from the container.
// PARAMETERS : slot - slot number for the unit you want
// RETURNS    : pointer to the unit in slot, or NULL if none
////////////////////////////////////////////////////////////////////////

Unit *UnitContainer::GetUnit( short slot ) const {
  UCNode *n = static_cast<UCNode *>( uc_units.GetNode( slot ) );
  return ( n ? n->uc_unit : NULL );
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitContainer::Allow
// DESCRIPTION: Check whether a unit is allowed to enter.
// PARAMETERS : unit - unit to check permission for
// RETURNS    : true if unit may enter, false if not
////////////////////////////////////////////////////////////////////////

bool UnitContainer::Allow( const Unit *unit ) const {
  if ( !Allow( unit->Type() ) ) return false;

  int i;
  for ( i = UnitCount() - 1; i >= 0; --i )
    if ( GetUnit(i) == unit ) return true;

  if ( unit->IsTransport() ) {
    const Transport *t = static_cast<const Transport *>(unit);

    if ( Crystals() + t->Crystals() > MaxCrystals() ) return false;

    for ( i = t->UnitCount() - 1; i >= 0; --i )
      if ( !Allow( t->GetUnit(i) ) ) return false;
  }

  return uc_slots >= uc_slots_full + unit->Weight();
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitContainer::Allow
// DESCRIPTION: Check whether a unit type is allowed to enter.
// PARAMETERS : type - unit type to check permission for
// RETURNS    : true if unit type may enter, false if not
////////////////////////////////////////////////////////////////////////

bool UnitContainer::Allow( const UnitType *type ) const {
  return (type->Weight() >= MinWeight()) &&
         (type->Weight() <= MaxWeight()) &&
         (type->Weight() + uc_slots_full <= uc_slots);
}

////////////////////////////////////////////////////////////////////////
// NAME       : Transport::Transport
// DESCRIPTION: Create a new transport instance.
// PARAMETERS : type   - unit type definition
//              player - transport controller
//              id     - unique transport unit identifier
//              pos    - position on map
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Transport::Transport( const UnitType *type, Player *player, unsigned short id,
                      const Point &pos ) :
     Unit( type, player, id, pos ), t_crystals(0) {
  uc_slots = u_type->Slots();
  uc_min_weight = u_type->MinWeight();
  uc_max_weight = u_type->MaxWeight();
}

////////////////////////////////////////////////////////////////////////
// NAME       : Transport::Load
// DESCRIPTION: Load a transport from a data file.
// PARAMETERS : file   - data file descriptor
//              type   - pointer to a unit type definition
//              player - pointer to the player this unit belongs to
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

int Transport::Load( MemBuffer &file, const UnitType *type, Player *player ) {
  Unit::Load( file, type, player );
  uc_slots = u_type->Slots();
  uc_min_weight = u_type->MinWeight();
  uc_max_weight = u_type->MaxWeight();
  SetCrystals( file.Read16() );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Transport::Save
// DESCRIPTION: Save transport information to a file.
// PARAMETERS : file - save file descriptor
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Transport::Save( MemBuffer &file ) const {
  Unit::Save( file );
  file.Write16( t_crystals );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Transport::SetPosition
// DESCRIPTION: When a transport is moved on the map, it must not only
//              keep track of its own position, but update the
//              information for all units inside as well.
// PARAMETERS : x - new horizontal coordinate
//              y - new vertical coordinate
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Transport::SetPosition( short x, short y ) {
  Unit::SetPosition( x, y );

  UCNode *n = static_cast<UCNode *>( uc_units.Head() );
  while ( n ) {
    n->uc_unit->SetPosition( x, y );
    n = static_cast<UCNode *>( n->Next() );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Transport::Hit
// DESCRIPTION: When a transport is hit chances are that the units
//              inside are damaged as well. If the transport is
//              destroyed, so are the carried units.
// PARAMETERS : damage - amount of damage taken
// RETURNS    : true if transport was destroyed, false otherwise
////////////////////////////////////////////////////////////////////////

bool Transport::Hit( unsigned short damage ) {
  if ( Unit::Hit( damage ) ) {                  // destroyed
    UCNode *n = static_cast<UCNode *>( uc_units.Head() );
    while ( n ) {
      n->uc_unit->Hit( MAX_GROUP_SIZE );
      n = static_cast<UCNode *>( n->Next() );
    }
    return true;
  } else {            // randomly damage units inside
    UCNode *n = static_cast<UCNode *>( uc_units.Head() );
    while ( n ) {
      Node *next = n->Next();
      if ( n->uc_unit->Hit( random(0, damage) ) ) RemoveUnit( n->uc_unit );
      n = static_cast<UCNode *>( next );
    }
    return false;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Transport::Allow
// DESCRIPTION: Check whether a unit is allowed to enter the transport.
// PARAMETERS : unit - unit to check permission for
// RETURNS    : true if unit may enter, false if not
////////////////////////////////////////////////////////////////////////

bool Transport::Allow( const Unit *unit ) const {
  if ( unit->Owner() == u_player ) {

    if ( unit->IsTransport() ) {
      const Transport *t = static_cast<const Transport *>(unit);
      if ( uc_slots < uc_slots_full - (Crystals()+9)/10
                      + unit->Weight()
                      + (Crystals() + t->Crystals() + 9)/10 )
        return false;
    }

    return UnitContainer::Allow( unit );
  }
  return false;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Transport::Weight
// DESCRIPTION: Get the weight of the transporter plus carried units.
// PARAMETERS : -
// RETURNS    : combined weight
////////////////////////////////////////////////////////////////////////

unsigned short Transport::Weight( void ) const {
  unsigned short w = Unit::Weight();

  UCNode *n = static_cast<UCNode *>( uc_units.Head() );
  while ( n ) {
    w += n->uc_unit->Weight();
    n = static_cast<UCNode *>( n->Next() );
  }
  return w;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Transport::SetCrystals
// DESCRIPTION: Set amount of crystals carried.
// PARAMETERS : crystals - new amount carried
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Transport::SetCrystals( unsigned short crystals ) {
  uc_slots_full -= (t_crystals + 9) / 10 - (crystals + 9) / 10;
  t_crystals = crystals;
}

