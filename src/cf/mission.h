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
// mission.h - the data construct to contain all level info
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_MISSION_H
#define _INCLUDE_MISSION_H

#include "map.h"
#include "event.h"
#include "lset.h"
#include "player.h"
#include "history.h"
#include "lang.h"

class Mission {
public:
  Mission( void ) { history = 0; }
  ~Mission( void );

  int Load( MemBuffer &file );
  Unit *LoadUnit( MemBuffer &file, bool dummy = false );
  int QuickLoad( MemBuffer &file );
  int Save( MemBuffer &file );

  Map &GetMap( void ) { return map; }
  TerrainSet &GetTerrainSet( void ) { return terrain_set; }
  UnitSet &GetUnitSet( void ) { return unit_set; }

  const char *GetSequel( void ) const { return GetInternalMessage(next_map); }
  const char *GetInfoMsg( void ) const { return GetMessage(level_info); }
  const char *GetCampaignInfo( void ) const { return GetMessage(campaign_info); }
  const char *GetName( void ) const { return GetMessage(name); }
  const char *GetCampaignName( void ) const { return GetMessage(campaign_name); }
  const char *GetMusic( void ) const { return GetMessage(music); }

  unsigned short GetTurn( void ) const { return turn; }
  unsigned short NextTurn( void ) { return ++turn; }
  unsigned char GetPhase( void ) const { return turn_phase; }
  unsigned short GetTime( void ) const { return (GetTurn() - 1) * 2 + current_player; }
  unsigned char GetHandicap( void ) const { return handicap; }
  unsigned short GetFlags( void ) const { return flags; }

  Unit *CreateUnit( unsigned char type, Player &p, const Point &pos,
	Direction dir = NORTH, unsigned char group = MAX_GROUP_SIZE, unsigned char xp = 0 );
  Unit *GetUnit( unsigned short id ) const;
  Building *GetShop( unsigned short id ) const;
  Event *GetEvent( unsigned short id ) const;

  const char *GetMessage( short id ) const;

  Player &GetPlayer( void ) { return GetPlayer(current_player); }
  Player &GetPlayer( unsigned char id )
         { return id == PLAYER_ONE ? p1 : p2; }
  Player &GetOtherPlayer( const Player &p )
         { return p.ID() == PLAYER_ONE ? p2 : p1; }
  Player &NextPlayer( void ) { current_player ^= 1; return GetPlayer(); }

  void RegisterBattle( Unit *att, Unit *def );
  History *GetHistory( void ) const { return history; }
  void SetHistory( History *h ) { history = h; }

  List &GetEvents( void ) { return events; }
  List &GetUnits( void ) { return units; }
  List &GetShops( void ) { return shops; }
  List &GetBattles( void ) { return battles; }

  void SetFlags( unsigned short f ) { flags = f; }
  void SetLocale( const string &lang );
  void SetSequel( signed char seqid ) { next_map = seqid; }
  void SetPhase( unsigned char phase ) { turn_phase = phase; }
  void SetHandicap( unsigned char hcap ) { handicap = hcap; }

private:
  const char *GetInternalMessage( short id ) const;
  unsigned short CreateUnitID( void ) const;

  unsigned short turn;
  unsigned char turn_phase;
  unsigned char current_player;
  unsigned char handicap;

  unsigned short flags;
  signed char name;
  signed char level_info;
  signed char campaign_name;
  signed char campaign_info;
  signed char next_map;
  signed char music;

  Map map;
  List units;
  List shops;
  List events;
  List battles;
  Locale messages;
  Language internal_messages; // non-translatable strings
  Player p1;
  Player p2;

  UnitSet unit_set;
  TerrainSet terrain_set;
  History *history;

  template <typename T>  // get a specific object from a list
  T *FindID( const List &l, unsigned short id ) const {
    for ( T *obj = static_cast<T *>(l.Head());
          obj; obj = static_cast<T *>(obj->Next()) ) {
      if ( obj->ID() == id ) return obj;
    }
    return 0;
  }
};

#endif  /* _INCLUDE_MISSION_H */

