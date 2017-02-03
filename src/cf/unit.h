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
// unit.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_UNIT_H
#define _INCLUDE_UNIT_H

#include "list.h"
#include "player.h"
#include "sound.h"
#include "misc.h"
#include "lset.h"

#define MO_UNIT      1
#define MO_BUILDING  2

class MapObject {
public:
  MapObject( unsigned short type ) : mo_type(type) {}
  virtual ~MapObject( void ) {}
  virtual const char *Name( void ) const = 0;
  virtual Player *Owner( void ) const = 0;
  virtual const Point &Position( void ) const = 0;

  bool IsUnit( void ) const { return mo_type == MO_UNIT; }
  bool IsShop( void ) const { return mo_type == MO_BUILDING; }

private:
  unsigned short mo_type;
};

#define CRYSTALS_REPAIR		5

class Unit : public Node, public MapObject {
public:
  Unit( void ) : MapObject(MO_UNIT) {}
  Unit( const UnitType *type, Player *player, unsigned short id, const Point &pos );

  virtual int Load( MemBuffer &file, const UnitType *type, Player *player );
  virtual int Save( MemBuffer &file ) const;

  unsigned short BaseImage( void ) const { return u_type->Image() + u_player->ID() * 6; }
  unsigned short BuildCost( void ) const { return u_type->Cost(); }
  unsigned long Flags( void ) const { return u_flags; }
  unsigned char GroupSize( void ) const { return u_group; }
  unsigned short ID( void ) const { return u_id; }
  unsigned char Facing( void ) const { return u_facing; }
  unsigned short Image( void ) const { return BaseImage() + Facing(); }
  unsigned char Moves( void ) const;
  const char *Name( void ) const { return u_type->Name(); }
  Player *Owner( void ) const { return u_player; }
  const Point &Position( void ) const { return u_pos; }
  const Point *Target( void ) const { return( (u_flags & U_ATTACKED) ? &u_target : NULL ); }
  unsigned short Terrain( void ) const { return u_type->Terrain(); }
  const UnitType *Type( void ) const { return u_type; }
  unsigned char WeaponRange( const Unit *u ) const;
  unsigned char XP( void ) const { return u_xp; }
  unsigned char XPLevel( void ) const { return XP()/XP_PER_LEVEL; }

  void AwardXP( unsigned char xp );
  void SetOwner( Player *player );
  virtual void SetPosition( short x, short y );
  void SetGroupSize( unsigned char size ) { u_group = size; }

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
  bool IsConquer( void ) const { return (u_flags & U_CONQUER) != 0; }
  bool IsSheltered( void ) const { return (u_flags & U_SHELTERED) != 0; }
  bool IsReady( void ) const { return (u_flags & (U_DESTROYED|U_DONE)) == 0; }
  bool IsAlive( void ) const { return (u_flags & U_DESTROYED) == 0; }
  bool IsBusy( void ) const { return (u_flags & U_BUSY) != 0; }
  bool IsDummy( void ) const { return (u_flags & U_DUMMY) != 0; }
  bool IsFloating( void ) const { return (u_flags & U_FLOATING) != 0; }
  bool IsDefensive( void ) const
    { return (u_type->Firepower(U_GROUND) + u_type->Firepower(U_SHIP) +
              u_type->Firepower(U_AIR)) == 0; }

  bool CanHit( const Unit *enemy ) const;
  bool CanHitType( const Unit *enemy ) const;
  bool CouldHit( const Unit *enemy ) const;
  void Attack( const Unit *enemy );
  virtual bool Hit( unsigned short damage );
  virtual unsigned short Weight( void ) const { return u_type->Weight(); }

  unsigned char OffensiveStrength( const Unit *target ) const;
  unsigned char DefensiveStrength( void ) const;

  void Repair( void );
  void Face( unsigned char dir ) { u_facing = dir; }

  SoundEffect *MoveSound( void ) { return u_type->MoveSound(); }
  SoundEffect *FireSound( void ) { return u_type->FireSound(); }

protected:
  Point u_pos;		// position on map
  unsigned long u_flags;
  unsigned short u_id;
  
  unsigned char u_facing;	// direction
  unsigned char u_group;	// group size
  unsigned char u_xp;		// experience

  Point u_target;

  const UnitType *u_type;
  Player *u_player;
};

#endif	/* _INCLUDE_UNIT_H */

