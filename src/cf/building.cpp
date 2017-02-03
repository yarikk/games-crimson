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
// building.cpp
////////////////////////////////////////////////////////////////////////

#include "building.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Building::Load
// DESCRIPTION: Load a building definition from a file.
// PARAMETERS : file - data file descriptor
// RETURNS    : -1 on error, owner identifier (>= 0) otherwise
////////////////////////////////////////////////////////////////////////

short Building::Load( MemBuffer &file ) {
  unsigned char pid, minw, maxw;

  b_id = file.Read16();
  b_pos.x = file.Read16();
  b_pos.y = file.Read16();
  b_flags = file.Read16();
  b_crystals = file.Read16();
  b_crystalstore = file.Read16();
  b_uprod = file.Read32();
  b_cprod = file.Read8();
  pid = file.Read8();
  minw = file.Read8();
  maxw = file.Read8();
  b_name_id = file.Read8();

  SetWeightLimits( minw, maxw );

  return pid;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Building::Save
// DESCRIPTION: Save the building to a file.
// PARAMETERS : file - save file descriptor
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Building::Save( MemBuffer &file ) const {
  unsigned char pid;

  if ( b_player ) pid = b_player->ID();
  else pid = PLAYER_NONE;

  file.Write16( b_id );
  file.Write16( b_pos.x );
  file.Write16( b_pos.y );
  file.Write16( b_flags );
  file.Write16( b_crystals );
  file.Write16( b_crystalstore );
  file.Write32( b_uprod );
  file.Write8( b_cprod );
  file.Write8( pid );
  file.Write8( MinWeight() );
  file.Write8( MaxWeight() );
  file.Write8( b_name_id );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Building::Allow
// DESCRIPTION: Check whether a unit is allowed to enter the building.
// PARAMETERS : unit - unit to check permission for
// RETURNS    : true if unit may enter, false if not
////////////////////////////////////////////////////////////////////////

bool Building::Allow( const Unit *unit ) const {
  if ( (unit->Owner() == b_player) || unit->IsConquer() )
    return UnitContainer::Allow( unit );
  return false;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Building::SetCrystals
// DESCRIPTION: Set crystals in stock to a specified value. This can
//              never be higher than b_crystalstore.
// PARAMETERS : crystals - requested amount of crystals
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Building::SetCrystals( unsigned short crystals ) {
  b_crystals = MIN( crystals, b_crystalstore );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Building::ModifyCrystals
// DESCRIPTION: Change the amount of crystals in stock.
// PARAMETERS : crystals - requested change in crystal storage
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Building::ModifyCrystals( short crystals ) {
  short crys = crystals + b_crystals;
  if ( crys > b_crystalstore ) b_crystals = b_crystalstore;
  else if ( crys < 0 ) b_crystals = 0;
  else b_crystals = crys;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Building::ModifyCrystalProduction
// DESCRIPTION: Change the current mining value.
// PARAMETERS : crystals - requested change in mining activity
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Building::ModifyCrystalProduction( short crystals ) {
  short crys = crystals + b_cprod;
  if ( crys > 200 ) b_cprod = 200;
  else if ( crys < 0 ) b_cprod = 0;
  else b_cprod = crys;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Building::SetOwner
// DESCRIPTION: Set the controller of the building. This also makes all
//              units inside change sides.
// PARAMETERS : player  - new controller
//              recurse - if true also change units inside
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Building::SetOwner( Player *player, bool recurse /* = true */ ) {
  b_player = player;

  if ( recurse ) {
    // convert all units inside to their new master
    UCNode *n = static_cast<UCNode *>( uc_units.Head() );
    while ( n ) {
      n->uc_unit->SetOwner( player );
      n->uc_unit->SetFlags( U_DONE );
      n = static_cast<UCNode *>( n->Next() );
    }
  }
}

