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

///////////////////////////////////////////////////////////////////////
// combat.cpp
///////////////////////////////////////////////////////////////////////

#include "combat.h"
#include "game.h"
#include "hexsup.h"
#include "msgs.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Combat::Combat
// DESCRIPTION: Keep the combat data until end of turn when all clashes
//              will be resolved.
// PARAMETERS : att - attacking unit
//              def - defending unit
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Combat::Combat( Unit *att, Unit *def ) :
  c_att(att), c_def(def), aamod(0), admod(0), damod(0), ddmod(0) {}

////////////////////////////////////////////////////////////////////////
// NAME       : Combat::Load
// DESCRIPTION: Load combat data from a file.
// PARAMETERS : file    - data file descriptor
//              mission - pointer to mission object
// RETURNS    : -1 on error, 0 on success
////////////////////////////////////////////////////////////////////////

int Combat::Load( MemBuffer &file, Mission &mission ) {
  aamod = admod = damod = ddmod = 0;
  c_att = mission.GetUnit( file.Read16() );
  c_def = mission.GetUnit( file.Read16() );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Combat::Save
// DESCRIPTION: Save combat data to a file.
// PARAMETERS : file - data file descriptor
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Combat::Save( MemBuffer &file ) const {
  file.Write16( c_att->ID() );
  file.Write16( c_def->ID() );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Combat::CalcModifiers
// DESCRIPTION: Calculate attack and defence modifiers for this combat.
//              This should be done after all pairings for this turn
//              are known and units cannot be moved but before the first
//              shot is fired.
// PARAMETERS : map - pointer to the map object
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Combat::CalcModifiers( const Map &map ) {
  const Point &apos = c_att->Position(), &dpos = c_def->Position();
  bool nextto = NextTo( apos, dpos );

  // calculate modifiers; these are already calculated when initiating
  // combat because they are subject to change during the execution of
  // attack commands; terrain modifiers do not apply for aircraft, of
  // course, and only if it's not a ranged attack
  // if, however, it is a ranged attack give a uniform bonus because
  // we are not directly engaged
  if ( !c_att->IsAircraft() ) {
    if ( nextto ) aamod += map.AttackMod( apos );
    else aamod += 5;
    admod = map.DefenceMod( apos );
  }

  if ( !c_def->IsAircraft() ) {
    damod += map.AttackMod( dpos );
    ddmod += map.DefenceMod( dpos );
  }

  // check for units supporting our attack (wedging) or supporting defence (blocking),
  // only for non-ranged attacks
  if ( nextto ) {
    Point nbors[6];
    Unit *u;
    map.GetNeighbors( dpos, nbors );

    for ( int i = NORTH; i <= NORTHWEST; ++i ) {
      if ( nbors[i].x != -1 ) {
        u = map.GetUnit( nbors[i] );
        if ( u ) {
          if ( u != c_att ) {
            if ( u->Owner() == c_att->Owner() ) {
              if ( u->CouldHit( c_def ) ) aamod += 10;
              else aamod += 5;

              // the unit might also help defend against the retaliation attack
              if ( NextTo( nbors[i], apos ) ) admod += 10;
            }
          } else {
            // if there's a supporter in the defender's back get another 10% plus
            Direction behind = ReverseDir( i );
            if ( nbors[behind].x != -1 ) {
              Unit *stab = map.GetUnit( nbors[behind] );
              if ( stab && (stab->Owner() == c_att->Owner()) ) {
                if ( stab->CouldHit( c_def ) ) aamod += 10;
                else aamod += 5;
              }
            }
          }
        }
      }
    }

    // check for units supporting defence (blocking), only for non-ranged attacks
    Point upos;
    Direction attdir = Hex2Dir( dpos, apos );

    // any unit can help with blocking
    if ( !map.Dir2Hex( dpos, TurnLeft(attdir), upos ) &&
         (u = map.GetUnit( upos )) && (u->Owner() == c_def->Owner()) ) ddmod += 10;
    if ( !map.Dir2Hex( dpos, TurnRight(attdir), upos ) &&
         (u = map.GetUnit( upos )) && (u->Owner() == c_def->Owner()) ) ddmod += 10;
  } else ddmod -= 5;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Combat::CalcResults
// DESCRIPTION: Resolve a firefight between two units with a known
//              outcome.
// PARAMETERS : atthits - hits scored by attacker
//              defhits - hits scored by defender
// RETURNS    : Point containing hits by the attacker and defender
////////////////////////////////////////////////////////////////////////

Point Combat::CalcResults( unsigned char atthits, unsigned char defhits ) const {
  unsigned short numatts = c_att->GroupSize(), numdefs = c_def->GroupSize();

  c_def->Hit( atthits );
  c_att->Hit( defhits );

  // award experience: 1 if the enemy troops were reduced, 3 if they were destroyed
  if ( !c_att->IsMine() ) {
    if ( !c_att->IsAlive() ) c_def->AwardXP( 3 );
    else if ( c_att->GroupSize() < numatts ) c_def->AwardXP( 1 );
  }
  if ( !c_def->IsMine() ) {
    if ( !c_def->IsAlive() ) c_att->AwardXP( 3 );
    else if ( c_def->GroupSize() < numdefs ) c_att->AwardXP( 1 );
  }

  return Point( atthits, defhits );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Combat::CalcResults
// DESCRIPTION: Resolve a firefight between two units.
// PARAMETERS : -
// RETURNS    : Point containing hits by the attacker and defender
//
// NOTE       : Should the terrain modifiers also apply for both
//              attack and defence? Currently they are used for attack
//              for the attacking unit and for defence for the defending
//              unit only.
////////////////////////////////////////////////////////////////////////

Point Combat::CalcResults( void ) {
  Point p1 = c_att->Position(), p2 = c_def->Position();
  unsigned short numatts = c_att->GroupSize(), numdefs = c_def->GroupSize(),
    dist = Distance( p1.x, p1.y, p2.x, p2.y );
  int i;
  unsigned char ahits = 0, dhits = 0, atohit, dtohit, atodef, dtodef,
                aapool = 0, adpool = 0, dapool = 0, ddpool = 0,
                aastr, adstr, dastr, ddstr,
                axp = c_att->XPLevel() * 2, dxp = c_def->XPLevel() * 2;
  atohit = MAX( 25 + axp - (dist - 1) * 2 + aamod, 1 );
  atodef = 25 + axp + admod;
  dtodef = 25 + dxp + ddmod;
  aastr = c_att->OffensiveStrength( c_def ) + axp;
  adstr = c_att->DefensiveStrength() + axp;
  dastr = c_def->OffensiveStrength( c_att ) + dxp;
  ddstr = c_def->DefensiveStrength() + dxp;

  // can the defender return fire?
  if ( (dist == 1) && c_def->CanHit( c_att ) ) {
    dtohit = 25 + dxp + damod;
  } else dtohit = 0;

  for ( i = 0; i < numatts; ++i ) {
    if ( random( 1, 100 ) <= atohit )
      aapool += aastr * random( 80, 120 ) / 100;
    if ( random( 1, 100 ) <= atodef )
      adpool += adstr * random( 80, 120 ) / 100;
  }

  for ( i = 0; i < numdefs; ++i ) {
    if ( random( 1, 100 ) <= dtohit )
      dapool += dastr * random( 80, 120 ) / 100;
    if ( random( 1, 100 ) <= dtodef )
      ddpool += ddstr * random( 80, 120 ) / 100;
  }

  // set defence pools to a minimum of the enemy unit strength;
  // that avoids division by zero and leads to somewhat sane results
  if ( adpool <= dastr ) adpool = MAX( 1, dastr );
  if ( ddpool < aastr ) ddpool = aastr;   // attacker can't have 0 strength

  // generally, the number of hits is determined by comparing the attack and
  // defence pool values. To reduce the impact of luck, however, we take the
  // average of the "randomized" and a strictly deterministic number.
  ahits = (aapool/ddpool +
          (aastr * atohit)/(ddstr * dtodef) + 1) / 2;
  dhits = (dapool/adpool +
          (dastr * dtohit)/(adstr * atodef) + 1) / 2;

  return CalcResults( ahits, dhits );
}
