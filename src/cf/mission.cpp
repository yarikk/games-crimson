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
// mission.cpp
////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "mission.h"
#include "fileio.h"
#include "strutil.h"
#include "globals.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::~Mission
// DESCRIPTION: Free mission resources.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Mission::~Mission( void ) {
  delete history;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::Load
// DESCRIPTION: Load a game from a mission file.
// PARAMETERS : file - mission file
// RETURNS    : 0 on success, non-null otherwise
////////////////////////////////////////////////////////////////////////

int Mission::Load( MemBuffer &file ) {
  int rc = -1;

  // read game info
  if ( (file.Read32() == FID_MISSION) && (file.Read8() == FILE_VERSION) ) {
    unsigned short len, i;
    string uset, tset;

    flags = file.Read16();
    turn = file.Read16();

    name = file.Read8();
    level_info = file.Read8();
    campaign_name = file.Read8();
    campaign_info = file.Read8();
    next_map = file.Read8();
    music = file.Read8();
    handicap = file.Read8();
    current_player = file.Read8();
    turn_phase = file.Read8();

    p1.Load( file );
    p2.Load( file );

    messages.Load( file );

    map.Load( file );               // load map

    len = file.Read16();            // load name of unit set
    uset = file.ReadS( len );

    File ufile( uset + ".units" );
    if ( !ufile.OpenData("rb") || unit_set.Load( ufile, uset.c_str() ) ) {
      cerr << "Error: Unit set '" << uset << "' not available" << endl;
      return -1;
    }
    ufile.Close();

    len = file.Read16();            // load name of terrain set
    tset = file.ReadS( len );

    File tfile( tset + ".tiles" );
    if ( !tfile.OpenData("rb") || terrain_set.Load( tfile, tset.c_str() ) ) {
      cerr << "Error: Terrain set '" << tset << "' not available" << endl;
      return -1;
    }
    tfile.Close();

    map.SetUnitSet( &unit_set );
    map.SetTerrainSet( &terrain_set );

    len = file.Read16();         // load shops
    for ( i = 0; i < len; ++i ) {
      Building *b = new Building();
      short pid = b->Load( file );
      b->SetOwner( pid == PLAYER_NONE ? 0 : &GetPlayer( pid ), false );
      shops.AddTail( b );
      map.SetBuilding( b, b->Position() );
    }

    len = file.Read16();         // load units
    for ( i = 0; i < len; ++i ) {
      Unit *u = LoadUnit( file );
      if ( u ) {
        units.AddTail( u );
        map.SetUnit( u, u->Position() );
      }
    }

    len = file.Read16();             // load battles
    for ( i = 0; i < len; ++i ) {    // only present in saved games
      Combat *combat = new Combat();
      combat->Load( file, *this );
      battles.AddTail( combat );
    }

    len = file.Read16();            // load events
    for ( i = 0; i < len; ++i ) {
      Event *e = new Event();
      short pid = e->Load( file );
      e->SetPlayer( GetPlayer( pid ) );
      events.AddTail( e );
    }

    internal_messages.ReadCatalog( file );

    history = new History();
    if ( history->Load( file, *this ) == 0 ) {
      if ( GetPlayer(current_player^1).IsHuman() ) {
        history->StartRecording( GetUnits() );
      } else {
        delete history;
        history = 0;
      }
    }

    rc = 0;
  } else
    cerr << "Warning: invalid header or version" << endl;

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::LoadUnit
// DESCRIPTION: Load a unit from a file.
// PARAMETERS : file  - file descriptor
//              dummy - if true don't create transporters (used by History)
// RETURNS    : pointer to loaded unit or NULL on error
////////////////////////////////////////////////////////////////////////

Unit *Mission::LoadUnit( MemBuffer &file, bool dummy /* = false */ ) {
  unsigned char tid, pid;
  const UnitType *type;
  Player *p = 0;
  Unit *u;

  tid = file.Read8();
  pid = file.Read8();

  if ( pid != PLAYER_NONE ) p = &GetPlayer( pid );
  type = unit_set.GetUnitInfo( tid );

  if ( (type->Flags() & U_TRANSPORT) && !dummy ) u = new Transport();
  else u = new Unit();

  u->Load( file, type, p );
  return u;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::QuickLoad
// DESCRIPTION: Load parts of a game from a mission file. This retrieves
//              only the information required for the map selection
//              lists.
// PARAMETERS : file - mission file
// RETURNS    : 0 on success, non-null otherwise
////////////////////////////////////////////////////////////////////////

int Mission::QuickLoad( MemBuffer &file ) {
  int rc = -1;

  // read game info
  if ( (file.Read32() == FID_MISSION) && (file.Read8() == FILE_VERSION) ) {
    flags = file.Read16();
    turn = file.Read16();

    name = file.Read8();
    level_info = file.Read8();
    campaign_name = file.Read8();
    campaign_info = file.Read8();
    next_map = file.Read8();
    music = file.Read8();
    handicap = file.Read8();
    current_player = file.Read8();
    turn_phase = file.Read8();

    p1.Load( file );
    p2.Load( file );

    messages.Load( file );

    rc = 0;
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::Save
// DESCRIPTION: Save the mission to a file.
// PARAMETERS : file - data file
// RETURNS    : 0 on successful write, non-zero otherwise
////////////////////////////////////////////////////////////////////////

int Mission::Save( MemBuffer &file ) {
  // save game info
  file.Write32( FID_MISSION );
  file.Write8( FILE_VERSION );

  file.Write16( flags );
  file.Write16( turn );
  file.Write8( name );
  file.Write8( level_info );
  file.Write8( campaign_name );
  file.Write8( campaign_info );
  file.Write8( next_map );
  file.Write8( music );
  file.Write8( handicap );
  file.Write8( current_player );
  file.Write8( turn_phase );

  p1.Save( file );      // save player data
  p2.Save( file );

  messages.Save( file );

  map.Save( file );     // save map

  // save mission set info
  file.Write16( unit_set.GetName().length() );
  file.WriteS( unit_set.GetName() );
  file.Write16( terrain_set.GetName().length() );
  file.WriteS( terrain_set.GetName() );

  // save shops
  file.Write16( shops.CountNodes() );
  for ( Building *b = static_cast<Building *>(shops.Head());
        b; b = static_cast<Building *>(b->Next()) ) b->Save( file );

  // save transports; basically, transports are not much different
  // from other units but we MUST make sure that transports having
  // other units on board are loaded before those units; we need to
  // make two passes through the list, first saving all unsheltered
  // units (which are possibly transports carrying other units), and
  // all sheltered units in the second run
  file.Write16( units.CountNodes() );
  const Unit *u;
  for ( u = static_cast<Unit *>(units.Head());
        u; u = static_cast<Unit *>(u->Next()) ) {  // make sure transports are
    if ( !u->IsSheltered() ) u->Save( file );      // stored before carried units
  }

  for ( u = static_cast<Unit *>(units.Head());
        u; u = static_cast<Unit *>(u->Next()) ) {
    if ( u->IsSheltered() ) u->Save( file );
  }

  // save combat data
  file.Write16( battles.CountNodes() );
  for ( Combat *com = static_cast<Combat *>(battles.Head());
        com; com = static_cast<Combat *>(com->Next()) )
    com->Save( file );

  // save events
  file.Write16( events.CountNodes() );
  for ( Event *ev = static_cast<Event *>(events.Head());
        ev; ev = static_cast<Event *>(ev->Next()) )
    ev->Save( file );

  internal_messages.WriteCatalog( file );

  if ( history ) history->Save( file );         // save turn history
  else file.Write16( 0 );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::CreateUnit
// DESCRIPTION: Create a new unit.
// PARAMETERS : type  - unit type identifier
//              p     - controlling player
//              pos   - position on map
//              dir   - direction the unit is facing (default is North)
//              group - group size (default is MAX_GROUP_SIZE)
//              xp    - initial experience level (default is 0)
// RETURNS    : created unit or NULL on error
////////////////////////////////////////////////////////////////////////

Unit *Mission::CreateUnit( unsigned char type, Player &p,
     const Point &pos, Direction dir, unsigned char group, unsigned char xp ) {
  Unit *u;
  const UnitType *utype = unit_set.GetUnitInfo( type );

  if ( utype->Flags() & U_TRANSPORT )
    u = new Transport( utype, &p, CreateUnitID(), pos );
  else
    u = new Unit( utype, &p, CreateUnitID(), pos );

  u->Face( dir );
  u->SetGroupSize( group );
  u->AwardXP( xp * XP_PER_LEVEL );
  units.AddTail( u );
  map.SetUnit( u, pos );

  if ( history ) history->RecordUnitEvent( *u, History::HIST_UEVENT_CREATE );

  u->SetFlags( U_DONE );     // can't move on the first turn

  return u;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::GetUnit
// DESCRIPTION: Find the unit corresponding to the given identifier.
// PARAMETERS : id - identifier of the unit to be searched for
// RETURNS    : pointer to the unit, or NULL if no unit with that ID
//              exists
////////////////////////////////////////////////////////////////////////

Unit *Mission::GetUnit( unsigned short id ) const {
  return FindID<Unit>( units, id );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::CreateUnitID
// DESCRIPTION: Get a unique identifier for a new unit.
// PARAMETERS : -
// RETURNS    : unique unit ID
////////////////////////////////////////////////////////////////////////

unsigned short Mission::CreateUnitID( void ) const {
  // find an unused unit ID; start from the back of the range to avoid
  // potential conflicts with IDs of destroyed units
  unsigned short id = 32000;

  while ( GetUnit( id ) != 0 )
    --id;

  return id;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::GetShop
// DESCRIPTION: Find the shop corresponding to the given identifier.
// PARAMETERS : id - identifier of the shop to be searched for
// RETURNS    : pointer to the shop, or NULL if no building with that ID
//              exists
////////////////////////////////////////////////////////////////////////

Building *Mission::GetShop( unsigned short id ) const {
  return FindID<Building>( shops, id );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::GetEvent
// DESCRIPTION: Find the event corresponding to the given identifier.
// PARAMETERS : id - identifier of the event to be searched for
// RETURNS    : pointer to the event, or NULL if no event with that ID
//              exists
////////////////////////////////////////////////////////////////////////

Event *Mission::GetEvent( unsigned short id ) const {
  return FindID<Event>( events, id );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::GetMessage
// DESCRIPTION: Get a message from the catalog.
// PARAMETERS : id - message identifier of the requested message
// RETURNS    : pointer to requested message if successful, 0 otherwise
////////////////////////////////////////////////////////////////////////

const char *Mission::GetMessage( short id ) const {
  return messages.GetMsg( id );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::GetInternalMessage
// DESCRIPTION: Get a message from the internal catalog which contains
//              untranslatable strings like next map name, or soundtrack
//              names.
// PARAMETERS : id - message identifier of the requested message
// RETURNS    : pointer to requested message if successful, 0 otherwise
////////////////////////////////////////////////////////////////////////

const char *Mission::GetInternalMessage( short id ) const {
  return internal_messages.GetMsg( id );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::RegisterBattle
// DESCRIPTION: File a new combat. Record it for historical replay.
// PARAMETERS : att - attacking unit
//              def - defending unit
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Mission::RegisterBattle( Unit *att, Unit *def ) {
  battles.AddTail( new Combat( att, def ) );
  att->Attack( def );
  if ( history ) history->RecordAttackEvent( *att, *def );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::SetLocale
// DESCRIPTION: Set the language to use.
// PARAMETERS : lang - locale identifier
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Mission::SetLocale( const string &lang ) {
  messages.SetDefaultLanguage( lang );
  unit_set.SetLocale( lang );

  p1.SetName( messages.GetMsg(p1.NameID()) );
  p2.SetName( messages.GetMsg(p2.NameID()) );

  for ( Building *b = static_cast<Building *>(shops.Head());
        b; b = static_cast<Building *>(b->Next()) ) {
    const char *name = messages.GetMsg(b->NameID());
    b->SetName( name != NULL ? name : "" );
  }
}

