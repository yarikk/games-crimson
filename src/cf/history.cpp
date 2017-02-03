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
// history.cpp
////////////////////////////////////////////////////////////////////////

#include "history.h"
#include "game.h"
#include "options.h"
#include "msgs.h"

extern Options CFOptions;

////////////////////////////////////////////////////////////////////////
// NAME       : HistEvent::Load
// DESCRIPTION: Load an event from a file.
// PARAMETERS : file - file descriptor
// RETURNS    : -1 on error, 0 otherwise
////////////////////////////////////////////////////////////////////////

int HistEvent::Load( MemBuffer &file ) {
  type = file.Read8();
  for ( int i = 0; i < 4; ++i ) data[i] = file.Read16();
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : HistEvent::Save
// DESCRIPTION: Save an event to a file.
// PARAMETERS : file - file descriptor
// RETURNS    : 0 on success, non-0 otherwise
////////////////////////////////////////////////////////////////////////

int HistEvent::Save( MemBuffer &file ) const {
  file.Write8( type );
  for ( int i = 0; i < 4; ++i ) file.Write16( data[i] );
  return 0;
}


////////////////////////////////////////////////////////////////////////
// NAME       : History::Load
// DESCRIPTION: Load a History from a file.
// PARAMETERS : file    - open file descriptor
//              mission - current mission
// RETURNS    : -1 on error, number of events loaded otherwise
////////////////////////////////////////////////////////////////////////

short History::Load( MemBuffer &file, Mission &mission ) {
  unsigned short num_events = file.Read16();

  if ( num_events > 0 ) {
    unsigned short num_units, i;
    num_units = file.Read16();

    for ( i = 0; i < num_events; ++i ) {
      HistEvent *he = new HistEvent();
      he->Load( file );
      events.AddTail( he );
    }

    for ( i = 0; i < num_units; ++i ) {
      Unit *u = mission.LoadUnit( file, true );
      if ( u ) units.AddTail( u );
    }
  }
  return num_events;
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::Save
// DESCRIPTION: Save a history to file.
// PARAMETERS : file    - open file descriptor
//              network - do some special processing if saving for
//                        sending over the wire in a network game
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int History::Save( MemBuffer &file, bool network /* = false */ ) const {
  unsigned short num;

  if ( network ) {
    num = 0;
    for ( HistEvent *he = static_cast<HistEvent *>( events.Head() );
          he; he = static_cast<HistEvent *>( he->Next() ) ) {
      // don't resend any events
      if ( !he->processed ) {
        if ( he->type == HIST_MOVE ||
             (he->type == HIST_UNIT && he->data[1] != HIST_UEVENT_DESTROY) ||
             he->type == HIST_ATTACK || he->type == HIST_COMBAT ||
             he->type == HIST_TRANSPORT_CRYSTALS || he->type == HIST_TRANSPORT_UNIT )
          ++num;
        else
          he->processed = true;
      }

    }
  } else num = events.CountNodes();

  file.Write16( num );
  if ( num == 0 ) return 0;

  file.Write16( units.CountNodes() );

  // save events
  for ( HistEvent *he = static_cast<HistEvent *>( events.Head() );
        he; he = static_cast<HistEvent *>( he->Next() ) )
    if ( !network || !he->processed )
      he->Save( file );

  // save units
  for ( Unit *u = static_cast<Unit *>( units.Head() );
        u; u = static_cast<Unit *>( u->Next() ) )
    u->Save( file );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::StartRecording
// DESCRIPTION: Create a new (empty) History from the current game.
// PARAMETERS : list - list of units
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void History::StartRecording( List &list ) {
  // create the unit copies
  for ( Unit *u = static_cast<Unit *>( list.Head() );
        u; u = static_cast<Unit *>( u->Next() ) ) {
    Unit *dummy = new Unit( *u );
    dummy->SetFlags( U_DUMMY );
    units.AddTail( dummy );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::Replay
// DESCRIPTION: Play all events that have been stored during the last
//              turn.
// PARAMETERS : mapwin - pointer to MapWindow
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void History::Replay( MapWindow *mapwin ) {
  if ( events.IsEmpty() ) return;

  View *view = mapwin->GetView();
  MapView *mv = mapwin->GetMapView();
  Map *map = mv->GetMap();
  ProgressWindow *progwin = NULL;
  HistEvent *he = static_cast<HistEvent *>( events.Head() );

  bool abort = false;
  bool replay = CFOptions.GetTurnReplay();
  bool quick = CFOptions.GetQuickReplay();

  if ( replay ) {
    List units_bak;
    BeginReplay( units_bak, map );

    if ( quick ) delay = 0;
    else {
      unsigned short count = events.CountNodes();

      // undo the map tile changes which have been recorded this turn
      while ( he && (he->type == HIST_TILE_INTERNAL) ) {
        map->SetHexType( he->data[1], he->data[2], he->data[0] );
        he = static_cast<HistEvent *>( he->Next() );
        --count;
      }

      delay = DEFAULT_DELAY;
      mv->Enable();
      mapwin->Draw();
      mapwin->Show();

      progwin = new ProgressWindow( mapwin->Width() / 4,
              mapwin->Height() - view->SmallFont()->Height() - 30,
              mapwin->Width() / 2, view->SmallFont()->Height() + 20,
              0, count, NULL, WIN_PROG_ABORT|WIN_PROG_DEFAULT, view );
    }

    while ( he && !abort ) {

      if ( he->type == HIST_MOVE )
        ReplayMoveEvent( *he, mapwin );
      else if ( he->type == HIST_COMBAT )
        ReplayCombatEvent( *he, mapwin );
      else if ( he->type == HIST_MSG )
        ReplayMessageEvent( *he, view );
      else if ( he->type == HIST_TILE )
        ReplayTileEvent( *he, mapwin );
      else if ( he->type == HIST_UNIT )
        ReplayUnitEvent( *he, mapwin );

      else if ( !quick ) {
        // these events are skipped completely for quick replays
        if ( he->type == HIST_ATTACK )
          ReplayAttackEvent( *he, mapwin );
      }

      if ( !quick ) {
        progwin->Advance( 1 );
        abort = progwin->Cancelled();
      }

      he = static_cast<HistEvent *>( he->Next() );
    }

    EndReplay( units_bak, map );
    if ( progwin ) view->CloseWindow( progwin );
  }


  if ( replay && !abort ) SDL_Delay( delay * 2 );
  else if ( !quick ) {
    // if replay has been stopped or disabled we still need to revert
    // all changes to the map and display the cached messages
    while ( he ) {
      if ( he->type == HIST_TILE ) map->SetHexType( he->data[1], he->data[2], he->data[0] );
      else if ( he->type == HIST_MSG ) ReplayMessageEvent( *he, view );
      he = static_cast<HistEvent *>( he->Next() );
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::BeginReplay
// DESCRIPTION: Prepare for viewing the last turn replay. We must store
//              the current list of units in a safe place and replace
//              them by their "historical" versions.
// PARAMETERS : backup - list to store current unit list in
//              map    - map
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void History::BeginReplay( List &backup, Map *map ) {
  List &gam_units = Gam->GetMission()->GetUnits();
  Unit *u;

  while ( !gam_units.IsEmpty() ) {
    u = static_cast<Unit *>( gam_units.RemHead() );
    backup.AddTail( u );
    if ( !u->IsSheltered() ) map->SetUnit( NULL, u->Position() );
  }

  Unit *next = NULL;
  for ( u = static_cast<Unit *>(units.Head()); u; u = next ) {
    next = static_cast<Unit *>( u->Next() );

    // we use the BUSY flag internally to signal units which do
    // not yet exist at the time replay is started
    if ( !u->IsBusy() ) {
      u->Remove();
      gam_units.AddTail( u );
      if ( !u->IsSheltered() ) map->SetUnit( u, u->Position() );
    }
  }

  lastunit = NULL;
  lastpos = Point(-1, -1);
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::EndReplay
// DESCRIPTION: End replay mode and initiate the next turn. Remove the
//              historical units and put the real ones back in place.
// PARAMETERS : backup - list containing the real units
//              map    - map
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void History::EndReplay( List &backup, Map *map ) {
  List &gam_units = Gam->GetMission()->GetUnits();
  Unit *u;

  while ( !gam_units.IsEmpty() ) {
    u = static_cast<Unit *>( gam_units.RemHead() );
    if ( !u->IsSheltered() ) map->SetUnit( NULL, u->Position() );
    delete u;
  }

  while ( !backup.IsEmpty() ) {
    u = static_cast<Unit *>( backup.RemHead() );
    gam_units.AddTail( u );
    if ( !u->IsSheltered() ) map->SetUnit( u, u->Position() );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::RecordMoveEvent
// DESCRIPTION: Record the movement of a unit from one hex to another,
//              neighboring hex.
// PARAMETERS : u   - unit to be moved
//              dir - direction to move the unit in
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int History::RecordMoveEvent( const Unit &u, Direction dir ) {
  HistEvent *he = new HistEvent;

  he->type = HIST_MOVE;
  he->data[0] = u.ID();
  he->data[1] = dir;
  he->data[2] = he->data[3] = -1;
  events.AddTail( he );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::RecordAttackEvent
// DESCRIPTION: Record an attack being inititated.
// PARAMETERS : u      - unit to inititate the attack
//              target - unit being attacked
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int History::RecordAttackEvent( const Unit &u, const Unit &target ) {
  HistEvent *he = new HistEvent;

  he->type = HIST_ATTACK;
  he->data[0] = u.ID();
  he->data[1] = target.Position().x;
  he->data[2] = target.Position().y;
  he->data[3] = -1;

  events.AddTail( he );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::RecordCombatEvent
// DESCRIPTION: Record the results of a combat.
// PARAMETERS : combat - combat data
//              loss1  - attackers' casualties
//              loss2  - defenders' casualties
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int History::RecordCombatEvent( const Combat &combat, unsigned char loss1,
                                unsigned char loss2 ) {
  HistEvent *he = new HistEvent;

  he->type = HIST_COMBAT;
  he->data[0] = combat.GetAttacker()->ID();
  he->data[1] = combat.GetDefender()->ID();
  he->data[2] = loss1;
  he->data[3] = loss2;

  events.AddTail( he );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::RecordTileEvent
// DESCRIPTION: Record a modification of the map. This event is special
//              in so far as two separate events are created for each
//              event. One allows for the initial map to be created from
//              the map at the end of the turn, the other represents the
//              actual map change.
// PARAMETERS : tile - tile type being set
//              old  - tile type being replaced
//              dx   - x hex being modified
//              dy   - y hex being modified
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int History::RecordTileEvent( unsigned short tile, unsigned short old,
                              short dx, short dy ) {
  HistEvent *he = new HistEvent;

  // first record the actual change
  he->type = HIST_TILE;
  he->data[0] = tile;
  he->data[1] = dx;
  he->data[2] = dy;
  he->data[3] = -1;
  events.AddTail( he );

  // now create the map state initializer event and put it at the
  // head of the list
  he = new HistEvent;
  he->type = HIST_TILE_INTERNAL;
  he->data[0] = old;
  he->data[1] = dx;
  he->data[2] = dy;
  he->data[3] = -1;
  events.AddHead( he );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::RecordMsgEvent
// DESCRIPTION: Record a message for the next player.
// PARAMETERS : title - title to use for the message window. If -1,
//                      used title will be "Message".
//              msg   - index of the message
//              pos   - location index of a hex on the map that should
//                      be shown when displaying the message
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int History::RecordMsgEvent( short title, unsigned short msg, short pos ) {
  HistEvent *he = new HistEvent;

  he->type = HIST_MSG;
  he->data[0] = title;
  he->data[1] = msg;
  he->data[2] = pos;
  he->data[3] = -1;

  events.AddTail( he );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::RecordUnitEvent
// DESCRIPTION: Record the creation or destruction of a unit.
// PARAMETERS : unit - created or destroyed unit
//              type - type of event (created, destroyed, repaired)
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int History::RecordUnitEvent( const Unit &unit, HistUnitEventType type ) {
  HistEvent *he = new HistEvent;

  he->type = HIST_UNIT;
  he->data[0] = unit.ID();
  he->data[1] = type;
  he->data[2] = he->data[3] = -1;

  events.AddTail( he );

  if ( type == HIST_UEVENT_CREATE ) {
    Unit *clone = new Unit( unit );
    clone->SetFlags( U_BUSY|U_DUMMY );
    units.AddTail( clone );
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::RecordTransportEvent
// DESCRIPTION: Record the (un)loading of crystals.
// PARAMETERS : source   - outer container to give/receive crystals
//              dest     - inner container to give/receive crystals
//              crystals - number of crystals to move from source to
//                         destination (may be negative)
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int History::RecordTransportEvent( const UnitContainer &source,
                            const Transport &dest, short crystals ) {
  HistEvent *he = new HistEvent;

  he->type = HIST_TRANSPORT_CRYSTALS;
  he->data[0] = dest.ID();
  he->data[1] = crystals;
  he->data[2] = he->data[3] = -1;
  events.AddTail( he );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::RecordTransportEvent
// DESCRIPTION: Record the movement of a unit from one container into
//              another (without leaving the current hex).
// PARAMETERS : source - outer container to remove unit from
//              dest   - inner container to add unit to
//              u      - unit to move from source to dest
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int History::RecordTransportEvent( const UnitContainer &source,
                            const Transport &dest, const Unit &u ) {
  HistEvent *he = new HistEvent;

  he->type = HIST_TRANSPORT_UNIT;
  he->data[0] = dest.ID();
  he->data[1] = u.ID();
  he->data[2] = he->data[3] = -1;
  events.AddTail( he );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::ReplayMoveEvent
// DESCRIPTION: Show the movement of a unit from one hex to another,
//              neighboring hex.
// PARAMETERS : event  - move event
//              mapwin - map window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void History::ReplayMoveEvent( const HistEvent &event, MapWindow *mapwin ) {
  MapView *mv = mapwin->GetMapView();
  Map *map = mv->GetMap();
  Mission *mission = Gam->GetMission();

  Unit *u = mission->GetUnit( event.data[0] );
  if ( u ) {
    if ( u != lastunit ) {
      if ( lastunit && lastunit->IsTransport() && !map->GetMapObject( lastpos ) ) {
        // we must adjust the position of carried units manually because
        // those pseudo-transports can't handle this themselves
        for ( Unit *carry = static_cast<Unit *>( mission->GetUnits().Head() );
              carry; carry = static_cast<Unit *>( carry->Next() ) ) {
          if ( carry->Position() == lastpos )
             carry->SetPosition( lastunit->Position().x, lastunit->Position().y );
        }
      }
      lastunit = u;
      lastpos = u->Position();

      SDL_Delay( delay );
    }

    mapwin->DisplayHex( u->Position() );

    if ( u->IsSheltered() ) {
      if ( map->GetUnit( u->Position() ) ) u->UnsetFlags( U_SHELTERED );
      else map->GetBuilding( u->Position() )->RemoveUnit( u );
    } else {
      map->SetUnit( NULL, u->Position() );
      mv->UpdateHex( u->Position() );
    }

    Gam->MoveUnit( u, (Direction)event.data[1] );
    map->SetUnit( u, u->Position() );
    if ( mv->Enabled() ) mapwin->Show( mv->UpdateHex( u->Position() ) );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::ReplayAttackEvent
// DESCRIPTION: Show a unit targetting another one.
// PARAMETERS : event  - attack event
//              mapwin - map window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void History::ReplayAttackEvent( const HistEvent &event, MapWindow *mapwin ) {
  MapView *mv = mapwin->GetMapView();
  Unit *u = Gam->GetMission()->GetUnit( event.data[0] );
  if ( u ) {

    if ( u != lastunit ) SDL_Delay( delay );
    if ( !mv->HexVisible( u->Position() ) ||
         !mv->HexVisible( Point(event.data[1], event.data[2]) ) ) {
      mv->CenterOnHex( Point((u->Position().x + event.data[1]) / 2,
                             (u->Position().y + event.data[2]) / 2) );
      mapwin->Show( *mv );
    }

    Point target( event.data[1], event.data[2]);
    mv->SetCursorImage( IMG_CURSOR_HIGHLIGHT );
    Rect upd = mv->SetCursor( u->Position() );
    mapwin->Show( upd );
    mv->SetCursorImage( IMG_CURSOR_ATTACK );
    upd = mv->SetCursor( target );
    mapwin->Show( upd );
    mapwin->FlashUnit ( target, 2 );
    mv->SetCursor( Point(-1,-1) );
    upd = mv->UpdateHex( u->Position() );
    mapwin->Show( upd );
    upd = mv->UpdateHex( target );
    mapwin->Show( upd );
    lastunit = u;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::ReplayMessageEvent
// DESCRIPTION: Display a message.
// PARAMETERS : event - message event
//              view  - current view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void History::ReplayMessageEvent( const HistEvent &event, View *view ) {
  MapWindow *mwin = Gam->GetMapWindow();
  Mission *m = Gam->GetMission();

  if ( mwin->GetMapView()->Enabled() && (event.data[2] >= 0) )
    mwin->DisplayHex( m->GetMap().Index2Hex( event.data[2] ) );

  MessageWindow *msgw = new MessageWindow(
    (event.data[0] != -1) ? m->GetMessage( event.data[0] ) : MSG(MSG_MESSAGE),
    m->GetMessage( event.data[1] ), view );
  msgw->EventLoop();
  view->CloseWindow( msgw );
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::ReplayTileEvent
// DESCRIPTION: Replay a tile switch.
// PARAMETERS : event  - tile event
//              mapwin - map window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void History::ReplayTileEvent( const HistEvent &event, MapWindow *mapwin ) {
  MapView *mv = mapwin->GetMapView();
  Map *map = mv->GetMap();
  Point pos( event.data[1], event.data[2] );

  if ( mv->Enabled() ) {
    const TerrainType *tt = Gam->GetMission()->GetTerrainSet().GetTerrainInfo( event.data[0] );
    mapwin->DisplayHex( pos );
    SDL_Delay( delay );
    mapwin->FadeInTerrain( tt->tt_image, pos );
    SDL_Delay( delay );
  }
  map->SetHexType( event.data[1], event.data[2], event.data[0] );
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::ReplayCombatEvent
// DESCRIPTION: Replay a fight switch.
// PARAMETERS : event  - combat event
//              mapwin - map window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void History::ReplayCombatEvent( const HistEvent &event, MapWindow *mapwin ) {

  // for the quick replay we may have to activate the display now
  MapView *mv = mapwin->GetMapView();
  if (!mv->Enabled()) {
    delay = DEFAULT_DELAY;
    mv->Enable();
    mapwin->Draw();
    mapwin->Show();
  }

  Combat cmb( Gam->GetMission()->GetUnit( event.data[0] ),
              Gam->GetMission()->GetUnit( event.data[1] ) );
  Point casualties( event.data[3], event.data[2] );
  Gam->ResolveBattle( &cmb, &casualties );
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::ReplayUnitEvent
// DESCRIPTION: Add a new unit to the map.
// PARAMETERS : event  - unit event
//              mapwin - map window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void History::ReplayUnitEvent( const HistEvent &event, MapWindow *mapwin ) {
  Mission *mission = Gam->GetMission();
  MapView *mv = mapwin->GetMapView();
  Map *map = mv->GetMap();

  Unit *u = GetDummy( event.data[0] );
  if ( u ) {
    if ( event.data[1] == HIST_UEVENT_CREATE ) { // create unit
      u->Remove();

      if ( !u->IsSheltered() ) {
        if ( mv->Enabled() ) {
          mapwin->DisplayHex( u->Position() );
          SDL_Delay( delay );
          mapwin->FadeInUnit( u->Image(), u->Position() );
          SDL_Delay( delay );
        }
        map->SetUnit( u, u->Position() );
      }
      mission->GetUnits().AddTail( u );
    } else if ( event.data[1] == HIST_UEVENT_DESTROY ) { // destroy unit
      if ( !u->IsSheltered() ) {
        map->SetUnit( NULL, u->Position() );
        if ( mv->Enabled() ) {
          mapwin->DisplayHex( u->Position() );
          SDL_Delay( delay );
          mapwin->FadeOutUnit( u->Image(), u->Position() );
          SDL_Delay( delay );
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::UndoMove
// DESCRIPTION: Erase all recorded movement and related events for a
//              specific unit.
// PARAMETERS : u - unit to erase movement for
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void History::UndoMove( const Unit &u ) {
  HistEvent *he = static_cast<HistEvent *>( events.Head() ), *tmp;

  while ( he ) {
    tmp = static_cast<HistEvent *>( he->Next() );

    if ( ((he->type == HIST_MOVE) ||
          (he->type == HIST_TRANSPORT_CRYSTALS) ||
          (he->type == HIST_TRANSPORT_UNIT)) &&
         (he->data[0] == u.ID()) ) {
      he->Remove();
      delete he;
    }

    he = tmp;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::GetDummy
// DESCRIPTION: Retrieve an internal unit definition.
// PARAMETERS : id - unit identifier
// RETURNS    : dummy for the requested unit, NULL if not found
////////////////////////////////////////////////////////////////////////

Unit *History::GetDummy( unsigned short id ) const {
  Unit *u;
  for ( u = static_cast<Unit *>( units.Head() );
        u && u->ID() != id; u = static_cast<Unit *>( u->Next() ) );
  return u;
}

////////////////////////////////////////////////////////////////////////
// NAME       : History::SetEventsProcessed
// DESCRIPTION: Set all events to processed.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void History::SetEventsProcessed( void ) const {
  for ( HistEvent *he = static_cast<HistEvent *>( events.Head() );
        he; he = static_cast<HistEvent *>( he->Next() ) ) {
    he->processed = true;
  }
}
