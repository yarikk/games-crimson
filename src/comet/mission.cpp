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

////////////////////////////////////////////////////////////////////////
// mission.cpp
////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "mission.h"
#include "fileio.h"
#include "strutil.h"
#include "globals.h"

const char *event_labels[] = {
      "Message", "Mining", "Score", "Configure", "Create Unit",
      "Manipulate Event", "Research", "Set Hex", "Set Timer",
      "Destroy Unit", 0 };
const char *etrigger_labels[] = {
      "Timer", "Unit Destruction", "Shop Owner", "Unit Owner",
      "Unit Position", "Handicap", "Have Crystals", 0 };

#define P1_COLOR_LIGHT	0x00FEA604		// yellow
#define P1_COLOR_SHADOW	0x0056360C		// brown
#define P2_COLOR_LIGHT	0x0000AAFF		// light blue
#define P2_COLOR_SHADOW	0x0002265C		// dark blue

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::Mission
// DESCRIPTION: Create a new mission.
// PARAMETERS : size - map size
//              ts   - tile set to be used
//              us   - unit set to be used
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Mission::Mission( const Point &size, TerrainSet *ts, UnitSet *us ) :
                  p1(PLAYER_ONE), p2(PLAYER_TWO) {
  flags = GI_SKIRMISH;
  name = -1;
  campaign_name = -1;
  level_info = -1;
  campaign_info = -1;
  SetTitle( "Unknown" );

  unit_set = us;
  terrain_set = ts;

  map.SetUnitSet( unit_set );
  map.SetTerrainSet( terrain_set );
  map.SetSize( size );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::~Mission
// DESCRIPTION: Destructor
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Mission::~Mission( void ) {
  delete unit_set;
  delete terrain_set;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::Load
// DESCRIPTION: Load a game from a mission file.
// PARAMETERS : filename - mission file name
// RETURNS    : 0 on success, non-zero otherwise
////////////////////////////////////////////////////////////////////////

int Mission::Load( const char *filename ) {
  int rc = -1;
  File file( filename );
  if ( !file.Open( "rb" ) ) return -1;

  // read game info
  if ( (file.Read32() == FID_MISSION) && (file.Read8() == FILE_VERSION) ) {
    unsigned short len, i;
    signed char mapid, musicid;
    string uset, tset;
    Building *b;
    Event *e;

    // set last filename
    last_file_name = file_part( filename );
    i = last_file_name.rfind( '.' );
    last_file_name.erase( i );

    flags = file.Read16();
    i = file.Read16();              // turn number - unused

    if ( flags & GI_SAVEFILE )      // refuse to load saved games
      return -1;

    name = file.Read8();
    level_info = file.Read8();
    campaign_name = file.Read8();
    campaign_info = file.Read8();
    mapid = file.Read8();
    musicid = file.Read8();
    file.Read8();                   // handicap - unused
    file.Read8();                   // current player - unused
    file.Read8();                   // turn phase - unused

    p1.Load( file );
    p2.Load( file );

    // load text messages
    messages.Load( file );

    map.Load( file );               // load map

    len = file.Read16();            // load name of unit set
    uset = file.ReadS( len );

    unit_set = new UnitSet;
    File ufile( uset + ".units" );
    if ( !ufile.OpenData("rb") || unit_set->Load( ufile, uset.c_str() ) ) {
      cerr << "Error: Unit set '" << uset << "' not available." << endl;
      return -1;
    }
    ufile.Close();

    len = file.Read16();            // load name of terrain set
    tset = file.ReadS( len );

    terrain_set = new TerrainSet;
    File tfile( tset + ".tiles" );
    if ( !tfile.OpenData("rb") || terrain_set->Load( tfile, tset.c_str() ) ) {
      cerr << "Error: Terrain set '" << tset << "' not available." << endl;
      return -1;
    }
    tfile.Close();

    map.SetUnitSet( unit_set );
    map.SetTerrainSet( terrain_set );

    len = file.Read16();                // load buildings
    for ( i = 0; i < len; ++i ) {
      b = new Building();
      b->Load( file );
      buildings.AddTail( b );
      map.SetBuilding( b, b->Position() );
    }

    len = file.Read16();                // load units
    for ( i = 0; i < len; ++i ) {
      unsigned char tid = file.Read8();

      Unit *u = new Unit( file, unit_set->GetUnitInfo(tid) );
      if ( u ) {
        units.AddTail( u );
        if ( !map.GetMapObject(u->Position()) )
          map.SetUnit( u, u->Position() );
        else u->SetFlags( U_SHELTERED );
      }
    }

    len = file.Read16();                // load battles

    // combat actions - only present in saved games

    len = file.Read16();                // load events
    for ( i = 0; i < len; ++i ) {
      e = new Event();
      e->Load( file );
      events.AddTail( e );
    }

    Language internal_messages;
    internal_messages.ReadCatalog( file );
    SetSequel( internal_messages.GetMsg( mapid ) );
    SetMusic( internal_messages.GetMsg( musicid ) );

    // set shop names
    for ( b = static_cast<Building *>(buildings.Head());
          b; b = static_cast<Building *>(b->Next()) )
      b->SetName( GetMessage( b->NameID() ) );

    // set special event properties
    for ( e = static_cast<Event *>(events.Head());
          e; e = static_cast<Event *>(e->Next()) ) {
      if ( (e->Type() == EVENT_CONFIGURE) &&
           (e->GetData(0) == 2) ) {    // set next map
        const char *nmap = internal_messages.GetMsg( e->GetData(1) );
        if ( nmap ) e->SetTmpBuf( nmap );
        else {
          e->SetTmpBuf("");
          e->SetData(1, -1);
        }
      }
    }

    len = file.Read16();

    // history - only present in saved games

    rc = 0;
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::Save
// DESCRIPTION: Save the mission to a file.
// PARAMETERS : filename - data file name
// RETURNS    : 0 on successful write, non-zero otherwise
////////////////////////////////////////////////////////////////////////

int Mission::Save( const char *filename ) {
  File file( filename );
  if ( !file.Open( "wb", false ) ) {
    cerr << "Failed to save mission!" << endl;
    return -1;
  }

  unsigned short num;
  signed char mapid = -1, musicid = -1;
  Language internal_messages;
  internal_messages.SetID( "--" );
  if ( !next_map.empty() ) {
    mapid = internal_messages.Size();
    internal_messages.AddMsg( next_map );
  }
  if ( !music.empty() ) {
    musicid = internal_messages.Size();
    internal_messages.AddMsg( music );
  }

  // get special event properties
  Event *e;
  for ( e = static_cast<Event *>( events.Head() );
        e; e = static_cast<Event *>( e->Next() ) ) {
    if ( (e->Type() == EVENT_CONFIGURE) && (e->GetData(0) == 2) ) { // next map
      if ( e->GetTmpBuf() != "" ) {
        short msgid = internal_messages.Find( e->GetTmpBuf() );
        if ( msgid != -1 ) e->SetData( 1, msgid );
        else {
          e->SetData( 1, internal_messages.Size() );
          internal_messages.AddMsg( e->GetTmpBuf() );
        }
      } else e->SetData( 1, -1 );
    }
  }

  // set last filename
  last_file_name = file_part( filename );
  num = last_file_name.rfind( '.' );
  last_file_name.erase( num );

  file.Write32( FID_MISSION );
  file.Write8( FILE_VERSION );

  file.Write16( flags );
  file.Write16( 1 );        // turn

  file.Write8( name );
  file.Write8( level_info );
  file.Write8( campaign_name );
  file.Write8( campaign_info );
  file.Write8( mapid );
  file.Write8( musicid );
  file.Write8( HANDICAP_NONE );
  file.Write8( PLAYER_ONE );
  file.Write8( TURN_START );

  p1.Save( file );             // save player data
  p2.Save( file );

  messages.Save( file );

  map.Save( file );            // save map

  // save mission set info
  file.Write16( unit_set->GetName().length() );
  file.WriteS( unit_set->GetName() );
  file.Write16( terrain_set->GetName().length() );
  file.WriteS( terrain_set->GetName() );

  // save buildings
  file.Write16( buildings.CountNodes() );
  for ( Building *b = static_cast<Building *>( buildings.Head() );
        b; b = static_cast<Building *>( b->Next() ) ) b->Save( file );

  // save transports; basically, transports are not much different
  // from other units but we MUST make sure that transports having
  // other units on board are loaded before those units; we need to
  // make two passes through the list, first saving all unsheltered
  // units (which are possibly transports carrying other units), and
  // all sheltered units in the second run
  file.Write16( units.CountNodes() ); 
  Unit *u;
  for ( u = static_cast<Unit *>( units.Head() );
        u; u = static_cast<Unit *>( u->Next() ) ) {    // make sure transports are
    if ( !u->IsSheltered() ) u->Save( file );          // stored before carried units
  }

  for ( u = static_cast<Unit *>( units.Head() );
        u; u = static_cast<Unit *>( u->Next() ) ) {
    if ( u->IsSheltered() ) u->Save( file );
  }

  file.Write16( 0 );        // save combat data

  // save events
  file.Write16( events.CountNodes() );
  for ( e = static_cast<Event *>( events.Head() );
        e; e = static_cast<Event *>( e->Next() ) )
    e->Save( file );

  internal_messages.WriteCatalog( file );

  file.Write16( 0 );        // turn history

  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::Export
// DESCRIPTION: Save the mission data to a plain text file.
// PARAMETERS : filename - save file name
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Mission::Export( const string &filename ) const {
  ofstream file( filename.c_str() );
  if ( !file ) return -1;

  string title( file_part(filename) );
  int suffix = title.rfind( '.' );
  title.erase( suffix );

  file << "[mission]\n";
  file << "name = " << (short)name << '\n';
  file << "mapwidth = " << map.Width() << '\n';
  file << "mapheight = " << map.Height() << '\n';
  file << "info = " << (short)level_info << '\n';
  file << "skirmish = " << (IsSkirmish() ? '1' : '0') << '\n';
  file << "campaign = " << (IsCampaign() ? '1' : '0') << '\n';
  file << "campaignname = " << (short)campaign_name << '\n';
  file << "campaigninfo = " << (short)campaign_info << '\n';

  if ( !next_map.empty() )
    file << "nextmap = " << next_map << '\n';
  if ( !music.empty() )
    file << "music = " << music << '\n';

  file << "players = " << (((flags & GI_AI) != 0) ? '1' : '2') << '\n';
  file << "tileset = " << terrain_set->GetName() << '\n';
  file << "unitset = " << unit_set->GetName() << "\n\n";

  map.Export( file );
  p1.Export( file );
  p2.Export( file );

  Unit *u;
  for ( u = static_cast<Unit *>(units.Head()); u;
        u = static_cast<Unit *>(u->Next()) )
    if ( !u->IsSheltered() ) u->Export( file );

  for ( u = static_cast<Unit *>(units.Head()); u;
        u = static_cast<Unit *>(u->Next()) )
    if ( u->IsSheltered() ) u->Export( file );

  for ( Building *b = static_cast<Building *>(buildings.Head()); b;
        b = static_cast<Building *>(b->Next()) )
    b->Export( file, unit_set );

  for ( Event *e = static_cast<Event *>(events.Head()); e;
        e = static_cast<Event *>(e->Next()) )
    e->Export( file, map );

  if ( messages.GetMsg(0) != 0 ) {
    const std::map<const string, Language> &lib = messages.GetLibrary();
    for ( std::map<const string, Language>::const_iterator iter = lib.begin();
          iter != lib.end(); ++iter ) {

      file << "[messages(" << iter->first << ")]\n";
      file << iter->second.GetMsg(0);

      const char *m;
      for ( int i = 1; (m = iter->second.GetMsg(i)) != 0; ++i ) {
        file << "\n%\n" << m;
      }
      file << "\n[/messages]\n\n";
    }
  }
  file.close();
  return 0;
}


////////////////////////////////////////////////////////////////////////
// NAME       : Mission::CreateUnit
// DESCRIPTION: Create a new unit.
// PARAMETERS : type - unit type info
//              pid  - identifier of cotrolling player
//              pos  - position on map
// RETURNS    : created unit or NULL on error
////////////////////////////////////////////////////////////////////////

Unit *Mission::CreateUnit( const UnitType *type, unsigned char pid, const Point &pos ) {
  Unit *u = new Unit( type, pid, GetUnitID(), pos );
  if ( u ) {
    units.AddTail( u );

    if ( !map.GetMapObject( pos ) ) map.SetUnit( u, pos );
    else u->SetFlags( U_SHELTERED );
  }
  return u;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::DeleteUnit
// DESCRIPTION: Delete a unit, and for transporters also all freight.
// PARAMETERS : u - unit to remove from mission
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Mission::DeleteUnit( Unit *u ) {
  u->Remove();

  if ( !u->IsSheltered() ) {
    map.SetUnit( NULL, u->Position() );

    if ( u->IsTransport() ) {
      Unit *next, *f = static_cast<Unit *>( units.Head() );
      while ( f ) {
        next = static_cast<Unit *>( f->Next() );

        if ( f->Position() == u->Position() ) {
          f->Remove();
          delete f;
        }

        f = next;
      }
    }
  }

  delete u;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::GetUnitByID
// DESCRIPTION: Find the unit corresponding to the given identifier.
// PARAMETERS : id - identifier of the unit to be searched for
// RETURNS    : pointer to the unit, or NULL if no unit with that ID
//              exists
////////////////////////////////////////////////////////////////////////

Unit *Mission::GetUnitByID( unsigned short id ) const {
  Unit *u = static_cast<Unit *>( units.Head() );
  while ( u ) {
    if ( u->ID() == id ) return u;
    u = static_cast<Unit *>( u->Next() );
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::GetUnitID
// DESCRIPTION: Get a unique identifier for a new unit.
// PARAMETERS : -
// RETURNS    : unique unit ID
////////////////////////////////////////////////////////////////////////

unsigned short Mission::GetUnitID( void ) const {
  unsigned short id = 0;
  Unit *u = static_cast<Unit *>( units.Head() );

  while ( u ) {
    if ( u->ID() == id ) {
      ++id;
      u = static_cast<Unit *>( units.Head() );
    } else u = static_cast<Unit *>( u->Next() );
  }

  return id;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::CreateBuilding
// DESCRIPTION: Create a new building.
// PARAMETERS : pid - identifier of cotrolling player
//              pos - position on map
// RETURNS    : created building or NULL on error
////////////////////////////////////////////////////////////////////////

Building *Mission::CreateBuilding( unsigned char pid, const Point &pos ) {
  Building *b = new Building( pos, GetBuildingID(), pid );
  if ( b ) {
    buildings.AddTail( b );
    map.SetBuilding( b, pos );
  }
  return b;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::DeleteBuilding
// DESCRIPTION: Delete a building and all units inside.
// PARAMETERS : b - building to remove from mission
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Mission::DeleteBuilding( Building *b ) {
  map.SetBuilding( NULL, b->Position() );
  b->Remove();

  Unit *next, *f = static_cast<Unit *>( units.Head() );
  while ( f ) {
    next = static_cast<Unit *>( f->Next() );

    if ( f->Position() == b->Position() ) {
      f->Remove();
      delete f;
    }

    f = next;
  }

  delete b;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::GetBuildingByID
// DESCRIPTION: Find the shop corresponding to the given identifier.
// PARAMETERS : id - identifier of the shop to be searched for
// RETURNS    : pointer to the shop, or NULL if no building with that ID
//              exists
////////////////////////////////////////////////////////////////////////

Building *Mission::GetBuildingByID( unsigned short id ) const {
  Building *b = static_cast<Building *>( buildings.Head() );
  while ( b ) {
    if ( b->ID() == id ) return b;
    b = static_cast<Building *>( b->Next() );
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::GetBuildingID
// DESCRIPTION: Get a unique identifier for a new building.
// PARAMETERS : -
// RETURNS    : unique building ID
////////////////////////////////////////////////////////////////////////

unsigned short Mission::GetBuildingID( void ) const {
  unsigned short id = 0;
  Building *b = static_cast<Building *>( buildings.Head() );

  while ( b ) {
    if ( b->ID() == id ) {
      ++id;
      b = static_cast<Building *>( buildings.Head() );
    } else b = static_cast<Building *>( b->Next() );
  }

  return id;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::CreateEvent
// DESCRIPTION: Create a new event. The event is initialized with some
//              default values (which may be invalid in some cases).
// PARAMETERS : type    - event type
//              trigger - event trigger type
// RETURNS    : created event or NULL on error
////////////////////////////////////////////////////////////////////////

Event *Mission::CreateEvent( unsigned char type, unsigned char trigger ) {
  Event *e = new Event( GetEventID(), type, trigger );
  if ( e ) events.AddTail( e );
  return e;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::DeleteEvent
// DESCRIPTION: Delete an event.
// PARAMETERS : e - event to remove from mission
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Mission::DeleteEvent( Event *e ) {
  e->Remove();
  delete e;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::GetEventByID
// DESCRIPTION: Find the event corresponding to the given identifier.
// PARAMETERS : id - identifier of the event to be searched for
// RETURNS    : pointer to the event, or NULL if no event with that ID
//              exists
////////////////////////////////////////////////////////////////////////

Event *Mission::GetEventByID( unsigned short id ) const {
  Event *e = static_cast<Event *>( events.Head() );
  while ( e ) {
    if ( e->ID() == id ) return e;
    e = static_cast<Event *>( e->Next() );
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::GetEventID
// DESCRIPTION: Get a unique identifier for a new event.
// PARAMETERS : -
// RETURNS    : unique event ID
////////////////////////////////////////////////////////////////////////

unsigned char Mission::GetEventID( void ) const {
  unsigned char id = 0;
  Event *e = static_cast<Event *>( events.Head() );

  while ( e ) {
    if ( e->ID() == id ) {
      ++id;
      e = static_cast<Event *>( events.Head() );
    } else e = static_cast<Event *>( e->Next() );
  }

  return id;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::SetNumPlayers
// DESCRIPTION: Set the intended number of human players for this map.
//              This is only a hint for the player.
// PARAMETERS : num - number of intended players (1 or 2)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Mission::SetNumPlayers( unsigned char num ) {
  if ( num == 1 ) flags |= GI_AI;
  else flags &= ~GI_AI;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::StorageLeft
// DESCRIPTION: Get the amount of free space in a transport, which can
//              be used to store crystals or units. Crystals require 1
//              storage unit per 10 crystals, units need Unit::Weight()
//              each.
// PARAMETERS : -
// RETURNS    : number of unused storage units; for non-transport units
//              returns 0
////////////////////////////////////////////////////////////////////////

short Mission::StorageLeft( Unit &u ) const {
  short space;

  if ( u.IsTransport() ) {
    space = u.Type()->Slots() - ((u.Crystals() + 9) / 10);

    if ( !u.IsSheltered() ) {
      for ( Unit *walk = static_cast<Unit *>(units.Head()); walk;
            walk = static_cast<Unit *>(walk->Next()) ) {
        if ( (u.Position() == walk->Position()) && walk->IsSheltered() )
          space -= walk->Weight();
      }
    }
  } else space = 0;
  return space;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::Validate
// DESCRIPTION: Check the mission for (logical) errors. This will catch
//              mistakes concerning settings which can not directly be
//              enforced by CoMET (e.g. you could create an event
//              targetting a unit and delete that unit afterwards).
// PARAMETERS : errors - stringstream; for each error found, a short
//                       message will be appended which can afterwards
//                       be presented to the user
// RETURNS    : number of errors and warnings detected
////////////////////////////////////////////////////////////////////////

unsigned short Mission::Validate( stringstream &errors ) {
  unsigned short errs = 0;

  if ( name == -1 ) {
    ++errs;
    errors << "Error: No level name configured.\n\n";
  } else if ( !GetMessage(name) ) {
    ++errs;
    errors << "Error: Invalid level name " << (short)name << " configured.\n\n";
  }

  Player *pls[2] = { &p1, &p2 };
  for ( int i = 0; i < 2; ++i ) {
    if ( pls[i]->NameID() == -1 ) {
      ++errs;
      errors << "Error: No name set for player " << i+1 << ".\n\n";
    } else if ( !GetMessage(pls[i]->NameID()) ) {
      ++errs;
      errors << "Error: Invalid name " << (short)pls[i]->NameID()
             << " set for player " << i+1 << ".\n\n";
    }

    if ( pls[i]->Briefing() == -1 ) {
      ++errs;
      errors << "Warning: No briefing set for player " << i+1 << ".\n\n";
    } else if ( !GetMessage(pls[i]->Briefing()) ) {
      ++errs;
      errors << "Error: Invalid briefing " << (short)pls[i]->Briefing()
             << " set for player " << i+1 << ".\n\n";
    }
  }

  if ( level_info == -1 ) {
    ++errs;
    errors << "Warning: No level info message configured.\n\n";
  } else if ( !GetMessage(level_info) ) {
    ++errs;
    errors << "Error: Invalid level info message " << (short)level_info << " configured.\n\n";
  }

  if ( (campaign_name != -1) && !GetMessage(campaign_name) ) {
    ++errs;
    errors << "Error: Invalid campaign name " << (short)campaign_name << " configured.\n\n";
  }

  if ( (campaign_info != -1) && !GetMessage(campaign_info) ) {
    ++errs;
    errors << "Error: Invalid campaign info message " << (short)campaign_info << " configured.\n\n";
  } else if ( (campaign_name != -1) && (campaign_info == -1) ) {
    ++errs;
    errors << "Warning: Campaign name set, but campaign info unset.\n\n";
  } else if ( (campaign_name == -1) && (campaign_info != -1) ) {
    ++errs;
    errors << "Warning: Campaign info set, but campaign name unset.\n\n";
  }

  if ( !IsCampaign() && (GetSequel() || (campaign_name != -1) || (campaign_info != -1)) ) {
    ++errs;
    errors << "Error: Campaign name, info, or sequel set, but map is not part of a campaign.\n\n";
  }

  errs += ValidateMap( errors );

  Node *n;
  for ( n = events.Head(); n; n = n->Next() )
    errs += ValidateEvent( *static_cast<Event *>(n), errors );

  for ( n = buildings.Head(); n; n = n->Next() )
    errs += ValidateShop( *static_cast<Building *>(n), errors );

  errors << "Validation summary\n"
         << "------------------\n"
         << "Detected " << errs << " errors or warnings.";

  return errs;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::ValidateMap
// DESCRIPTION: Check the map for errors.
// PARAMETERS : errors - error stream
// RETURNS    : number of errors and warnings detected
////////////////////////////////////////////////////////////////////////

unsigned short Mission::ValidateMap( stringstream &errors ) const {
  unsigned short errs = 0;
  Point p;

  for ( p.y = 0; p.y < map.Height(); ++p.y ) {
    for ( p.x = 0; p.x < map.Width(); ++p.x ) {

      Building *b = map.GetBuilding( p );
      bool isshop = map.IsBuilding( p );
      if ( isshop && !b ) {
        ++errs;
        errors << "Error: Tile at " << p.x << '/' << p.y
               << " denotes a shop entrance, but no shop has been created.\n\n";
      } else if ( !isshop && b ) {
        ++errs;
        errors << "Error: A shop exists at " << p.x << '/' << p.y
               << ", but there is no entrance.\n\n";
      }

      Unit *u = map.GetUnit( p );
      if ( u && !u->IsSheltered() &&
           ((map.TerrainTypes(p) & u->Terrain()) == 0) ) {
        ++errs;
        errors << "Warning: '" << u->Name() << "' (" << u->ID() << ") at "
               << p.x << '/' << p.y << " walking on invalid terrain.\n\n";
      }
    }
  }
  return errs;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::ValidateEvent
// DESCRIPTION: Check the event for errors.
// PARAMETERS : e - event to check
//              errors - error stream
// RETURNS    : number of errors and warnings detected
////////////////////////////////////////////////////////////////////////

unsigned short Mission::ValidateEvent( Event &e, stringstream &errors ) const {
  unsigned short errs = 0;

  if ( (e.Title() != -1) && !GetMessage(e.Title()) ) {
    ++errs;
    errors << "Error: Invalid title " << (short)e.Title() << " for "
           << e.Name() << " (" << (short)e.ID() << ") event.\n\n";
  }

  if ( (e.Message() != -1) && !GetMessage(e.Message()) ) {
    ++errs;
    errors << "Error: Invalid message " << (short)e.Message() << " for "
           << e.Name() << " (" << (short)e.ID() << ") event.\n\n";
  }

  if ( (e.Dependency() != -1) && !GetEventByID(e.Dependency()) ) {
    ++errs;
    errors << "Warning: Event " << (short)e.ID()
           << " depends on non-existing event " << (short)e.Dependency() << ".\n\n";
  }

  if ( (e.Discard() != -1) && !GetEventByID(e.Discard()) ) {
    ++errs;
    errors << "Warning: Event " << (short)e.ID()
           << " discards non-existing event " << (short)e.Discard() << ".\n\n";
  }

  switch ( e.Trigger() ) {
  case ETRIGGER_HANDICAP:
    break;

  case ETRIGGER_HAVE_BUILDING:
    if ( (e.GetTData(0) == -1) || !GetBuildingByID(e.GetTData(0)) ) {
      ++errs;
      errors << "Error: Trigger of " << e.Name() << " (" << (short)e.ID()
             << ") event targets non-existing shop with ID " << e.GetTData(0) << ".\n\n";
    }
    break;

  case ETRIGGER_HAVE_CRYSTALS:
    if ( (e.GetTData(2) >= 0) && !GetBuildingByID(e.GetTData(2)) ) {
      ++errs;
      errors << "Error: Trigger of " << e.Name() << " (" << (short)e.ID()
             << ") event targets non-existing shop with ID " << e.GetTData(2) << ".\n\n";
    }
    break;

  case ETRIGGER_HAVE_UNIT:
    if ( (e.GetTData(0) < 0) || !GetUnitByID(e.GetTData(0)) ) {
      ++errs;
      errors << "Error: Trigger of " << e.Name() << " (" << (short)e.ID()
             << ") event targets non-existing unit with ID " << e.GetTData(0) << ".\n\n";
    }
    break;

  case ETRIGGER_TIMER:
    break;

  case ETRIGGER_UNIT_DESTROYED:
    if ( e.GetTData(0) >= 0 ) {
      Unit *u = GetUnitByID( e.GetTData(0) );

      if ( !u ) {
        ++errs;
        errors << "Error: Trigger of " << e.Name() << " (" << (short)e.ID()
               << ") event targets non-existing unit with ID " << e.GetTData(0) << ".\n\n";
      } else if ( u->Owner() != e.GetTData(1) ) {
        ++errs;
        errors << "Warning: Trigger of " << e.Name() << " (" << (short)e.ID()
               << ") event had incorrect owner of " << u->Name()
               << " (" << u->ID() << "). Fixed.\n\n";
        e.SetTData( 1, u->Owner() );
      }
    }
    break;

  case ETRIGGER_UNIT_POSITION:
    if ( (e.GetTData(0) > 0) && !GetUnitByID(e.GetTData(0)) ) {
      ++errs;
      errors << "Error: Trigger of " << e.Name() << " (" << (short)e.ID()
             << ") event targets non-existing unit with ID " << e.GetTData(0) << ".\n\n";
    } else if ( (e.GetTData(0) < -1) && (-e.GetTData(0) - 2 >= unit_set->NumTiles()) ) {
      ++errs;
      errors << "Error: Trigger of " << e.Name() << " (" << (short)e.ID()
             << ") event targets invalid unit type with ID " << -e.GetTData(0) - 2 << ".\n\n";
    }
    break;

  default:
    ++errs;
    errors << "Warning: Unknown event trigger " << e.TriggerName() << " for "
           << e.Name() << " (" << (short)e.ID() << ") event.\n\n";
  }


  switch ( e.Type() ) {
  case EVENT_CONFIGURE:
    if ( (e.GetData(1) != -1) && !GetMessage(e.GetData(1)) ) {
    ++errs;
    errors << "Error: Invalid message " << e.GetData(1) << " for "
           << e.Name() << " (" << (short)e.ID() << ") event.\n\n";
  }

  case EVENT_MINING:
  case EVENT_RESEARCH:
    if ( (e.GetData(0) == -1) || !GetBuildingByID(e.GetData(0)) ) {
      ++errs;
      errors << "Error: " << e.Name() << " (" << (short)e.ID()
             << ") event targets non-existing shop with ID " << e.GetData(0) << ".\n\n";
    }
    break;

  case EVENT_MANIPULATE_EVENT:
    if ( (e.GetData(0) == -1) || !GetEventByID(e.GetData(0)) ) {
      ++errs;
      errors << "Error: " << e.Name() << " (" << (short)e.ID()
             << ") event targets non-existing event with ID " << e.GetData(0) << ".\n\n";
    }
    break;

  case EVENT_CREATE_UNIT:
  case EVENT_MESSAGE:
  case EVENT_SET_HEX:
    break;
  case EVENT_DESTROY_UNIT:
    if ((e.GetData(0) >= 0) && !GetUnitByID(e.GetData(0))) {
      ++errs;
      errors << "Error: Invalid unit " << e.GetData(0) << " for "
             << e.Name() << " (" << (short)e.ID() << ") event.\n\n";
    }
    break;
  case EVENT_SCORE:
    if ( (e.GetData(1) != -1) && !GetMessage(e.GetData(1)) ) {
      ++errs;
      errors << "Error: Invalid message " << e.GetData(1) << " for "
             << e.Name() << " (" << (short)e.ID() << ") event.\n\n";
    }

    if ( (e.GetData(1) != -1) && (e.GetData(2) != -1) && !GetMessage(e.GetData(2)) ) {
      ++errs;
      errors << "Error: Event " << (short)e.ID() << " references illegal message "
             << e.GetData(2) << ".\n\n";
    } else if ( (e.GetData(1) == -1) && (e.GetData(2) != -1) ) {
      ++errs;
      errors << "Warning: Event " << (short)e.ID() << " set a title but no message.\n\n";
    }
    break;

  case EVENT_SET_TIMER:
    if ( (e.GetData(0) == -1) || !GetEventByID(e.GetData(0)) ) {
      ++errs;
      errors << "Error: " << e.Name() << " (" << (short)e.ID()
             << ") event targets non-existing event with ID " << e.GetData(0) << ".\n\n";
    } else {
      Event *tev = GetEventByID(e.GetData(0));
      if ( tev->Trigger() != ETRIGGER_TIMER ) {
        ++errs;
        errors << "Error: " << e.Name() << " (" << (short)e.ID()
               << ") event targets event that has no "
               << etrigger_labels[ETRIGGER_TIMER] << " trigger.\n\n";
      }
    }
    break;

  default:
    ++errs;
    errors << "Warning: Unknown event type " << e.Name() << " for event ID "
           << (short)e.ID() << ".\n\n";
  }

  return errs;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Mission::ValidateShop
// DESCRIPTION: Check the shop for errors.
// PARAMETERS : b      - shop to check
//              errors - error stream
// RETURNS    : number of errors and warnings detected
////////////////////////////////////////////////////////////////////////

unsigned short Mission::ValidateShop( Building &b, stringstream &errors ) const {
  unsigned short errs = 0;

  if ( b.NameID() == -1 ) {
    ++errs;
    errors << "Error: No name set for shop " << b.ID() << ".\n\n";
  } else if ( !b.Name() ) {
    ++errs;
    errors << "Error: Invalid name " << (short)b.NameID() << " for shop " << b.ID() << ".\n\n";
  } else if ( strlen(b.Name()) > 30 ) {
    ++errs;
    errors << "Error: Name for shop " << b.ID() << " is longer than 30 characters.\n\n";
  }

  return errs;
}


////////////////////////////////////////////////////////////////////////
// NAME       : Player::Player
// DESCRIPTION: Create a new player.
// PARAMETERS : id - player identifier (PLAYER_ONE or PLAYER_TWO)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Player::Player( unsigned char id ) :
        p_col_light(id == PLAYER_ONE ? P1_COLOR_LIGHT : P2_COLOR_LIGHT ),
        p_col_dark(id == PLAYER_ONE ? P1_COLOR_SHADOW : P2_COLOR_SHADOW ) {
  p_id = id;
  p_success = 0;
  p_briefing = -1;
  p_name_id = -1;
  p_type = COMPUTER;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Player::Load
// DESCRIPTION: Load player information from a file.
// PARAMETERS : file - data file descriptor
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Player::Load( MemBuffer &file ) {
  p_id = file.Read8();
  p_type = file.Read8();
  p_success = file.Read8();
  p_briefing = file.Read8();
  p_name_id = file.Read8();

  unsigned char len = file.Read8();
  p_password = StringUtil::crypt(file.ReadS(len));

  p_col_light.r = file.Read8();
  p_col_light.g = file.Read8();
  p_col_light.b = file.Read8();
  p_col_dark.r = file.Read8();
  p_col_dark.g = file.Read8();
  p_col_dark.b = file.Read8();
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Player::Save
// DESCRIPTION: Save player information to a file.
// PARAMETERS : file - data file descriptor
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int Player::Save( MemBuffer &file ) const {
  file.Write8( p_id );
  file.Write8( p_type );
  file.Write8( p_success );
  file.Write8( p_briefing );
  file.Write8( p_name_id );
  file.Write8( p_password.size() );
  file.WriteS( StringUtil::crypt( p_password ) );

  file.Write8( p_col_light.r );
  file.Write8( p_col_light.g );
  file.Write8( p_col_light.b );
  file.Write8( p_col_dark.r );
  file.Write8( p_col_dark.g );
  file.Write8( p_col_dark.b );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Player::Export
// DESCRIPTION: Save the player to a plain text file.
// PARAMETERS : file - data file descriptor
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Player::Export( ofstream &file ) const {
  file << "[player]\n";
  file << "name = " << (short)p_name_id << '\n';
  file << "briefing = " << (short)p_briefing << '\n';
  file << "fcolor = " << (short)p_col_light.r << ',' << (short)p_col_light.g << ','
                      << (short)p_col_light.b << '\n';
  file << "bcolor = " << (short)p_col_dark.r << ',' << (short)p_col_dark.g << ','
                      << (short)p_col_dark.b << "\n\n";
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Event::Event
// DESCRIPTION: Create a new event.
// PARAMETERS : id      - unique event identifier
//              type    - event type
//              trigger - event trigger type
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Event::Event( unsigned char id, unsigned char type, unsigned char trigger ) {
  e_id = id;
  e_type = type;
  e_trigger = trigger;
  e_depend = -1;
  e_discard = -1;
  e_title = -1;
  e_message = -1;
  e_flags = 0;
  e_player = PLAYER_ONE;

  // set event data defaults
  switch ( e_type ) {
  case EVENT_CONFIGURE:
    e_data[0] = 0;         // set player 1 briefing
    e_data[1] = -1;        // no briefing available
    e_data[2] = 0;
    break;
  case EVENT_CREATE_UNIT:
    e_data[0] = 0;        // build first unit type in set
    e_data[1] = 0;        // build at position (0, 0)
    e_data[2] = NORTH | (MAX_GROUP_SIZE << 3) | (0 << 6);
                          // face northwards, MAX_GROUP_SIZE, no xp
    break;
  case EVENT_DESTROY_UNIT:
    e_data[0] = -1;       // destroy any unit
    e_data[1] = -1;       // any owner
    e_data[2] = 0;        // at position 0/0
    break;
  case EVENT_MANIPULATE_EVENT:
    e_data[0] = -1;      // modify event -1
    e_data[1] = 0;       // flags affected
    e_data[2] = 0;       // set selected flags
    break;
  case EVENT_MESSAGE:
    e_data[0] = -1;      // no default focus
    e_data[1] = e_data[2] = 0;
    break;
  case EVENT_MINING:
    e_data[0] = -1;        // change mining in shop -1
    e_data[1] = 0;         // set crystals to this amount
    e_data[2] = 0;         // set storage to absolute amount
    break;
  case EVENT_RESEARCH:
    e_data[0] = -1;      // research in shop -1
    e_data[1] = 0;       // research first unit in set
    e_data[2] = 0;       // allow production of unit
    break;
  case EVENT_SCORE:
    e_data[0] = 100;       // award 100 points
    e_data[1] = e_data[2] = -1; // don't show a message to the loser
    break;
  case EVENT_SET_HEX:
    e_data[0] = 0;       // set to first tile
    e_data[1] = 0;       // change hex at (0, 0)
    e_data[2] = 0;
    break;
  case EVENT_SET_TIMER:
    e_data[0] = -1;      // invalid event
    e_data[1] = 0;       // set timer to 0
    e_data[2] = 2;       // relative to current configuration
    break;
  }

  switch ( e_trigger ) {
  case ETRIGGER_TIMER:
    e_tdata[0] = 0;     // activate at beginning of first turn
    e_tdata[1] = e_tdata[2] = 0;
    break;
  case ETRIGGER_UNIT_DESTROYED:
    e_tdata[0] = -1;   // destroy all enemy units
    e_tdata[1] = PLAYER_ONE;
    e_tdata[2] = 0;
    break;
  case ETRIGGER_HAVE_CRYSTALS:
    e_tdata[0] = 100;         // >= 100 crystals
    e_tdata[1] = PLAYER_ONE;  // owned by first player
    e_tdata[2] = -1;          // check all shops
    break;
  case ETRIGGER_HAVE_BUILDING:
  case ETRIGGER_HAVE_UNIT:
    e_tdata[0] = -1;          // check shop/unit -1
    e_tdata[1] = PLAYER_ONE;  // owned by first player
    e_tdata[2] = -1;          // activate any turn if condition is true
    break;
  case ETRIGGER_UNIT_POSITION:
    e_tdata[0] = -1;         // require no specific unit
    e_tdata[1] = 0;          // coordinates of target hex (0/0)
    e_tdata[2] = PLAYER_ONE; // player required to control the unit
    break;
  case ETRIGGER_HANDICAP:
    e_tdata[0] = HANDICAP_NONE|HANDICAP_P1|HANDICAP_P2; // trigger always
    e_tdata[1] = e_tdata[2] = 0;
    break;
  }

  e_tmpbuf = "";
}

////////////////////////////////////////////////////////////////////////
// NAME       : Event::Load
// DESCRIPTION: Load an event from a file.
// PARAMETERS : file - file descriptor
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Event::Load( MemBuffer &file ) {
  int i;

  e_id = file.Read8();
  e_type = file.Read8();
  e_trigger = file.Read8();
  e_depend = file.Read8();
  e_discard = file.Read8();
  for ( i = 0; i < 3; ++i ) e_tdata[i] = file.Read16();
  for ( i = 0; i < 3; ++i ) e_data[i] = file.Read16();
  e_title = file.Read16();
  e_message = file.Read16();
  e_flags = file.Read16();
  e_player = file.Read8();
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Event::Save
// DESCRIPTION: Save the event data to a file.
// PARAMETERS : file - data file descriptor
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Event::Save( MemBuffer &file ) const {
  int i;

  file.Write8( e_id );
  file.Write8( e_type );
  file.Write8( e_trigger );
  file.Write8( e_depend );
  file.Write8( e_discard );
  for ( i = 0; i < 3; ++i ) file.Write16( e_tdata[i] );
  for ( i = 0; i < 3; ++i ) file.Write16( e_data[i] );
  file.Write16( e_title );
  file.Write16( e_message );
  file.Write16( e_flags );
  file.Write8( e_player );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Event::Export
// DESCRIPTION: Save the event to a plain text file.
// PARAMETERS : file - data file descriptor
//              map  - current map
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Event::Export( ofstream &file, const Map &map ) const {
  const char *type[] = { "message", "mining", "score", "configure",
        "createunit", "manipulateevent", "research", "sethex", "settimer",
        "destroyunit" };
  Point loc;
  const UnitSet *uset = map.GetUnitSet();

  file << "[event]\n";
  file << "id = " << (short)e_id << '\n';
  file << "type = " << type[e_type-1] << '\n';
  file << "player = " << (e_player + 1) << '\n';
  if ( e_depend != -1 )  file << "depend = " << (short)e_depend << '\n';
  if ( e_discard != -1 )  file << "discard = " << (short)e_discard << '\n';
  if ( e_title != -1 )   file << "title = " << e_title << '\n';
  if ( e_message != -1 ) file << "message = " << e_message << '\n';
  if ( e_flags != 0 )    file << "flags = " << e_flags << '\n';

  file << "trigger = ";
  switch ( e_trigger ) {
  case ETRIGGER_TIMER:
    file << "timer\n";
    file << "ttime = " << e_tdata[0] << '\n';
    break;
  case ETRIGGER_UNIT_DESTROYED:
    file << "unitdestroyed\n";
    file << "tunit = ";
    if ( e_tdata[0] >= -1 ) file << e_tdata[0] << '\n';
    else file << uset->GetUnitInfo(-e_tdata[0] - 2)->Name() << '\n';
    file << "towner = " << e_tdata[1] + 1 << '\n';
    break;
  case ETRIGGER_HAVE_BUILDING:
    file << "havebuilding\n";
    file << "tbuilding = " << e_tdata[0] << '\n';
    file << "towner = " << e_tdata[1] + 1 << '\n';
    file << "ttime = " << e_tdata[2] << '\n';
    break;
  case ETRIGGER_HAVE_CRYSTALS:
    file << "havecrystals\n";
    file << "tcrystals = " << e_tdata[0] << '\n';
    file << "towner = " << e_tdata[1] + 1 << '\n';
    file << "tbuilding = " << e_tdata[2] << '\n';
    break;
  case ETRIGGER_HAVE_UNIT:
    file << "haveunit\n";
    file << "tunit = " << e_tdata[0] << '\n';
    file << "towner = " << e_tdata[1] + 1 << '\n';
    file << "ttime = " << e_tdata[2] << '\n';
    break;
  case ETRIGGER_UNIT_POSITION:
    file << "unitposition\n";
    file << "tunit = ";
    if ( e_tdata[0] >= -1 ) file << e_tdata[0] << '\n';
    else file << uset->GetUnitInfo(-e_tdata[0] - 2)->Name() << '\n';
    file << "towner = " << e_tdata[2] + 1 << '\n';
    loc = map.Index2Hex( e_tdata[1] );
    file << "tpos = " << loc.x << '/' << loc.y << '\n';
    break;
  case ETRIGGER_HANDICAP:
    file << "handicap\n";
    file << "thandicap = " << e_tdata[0] << '\n';
    break;
  }

  switch ( e_type ) {
  case EVENT_CONFIGURE:
    file << "setting = ";
    switch ( e_data[0] ) {
    case 0: file << "briefing1"; break;
    case 1: file << "briefing2"; break;
    case 2: file << "nextmap"; break;
    }
    file << '\n' << "value = ";
    switch ( e_data[0] ) {
    case 0:
    case 1: file << e_data[1]; break;
    case 2: file << e_tmpbuf; break;
    }
    file << '\n';
    break;
  case EVENT_CREATE_UNIT:
    file << "unit = " << uset->GetUnitInfo(e_data[0])->Name() << '\n';
    loc = map.Index2Hex( e_data[1] );
    file << "pos = " << loc.x << '/' << loc.y << '\n';
    file << "face = " << (e_data[2] & 0x07) << '\n';
    if ( ((e_data[2] & 0x38) >> 3) != MAX_GROUP_SIZE )
      file << "size = " << ((e_data[2] & 0x38) >> 3) << '\n';
    if ( ((e_data[2] & 0x1C0) >> 6) != 0 )
      file << "xp = " << ((e_data[2] & 0x1C0) >> 6) << '\n';
    break;
  case EVENT_DESTROY_UNIT:
    file << "unit = " << e_data[0] << '\n';
    file << "owner = " << (e_data[1] == -1 ? -1 : e_data[1] + 1) << '\n';
    file << "pos = " << e_data[2] << '\n';
    break;
  case EVENT_MANIPULATE_EVENT:
    file << "event = " << e_data[0] << '\n';
    file << "eflags = " << e_data[1] << '\n';
    file << "action = " << e_data[2] << '\n';
    break;
  case EVENT_MESSAGE:
    loc = map.Index2Hex( e_data[0] );
    if ( loc != Point(-1,-1) )
      file << "pos = " << loc.x << '/' << loc.y << '\n';
    break;
  case EVENT_MINING:
    file << "building = " << e_data[0] << '\n';
    file << "crystals = " << e_data[1] << '\n';
    file << "action = " << e_data[2] << '\n';
    break;
  case EVENT_RESEARCH:
    file << "building = " << e_data[0] << '\n';
    file << "unit = " << uset->GetUnitInfo(e_data[1])->Name() << '\n';
    if ( e_data[2] != 0 ) file << "action = " << e_data[2] << '\n';
    break;
  case EVENT_SCORE:
    file << "success = " << e_data[0] << '\n';
    file << "othermsg = " << e_data[1] << '\n';
    file << "othertitle = " << e_data[2] << '\n';
    break;
  case EVENT_SET_HEX:
    file << "tile = " << e_data[0] << '\n';
    loc = map.Index2Hex( e_data[1] );
    file << "pos = " << loc.x << '/' << loc.y << '\n';
    break;
  case EVENT_SET_TIMER:
    file << "event = " << e_data[0] << '\n';
    file << "time = " << e_data[1] << '\n';
    file << "offset = " << e_data[2] << '\n';
    break;
  }

  file << '\n';
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Event::Name
// DESCRIPTION: Get the name of the event (or rather, the event type).
// PARAMETERS : -
// RETURNS    : event type string
////////////////////////////////////////////////////////////////////////

const char *Event::Name( void ) const {
  return event_labels[Type()-1];
}

////////////////////////////////////////////////////////////////////////
// NAME       : Event::TriggerName
// DESCRIPTION: Get the name of the event trigger type.
// PARAMETERS : -
// RETURNS    : event trigger type string
////////////////////////////////////////////////////////////////////////

const char *Event::TriggerName( void ) const {
  return etrigger_labels[Trigger()];
}

