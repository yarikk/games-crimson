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

/////////////////////////////////////////////////////////////////////////
// unit.cpp
////////////////////////////////////////////////////////////////////////

#include "unit.h"
#include "hexsup.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::Unit
// DESCRIPTION: Create a new unit.
// PARAMETERS : type   - pointer to a unit type definition
//              player - pointer to the player this unit belongs to
//              id     - unique unit identifier
//              pos    - position on map
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Unit::Unit( const UnitType *type, Player *player, unsigned short id, const Point &pos ) :
      MapObject( MO_UNIT ), u_pos(pos), u_id(id), u_type(type), u_player(0) {
  u_flags = type->Flags();
  u_facing = NORTH;
  u_group = MAX_GROUP_SIZE;
  u_xp = 0;

  u_target.x = u_target.y = 0;

  SetOwner( player );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::Load
// DESCRIPTION: Load a unit from a data file.
// PARAMETERS : file   - descriptor of an open data file
//              type   - pointer to a unit type definition
//              player - pointer to the player this unit belongs to
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

int Unit::Load( MemBuffer &file, const UnitType *type, Player *player ) {
  u_pos.x = file.Read16();
  u_pos.y = file.Read16();
  u_flags = file.Read32();
  u_id = file.Read16();
  u_facing = file.Read8();
  u_group = file.Read8();
  u_xp = file.Read8();
  u_target.x = file.Read16();
  u_target.y = file.Read16();

  u_type = type;
  u_player = 0;

  SetOwner( player );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::Save
// DESCRIPTION: Save the unit to a data file.
// PARAMETERS : file - descriptor of the save file
// RETURNS    : 0 on succes, non-zero on error
////////////////////////////////////////////////////////////////////////

int Unit::Save( MemBuffer &file ) const {
  file.Write8( u_type->ID() );
  file.Write8( u_player ? u_player->ID() : PLAYER_NONE );

  file.Write16( u_pos.x );
  file.Write16( u_pos.y );
  file.Write32( u_flags );
  file.Write16( u_id );

  file.Write8( u_facing );
  file.Write8( u_group );
  file.Write8( u_xp );

  file.Write16( u_target.x );
  file.Write16( u_target.y );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::AwardXP
// DESCRIPTION: Raise the unit's experience level. This will improve its
//              chances to survive a battle and inflict more damage to
//              the enemy.
// PARAMETERS : xp - number of experience points to give; a unit
//                   advances to a new experience level every 3 points
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Unit::AwardXP( unsigned char xp ) {
  // XP_MAX_LEVEL is the maximum experience level a unit can reach,
  // i.e. XP_MAX_LEVEL * XP_PER_LEVEL is the upper limit in points
  u_xp = MIN( u_xp + xp, XP_MAX_LEVEL * XP_PER_LEVEL );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::SetOwner
// DESCRIPTION: Change the controller of the unit.
// PARAMETERS : player - pointer to new controller
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Unit::SetOwner( Player *player ) {
  if ( player != u_player ) {
    if ( !IsDummy() ) {
      if ( u_player ) u_player->Units( -1 );
      player->Units( 1 );
    }
    u_player = player;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::SetPosition
// DESCRIPTION: Change the position of the unit on the map. This is just
//              the low level function. It does not update the display
//              or do other fancy things. It's just about data shuffling.
// PARAMETERS : x - new x position
//              y - new y position
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Unit::SetPosition( short x, short y ) {
  u_pos.x = x;
  u_pos.y = y;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::Attack
// DESCRIPTION: Store an attack target. Again, this function does not
//              handle the display update, or the game mode change.
// PARAMETERS : enemy - unit to attack
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Unit::Attack( const Unit *enemy ) {
  u_target = enemy->Position();
  SetFlags( U_ATTACKED|U_DONE );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::DefensiveStrength
// DESCRIPTION: Get a number measuring the defensive capabilities
//              of the unit.
// PARAMETERS : -
// RETURNS    : armour value
////////////////////////////////////////////////////////////////////////

unsigned char Unit::DefensiveStrength( void ) const {
  return u_type->Armour();
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::OffensiveStrength
// DESCRIPTION: Return the combat value of the unit against the given
//              target.
// PARAMETERS : target - unit to attack
// RETURNS    : offensive strength
////////////////////////////////////////////////////////////////////////

unsigned char Unit::OffensiveStrength( const Unit *target ) const {
  unsigned char pow;

  if ( target->IsShip() || target->IsFloating() ) pow = u_type->Firepower(U_SHIP);
  else if ( target->IsAircraft() ) pow = u_type->Firepower(U_AIR);
  else pow = u_type->Firepower(U_GROUND);
  return pow;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::Hit
// DESCRIPTION: Inflict damage to the unit and destroy it if necessary.
// PARAMETERS : damage - how much damage is done
// RETURNS    : true if the unit was destroyed, false otherwise
////////////////////////////////////////////////////////////////////////

bool Unit::Hit( unsigned short damage ) {
  if ( IsAlive() ) {

    if ( u_group <= damage ) {     // unit destroyed
      u_group = 0;
      SetFlags( U_DESTROYED );
      u_pos.x = u_pos.y = -1;

      if ( !IsDummy() ) u_player->Units( -1 );
      return true;
    }

    u_group -= damage;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::CanHitType
// DESCRIPTION: Check whether the unit can shoot at an enemy unit. The
//              test is performed looking only at the enemy unit type
//              (ground, ship. aircraft, or submarine). If the function
//              returns true, this doesn't mean that the enemy is
//              actually in range for the unit's weapons systems.
// PARAMETERS : enemy - enemy unit
// RETURNS    : true if the unit type can be shot; false otherwise
////////////////////////////////////////////////////////////////////////

bool Unit::CanHitType( const Unit *enemy ) const {
  return WeaponRange( enemy ) > 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::CanHit
// DESCRIPTION: Check whether the unit can shoot at an enemy unit. The
//              test is performed with regard to the enemy unit type
//              (ground, ship. aircraft, or submarine) and the distance
//              of the target.
// PARAMETERS : enemy - enemy unit
// RETURNS    : true if the unit can be shot; false if it's the wrong
//              type, too far away, or not hostile
////////////////////////////////////////////////////////////////////////

bool Unit::CanHit( const Unit *enemy ) const {
  if ( (u_player == enemy->Owner()) ||
       (u_flags & (U_DONE|U_SHELTERED)) ||
       enemy->IsSheltered() ) return false;
  return CouldHit( enemy );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::CouldHit
// DESCRIPTION: Check whether the unit could shoot at an enemy unit.
//              Current unit status is not taken into account, so this
//              function may return true, even though the unit already
//              attacked this turn or moved and can't attack any more.
// PARAMETERS : enemy - enemy unit
// RETURNS    : true if the unit type could be shot; false otherwise
////////////////////////////////////////////////////////////////////////

bool Unit::CouldHit( const Unit *enemy ) const {
  unsigned short dist = Distance( u_pos, enemy->Position() );
  unsigned long type;

  // be careful with amphibian units
  if ( enemy->IsAircraft() ) type = U_AIR;
  else if ( enemy->IsShip() || enemy->IsFloating() ) type = U_SHIP;
  else type = U_GROUND;

  return u_type->IsInFOF( dist, type );
}

////////////////////////////////////////////////////////////////////
// NAME       : Unit::Repair
// DESCRIPTION: Repair the unit (completely). This will get the
//              group size of the unit back to the maximum, but will
//              also lower its experience by one point per "rookie".
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Unit::Repair( void ) {
  if ( u_group < MAX_GROUP_SIZE ) {
    u_xp = MAX( 0, u_xp - (MAX_GROUP_SIZE - u_group) );
    u_group = MAX_GROUP_SIZE;
    SetFlags( U_DONE );	// can't move this turn
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::WeaponRange
// DESCRIPTION: Get the distance from which the unit may shoot at a
//              given target.
// PARAMETERS : u - target unit
// RETURNS    : maximum range of fire
////////////////////////////////////////////////////////////////////////

unsigned char Unit::WeaponRange( const Unit *u ) const {
  unsigned char range;

  if ( u->IsAircraft() ) range = u_type->MaxFOF(U_AIR);
  else if ( u->IsShip() || u->IsFloating() ) range = u_type->MaxFOF(U_SHIP);
  else range = u_type->MaxFOF(U_GROUND);
  return range;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::Moves
// DESCRIPTION: Get movement points for this unit.
// PARAMETERS : -
// RETURNS    : movement points
////////////////////////////////////////////////////////////////////////

unsigned char Unit::Moves( void ) const {
  unsigned char mp;

  if ( Flags() & (U_ATTACKED|U_MOVED|U_DONE) ) mp = 0;
  // mines inside shops or transporters get MCOST_MIN points so they
  // can move out
  else if ( IsMine() && IsSheltered() ) mp = 1;
  else mp = u_type->Speed();
  return mp;
}

