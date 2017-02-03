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
// mission.h - the data construct to contain all level info
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_ED_MISSION_H
#define _INCLUDE_ED_MISSION_H

#include <string>
#include <fstream>
#include <sstream>
using namespace std;

#include "map.h"

class Player {
public:
  Player( unsigned char id );

  int Load( MemBuffer &file );
  int Save( MemBuffer &file ) const;
  int Export( ofstream &file ) const;

  void SetID( unsigned char id ) { p_id = id; }
  void SetNameID( signed char name ) { p_name_id = name; }
  void SetBriefing( signed char briefing ) { p_briefing = briefing; }
  void SetLightColor( const Color &col ) { p_col_light = col; }
  void SetDarkColor( const Color &col ) { p_col_dark = col; }

  signed char NameID( void ) const { return p_name_id; }
  signed char Briefing( void ) const { return p_briefing; }
  Color LightColor( void ) const { return p_col_light; }
  Color DarkColor( void ) const { return p_col_dark; }

private:
  unsigned char p_id;
  unsigned char p_type;
  unsigned char p_success;
  signed char p_briefing;
  signed char p_name_id;
  string p_password;

  Color p_col_light;
  Color p_col_dark;
};

class Event : public Node {
public:
  Event( void ) {}
  Event( unsigned char id, unsigned char type, unsigned char trigger );

  int Load( MemBuffer &file );
  int Save( MemBuffer &file ) const;
  int Export( ofstream &file, const Map &map ) const;

  unsigned char ID( void ) const { return e_id; }
  void SetID( unsigned char id ) { e_id = id; }
  const char *Name( void ) const;
  unsigned char Trigger( void ) const { return e_trigger; }
  void SetTrigger( unsigned char trig ) { e_trigger = trig; }
  const char *TriggerName( void ) const;
  unsigned char Type( void ) const { return e_type; }
  void SetType( unsigned char type ) { e_type = type; }

  unsigned char Player( void ) const { return e_player; }
  void SetPlayer( unsigned char player ) { e_player = player; }
  short Title( void ) const { return e_title; }
  void SetTitle( short title ) { e_title = title; }
  short Message( void ) const { return e_message; }
  void SetMessage( short msg ) { e_message = msg; }
  unsigned short Flags( void ) const { return e_flags; }
  void SetFlags( unsigned short f ) { e_flags = f; }
  void ToggleFlags( unsigned short flags )
       { e_flags ^= flags; }
  signed char Dependency( void ) const { return e_depend; }
  void SetDependency( signed char dep ) { e_depend = dep; }
  signed char Discard( void ) const { return e_discard; }
  void SetDiscard( signed char ev ) { e_discard = ev; }

  short GetData( unsigned short index ) const { return e_data[index]; }
  void SetData( unsigned short index, short value )
       { e_data[index] = value; }
  short GetTData( unsigned short index ) const { return e_tdata[index]; }
  void SetTData( unsigned short index, short value )
       { e_tdata[index] = value; }

  void SetTmpBuf( const string &s ) { e_tmpbuf = s; }
  const string &GetTmpBuf( void ) const { return e_tmpbuf; }

private:
  unsigned char e_id;
  unsigned char e_type;
  unsigned char e_trigger;
  signed char e_depend;
  signed char e_discard;
  short e_tdata[3];
  short e_data[3];
  short e_title;
  short e_message;
  unsigned short e_flags;
  unsigned char e_player;
  string e_tmpbuf;
};

class Mission {
public:
  Mission( void ) : flags(GI_SKIRMISH), p1(PLAYER_ONE), p2(PLAYER_TWO),
                    unit_set(0), terrain_set(0) {}
  Mission( const Point &size, TerrainSet *ts, UnitSet *us );
  ~Mission( void );

  Map &GetMap( void ) { return map; }
  const string &GetTitle( void ) const { return last_file_name; }
  void SetTitle( const string &title ) { last_file_name = title; }

  const char *GetSequel( void ) const
    { return next_map.empty() ? 0 : next_map.c_str(); }
  void SetSequel( const char *map )
    { map ? next_map = map : next_map.erase(); }

  const char *GetMusic( void ) const
    { return music.empty() ? 0 : music.c_str(); }
  void SetMusic( const char *track )
    { track ? music = track : music.erase(); }

  TerrainSet &GetTerrainSet( void ) const { return *terrain_set; }
  const UnitSet &GetUnitSet( void ) const { return *unit_set; }
  signed char GetLevelInfoMsg( void ) const { return level_info; }
  void SetLevelInfoMsg( signed char msg ) { level_info = msg; }
  signed char GetCampaignInfo( void ) const { return campaign_info; }
  void SetCampaignInfo( signed char msg ) { campaign_info = msg; }
  signed char GetName( void ) const { return name; }
  void SetName( signed char msg ) { name = msg; }
  signed char GetCampaignName( void ) const { return campaign_name; }
  void SetCampaignName( signed char msg ) { campaign_name = msg; }
  unsigned char GetNumPlayers( void ) const
                { return ((flags & GI_AI) == 0) ? 2 : 1; }
  void SetNumPlayers( unsigned char num );

  Player &GetPlayer( unsigned char id )
         { return (id == PLAYER_ONE) ? p1 : p2; }

  int Load( const char *filename );
  int Save( const char *filename );
  int Export( const string &filename ) const;
  unsigned short Validate( stringstream &errors );

  Unit *CreateUnit( const UnitType *type, unsigned char pid, const Point &pos );
  void DeleteUnit( Unit *u );
  Unit *GetUnitByID( unsigned short id ) const;

  Building *CreateBuilding( unsigned char pid, const Point &pos );
  void DeleteBuilding( Building *b );
  Building *GetBuildingByID( unsigned short id ) const;

  Event *CreateEvent( unsigned char type, unsigned char trigger );
  void DeleteEvent( Event *e );
  Event *GetEventByID( unsigned short id ) const;

  short StorageLeft( Unit &u ) const;

  List &GetEvents( void ) { return events; }
  List &GetUnits( void ) { return units; }
  List &GetBuildings( void ) { return buildings; }
  Locale &GetMessages( void ) { return messages; }
  const char *GetMessage( short id ) const { return messages.GetMsg(id); }

  bool IsCampaign( void ) const { return (flags & GI_CAMPAIGN) != 0; }
  void SetCampaign( bool c )
    { c ? flags |= GI_CAMPAIGN : flags &= ~(GI_CAMPAIGN); }
  bool IsSkirmish( void ) const { return (flags & GI_SKIRMISH) != 0; }
  void SetSkirmish( bool s )
    { s ? flags |= GI_SKIRMISH : flags &= ~(GI_SKIRMISH); }

protected:
  unsigned short GetUnitID( void ) const;
  unsigned short GetBuildingID( void ) const;
  unsigned char GetEventID( void ) const;

  unsigned short ValidateEvent( Event &e, stringstream &errors ) const;
  unsigned short ValidateShop( Building &b, stringstream &errors ) const;
  unsigned short ValidateMap( stringstream &errors ) const;

  unsigned short flags;
  signed char name;
  signed char level_info;
  signed char campaign_name;
  signed char campaign_info;
  string next_map;
  string music;

  Map map;
  List units;
  List buildings;
  List events;
  Player p1;
  Player p2;

  UnitSet *unit_set;
  TerrainSet *terrain_set;
  Locale messages;

  string last_file_name;
};

#endif  // _INCLUDE_ED_MISSION_H

