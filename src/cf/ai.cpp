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
// ai.cpp
//
// The model used for the computer player in Crimson Fields is a
// variation of the widely used General/Sergeant scheme. At the
// beginning of a turn, the "General" assesses the overall situation
// on the battlefield, and identifies the major objectives for the
// computer player (usually buildings). Units are assigned to those
// objectives in order of priority, i.e. more important objectives
// are served first, and less important ones may end up with less
// firepower than required.
//   Now the "Sergeants" take over. Each of them gets one objective
// and all the units assigned to it, and it's up to those division
// commanders to decide on the actual moves.
//   Right now, the AI player is recreated each turn. This, of course,
// does not allow for collecting and analyzing long-term intelligence
// data, so maybe this should be changed in the future.
////////////////////////////////////////////////////////////////////////

#include "ai.h"
#include "game.h"
#include "misc.h"

////////////////////////////////////////////////////////////////////////
// NAME       : AI::AI
// DESCRIPTION: Initialize a computer controlled player.
// PARAMETERS : mission - current mission object
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

AI::AI( Mission &mission ) : mission(mission) {
  player = &mission.GetPlayer();
  map = &mission.GetMap();
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::Play
// DESCRIPTION: Run the computer player.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::Play( void ) {
  View *view = Gam->GetMapWindow()->GetView();

  // set up progress indicator; number of steps is unit count plus 3
  // for objectives identification, objectives assignment, and production
  progress = new ProgressWindow( 0, 0, view->Width()/2, 30,
                                 1, 3 + player->Units(0), NULL,
                                 WIN_CENTER, view );

  IdentifyObjectives();
  progress->Advance( 1 );
  AssignObjectives();

  for ( AIObj *obj = static_cast<AIObj *>( objectives.Head() );
        obj; obj = static_cast<AIObj *>( obj->Next() ) )
    ProcessObjective( *obj );

  progress->Advance( 1 );
  BuildReinforcements();
  view->CloseWindow( progress );
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::IdentifyObjectives
// DESCRIPTION: Examine the map for targets and put them into the list
//              of objectives. Generally, all buildings are considered
//              objectives.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::IdentifyObjectives( void ) {
  AIObj *obj;

  for ( Building *b = static_cast<Building *>(mission.GetShops().Head());
        b; b = static_cast<Building *>(b->Next()) ) {
    unsigned char pri = AI_PRI_LOW;
    obj = new AIObj;
    obj->pos = b->Position();
    obj->flags = 0;
    obj->needed_ground = obj->needed_ship = obj->needed_air = 0;

    if ( b->Owner() == player ) obj->type = AI_OBJ_DEFEND;
    else obj->type = AI_OBJ_CONQUER;

    // detect enemy unit presence in the vicinity
    UnitPresence( &mission.GetOtherPlayer(*player), obj );

    if ( b->IsFactory() ) pri += 10;
    else if ( b->IsWorkshop() ) pri += 2;
    if ( b->IsMine() ) pri += 5;

    if ( obj->type == AI_OBJ_DEFEND ) {
      // check enemy presence; this also affects priority
      // locate closest enemy unit able to conquer buildings or transport
      Unit *u = ClosestUnit( &mission.GetOtherPlayer(*player), obj->pos,
                             U_CONQUER|U_TRANSPORT, U_DESTROYED );
      if ( u && (Distance( u->Position(), obj->pos ) <= AI_ATTENTION_RADIUS) )
        pri = AI_PRI_CRITICAL;
      else pri = MIN( obj->needed_ground + obj->needed_air + obj->needed_ship, AI_PRI_MAX );
    } else if ( obj->type == AI_OBJ_CONQUER ) {
      // the farther away we are the lower the priority
      pri += AI_PRI_MEDIUM;
      Unit *u = ClosestUnit( player, obj->pos, U_CONQUER, U_DESTROYED );
      if ( u ) pri = MAX( AI_PRI_LOW, pri - Distance( obj->pos, u->Position() ) );
    }

    obj->priority = pri;
    AddObjective( obj );
  }

  // create an objective which simply says: "destroy all enemy units"
  // this will be used for all units we couldn't use otherwise
  obj = new AIObj;
  obj->type = AI_OBJ_ATTACK;
  obj->pos.x = -1;
  obj->pos.y = -1;
  obj->flags = 0;
  obj->needed_ground = obj->needed_ship = obj->needed_air = 0;
  obj->requested_ground = obj->requested_ship = obj->requested_air = 1;
  obj->priority = AI_OBJ_TRANSPORT + 1;
  AddObjective( obj );
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::AssignObjectives
// DESCRIPTION: Go through the list of units available and assign them
//              to one of the objectives we identified earlier.
// PARAMETERS : -
// RETURNS    : -
//
// NOTE       : Transports are not assigned to standard objectives in
//              the first iteration. They are either pulled in by other
//              units requesting their service or simply added to more
//              or less random objectives at the end of the block. This
//              is ok as long as transports are only weak fighters but
//              will probably result in severely reduced effectiveness
//              otherwise.
////////////////////////////////////////////////////////////////////////

void AI::AssignObjectives( void ) {
  AIObj *obj = static_cast<AIObj *>(objectives.Head()),
        *attack_all = static_cast<AIObj *>(objectives.Tail());
  Unit *u, *last;

  while ( obj ) {
    if ( obj->type == AI_OBJ_CONQUER ) {
      // find ourselves a unit which can take over buildings
      bool done = false, found = true;
      last = NULL;

      do {
        u = ClosestUnit( player, obj->pos, U_CONQUER, U_BUSY|U_TRANSPORT, last );
        if ( u ) {
          if ( !UnitCanReach( u, obj->pos, 0 ) ) last = u;
          else {
            obj->AssignUnit( u, UnitStrength(u) );
            done = true;
          }
        } else {
          // we can't conquer the building, so remove the objective
          AIObj *next = static_cast<AIObj *>(obj->Next());
          obj->Remove();
          delete obj;
          obj = next;
          done = true;
          found = false;
        }
      } while ( !done );

      if ( !found ) continue;      // go to next objective
    }

    // cycle through the units list
    // start with the unit closest to the target
    last = NULL;
    while ( (u = ClosestUnit( player, obj->pos, U_GROUND|U_AIR|U_SHIP, U_BUSY|U_TRANSPORT, last )) ) {
      if ( u->IsMine() ) u->SetFlags( U_BUSY|U_DONE );
      else if ( u->Moves() == 0 ) {
        // stationary units may attack anything that moves...
        attack_all->AssignUnit( u, 0 );
      } else if ( (!u->IsConquer() || (obj->type == AI_OBJ_CONQUER)) &&      // use infantry only
                UnitCanReach( u, obj->pos, AI_ATTENTION_RADIUS ) ) {       // for taking buildings
        const UnitType *type = u->Type();
        if ( ((obj->needed_air > 0) && (type->Firepower(U_AIR) > 0)) ||
             ((obj->needed_ground > 0) && (type->Firepower(U_GROUND) > 0)) ||
             ((obj->needed_ship > 0) && (type->Firepower(U_SHIP) > 0)) ) {
          // assign unit to target
          obj->AssignUnit( u, UnitStrength(u) );
          if ( obj->needed_air + obj->needed_ground + obj->needed_ship == 0 )
            break;      // requested firepower allocated; next objective
        }
      }
      last = u;
    }

    obj = static_cast<AIObj *>(obj->Next());
  }

  // now check from the back of the list and remove any offensive objectives
  // which have not had any one unit assigned as well as those which have not
  // had the requested number of units assigned except the one with the
  // highest priority

  bool saved_hipri = false;
  obj = static_cast<AIObj *>( objectives.Head() );
  while ( obj ) {
    if ( (obj->type == AI_OBJ_CONQUER) &&
         (obj->needed_ground + obj->needed_air + obj->needed_ship > 0) ) {
      if ( obj->alloc_units.IsEmpty() || saved_hipri ) {
        AIObj *next = static_cast<AIObj *>(obj->Next());
        obj->ReleaseUnits();
        obj->Remove();
        delete obj;
        obj = next;
        continue;
      } else saved_hipri = true;
    }
    obj = static_cast<AIObj *>( obj->Next() );
  }

  // lastly, assign all unassigned units to a task
  for ( u = static_cast<Unit *>( mission.GetUnits().Head() );
        u; u = static_cast<Unit *>(u->Next()) ) {
    if ( (u->Owner() == player) && !u->IsBusy() ) {
      AIObj *best = NULL;
      unsigned short bestval = 0;

      for ( obj = static_cast<AIObj *>(objectives.Head());
            obj; obj = static_cast<AIObj *>(obj->Next()) ) {
        const UnitType *type = u->Type();
        if ( (obj->requested_ground && type->Firepower(U_GROUND)) ||
             (obj->requested_air && type->Firepower(U_AIR)) ||
             (obj->requested_ship && type->Firepower(U_SHIP)) ) {
          unsigned short val = 1000 + obj->priority;
          if ( obj->pos != Point(-1,-1) ) val -= Distance( obj->pos, u->Position() );
          else val -= random( 1, 15 );

          if ( val > bestval ) {
            bestval = val;
            best = obj;
          }
        }
      }

      // if we didn't find a suitable objective, assign the unit to
      // the general "attack-everything-that-moves" objective
      if ( best ) best->AssignUnit( u, UnitStrength(u) );
      else attack_all->AssignUnit( u, 0 );
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::BuildReinforcements
// DESCRIPTION: Build new units in our factories.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::BuildReinforcements( void ) const {
  const UnitType *type;
  unsigned short p1_air = 0, p1_ground = 0, p1_ship = 0,
                 p2_aair = 1, p2_aground = 1, p2_aship = 1,  // prevent div by 0
                 amul, gmul, smul;

  for ( Unit *u = static_cast<Unit *>(mission.GetUnits().Head()); u;
        u = static_cast<Unit *>(u->Next()) ) {
    if ( u->Owner() == player ) {
      unsigned short defxp = u->DefensiveStrength() + 3 * u->XPLevel();
      type = u->Type();

      if ( type->Firepower(U_AIR) > 0 )
        p2_aair += (type->Firepower(U_AIR) + defxp) * u->GroupSize() / MAX_GROUP_SIZE;
      if ( type->Firepower(U_GROUND) > 0 )
        p2_aground += (type->Firepower(U_GROUND) + defxp) * u->GroupSize() / MAX_GROUP_SIZE;
      if ( type->Firepower(U_SHIP) > 0 )
        p2_aship += (type->Firepower(U_SHIP) + defxp) * u->GroupSize() / MAX_GROUP_SIZE;
    } else if ( u->Owner() ) {          // controlled by enemy player
      unsigned short str = UnitStrength( u ) * 4;
      if ( u->IsAircraft() ) p1_air += str;
      else if ( u->IsShip() || u->IsFloating() ) p1_ship += str;
      else p1_ground += str;
    }
  }

  amul = p1_air / p2_aair;
  gmul = p1_ground / p2_aground;
  smul = p1_ship / p2_aship;

  for ( Building *b = static_cast<Building *>(mission.GetShops().Head());
        b; b = static_cast<Building *>(b->Next()) ) {

    if ( (b->Owner() == player) && b->IsFactory() ) {
      const UnitType *best;
      unsigned short bestval, crystals;
      unsigned long blueprints = b->UnitProduction();

      do {
        best = NULL;
        bestval = 0;
        crystals = b->Crystals();

        for ( int i = 0; i < 32; ++i ) {
          if ( blueprints & (1 << i) ) {
            type = mission.GetUnitSet().GetUnitInfo( i );
            if ( crystals >= type->Cost() ) {
              unsigned short val = type->Firepower(U_AIR) * amul +
                                   type->Firepower(U_GROUND) * gmul +
                                   type->Firepower(U_SHIP) * smul +
                                   type->Armour() / 2;

              if ( val > bestval ) {
                bestval = val;
                best = type;
              }
            }
          }
        }

        if ( best ) {
          mission.CreateUnit( best->ID(), *player, b->Position() );
          b->SetCrystals( crystals - best->Cost() );

          // recalculate unit type multipliers
          if ( best->Firepower(U_AIR) > 0 ) {
            p2_aair += best->Firepower(U_AIR) + best->Armour();
            amul = p1_air / p2_aair;
          }
          if ( best->Firepower(U_GROUND) > 0 ) {
            p2_aground += best->Firepower(U_GROUND) + best->Armour();
            gmul = p1_ground / p2_aground;
          }
          if ( best->Firepower(U_SHIP) > 0 ) {
            p2_aship += best->Firepower(U_SHIP) + best->Armour();
            smul = p1_ship / p2_aship;
          }
        }
      } while ( best );
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::ProcessObjective
// DESCRIPTION: This function implements the duties of the sergeant,
//              who decides on the actual moves a unit will make this
//              turn in order to accomplish the assigned objective.
// PARAMETERS : obj - objective the sergeant is responsible for
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::ProcessObjective( AIObj &obj ) {

  for ( AIObj::AIAllocNode *n = static_cast<AIObj::AIAllocNode *>( obj.alloc_units.Head() );
        n; n = static_cast<AIObj::AIAllocNode *>(n->Next()) ) {
    Unit *u = n->unit;

    if ( u->IsReady() ) {
      Gam->SelectUnit( u );
      CommandUnit( u, obj );
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::AddObjective
// DESCRIPTION: Add another target to the list of objectives.
// PARAMETERS : obj - new objective
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::AddObjective( AIObj *obj ) {
  // insert the objective into the list according to its priority
  AIObj *aio, *prev = NULL;
  for ( aio = static_cast<AIObj *>(objectives.Head());
        aio && (aio->priority > obj->priority);
        aio = static_cast<AIObj *>(aio->Next()) ) prev = aio;
  objectives.InsertNode( obj, prev );
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::GetObjectiveForUnit
// DESCRIPTION: Find the objective the unit is assigned to.
// PARAMETERS : u - unit to find objective for
// RETURNS    : pointer to objective or NULL if not found
////////////////////////////////////////////////////////////////////////

AI::AIObj *AI::GetObjectiveForUnit( const Unit *u ) const {
  for ( AIObj *o = static_cast<AIObj *>(objectives.Head());
        o; o = static_cast<AIObj *>(o->Next()) ) {
    if ( o->UnitAssigned( u ) ) return o;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::CommandUnit
// DESCRIPTION: This function does the Sergeant's dirty work. It
//              inspects the given unit and decides on what this unit
//              should do - where it should go, whom it should attack,
//              and so on.
// PARAMETERS : u   - the unit to be decided on
//              obj - objective the unit was assigned to
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::CommandUnit( Unit *u, AIObj &obj ) {
  progress->Advance( 1 );

  // maybe we need an overhaul?
  if ( (obj.priority < AI_PRI_CRITICAL) ||
       (u->GroupSize() >= MAX_GROUP_SIZE / 2) ||
       !CommandUnitRepair( u ) ) {

    switch ( obj.type ) {
      case AI_OBJ_DEFEND:
        CommandUnitDefend( u, obj );
        break;
      case AI_OBJ_CONQUER:
        CommandUnitConquer( u, obj );
        break;
      case AI_OBJ_ATTACK:
        CommandUnitAttack( u, obj );
        break;
      case AI_OBJ_TRANSPORT:
        CommandUnitTransport( u, obj );
        break;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::CommandUnitDefend
// DESCRIPTION: Decide on how the unit can be used to defend the
//              objective.
// PARAMETERS : u   - the unit to be decided on
//              obj - objective of the unit (type AI_OBJ_DEFEND)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::CommandUnitDefend( Unit *u, AIObj &obj ) {
  Unit *tg;

  if ( Distance( u->Position(), obj.pos ) > AI_ATTENTION_RADIUS ) {
    // find a nice target in the direction we need to go
    bool attacked = false;
    Unit *last = NULL;
    do {
      tg = ClosestUnit( &mission.GetOtherPlayer(*player), u->Position(),
                  U_GROUND|U_AIR|U_SHIP, U_DESTROYED|U_SHELTERED, last );
      if ( tg && u->CanHitType( tg ) && SameDirection( u->Position(), tg->Position(), obj.pos ) &&
           UnitGoTo( u, tg->Position(), u->WeaponRange(tg) ) ) {
        attacked = true;
        if ( u->CanHit( tg ) ) mission.RegisterBattle( u, tg );
        break;
      }
      last = tg;
    } while ( tg );

    if ( !attacked ) UnitGoTo( u, obj.pos, AI_ATTENTION_RADIUS );
  } else {
    tg = ClosestUnit( &mission.GetOtherPlayer(*player), obj.pos,
                      U_GROUND|U_AIR|U_SHIP, U_DESTROYED|U_SHELTERED );

    // if the enemy comes too close attack
    // if our objective is between us and the enemy try to take a position in front of the objective
    if ( tg && ((Distance( u->Position(), tg->Position() ) <= AI_ATTENTION_RADIUS) ||
                SameDirection( u->Position(), tg->Position(), obj.pos )) ) {
        CommandUnitAttack( u, obj );
    } else if ( obj.pos == u->Position() ) CommandUnitAttack( u, obj );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::CommandUnitConquer
// DESCRIPTION: Try to conquer the objective or help another unit take
//              it.
// PARAMETERS : u   - the unit to be decided on
//              obj - objective of the unit (type AI_OBJ_CONQUER)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::CommandUnitConquer( Unit *u, AIObj &obj ) {
  Unit *enemy;

  if ( u->IsConquer() ) {
    // this unit won't attack anyone; we need it to get the target building
    UnitGoTo( u, obj.pos, 0 );
  } else {          // normal units move towards the objective and attack
                    // hostile units along the way
    enemy = FindBestTarget( u );
    if ( enemy && UnitGoTo( u, enemy->Position(), u->WeaponRange(enemy) ) ) {
      if ( u->CanHit( enemy ) ) mission.RegisterBattle( u, enemy );
    } else if ( !enemy ) CommandUnitReturnToBase( u );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::CommandUnitAttack
// DESCRIPTION: Attack any enemy units we can get our hands on.
// PARAMETERS : u   - the unit to be decided on
//              obj - objective of the unit (type AI_OBJ_ATTACK; unused)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::CommandUnitAttack( Unit *u, AIObj &obj ) {
  Unit *tg = FindBestTarget( u );
  if ( tg ) {
    Point dest = FindBestHex( u, tg );
    if ( (dest == u->Position()) && !u->CanHit(tg) )
      UnitGoTo( u, tg->Position(), 0 ); // FIXME: is 0 ok, or should it be WeaponRange? How to get units out then?
    else {
      UnitGoTo( u, dest, 0 );
      if ( u->CanHit( tg ) ) mission.RegisterBattle( u, tg );
    }
  } else CommandUnitReturnToBase( u );
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::CommandUnitTransport
// DESCRIPTION: This is called only for transports. Make sure the mail
//              is delivered.
// PARAMETERS : u   - unit to process (actually a transport)
//              obj - objective of the unit (type AI_OBJ_TRANSPORT)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::CommandUnitTransport( Unit *u, AIObj &obj ) {
  Transport *t = static_cast<Transport *>(u);

  if ( t->IsReady() ) {
    // now try all of the loaded units
    bool moved = false;
    TransPath tp( map, t );
    Unit *unit;

    for ( int i = t->UnitCount() - 1; (i >= 0) && !moved; --i ) {
      unit = t->GetUnit( i );
      if ( tp.Find( unit, t->Position(), obj.pos, PATH_BEST, obj.flags ) != -1 ) {
        Point dest = FollowPath( t, tp, 1 );
        Gam->MoveUnit( t, dest );
        moved = true;
      }
    }

    if ( moved ) {
      // maybe we can move the unit(s) now
      for ( int j = t->UnitCount() - 1; j >= 0; --j ) {
        unit = t->GetUnit( j );
        if ( unit->IsReady() )
          CommandUnit( unit, *GetObjectiveForUnit( unit ) );
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::CommandUnitRepair
// DESCRIPTION: Try to repair a unit.
// PARAMETERS : u - the unit to be repaired
// RETURNS    : TRUE if a suitable base with a sufficient amount of
//              crystals is found, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool AI::CommandUnitRepair( Unit *u ) {
  Building *b, *last = NULL;
  Path path(map);

  do {
    b = ClosestBuilding( player, u->Position(), last );

    int way;
    if ( b && (b->Crystals() >= CRYSTALS_REPAIR) &&
       ((way = path.Find( u, u->Position(), b->Position() )) >= 0) &&
       (way <= 5) ) {
      UnitGoTo( u, b->Position(), 0 );

      if ( u->Position() == b->Position() ) {
        b->SetCrystals( b->Crystals() - CRYSTALS_REPAIR );
        if ( mission.GetHistory() )
          mission.GetHistory()->RecordUnitEvent( *u, History::HIST_UEVENT_REPAIR );

        u->Repair();
      }
      return true;
    } else last = b;
  } while ( b );
  return false;
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::CommandUnitReturnToBase
// DESCRIPTION: Let this unit retreat to a friendly base. If we do not
//              own any buildings, the unit won't move at all...
// PARAMETERS : u - the retreating unit
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::CommandUnitReturnToBase( Unit *u ) {
  Building *b, *last = NULL;

  do {
    b = ClosestBuilding( player, u->Position(), last );

    if ( b && UnitGoTo( u, b->Position(), 0 ) ) break;
    else last = b;
  } while ( b );
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::UnitStrength
// DESCRIPTION: Calculate the combat strength of a unit. Putting this
//              into a single number makes this approach slightly
//              inaccurate, but easier to handle. This value is only
//              used internally by the computer player.
// PARAMETERS : u - unit
// RETURNS    : combat strength
////////////////////////////////////////////////////////////////////////

unsigned short AI::UnitStrength( Unit *u ) const {
  const UnitType *type = u->Type();
  return (MAX( MAX( type->Firepower(U_GROUND), type->Firepower(U_SHIP) ),
         type->Firepower(U_AIR) ) + type->Armour() + 3 * u->XPLevel())
         * u->GroupSize() / MAX_GROUP_SIZE;
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::UnitPresence
// DESCRIPTION: Calculate the combined firepower of all units in a given
//              area which are controlled by the given player.
// PARAMETERS : owner  - player controlling wanted units
//              obj    - objective; the objective position defines the
//                       center of the area to be scanned, and the
//                       needed_xxx values will be filled with our
//                       findings
//              radius - area radius; if this is -1, all enemy units
//                       will be considered according to their
//                       distance to the objective (default -1)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::UnitPresence( Player *owner, AIObj *obj, short radius /* = -1 */ ) const {
  for ( Unit *u = static_cast<Unit *>(mission.GetUnits().Head());
        u; u = static_cast<Unit *>(u->Next()) ) {
    if ( (u->Owner() == owner) && ((radius == -1) ||
       (Distance( obj->pos, u->Position() ) <= radius)) ) {
      unsigned short str = MAX( 0, UnitStrength(u) - Distance( obj->pos, u->Position() ) * 2 );
      if ( u->IsAircraft() ) obj->needed_air += str;
      else if ( u->IsGround() ) obj->needed_ground += str;
      else obj->needed_ship += str;
    }
  }
  obj->requested_ground = obj->needed_ground > 0;
  obj->requested_air = obj->needed_air > 0;
  obj->requested_ship = obj->needed_ship > 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::ClosestUnit
// DESCRIPTION: Find the unit with a given set of abilities which is
//              closest to the target location.
// PARAMETERS : owner   - player who controls the unit
//              p       - target location
//              uflags  - flags which should be looked for
//              nuflags - flags which must not be set
//              last    - last unit found. If this is given, find the
//                        next unit which is as far or further away
//                        than that one and matches the criteria. If
//                        it is NULL (default) return the closest unit.
// RETURNS    : closest unit which matches any ONE of the uflags and
//              NONE of the nuflags; or NULL if no appropriate unit found
////////////////////////////////////////////////////////////////////////

Unit *AI::ClosestUnit( Player *owner, const Point &p, unsigned long uflags,
                       unsigned long nuflags, const Unit *last /* = NULL */ ) const {
  Unit *u, *best = NULL;
  int last_dist, best_dist = 9999;

  if ( last ) {
    last_dist = Distance( p, last->Position() );
    u = static_cast<Unit *>( last->Next() );
    if ( !u ) {
      u = static_cast<Unit *>( mission.GetUnits().Head() );
      ++last_dist;
    }
  } else {
    u = static_cast<Unit *>( mission.GetUnits().Head() );
    last_dist = -1;
  }

  while ( u != last ) {
    int dist = Distance( u->Position(), p );

    if ( (u->Flags() & uflags) && !(u->Flags() & nuflags) &&
         (u->Owner() == owner) &&
         (dist >= last_dist) && (dist < best_dist) ) {
      best_dist = dist;
      best = u;
    }
    u = static_cast<Unit *>( u->Next() );
    if ( !u && last ) {
      u = static_cast<Unit *>( mission.GetUnits().Head() );
      ++last_dist;      // increase distance for the next loop
    }
  }

  return best;
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::ClosestBuilding
// DESCRIPTION: Find the building closest to a given hex.
// PARAMETERS : owner - owning player
//              p     - target location
//              last  - last building found. If this is given, find
//                      the next building which is as far or further
//                      away than that one. If it is NULL (default)
//                      return the closest building.
// RETURNS    : closest building owned by the given player, or NULL if
//              no appropriate building found
////////////////////////////////////////////////////////////////////////

Building *AI::ClosestBuilding( Player *owner, const Point &p,
                               const Building *last /* = NULL */ ) const {
  Building *b, *best = NULL;
  int last_dist, best_dist = 9999;

  if ( last ) {
    last_dist = Distance( p, last->Position() );
    b = static_cast<Building *>( last->Next() );
    if ( !b ) {
      b = static_cast<Building *>( mission.GetShops().Head() );
      ++last_dist;
    }
  } else {
    b = static_cast<Building *>( mission.GetShops().Head() );
    last_dist = -1;
  }

  while ( b != last ) {
    int dist = Distance( b->Position(), p );

    if ( (b->Owner() == owner) && (dist >= last_dist) && (dist < best_dist) ) {
      best_dist = dist;
      best = b;
    }
    b = static_cast<Building *>( b->Next() );
    if ( !b && last ) {
      b = static_cast<Building *>( mission.GetShops().Head() );
      ++last_dist;      // increase distance for the next loop
    }
  }

  return best;
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::UnitCanReach
// DESCRIPTION: Check whether a unit can get close to a given hex,
//              either by itself or by using an available transport.
// PARAMETERS : u    - unit
//              pos  - location to be checked for
//              dist - sometimes it's not necessary to get to the
//                     specified coordinates. Here you can specify the
//                     tolerable distance if you only want to get close.
// RETURNS    : TRUE if the unit can get close enough to the destination
//              FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool AI::UnitCanReach( const Unit *u, const Point &pos, unsigned short dist ) {
  Path path(map);

  return (path.Find( u, u->Position(), pos, PATH_FAST, dist ) != -1) ||
         FindTransport( u, pos, dist, false );
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::FollowPath
// DESCRIPTION: If a path has been found, determine how far the unit
//              can get this turn.
// PARAMETERS : u    - unit
//              path - path
//              dist - stop when dist steps left to go (defaults to 0)
// RETURNS    : furthest hex along the path which can be reached this
//              turn
////////////////////////////////////////////////////////////////////////

Point AI::FollowPath( const Unit *u, const Path &path, unsigned short dist ) const {
  Point p = u->Position(), next;
  unsigned short steps = path.StepsToDest( p );
  bool leave;
  short dir;

  MoveShader shader( map, mission.GetUnits() );
  shader.ShadeMap( u );

  do {
    leave = true;
    if ( steps > dist ) {
      dir = path.GetStep( p );
      if ( dir != -1 ) {
        map->Dir2Hex( p, (Direction)dir, next );
        if ( shader.GetStep( next ) != -1 ) {
          Unit *block = map->GetUnit( next );
          if ( !block || ((block->Owner() == u->Owner()) &&
               block->IsTransport() && static_cast<Transport *>(block)->Allow(u)) ) {
            p = next;
            --steps;
            leave = false;
          }
        }
      }
    }
  } while ( !leave );

  return p;
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::FindBestTarget
// DESCRIPTION: Decide which enemy unit to attack, if any.
// PARAMETERS : u - unit to check targets for
// RETURNS    : enemy unit considered the best target, or NULL if none
////////////////////////////////////////////////////////////////////////

Unit *AI::FindBestTarget( const Unit *u ) {
  Unit *best = NULL;
  unsigned short bestval = 0;

  if ( u->IsDefensive() ) return NULL;

  Path path( map );
  for ( Unit *tg = static_cast<Unit *>( mission.GetUnits().Head() );
        tg; tg = static_cast<Unit *>( tg->Next() ) ) {
    if ( (tg->Owner() == &mission.GetOtherPlayer(*player)) && u->CanHitType( tg )
         && !tg->IsSheltered() ) {

      if ( UnitCanReach( u, tg->Position(), u->WeaponRange(tg) ) ) {
        unsigned short val = 0;
        short cost = path.Find( u, u->Position(), tg->Position(),
                                PATH_FAST, u->WeaponRange( tg ) );

        if ( cost >= 0 ) {
          val = MAX( 0, 10000 - UnitStrength( tg ) - cost * 50 );
          if ( !tg->CanHitType( u ) ) val += 10;

          if ( tg->IsConquer() ) val += 20;
          else if ( tg->IsTransport() ) val += 15;
          else if ( tg->IsMine() ) val = MAX( 0, val - 40 );

          if ( (u->IsSlow() || (u->Type()->Speed() == 0)) && u->CanHit( tg ) ) val += 80;
        } else {
          // not yet directly accessible - make that a relatively unlikely target
          val = MAX( 0, 5000 - UnitStrength( tg ) );
        }

        if ( val > bestval ) {
          bestval = val;
          best = tg;
        }
      }
    }
  }
  return best;
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::FindBestHex
// DESCRIPTION: Determine the hex from which an attack can be expected
//              to bear the best result.
// PARAMETERS : u     - attacking unit
//              enemy - target unit
// RETURNS    : best attacking position or current location if target
//              cannot be reached this turn (however, the current
//              location may just as well be the best ambush point
//              indeed, so check the result)
////////////////////////////////////////////////////////////////////////

#define _CF_BEST_HEX_INVALID	-1000
Point AI::FindBestHex( const Unit *u, const Unit *enemy ) const {
  Point hex = u->Position();
  Path path(map);
  short turns = path.Find( u, u->Position(), enemy->Position(),
                           PATH_BEST, u->WeaponRange( enemy ) );

  // if we can't reach the target this turn get as close as possible;
  // for long-range attacks, terrain type etc. are not important
  if ( (turns > 1) || ((u->WeaponRange(enemy) > 1) && (turns >= 0)) )
    hex = FollowPath( u, path );
  else if ( turns >= 0 ) {
    Unit *sup;
    Point nb[6], hlp;
    short vals[6], bestval = _CF_BEST_HEX_INVALID;
    int i;

    map->GetNeighbors( enemy->Position(), nb );
    for ( i = NORTH; i <= NORTHWEST; ++i ) {
      if ( (nb[i].x != -1) && (nb[i].y != -1) && !map->GetUnit( nb[i] ) &&
           ((map->TerrainTypes( nb[i] ) & TT_ENTRANCE) == 0) &&
           (path.Find( u, u->Position(), nb[i], PATH_FAST ) == 1) ) {
        vals[i] = -Distance( u->Position(), nb[i] );
      } else vals[i] = _CF_BEST_HEX_INVALID;
    }

    for ( i = NORTH; i <= NORTHWEST; ++i ) {
      if ( vals[i] != _CF_BEST_HEX_INVALID ) {
        vals[i] += map->AttackMod( nb[i] );

        // check for support in the back of the enemy
        int j = ReverseDir( (Direction)i );
        if ( (nb[j].x != -1) && (nb[j].y != -1) ) {
          sup = map->GetUnit( nb[j] );
          if ( sup && (sup->Owner() == player) ) {
            if ( sup->CouldHit( enemy ) ) vals[i] += 7;
            else vals[i] += 2;
          }
        }

        // check for enemy units supporting defence
        Direction attdir = Hex2Dir( nb[i], enemy->Position() );
        if ( !map->Dir2Hex( nb[i], TurnLeft(attdir), hlp ) &&
           (sup = map->GetUnit( hlp )) && (sup->Owner() == player) ) vals[i] -= 8;
        if ( !map->Dir2Hex( nb[i], TurnRight(attdir), hlp ) &&
           (sup = map->GetUnit( hlp )) && (sup->Owner() == player) ) vals[i] -= 8;
      }
    }

    for ( i = NORTH; i <= NORTHWEST; ++i ) {
      if ( vals[i] > bestval ) {
        bestval = vals[i];
        hex = nb[i];
      }
    }
  }

  return hex;
}
#undef _CF_BEST_HEX_INVALID

////////////////////////////////////////////////////////////////////////
// NAME       : AI::FindTransport
// DESCRIPTION: Find out whether a unit can reach its destination
//              through the service of a kind transporter.
// PARAMETERS : u       - unit to be carried
//              dest    - destination hex
//              dist    - maximum acceptable distance to destination
//              forreal - should we 'book' the transport if we find one
//                        or is this just a test
// RETURNS    : a transport which can do the job, or NULL if none is
//              available
////////////////////////////////////////////////////////////////////////

Transport *AI::FindTransport( const Unit *u, const Point &dest,
                              unsigned short dist, bool forreal ) {
  Transport *t, *best = NULL;
  AIObj *obj;
  unsigned short val, bestval = 0;
  bool booked = false;   // make sure we don't overwrite existing transport targets

  // first of all, search the objectives for a transport which already goes
  // where we want to be
  for ( obj = static_cast<AIObj *>(objectives.Head());
        obj; obj = static_cast<AIObj *>(obj->Next()) ) {
    if ( obj->type == AI_OBJ_TRANSPORT ) {
      t = static_cast<Transport *>(static_cast<AIObj::AIAllocNode *>(obj->alloc_units.Head())->unit);
      if ( (t != u) && t->Allow( u ) ) {
        TransPath tp( map, t );
        if ( tp.Find( u, t->Position(), u->Position() ) != -1 ) {
          Path p( map );
          if ( p.Find( u, obj->pos, dest, PATH_FAST, dist ) != -1 ) {
            val = 500 - Distance( u->Position(), t->Position() );
            if ( val > bestval ) {
              bestval = val;
              best = t;
              booked = true;
            }
          }
        }
      }
    }
  }

  Unit *last = NULL;
  while ( (t = static_cast<Transport *>(
          ClosestUnit( player, u->Position(), U_TRANSPORT, U_DONE, last ))) ) {
    last = t;

    if ( (t != u) && t->Allow( u ) ) {
      if ( t->IsBusy() ) {
        AIObj *to = GetObjectiveForUnit( t );
        // AI_OBJ_TRANSPORTs were checked above
        if ( to->type == AI_OBJ_TRANSPORT ) continue;
      }

      TransPath tp2( map, t );
      if ( (tp2.Find( u, t->Position(), u->Position(), PATH_FAST, dist ) != -1) &&
           (tp2.Find( u, t->Position(), dest, PATH_FAST, dist ) != -1) ) {
        // both hexes accessible -> ok
        // prefer these transports over those found above as this way we
        // might get a private one all for ourselves
        val = 520 - Distance( u->Position(), t->Position() );
        if ( val > bestval ) {
          bestval = val;
          best = t;
          booked = false;
        }
      }
    }
  }

  if ( best && !booked && forreal ) {
    if ( best->IsBusy() ) GetObjectiveForUnit( best )->ReleaseUnit( best );

    obj = new AIObj;
    obj->pos = dest;
    obj->priority = AI_PRI_TRANSPORT;
    obj->type = AI_OBJ_TRANSPORT;
    obj->flags = dist;		// I know, I know, dirty hack...
    AddObjective( obj );
    obj->AssignUnit( best, 0 );
  }
  return best;
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::UnitGoTo
// DESCRIPTION: Try to move a unit from its current location towards a
//              destination hex. This may include using a transport.
// PARAMETERS : u    - unit
//              dest - destination hex
//              dist - maximum acceptable distance to destination
// RETURNS    : TRUE if unit was moved, FALSE if no valid path could be
//              found
////////////////////////////////////////////////////////////////////////

bool AI::UnitGoTo( Unit *u, const Point &dest, unsigned short dist ) {
  bool rc = true;
  Path p( map );
  Point end;

  if ( p.Find( u, u->Position(), dest, PATH_BEST, dist ) != -1 ) {
    end = FollowPath( u, p );
    Gam->MoveUnit( u, end );
  } else {
    // try to find a way using a transporter
    short turns;
    Transport *t = FindTransport( u, dest, dist, true );
    if ( t ) {
      turns = p.Find( u, u->Position(), t->Position() );
      if ( (turns != 1) && (t->UnitCount() == 0) ) {
        // get the transport closer first (only if it's empty)
        TransPath tp( map, t );
        if ( tp.Find( u, t->Position(), u->Position() ) != -1 ) {
          Gam->SelectUnit( t );
          end = FollowPath( t, tp, 1 );
          Gam->MoveUnit( t, end );
          Gam->SelectUnit( u );

          turns = p.Find( u, u->Position(), end );
          if ( turns != -1 ) {
            end = FollowPath( u, p );
            Gam->MoveUnit( u, end );
          } else {
            tp.Reverse();
            end = FollowPath( u, tp );
            Gam->MoveUnit( u, end );
          }
        }
      } else if ( turns >= 0 ) {
        end = FollowPath( u, p );
        Gam->MoveUnit( u, end );
      }
    } else rc = false;
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::SameDirection
// DESCRIPTION: Check whether two target hexes are roughly lying in the
//              same direction.
// PARAMETERS : pos   - current position to check from
//              dest1 - first destination hex
//              dest2 - second destination hex
// RETURNS    : TRUE if dest1 and dest2 are exactly the same direction
//              or differ by one tick only, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool AI::SameDirection( const Point &pos, const Point &dest1, const Point &dest2 ) const {
  Direction dd1 = Hex2Dir( pos, dest1 );
  Direction dd2 = Hex2Dir( pos, dest2 );
  return ( (dd1 == dd2) || ((dd1 + 1) % 6 == dd2) || (dd1 == (dd2 + 1) % 6) );
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::AIObj::AssignUnit
// DESCRIPTION: Assign a unit to the objective.
// PARAMETERS : unit - unit
//              str  - unit strength value (see AI::UnitStrength())
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::AIObj::AssignUnit( Unit *unit, unsigned short str ) {
  AIAllocNode *n = new AIAllocNode( unit );

  if ( pos == Point(-1,-1) ) {
    if ( unit->IsTransport() ) alloc_units.AddTail( n );
    else alloc_units.AddHead( n );
  } else {
    // add unit to list according to its distance to the objective;
    // smaller distance => higher priority
    AIAllocNode *u = static_cast<AIAllocNode *>( alloc_units.Tail() );
    unsigned short dist = Distance( unit->Position(), pos );

    while ( u && (Distance(u->unit->Position(), pos) > dist) ) {
      u = static_cast<AIAllocNode *>( u->Prev() );
    }
    alloc_units.InsertNode( n, u );
  }

  unit->SetFlags( U_BUSY );
  const UnitType *type = unit->Type();
  unsigned char num = 0;
  if ( type->Firepower(U_AIR) > 0 ) ++num;
  if ( type->Firepower(U_GROUND) > 0 ) ++num;
  if ( type->Firepower(U_SHIP) > 0 ) ++num;

  if ( num > 0 ) {
    str /= num;
    if ( type->Firepower(U_AIR) > 0 ) needed_air = MAX( 0, needed_air - str );
    if ( type->Firepower(U_GROUND) > 0 ) needed_ground = MAX( 0, needed_ground - str );
    if ( type->Firepower(U_SHIP) > 0 ) needed_ship = MAX( 0, needed_ship - str );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::AIObj::ReleaseUnits
// DESCRIPTION: Discard the objective and release all units so they can
//              be assigned to other objectives.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::AIObj::ReleaseUnits( void ) {
  while ( !alloc_units.IsEmpty() ) {
    AIAllocNode *n = static_cast<AIAllocNode *>( alloc_units.RemHead() );
    n->unit->UnsetFlags( U_BUSY );
    delete n;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::AIObj::ReleaseUnit
// DESCRIPTION: Release a single unit from the objective.
// PARAMETERS : u - unit to release
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void AI::AIObj::ReleaseUnit( Unit *u ) const {
  for ( AIAllocNode *n = static_cast<AIAllocNode *>( alloc_units.Head() );
        n; n = static_cast<AIAllocNode *>( n->Next() ) ) {
    if ( n->unit == u ) {
      n->Remove();
      delete n;
      u->UnsetFlags( U_BUSY );
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : AI::AIObj::UnitAssigned
// DESCRIPTION: Check whether a specific unit is assigned to this
//              objective.
// PARAMETERS : u - unit to check for
// RETURNS    : TRUE if unit is assigned to the objective, FALSE
//              otherwise
////////////////////////////////////////////////////////////////////////

bool AI::AIObj::UnitAssigned( const Unit *u ) const {
  for ( AIAllocNode *n = static_cast<AIAllocNode *>( alloc_units.Head() );
        n; n = static_cast<AIAllocNode *>( n->Next() ) ) {
    if ( n->unit == u ) return true;
  }
  return false;
}

