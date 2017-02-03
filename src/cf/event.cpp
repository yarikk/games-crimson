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
// event.cpp
////////////////////////////////////////////////////////////////////////

#include "event.h"
#include "game.h"
#include "extwindow.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Event::Load
// DESCRIPTION: Load an event from a file.
// PARAMETERS : file - file descriptor
// RETURNS    : identifier of the event owner, -1 on error
////////////////////////////////////////////////////////////////////////

short Event::Load( MemBuffer &file ) {
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
  e_player = 0;

  return file.Read8();
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
  file.Write8( e_player->ID() );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Event::Check
// DESCRIPTION: Check whether the event can be executed. The trigger
//              conditions and event dependencies must be met.
// PARAMETERS : -
// RETURNS    : TRUE if the activation conditions are met and the event
//              should be executed, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool Event::Check( void ) {
  bool rc = CheckTrigger();

  if ( rc ) {
    TLWList deps;
    deps.AddHead( new TLWNode( "", this, ID() ) );

    rc = CheckDependencies( deps );
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Event::CheckTrigger
// DESCRIPTION: Check whether the event trigger conditions are met.
// PARAMETERS : -
// RETURNS    : TRUE if trigger conditions met, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool Event::CheckTrigger( void ) {
  bool rc = Discarded();
  Mission *mis = Gam->GetMission();

  if ( !rc && !Disabled() ) {
    switch ( e_trigger ) {
    case ETRIGGER_TIMER:
      // the event numbers "turns" starting with 0 with two phases each turn
      rc = (mis->GetTime() >= e_tdata[0]);
      break;
    case ETRIGGER_UNIT_DESTROYED:
      if ( e_tdata[0] == -1 ) {                                // destroy all enemy units to score
        rc = ( mis->GetPlayer( e_tdata[1] ).Units(0) == 0 );
      } else if ( e_tdata[0] < -1 ) {                          // destroy all units of specified type
        unsigned char utype = -e_tdata[0] - 2;
        rc = true;
        for ( Unit *u = static_cast<Unit *>(mis->GetUnits().Head()); u;
              u = static_cast<Unit *>(u->Next()) ) {
          if ( u->Owner() && (u->Owner()->ID() == e_tdata[1]) &&
               (u->Type()->ID() == utype) && u->IsAlive() ) {
            rc = false;
            break;
          }
        }
      } else {                                                 // trigger if
        Unit *u = mis->GetUnit( e_tdata[0] );                  //  * unit not found
        rc = ( !u || !u->IsAlive() ||                          //  * unit found, but already dead
           (u->Owner() && (u->Owner()->ID() != e_tdata[1])) ); //  * owner != original owner (captured)
      }
      break;
    case ETRIGGER_HAVE_BUILDING:
      if ( (e_tdata[2] == -1) || (mis->GetTime() >= e_tdata[2]) ) {
        Building *b = mis->GetShop( e_tdata[0] );
        rc = ( b && b->Owner() && (b->Owner()->ID() == e_tdata[1]) );
      }
      break;
    case ETRIGGER_HAVE_CRYSTALS: {
      unsigned short crystals = 0;
      Building *b;
      if ( e_tdata[2] >= 0 ) {
        b = mis->GetShop( e_tdata[2] );
        if ( b && b->Owner() && (b->Owner()->ID() == e_tdata[1]) )
          crystals = b->Crystals();
      } else {
        for ( b = static_cast<Building *>(mis->GetShops().Head()); b;
              b = static_cast<Building *>(b->Next()) ) {
          if ( b->Owner() && (b->Owner()->ID() == e_tdata[1]) )
            crystals += b->Crystals();
        }

        if ( (e_tdata[2] == -2) &&
             (((e_tdata[0] < 0) && (crystals < -e_tdata[0])) ||
              ((e_tdata[0] > 0) && (crystals < e_tdata[0]))) ) {
          // also check units
          for ( Unit *u = static_cast<Unit *>(mis->GetUnits().Head()); u;
                u = static_cast<Unit *>(u->Next()) ) {
            if ( u->IsTransport() && !u->IsSheltered() &&
                 u->Owner() && (u->Owner()->ID() == e_tdata[1]) &&
                 u->IsAlive() ) {
              crystals += static_cast<Transport *>(u)->Crystals();
            }
          }
        }
      }
      rc = ((e_tdata[0] < 0) && (crystals < -e_tdata[0])) ||
            ((e_tdata[0] > 0) && (crystals >= e_tdata[0]));
      break; }
    case ETRIGGER_HAVE_UNIT:
      if ( (e_tdata[2] == -1) || (mis->GetTime() >= e_tdata[2]) ) {
        Unit *u = mis->GetUnit( e_tdata[0] );
        rc = ( u && u->Owner() && (u->Owner()->ID() == e_tdata[1]) );
      }
      break;
    case ETRIGGER_UNIT_POSITION: {
      Point loc = mis->GetMap().Index2Hex(e_tdata[1]);
      if ( e_tdata[0] < 0 ) {
        // -1 means any unit, -X is unit of type X - 2
        MapObject *mo = mis->GetMap().GetMapObject( loc );
        if ( mo && mo->Owner() && (mo->Owner()->ID() == e_tdata[2]) ) {
          if (e_tdata[0] == -1) {
            rc = mo->IsUnit() || (static_cast<Building *>(mo)->UnitCount() > 0);
          } else {
            unsigned short type = -e_tdata[0] - 2;
            UnitContainer *c = NULL;

            if ( mo->IsUnit() ) {
              Unit *u = static_cast<Unit *>(mo);
              if ( u->Type()->ID() == type ) rc = true;
              else if ( u->IsTransport() ) c = static_cast<Transport *>(u);
            } else c = static_cast<Building *>(mo);

            if ( c ) {
              for ( int i = c->UnitCount() - 1; i >= 0; --i ) {
                if ( c->GetUnit( i )->Type()->ID() == type ) {
                  rc = true;
                  break;
                }
              }
            }
          }
        }
      } else {
        Unit *u = mis->GetUnit( e_tdata[0] );
        rc = ( u && u->Owner() && (u->Owner()->ID() == e_tdata[2])
                 && (u->Position() == loc) );
      }
      break; }
    case ETRIGGER_HANDICAP:
      rc = ( (mis->GetHandicap() & e_tdata[0]) != 0 );
      break;
    }
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Event::CheckDependencies
// DESCRIPTION: Check whether event dependencies are met.
// PARAMETERS : deps - list of dependent events already checked (with
//                     positive results)
// RETURNS    : TRUE if all dependencies are met, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool Event::CheckDependencies( TLWList &deps ) {
  bool rc = true;

  if ( e_depend != -1 ) {

    // if the event does not exist anymore it has already been executed
    Event *dep = Gam->GetMission()->GetEvent( e_depend );
    if ( dep && !dep->Discarded() ) {

      // did we already check this event earlier in the cycle?
      if ( !deps.GetNodeByID( dep->ID() ) ) {
        if ( dep->CheckTrigger() ) {
          deps.AddTail( new TLWNode( "", dep, dep->ID() ) );
          rc = dep->CheckDependencies( deps );
        } else rc = false;
      }
    }

  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Event::Execute
// DESCRIPTION: Execute this event. What this means depends on the event
//              type.
// PARAMETERS : view - pointer to view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Event::Execute( View *view ) {
  Mission *mis = Gam->GetMission();
  MapWindow *mwin = Gam->GetMapWindow();
  MapView *mv = mwin->GetMapView();

  bool show_msg = true;

  switch ( e_type ) {
  case EVENT_CONFIGURE:
    switch ( e_data[0] ) {
    case 0:   // briefing player 1
    case 1:   // briefing player 2
      mis->GetPlayer( e_data[0] ).SetBriefing( e_data[1] );
      break;
    case 2:   // next map
      mis->SetSequel( e_data[1] );
      break;
    }
    break;
  case EVENT_CREATE_UNIT: {
    Point p = mis->GetMap().Index2Hex( e_data[1] );
    MapObject *mo = mis->GetMap().GetMapObject( p );

    if ( mo ) {
      const UnitType *type = mis->GetUnitSet().GetUnitInfo(e_data[0]);
      show_msg = (mo->Owner() == e_player)
        && ((mo->IsUnit() && static_cast<Unit *>(mo)->IsTransport())
             || !mo->IsUnit())
        && dynamic_cast<UnitContainer *>(mo)->Allow( type );
    }

    if ( show_msg ) {
      Unit *u = mis->CreateUnit( e_data[0], *e_player, p, (Direction)(e_data[2] & 0x07),
                                 (e_data[2] & 0x38) >> 3, (e_data[2] & 0x1C0) >> 6 );
      if ( u ) {
        u->UnsetFlags( U_DONE );
        if ( !u->IsSheltered() && mv->Enabled() ) {
          mwin->DisplayHex( p );
          mwin->FadeInUnit( u->Image(), p );
          mwin->Show( mv->UpdateHex( p ) );
        }
      }
    }
    break; }
  case EVENT_DESTROY_UNIT: {
    Unit *u;

    if ( e_data[0] >= 0 ) {
      u = mis->GetUnit( e_data[0] );
    } else {
      u = mis->GetMap().GetUnit( mis->GetMap().Index2Hex( e_data[2] ) );
    }

    if ( u && ((e_data[1] == -1) ||
        (u->Owner() && (e_data[1] == u->Owner()->ID())))) {
      Point p = u->Position();
      bool show = !u->IsSheltered() && mv->Enabled();

      if ( show ) mwin->DisplayHex( p );

      if ( mis->GetHistory() )
        mis->GetHistory()->RecordUnitEvent( *u, History::HIST_UEVENT_DESTROY );

      u->Hit( MAX_GROUP_SIZE );
      mis->GetMap().SetUnit( 0, p );

      if ( show ) {
        mwin->FadeOutUnit( u->Image(), p );
        mwin->Show( mv->UpdateHex( p ) );
      }
    } else show_msg = false;
    break; }
  case EVENT_MANIPULATE_EVENT: {
    Event *e = mis->GetEvent( e_data[0] );
    if ( e ) {
      if ( e_data[2] == 0 ) e->SetFlags( e_data[1] );
      else if ( e_data[2] == 1 ) e->UnsetFlags( e_data[1] );
      else if ( e_data[2] == 2 ) e->ToggleFlags( e_data[1] );
    }
    break; }
  case EVENT_MESSAGE:
    break;
  case EVENT_MINING: {
    Building *b = mis->GetShop( e_data[0] );
    if ( b ) {
      if ( e_data[2] == 0 ) b->SetCrystals( e_data[1] );
      else if ( e_data[2] == 1 ) b->ModifyCrystals( e_data[1] );
      else if ( e_data[2] == 2 ) b->SetCrystalProduction( e_data[1] );
      else if ( e_data[2] == 3 ) b->ModifyCrystalProduction( e_data[1] );
    } else show_msg = false;
    break; }
  case EVENT_RESEARCH: {
    Building *b = mis->GetShop( e_data[0] );
    if ( b && (b->Owner() == e_player) ) {
      if ( e_data[2] == 0 ) b->SetUnitProduction( 1 << e_data[1] );
      else b->UnsetUnitProduction( 1 << e_data[1] );
    } else show_msg = false;
    break; }
  case EVENT_SCORE:
    // only execute score events when score is below 100
    // this way we only get one message when multiple victory conditions
    // are met on a single turn
    if ( e_player->Success( 0 ) < 100 ) {
      e_player->Success( (signed char)e_data[0] );
      DisplayMessage( &mis->GetOtherPlayer(*e_player), e_data[1], e_data[2],
                      mis, true, view );
    } else show_msg = false;
    break;
  case EVENT_SET_HEX: {
    Point pos = mis->GetMap().Index2Hex( e_data[1] );
    if ( mv->Enabled() ) {
      const TerrainType *tt = mis->GetTerrainSet().GetTerrainInfo( e_data[0] );
      mwin->DisplayHex( pos );
      mwin->FadeInTerrain( tt->tt_image, pos );
    }

    Map &map = mis->GetMap();
    if ( mis->GetHistory() ) {
      mis->GetHistory()->RecordTileEvent( e_data[0],
                         map.HexTypeID( pos ), pos.x, pos.y );
    }
    map.SetHexType( pos.x, pos.y, e_data[0] );

    if ( mv->Enabled() ) mwin->Show( mv->UpdateHex( pos ) );
    break; }
  case EVENT_SET_TIMER: {
    Event *e = mis->GetEvent( e_data[0] );
    if ( e && (e->Trigger() == ETRIGGER_TIMER) ) {
      short time = e_data[1];
      if ( e_data[2] == 1 ) time += mis->GetTime();
      else if ( e_data[2] == 2 ) time += e->GetTData( 0 );
      e->SetTData( 0, time );
      e->UnsetFlags( EFLAG_DISABLED );
    }
    break; }
  }

  DisplayMessage( e_player, e_message, e_title, mis,
                  show_msg, view );

  if ( e_discard != -1 ) {
    Event *dis = mis->GetEvent( e_discard );
    if ( dis ) dis->Discard();
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Event::DisplayMessage
// DESCRIPTION: Check whether the message is to be displayed, and pop up
//              a MessageWindow if it is.
// PARAMETERS : p     - player to show message to
//              msg   - message index
//              title - title index (if -1 use player name)
//              m     - pointer to mission object
//              show  - if FALSE don't show (and don't record) message
//              view  - current display
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Event::DisplayMessage( Player *p, short msg, short title, Mission *m,
                            bool show, View *view ) const {
  if ( (msg == -1) || !p->IsInteractive() ) show = false;

  if ( show ) {
    MapWindow *mwin = Gam->GetMapWindow();
    if ( &m->GetPlayer() == p ) {
      const char *tstr;
      if ( title == -1 ) tstr = p->Name();
      else tstr = m->GetMessage(title);

      if ( mwin->GetMapView()->Enabled() ) {
        Point hex = GetFocus();
        if ( hex != Point(-1, -1) ) mwin->DisplayHex( hex );
      }

      MessageWindow *win = new MessageWindow( tstr, m->GetMessage(msg), view );
      win->Draw();
      win->Show();
      win->EventLoop();
      view->CloseWindow( win );
    } else {
      // if the other player is human (ie. we have a history) put the message
      // into the turn replay queue
      History *hist = m->GetHistory();
      if ( hist ) {
        Point hex = GetFocus();
        int hexidx = (hex == Point(-1, -1) ? -1 :  mwin->GetMapView()->GetMap()->Hex2Index(hex));
        hist->RecordMsgEvent( title, msg, hexidx );
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Event::GetFocus
// DESCRIPTION: When we display a message it would be nice to focus the
//              display on the portion of the map that relates to the
//              message (if any). This method tries to determine this
//              focal point.
// PARAMETERS : -
// RETURNS    : hex to focus on or -1/-1 if no such hex found
////////////////////////////////////////////////////////////////////////

Point Event::GetFocus( void ) const {
  Mission *mis = Gam->GetMission();
  Point p( -1, -1 );
  MapObject *mo;

  switch ( e_type ) {
  case EVENT_CREATE_UNIT:
    p = mis->GetMap().Index2Hex( e_data[1] );
    break;
  case EVENT_MESSAGE:
    p = mis->GetMap().Index2Hex( e_data[0] );
    break;
  case EVENT_MINING:
  case EVENT_RESEARCH:
    mo = mis->GetShop( e_data[0] );
    if ( mo ) p = mo->Position();
    break;
  case EVENT_SET_HEX:
    p = mis->GetMap().Index2Hex( e_data[1] );
    break;
  }

  if ( p == Point( -1, -1 ) ) {
    switch ( e_trigger ) {
    case ETRIGGER_UNIT_DESTROYED:
      if ( e_tdata[0] >= 0 ) {
        mo = mis->GetUnit( e_tdata[0] );
        if ( mo ) p = mo->Position();
      }
      break;
    case ETRIGGER_HAVE_BUILDING:
      mo = mis->GetShop( e_tdata[0] );
      if ( mo ) p = mo->Position();
      break;
    case ETRIGGER_HAVE_CRYSTALS:
      if ( e_tdata[0] >= 0 ) {
        mo = mis->GetShop( e_tdata[0] );
        if ( mo ) p = mo->Position();
      }
      break;
    case ETRIGGER_HAVE_UNIT:
      mo = mis->GetUnit( e_tdata[0] );
      if ( mo ) p = mo->Position();
      break;
    case ETRIGGER_UNIT_POSITION:
      p = mis->GetMap().Index2Hex(e_tdata[1]);
      break;
    }
  }
  return p;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Event::Discard
// DESCRIPTION: Discard this event, ie. disable it and mark it for
//              deletion. Also recursively discard another event if told
//              to do so.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Event::Discard( void ) {
  if ( !Discarded() ) {
    SetFlags( EFLAG_DISCARDED );

    if ( e_discard != -1 ) {
      Event *dis = Gam->GetMission()->GetEvent( e_discard );
      if ( dis ) dis->Discard();
    }
  }
}

