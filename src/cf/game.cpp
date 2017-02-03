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
// game.cpp
////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string.h>

#include "game.h"
#include "path.h"
#include "unitwindow.h"
#include "gamewindow.h"
#include "filewindow.h"
#include "initwindow.h"
#include "fileio.h"
#include "sound.h"
#include "ai.h"
#include "options.h"
#include "strutil.h"
#include "msgs.h"

extern Options CFOptions;

// button identifiers used by the game hook function dispatcher
enum {
  G_BUTTON_END_TURN = 10,
  G_BUTTON_MAP,
  G_BUTTON_BRIEFING,
  G_BUTTON_SAVE,
  G_BUTTON_LEV_INFO,
  G_BUTTON_GENERAL_OPTIONS,
  G_BUTTON_LANGUAGE_OPTIONS,
  G_BUTTON_VIDEO_OPTIONS,
  G_BUTTON_SOUND_OPTIONS,
  G_BUTTON_KEYBOARD_OPTIONS,
  G_BUTTON_ABORT,
  G_BUTTON_QUIT,

  G_BUTTON_SAVE_AND_SHUTDOWN,
  G_BUTTON_SHUTDOWN,

  G_BUTTON_UNIT_INFO,
  G_BUTTON_UNIT_CONTENT,
  G_BUTTON_UNIT_SWEEP,
  G_BUTTON_UNIT_UNDO,
  G_BUTTON_MINIMIZE_WINDOW
};

enum {
  G_KEY_QUIT = 0,
  G_KEY_INFO,
  G_KEY_MAP,
  G_KEY_END_TURN,
  G_KEY_CONTENT,
  G_KEY_SWEEP
};

#ifndef DISABLE_NETWORK
class NetworkProgressWindow : public ProgressWindow {
public:
  NetworkProgressWindow( unsigned short w, unsigned short h,
                         const char *msg, View *view );

  bool Cancelled( void );

private:
  bool cancelled;
};

NetworkProgressWindow::NetworkProgressWindow( unsigned short w,
                       unsigned short h, const char *msg, View *view ) :
       ProgressWindow( 0, 0, w, h, 0, 100, msg,
                       WIN_CENTER|WIN_PROG_ABORT, view ), cancelled(false) {}

bool NetworkProgressWindow::Cancelled( void ) {
  if ( !cancelled ) {
    if ( Get() < 100 ) Advance( 1 );
    else Set( 0 );

    bool cancel = ProgressWindow::Cancelled();

    if ( cancel ) {
      // ask for confirmation
      string buttons;
      buttons.append( MSG(MSG_B_YES) );
      buttons += '|';
      buttons.append( MSG(MSG_B_NO) );
      DialogWindow *req = new DialogWindow( NULL,
          MSG(MSG_ASK_ABORT_NETWORK), buttons, 1, 0, view );
      req->SetButtonID( 1, 0 );
      cancelled = (req->EventLoop() == GUI_CLOSE);
      view->CloseWindow( req );
    }
  }

  return cancelled;
}
#endif

////////////////////////////////////////////////////////////////////////
// NAME       : Game::Game
// DESCRIPTION: Create a new game.
// PARAMETERS : view - the display surface
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Game::Game( View *view ) : mission(0), mwin(0), unit(0), shader(0), view(view) {
  InitKeys();

#ifndef DISABLE_NETWORK
  peer = NULL;
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::~Game
// DESCRIPTION: Destroy the game.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Game::~Game( void ) {
  if ( mwin ) view->CloseWindow( mwin );
  delete mission;
  delete shader;

#ifndef DISABLE_NETWORK
  delete peer;
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::InitKeys
// DESCRIPTION: Initialize the array of keyboard commands. These
//              shortcuts change with the locale the user selected.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::InitKeys( void ) {
  short labels[] = { MSG_B_QUIT, MSG_B_UNIT_INFO, MSG_B_MAP, MSG_B_END_TURN,
                     MSG_B_UNIT_CONTENT, MSG_B_UNIT_SWEEP };
  for ( int i = 0; i <= G_KEY_SWEEP; ++i ) {
    string label( MSG(labels[i]) );
    size_t pos = label.find( '_' );
    if ( pos != string::npos ) keys[i] = StringUtil::utf8chartoascii(&label[pos+1]);
    else keys[i] = 0;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::InitWindows
// DESCRIPTION: Initialize the game windows. We need a valid mission
//              loaded to do that.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::InitWindows( void ) {
  if ( mission ) {
    mwin = new MapWindow( 0, 0, view->Width(), view->Height(), 0, view );
    if ( CFOptions.GetDamageIndicator() ) mwin->GetMapView()->EnableUnitStats();
    mwin->GetMapView()->SetMap( &mission->GetMap() );  // attach map to map window
    shader = new MoveShader( &mission->GetMap(), mission->GetUnits(),
                             mwin->GetMapView()->GetFogBuffer() );
    ExecPreStartEvents();
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::Quit
// DESCRIPTION: Ask the user for confirmation and exit.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::Quit( void ) const {
  Audio::PlaySfx( Audio::SND_GUI_ASK, 0 );
  string buttons;
  buttons.append( MSG(MSG_B_YES) );
  buttons += '|';
  buttons.append( MSG(MSG_B_NO) );

  DialogWindow *req = new DialogWindow( NULL, MSG(MSG_ASK_QUIT),
                      buttons, 1, 0, view );
  req->SetButtonID( 0, GUI_QUIT );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::CreateSaveFileName
// DESCRIPTION: Get the name of the file to save to. Pop up a file
//              requester if necessary.
// PARAMETERS : filename - default filename
// RETURNS    : file name or empty string on error
////////////////////////////////////////////////////////////////////////

string Game::CreateSaveFileName( const char *filename ) const {
  string file;
  string yesno;
  yesno.append( MSG(MSG_B_YES) );
  yesno += '|';
  yesno.append( MSG(MSG_B_NO) );

  if ( filename ) file.append( filename );

  bool filesel;
  do {
    GUI_Status rc;
    DialogWindow *dw;
    filesel = false;

    if ( file.length() == 0 ) filesel = true;
    else if ( File::Exists( file ) ) {
      // if file exists let user confirm the write
      string conmsg = StringUtil::strprintf( MSG(MSG_ASK_OVERWRITE), file );

      dw = new DialogWindow( NULL, conmsg, yesno, 1, 0, view );
      dw->SetButtonID( 0, 1 );
      dw->SetButtonID( 1, 0 );
      rc = dw->EventLoop();
      if ( rc == 0 ) filesel = true;
      view->CloseWindow( dw );
    }

    if ( filesel ) {
      bool done = false;
      FileWindow *fw = new FileWindow( get_save_dir().c_str(), last_file_name.c_str(),
                                       ".sav", WIN_FILE_SAVE, view );
      fw->ok->SetID( 1 );
      fw->cancel->SetID( 0 );

      do {
        rc = fw->EventLoop();

        if ( rc == 1 ) {
          file = fw->GetFile();
          if ( file.length() != 0 ) {
            view->CloseWindow( fw );
            done = true;
          }
        } else if ( rc == 0 ) {
          if ( mission->GetFlags() & GI_PBEM ) {
            // if saving a PBeM game is aborted the game data is lost...
            dw = new DialogWindow( MSG(MSG_WARNING), MSG(MSG_ASK_ABORT_PBEM),
                                   yesno, 1, WIN_FONT_BIG, view );
            Audio::PlaySfx( Audio::SND_GUI_ASK, 0 );
            dw->SetButtonID( 0, 1 );
            dw->SetButtonID( 1, 0 );
            rc = dw->EventLoop();
            view->CloseWindow( dw );

            if ( rc != 0 ) {
              view->CloseWindow( fw );
              file = "";
              done = true;
              filesel = false;
            }
          } else {
            view->CloseWindow( fw );
            file.assign( "" );
            done = true;
            filesel = false;
          }
        }

      } while ( !done );
    }
  } while ( filesel );

  return file;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::Save
// DESCRIPTION: Save the current game to a file.
// PARAMETERS : filename - data file name; if NULL pops up a file
//                         selection window
// RETURNS    : 0 on successful write, non-zero otherwise
////////////////////////////////////////////////////////////////////////

int Game::Save( const char *filename ) {
  string fname( CreateSaveFileName( filename ) );
  if ( fname.length() == 0 ) return -1;

  File file( fname );
  if ( !file.Open( "wb", false ) ) {
    new NoteWindow( MSG(MSG_ERROR), MSG(MSG_ERR_WRITE), 0, view );
    return -1;
  }

  // set last filename
  last_file_name = file_part( fname );
  unsigned int num = last_file_name.rfind( '.' );
  last_file_name.erase( num );

  unsigned short mflags = mission->GetFlags();
  mission->SetFlags( mflags|GI_SAVEFILE );
  mission->Save( file );
  file.Close();

  if ( mflags & GI_PBEM ) {
    string msg = StringUtil::strprintf( MSG(MSG_GAME_SAVED_PBEM), fname );
    NoteWindow *nw = new NoteWindow( MSG(MSG_GAME_SAVED), msg, 0, view );
    nw->SetButtonID( 0, G_BUTTON_SHUTDOWN );
    nw->SetButtonHook( 0, this );
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::Load
// DESCRIPTION: Load a game from a mission file (start a new game) or
//              a save file (resume game).
// PARAMETERS : filename - data file name
// RETURNS    : 0 on success, -1 otherwise
////////////////////////////////////////////////////////////////////////

int Game::Load( const char *filename ) {
  File file( filename );
  if ( !file.Open( "rb" ) ) return -1;

  int rc = Load( file );
  if ( rc != -1 ) {
    // set last filename
    last_file_name = file_part( filename );
    unsigned int i = last_file_name.rfind( '.' );
    last_file_name.erase( i );
  } else {
    cerr << "Error loading " << filename << endl;
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::Load
// DESCRIPTION: Load a game from a buffer.
// PARAMETERS : buffer - data buffer
// RETURNS    : 0 on success, -1 otherwise
////////////////////////////////////////////////////////////////////////

int Game::Load( MemBuffer &buffer ) {
  int rc = -1;

  mission = new Mission();
  if ( mission->Load( buffer ) != -1 ) {

    mission->SetLocale( CFOptions.GetLanguage() );

    unsigned short flags = mission->GetFlags();
    if ( flags & GI_SAVEFILE ) {
      // update CFOptions
      if ( flags & GI_CAMPAIGN ) CFOptions.SetCampaign( true );
      if ( flags & GI_AI ) CFOptions.SetGameType( GTYPE_AI );
      else if ( flags & GI_PBEM ) CFOptions.SetGameType( GTYPE_PBEM );
      else if ( flags & GI_NETWORK ) CFOptions.SetGameType( GTYPE_NET_SERVER );
    } else {
      // if this is a new map (GI_SAVEFILE not set) GI_SKIRMISH,
      // GI_CAMPAIGN, and GI_AI only serve as information
      flags &= ~(GI_AI|GI_CAMPAIGN|GI_SKIRMISH);
      flags |= ((CFOptions.IsPBEM() ? GI_PBEM : 0)|
               (CFOptions.IsNetwork() ? GI_NETWORK : 0)|
               (CFOptions.IsAI() ? GI_AI : 0)|
               (CFOptions.GetCampaign() ? GI_CAMPAIGN : 0));
      mission->SetFlags( flags );
    }

    rc = 0;

#ifdef DISABLE_NETWORK
    if ( flags & GI_NETWORK ) {
      cerr << "Error: " << PROGRAMNAME << " was compiled without networking support" << endl;
      rc = -1;

      delete mission;
      mission = NULL;
    }
#endif
  } else {
    delete mission;
    mission = NULL;
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::SwitchMap
// DESCRIPTION: Dump the current map and move on to another one.
// PARAMETERS : sequel - map file name (excluding path and suffix)
// RETURNS    : -1 on error, 0 otherwise
////////////////////////////////////////////////////////////////////////

int Game::SwitchMap( const char *sequel ) {
  string mapname = get_levels_dir();
  mapname.append( sequel );
  mapname.append( ".lev" );

  mwin->GetMapView()->Disable();
  mwin->Draw();
  mwin->Show();

  unsigned char p1type = mission->GetPlayer(PLAYER_ONE).Type();
  unsigned char p2type = mission->GetPlayer(PLAYER_TWO).Type();
  unsigned char handicap = mission->GetHandicap();

  // remove current game data
  delete shader;
  shader = NULL;
  delete mission;
  mission = NULL;

  // reset variables
  unit = NULL;

  // load new map
  int rc = Load( mapname.c_str() );
  if ( !rc ) {
    mission->GetPlayer(PLAYER_ONE).SetType( p1type );
    mission->GetPlayer(PLAYER_TWO).SetType( p2type );
    mission->SetHandicap( handicap );
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::StartTurn
// DESCRIPTION: Start a turn, i.e. draw the map to the display.
// PARAMETERS : -
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status Game::StartTurn( void ) {
  GUI_Status rc = GUI_OK;
  MapView *mv = mwin->GetMapView();
  Player &player = mission->GetPlayer();

  // if music is already playing this will do nothing
  Audio::PlayMusic( mission->GetMusic() ? mission->GetMusic() : CF_MUSIC_DEFAULT );

  view->SetFGPen( player.LightColor() );   // set player color scheme
  view->SetBGPen( player.DarkColor() );

  if ( player.IsInteractive() ) {
    NoteWindow *nw;

    // show turn information and greeting dialog
    string turnmsg( MSG(MSG_TURN) );
    turnmsg += ' ';
    turnmsg.append( StringUtil::tostring(mission->GetTurn()) );

    nw = new NoteWindow( player.Name(), turnmsg, WIN_FONT_BIG|WIN_CENTER, view );
    nw->EventLoop();
    view->CloseWindow( nw );

    // in email games set up or check player passwords
    if ( mission->GetFlags() & GI_PBEM ) {
      if ( !player.Password() ) {
        if ( !SetPlayerPassword( &player ) ) return GUI_RESTART;
      } else if ( !CheckPassword( player.Name(), MSG(MSG_ENTER_PASSWORD),
                  player.Password(), 2 ) )
        return GUI_RESTART;
    }
  }

  undo.Disable();

  if ( mission->GetPhase() == TURN_START ) {
    // replay
    History *history = mission->GetHistory();
    if ( history ) {
      if ( player.IsInteractive() ) history->Replay( mwin );
      mission->SetHistory( NULL );
      delete history;
    }

    // begin new turn
    if ( mission->GetOtherPlayer(player).IsHuman() ) {
      history = new History();
      history->StartRecording( mission->GetUnits() );
      mission->SetHistory( history );
    }

    // check for victory conditions
    if ( mission->GetFlags() & GI_GAME_OVER )
      return ShowDebriefing( player, true );

    mission->SetPhase( TURN_IN_PROGRESS );
    mv->SetCursorImage( IMG_CURSOR_IDLE );
  }

  if ( !player.IsHuman() ) {
    CheckEvents();
    AI ai( *mission );
    ai.Play();
    rc = EndTurn();

#ifndef DISABLE_NETWORK
  } else if ( player.IsRemote() ) {
    // show message while waiting
    NetworkProgressWindow *pw = new NetworkProgressWindow(
        view->Width() * 2 / 3, view->SmallFont()->Height() + 20,
        MSG(MSG_NET_WAITING), view );
    DynBuffer *updates = peer->Receive( pw );
    if ( updates ) {
      History remote;
      remote.Load( *updates, *mission );

      CheckEvents();
      Execute( remote );
    } else {
      if ( pw->Cancelled() ) {
        // user aborted, so no error
        return GUI_RESTART;
      } else {
        HandleNetworkError();
        return GUI_OK;
      }
    }

    view->CloseWindow( pw );

    // make some noise so the local player know it's his turn now
    Audio::PlaySfx( Audio::SND_GUI_ASK, 0 );

    rc = EndTurn();
#endif

  } else {

    mv->Enable();
    mv->SetFlags( MV_DIRTY );
    CheckEvents();
    mv->UnsetFlags( MV_DIRTY );

    // set the cursor to one of the player's units
    Point startcursor( 0, 0 );
    for ( Unit *u = static_cast<Unit *>(mission->GetUnits().Head());
          u; u = static_cast<Unit *>(u->Next()) ) {
      if ( u->Owner() == &player ) {
        startcursor = u->Position();
        break;
      }
    }

    view->DisableUpdates();
    mv->CenterOnHex( startcursor );
    SetCursor( startcursor );
    view->EnableUpdates();
    view->Refresh();
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::SetPlayerPassword
// DESCRIPTION: Set a password for a player in an email game.
// PARAMETERS : player - player to set password for
// RETURNS    : TRUE if password successfully set, FALSE on error
////////////////////////////////////////////////////////////////////////

bool Game::SetPlayerPassword( Player *player ) const {
  bool pw_ok = false;
  PasswordWindow *pwin;
  const char *pw, *msg1 = MSG(MSG_CHOOSE_PASSWORD);

  pwin = new PasswordWindow( player->Name(), msg1,
                             player->Password(), false, view );

  do {
    pwin->EventLoop();
    pw = pwin->string->String();

    // only accept if player has entered a password
    if ( pw ) {
      pwin->NewPassword( pw );
      pwin->string->SetTitle( MSG(MSG_CONFIRM_PASSWORD) );
      pwin->string->SetString( NULL );
      pwin->Draw();
      pwin->Show();
      pwin->string->SetFocus();

      if ( pwin->EventLoop() == GUI_RESTART ) break;

      pw_ok = pwin->PasswordOk();

      if ( !pw_ok ) {
        Audio::PlaySfx( Audio::SND_GUI_ERROR, 0 );
        pwin->NewPassword( NULL );
        pwin->string->SetTitle( msg1 );
        pwin->string->SetString( NULL );
        pwin->Draw();
        pwin->Show();
        pwin->string->SetFocus();
      }
    } else pwin->string->SetFocus();
  } while ( !pw_ok );

  player->SetPassword( pw );
  view->CloseWindow( pwin );

  return pw_ok;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::CheckPassword
// DESCRIPTION: Request a password from the player and compare it to a
//              given string.
// PARAMETERS : title - window title
//              msg   - message
//              pw    - password to check against
//              retry - number of retries if player enters a wrong
//                      password
// RETURNS    : TRUE if pw and player's password match, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool Game::CheckPassword( const char *title, const char *msg,
                          const char *pw, short retry ) const {
  PasswordWindow *pwin = new PasswordWindow( title, msg, pw, true, view );
  bool pw_ok = false;

  do {
    if ( pwin->EventLoop() == GUI_RESTART ) break;
    pw_ok = pwin->PasswordOk();

    if ( !pw_ok ) {
      Audio::PlaySfx( Audio::SND_GUI_ERROR, 0 );
      if ( retry ) {
        pwin->string->SetString( NULL, true );
        pwin->string->SetFocus();
      }
    }
  } while ( (--retry >= 0) && !pw_ok );

  view->CloseWindow( pwin );
  return pw_ok;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::CheckEvents
// DESCRIPTION: Check for pending game events.
// PARAMETERS : -
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status Game::CheckEvents( void ) {
  Event *e = static_cast<Event *>( mission->GetEvents().Head() ), *e2;

  while ( e ) {
    e2 = static_cast<Event *>( e->Next() );
    if ( e->Discarded() ) {
      e->Remove();
      delete e;
    } else if ( e->Check() ) {
      // event will be executed and can be taken out of the queue
      e->Remove();
      e->Execute( view );
      delete e;

      // after events have been triggered, undo is no longer allowed
      undo.Disable();
    }
    e = e2;
  }
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::ExecPreStartEvents
// DESCRIPTION: This method is called before the very first turn. It is
//              used to execute initial ETRIGGER_HANDICAP events without
//              affecting the turn history (this means e.g. that units
//              created due to handicaps to not appear in turn replays).
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::ExecPreStartEvents( void ) {
  if ( (mission->GetTime() == 0) && (mission->GetPhase() == TURN_START) ) {
    Event *e = static_cast<Event *>( mission->GetEvents().Head() ), *e2;
    while ( e ) {
      e2 = static_cast<Event *>( e->Next() );
      if ( (e->Trigger() == ETRIGGER_HANDICAP) && e->Check() ) {
        e->Remove();
        e->Execute( view );
        delete e;
      }
      e = e2;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::HaveWinner
// DESCRIPTION: Check whether one of the players has won completed his
//              mission. Notify players and quit the current game if so.
// PARAMETERS : -
// RETURNS    : true if mission is completed by any player AND both
//              players have been informed so, false otherwise
////////////////////////////////////////////////////////////////////////

bool Game::HaveWinner( void ) {
  bool quit = false;

  if ( (mission->GetPlayer(PLAYER_ONE).Success( 0 ) >= 100) ||
       (mission->GetPlayer(PLAYER_TWO).Success( 0 ) >= 100) ) {
    mission->SetFlags( mission->GetFlags()|GI_GAME_OVER );

    Player &p = mission->GetPlayer();
    quit = !mission->GetOtherPlayer(p).IsHuman();

    if ( ShowDebriefing( p, quit ) != GUI_OK ) quit = true;
  }
  return quit;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::EndTurn
// DESCRIPTION: End turn for the current player. Execute combat orders
//              and prepare everything for the next player.
// PARAMETERS : -
// RETURNS    : GUI_Status (GUI_QUIT if mission is complete)
////////////////////////////////////////////////////////////////////////

GUI_Status Game::EndTurn( void ) {
  GUI_Status rc = GUI_OK;
  List &battles = mission->GetBattles();

  if ( unit ) DeselectUnit();
  mwin->GetMapView()->DisableCursor();
  mwin->GetPanel()->Update(NULL);

  // calculate combat modifiers
  Combat *com = static_cast<Combat *>( battles.Head() );
  while ( com ) {
    com->CalcModifiers( mission->GetMap() );
    com = static_cast<Combat *>( com->Next() );
  }

  // execute combat orders
  while ( !battles.IsEmpty() ) {
    Combat *com = static_cast<Combat *>(battles.RemHead());
    ResolveBattle( com );
    delete com;
  }

  // destroyed units may have triggered events...
  rc = CheckEvents();

  // check for mission completion
  if ( !HaveWinner() ) {
    // set new player
    Player &p = mission->NextPlayer();
    if ( p.ID() == PLAYER_ONE ) mission->NextTurn();
    mission->SetPhase( TURN_START );

    // remove all destroyed units from the list,
    // restore movement points, reset status
    Unit *next, *u = static_cast<Unit *>(mission->GetUnits().Head());
    while ( u ) {
      next = static_cast<Unit *>(u->Next());
      if ( !u->IsAlive() ) {
        u->Remove();
        delete u;
      } else if ( u->Owner() != &p ) {
        u->UnsetFlags( U_MOVED|U_ATTACKED|U_DONE|U_BUSY );
      }
      u = next;
    }

    // produce crystals in mines
    Building *b = static_cast<Building *>(mission->GetShops().Head());
    while ( b ) {
      if ( (b->Owner() == &p) && b->IsMine() )
        b->SetCrystals( b->Crystals() + b->CrystalProduction() );
      b = static_cast<Building *>(b->Next());
    }

    // check if we're playing an email game. if so, save and exit
    if ( mission->GetFlags() & GI_PBEM ) {
      string filebuf( get_save_dir() );
      filebuf.append( last_file_name );
      filebuf.append( ".sav" );

      int err = Save( filebuf.c_str() );
      if ( err ) {
        NoteWindow *nw = new NoteWindow( MSG(MSG_ERROR), MSG(MSG_ERR_SAVE), WIN_CLOSE_ESC, view );
        nw->SetButtonID( 0, GUI_RESTART );
      }
    } else {

#ifndef DISABLE_NETWORK
      if ( p.IsRemote() ) {
        // send data to peer
        History *history = mission->GetHistory();

        if ( history ) {
          DynBuffer buf;
          history->Save( buf, true );
          if ( !peer->Send( buf ) ) {
            HandleNetworkError();
            return GUI_OK;
          }
        } else {
          cerr << "Error: No history available in network game" << endl;
          return GUI_ERROR;
        }
      }
#endif

      mwin->GetMapView()->Disable();
      mwin->Draw();
      mwin->Show();

      rc = StartTurn();
    }
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::ResolveBattle
// DESCRIPTION: Calculate and display battle outcomes for all clashes
//              queued.
// PARAMETERS : com    - battle to resolve
//              result - optional precalculated battle results (hits
//                       scored by attacker/defender, NULL to calculate)
//
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

void Game::ResolveBattle( Combat *com, const Point *result /* = NULL */ ) {
  Unit *att = com->GetAttacker();
  Unit *def = com->GetDefender();
  if ( !att->IsAlive() || !def->IsAlive() ) return;

  // calculate modifiers for combat
  Map &map = mission->GetMap();
  History *hist = mission->GetHistory();
  CombatWindow *cwin = NULL;

  if ( mission->GetPlayer().IsInteractive() )
    cwin = new CombatWindow( com, mwin, view );

  Point apos( att->Position() ), dpos( def->Position() );
  Point hits;

  if ( result == NULL )
    hits = com->CalcResults();
  else
    hits = com->CalcResults( result->x, result->y );

  // record as a combat event
  if ( hist && !att->IsDummy() )
    hist->RecordCombatEvent( *com, hits.y, hits.x );

  if ( !att->IsAlive() ) map.SetUnit( NULL, apos );
  if ( !def->IsAlive() ) map.SetUnit( NULL, dpos );

  if ( cwin ) {
    cwin->Draw();
    cwin->Show();
    cwin->EventLoop();
    view->CloseWindow( cwin );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::SetCursor
// DESCRIPTION: Set the cursor to a new hex on the map. Contrary to the
//              low-level function in MapView this updates the display
//              at the old and new position if necessary.
// PARAMETERS : cursor - new cursor position
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::SetCursor( const Point &cursor ) const {
  MapView *mv = mwin->GetMapView();
  MapObject *mobj = NULL;
  Unit *u;
  Rect upd;
  Player *player = &mission->GetPlayer();
  Map *map = &mission->GetMap();

  if ( mv->CursorEnabled() ) {
    Point old = mv->Cursor();

    upd = mv->SetCursor( Point(-1,-1) ); // disable cursor for hex update
    mwin->Show( upd );                   // update previous cursor position

    if ( player->Mode() == MODE_IDLE ) {
      // if we had highlighted a unit's target we need to remove that mark
      const Point *target;
      u = map->GetUnit( old );
      if ( u && (u->Owner() == player) && (target = u->Target()) ) {
        upd = mv->UpdateHex( *target );
        mwin->Show( upd );
      }
    } else if ( player->Mode() == MODE_BUSY ) mv->SetCursorImage( IMG_CURSOR_SELECT );
  }

  if ( cursor.x != -1 ) {
    u = map->GetUnit( cursor );
    if ( u ) {
      if ( u->Owner() != player ) {
        if ( (player->Mode() == MODE_BUSY) && unit->CanHit( u ) )
          mv->SetCursorImage( IMG_CURSOR_ATTACK );
      } else if ( player->Mode() == MODE_IDLE ) {
        // if it's a unit of the active player, highlight its target if it has one
        const Point *target = u->Target();
        if ( target ) {
          Point p = mv->Hex2Pixel( *target );
          mv->DrawTerrain( IMG_CURSOR_HIGHLIGHT, mwin, p.x, p.y, *mwin);
          mwin->Show( Rect(p.x, p.y, mv->TileWidth(), mv->TileHeight()) );
        }
      }
    }

    mobj = map->GetMapObject( cursor );
  }
  upd = mv->SetCursor( cursor );
  mwin->GetPanel()->Update( mobj );
  mwin->Show( upd );
}


////////////////////////////////////////////////////////////////////////
// NAME       : Game::MoveUnit
// DESCRIPTION: Move a unit to another hex.
// PARAMETERS : u  - unit to be moved
//              hx - destination hex x
//              hy - destination hex y
// RETURNS    : the unit if it's still available, or NULL if it
//              cannot be selected this turn (read: deselect it)
////////////////////////////////////////////////////////////////////////

Unit *Game::MoveUnit( Unit *u, const Point &dest ) {
  if ( u->Position() != dest ) {
    MapView *mv = mwin->GetMapView();
    Map *map = mv->GetMap();
    const Point &pos = u->Position();

    if ( mission->GetPlayer().IsHuman() ) {
      if ( shader->GetStep(dest) == -1 ) return u;

      // if we're moving a transport out of a shop/transport check
      // whether the player wants to take other units with him
      if ( u->IsTransport() && u->IsSheltered() ) {
        MapObject *pobj = map->GetMapObject( pos );
        UnitContainer *parent;
        if ( pobj->IsUnit() ) parent = static_cast<Transport *>(pobj);
        else parent = static_cast<Building *>(pobj);
        UnitLoadWindow *ulw = new UnitLoadWindow(
            *static_cast<Transport *>(u), *parent,
            *map->GetUnitSet(), *map->GetTerrainSet(),
            mission->GetHistory(), view );

        bool aborted = false;
        if ( ulw->Opened() ) aborted = (ulw->EventLoop() == GUI_CLOSE);
        view->CloseWindow( ulw );

        if ( aborted ) {
          DeselectUnit();
          ContainerContent( parent );
          return u;
        }
      }
    }

    Path path( map );
    if ( path.Find( u, pos, dest ) == -1 ) {
      cerr << "Internal error: FindPath() failed!" << endl;
      return u;
    }

    SoundEffect *sfx = u->MoveSound();
    if ( sfx ) sfx->Play( Audio::SFX_LOOP );

    undo.Register( u );
    RemoveUnit( u );

    short step, n = 0;
    while ( (step = path.GetStep( u->Position() )) != -1) {
      MoveUnit( u, (Direction)step, (n < 2) );
      ++n;
    }

    if ( sfx ) sfx->Stop();

    EndMovement( u );

    mwin->Show( mv->UpdateHex( u->Position() ) );

    if ( !u->IsReady() ) {
      if ( u == unit ) DeselectUnit();
      CheckEvents();
      return NULL;
    }

    shader->ShadeMap( u );
    if ( mv->CursorEnabled() ) mwin->GetPanel()->Update( u );
    mwin->Draw();
    mwin->Show();
    CheckEvents();
  }
  return u;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::MoveUnit
// DESCRIPTION: Move a unit one hex in a given direction.
// PARAMETERS : u     - unit to be moved
//              dir   - direction
//              blink - if TRUE make target cursor blink (default is
//                      FALSE)
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Game::MoveUnit( Unit *u, Direction dir, bool blink /* = false */ ) {
  MapView *mv = mwin->GetMapView();
  Map *map = mv->GetMap();
  const Point &pos = u->Position();
  Point posnew;
  if ( map->Dir2Hex( pos, dir, posnew ) ) return -1;

  u->Face( dir );

  if ( mv->Enabled() &&
       (mv->HexVisible(pos) || mv->HexVisible(posnew)) )
    mwin->MoveHex( u->Image(), mission->GetUnitSet(),
                   pos, posnew, ANIM_SPEED_UNIT, blink );

  u->SetPosition( posnew.x, posnew.y );
  if ( !u->IsDummy() ) {
    History *h = mission->GetHistory();
    if ( h ) h->RecordMoveEvent( *u, dir );
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::EndMovement
// DESCRIPTION: Finalize a unit move.
// PARAMETERS : u - unit to be moved
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::EndMovement( Unit *u ) const {
  Map &map = mission->GetMap();

  u->SetFlags( U_MOVED );

  const Point &pos = u->Position();
  short oldhex = map.HexTypeID( pos );
  int conquer = map.SetUnit( u, pos );

  if ( conquer == 1 ) {              // a building was conquered
    History *h = mission->GetHistory();
    if ( h ) h->RecordTileEvent( map.HexTypeID( pos ), oldhex, pos.x, pos.y );
  }

  if ( u->IsSlow() || !UnitTargets( u ) )
    u->SetFlags( U_DONE );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::RemoveUnit
// DESCRIPTION: Remove a unit from the map.
// PARAMETERS : u - unit to be removed
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::RemoveUnit( Unit *u ) {
  Map &map = mission->GetMap();

  if ( !u->IsSheltered() ) {
    map.SetUnit( NULL, u->Position() );
    mwin->GetMapView()->UpdateHex( u->Position() );
  } else {
    MapObject *o = map.GetMapObject( u->Position() );
    if ( o ) {
      if ( o->IsUnit() ) static_cast<Transport *>( o )->RemoveUnit( u );
      else static_cast<Building *>(o)->RemoveUnit( u );
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::UnitTargets
// DESCRIPTION: Find out whether the unit still has things to do, i.e.
//              enemies in range, mines to clear etc.
// PARAMETERS : u - unit to check for
// RETURNS    : TRUE if there are options left, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool Game::UnitTargets( Unit *u ) const {
  bool rc = false;
  Unit *tg = static_cast<Unit *>( mission->GetUnits().Head() );

  while ( tg && !rc ) {
    rc = u->CanHit( tg );
    tg = static_cast<Unit *>( tg->Next() );
  }

  if ( !rc ) rc = MinesweeperTargets( u );

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::MinesweeperTargets
// DESCRIPTION: If the unit is a minesweeper check if there are any
//              mines to be cleared.
// PARAMETERS : u - unit to check for
// RETURNS    : TRUE if there are options left, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool Game::MinesweeperTargets( Unit *u ) const {
  bool rc = false;

  if ( u->IsMinesweeper() && u->IsReady() ) {
    Map *map = &mission->GetMap();
    Point adj[6];

    map->GetNeighbors( u->Position(), adj );
    for ( int i = NORTH; (i <= NORTHWEST) && !rc; ++i ) {
      if ( adj[i].x != -1 ) {
        Unit *m = map->GetUnit( adj[i] );
        if ( m && m->IsMine() ) {
          // you can only remove an enemy mine if no other enemy unit
          // sits next to it
          rc = true;

          if ( m->Owner() != u->Owner() ) {
            Point madj[6];
            map->GetNeighbors( m->Position(), madj );

            for ( int j = NORTH; (j <= NORTHWEST) && rc; ++j ) {
              if ( madj[j].x != -1 ) {
                Unit *e = map->GetUnit( madj[j] );
                if ( e && (e->Owner() != u->Owner()) && !e->IsMine() ) rc = false;
              }
            }
          }
        }
      }
    }
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::SelectUnit
// DESCRIPTION: Select a unit to move/attack.
// PARAMETERS : u - unit to be selected
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::SelectUnit( Unit *u ) {
  if ( u->IsReady() ) {
    MapView *mv = mwin->GetMapView();

    if ( unit ) DeselectUnit( false );

    unit = u;
    mission->GetPlayer().SetMode( MODE_BUSY );

    if ( mv->Enabled() ) {
      Audio::PlaySfx( Audio::SND_GAM_SELECT, 0 );
      mv->EnableFog();
      shader->ShadeMap( unit );
      mwin->Draw();
      mwin->Show();

      mv->SetCursorImage( IMG_CURSOR_SELECT );
      SetCursor( u->Position() );
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::DeselectUnit
// DESCRIPTION: Deselect the currently active unit.
// PARAMETERS : update - whether to update the display or not (default
//                       value is "true")
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::DeselectUnit( bool display /* = true */ ) {
  MapView *mv = mwin->GetMapView();
  mission->GetPlayer().SetMode( MODE_IDLE );

  if ( mv->Enabled() ) {
    mv->SetCursorImage( IMG_CURSOR_IDLE );

    mv->DisableFog();
    if ( display ) {
      mwin->Draw();
      mwin->Show();

      SetCursor( unit->Position() );
    }
  }
  unit = NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::SelectNextUnit
// DESCRIPTION: Select the current player's next available unit.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::SelectNextUnit( void ) {
  List &units = mission->GetUnits();
  Unit *start, *u;

  if ( unit ) start = unit;
  else start = static_cast<Unit *>( units.Head() );

  u = static_cast<Unit *>( units.NextNode(start) );

  while ( u && (u != start) ) {

    if ( (u->Owner() == &mission->GetPlayer()) && u->IsReady() && !u->IsSheltered() ) {
      // we ignore SHELTERED units because it wouldn't be obvious which
      // of the units inside the transport was selected
      SelectUnit( u );
      break;
    }

    u = static_cast<Unit *>( units.NextNode(u) );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::EnterSpecialMode
// DESCRIPTION: Activate one of the special game modes (for pioneers,
//              mine-sweepers, or depot builders)
// PARAMETERS : mode - mode to enter (see player.h for definitions)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::EnterSpecialMode( unsigned char mode ) {
  MapView *mv = mwin->GetMapView();

  mission->GetPlayer().SetMode( mode );

  if ( mv->Enabled() ) {
    MinesweeperShader mss( &mission->GetMap(), mission->GetUnits(), mv->GetFogBuffer() );

    mv->EnableFog();
    mss.ShadeMap( unit );
    mwin->Draw();
    mwin->Show();

    mv->SetCursorImage( IMG_CURSOR_SPECIAL );
    SetCursor( unit->Position() );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::ClearMine
// DESCRIPTION: Try to move a mine from the map into an adjacent mine
//              sweeper unit.
// PARAMETERS : sweeper - mine sweeper unit (currently selected unit)
//              mine    - mine to be cleared
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::ClearMine( Transport *sweeper, Unit *mine ) {
  const Point &spos = sweeper->Position();
  bool allow;

  if ( mine->Owner() != sweeper->Owner() ) {
    Player *mowner = mine->Owner();
    mine->SetOwner( sweeper->Owner() );
    allow = sweeper->Allow( mine );
    if ( !allow ) mine->SetOwner( mowner );
  } else allow = sweeper->Allow( mine );

  if ( allow ) {
    MoveUnit( mine, spos );

    if ( !MinesweeperTargets(sweeper) ) {
      sweeper->SetFlags( U_DONE );
      DeselectUnit();
    }

    undo.Disable();

  } else new NoteWindow( MSG(MSG_ERROR), MSG(MSG_ERR_SWEEPER_FULL),
                         WIN_CLOSE_ESC, view );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::Undo
// DESCRIPTION: Undo the last move command the player issued.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::Undo( void ) {
  if ( unit ) DeselectUnit();

  Map &map = mission->GetMap();
  Unit *u = undo.GetUnit();

  RemoveUnit( u );
  u->Face( undo.GetDirection() );
  u->UnsetFlags( U_DONE|U_MOVED );

  // if the unit is a transport and came out of another
  // container we also need to reset the carried units' flags
  if ( u->IsTransport() && map.GetMapObject( undo.GetPosition() ) ) {
    Transport *t = static_cast<Transport *>(u);
    for ( int i = 0; i < t->UnitCount(); ++i )
      t->GetUnit( i )->UnsetFlags( U_DONE|U_MOVED );
  }

  map.SetUnit( u, undo.GetPosition() );
  mwin->GetMapView()->UpdateHex( undo.GetPosition() );
  mwin->Show();

  if ( mission->GetHistory() )
    mission->GetHistory()->UndoMove( *u );

  undo.Disable();
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::HandleEvent
// DESCRIPTION: Handle key and mouse button events special to the map
//              window.
// PARAMETERS : event - event received by the event handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status Game::HandleEvent( const SDL_Event &event ) {
  GUI_Status rc = GUI_OK;
  MapView *mv = mwin->GetMapView();
  Map &map = mission->GetMap();

  // check for keyboard commands
  if ( event.type == SDL_KEYDOWN ) {
    // some SDL ports return illegal key values
    short key = event.key.keysym.sym & 0x1ff;
    const SDLKey *keymap = CFOptions.GetKeyBindings();

    // map user- and locale-defined keys to standard ones
    if ((key > SDLK_FIRST) && (key < SDLK_LAST)) {
      // KEYBIND_MINIMIZE is a global binding that is not specific
      // to the Game class, so it isn't handled here

      // user-defined keys first
      for ( int i = 0; i < KEYBIND_COUNT; ++i ) {
        if ( key == keymap[i] ) {
          key = -i;
          break;
        }
      }

      // now check locale-defined keys
      if ( key >= 0 ) {
        if ( key == keys[G_KEY_INFO] ) key = -KEYBIND_UNIT_INFO;
        else if ( key == keys[G_KEY_CONTENT] ) key = -KEYBIND_UNIT_CONTENT;
        else if ( key == keys[G_KEY_SWEEP] ) key = -KEYBIND_UNIT_SWEEP;
        else if ( key == keys[G_KEY_END_TURN] ) key = -KEYBIND_END_TURN;
        else if ( key == keys[G_KEY_MAP] ) key = -KEYBIND_SHOW_MAP;
      }
    }

    switch ( key ) {
    case SDLK_KP1: case SDLK_KP2: case SDLK_KP3:
    case SDLK_KP4: case SDLK_KP6: case SDLK_KP7:
    case SDLK_KP8: case SDLK_KP9:
      if ( !(event.key.keysym.mod & KMOD_NUM) ) {
        ScrollCommand( key );
        break;
      }
      // fall through
    case SDLK_LEFT: case SDLK_RIGHT: case SDLK_UP: case SDLK_DOWN:
      MoveCommand( key );
      break;
    case SDLK_ESCAPE:
      if ( unit ) {
        DeselectUnit();
        break;
      }
      GameMenu();
      break;

    // the keys for the following commands can be modified by
    // user and/or locale, so we use fixed (negative so we don't
    // collide with regular keysyms) values which get mapped
    // before the switch statement
    case -KEYBIND_END_TURN:
      rc = EndTurn();
      break;
    case -KEYBIND_SHOW_MAP:
      new TacticalWindow( mv, *mission, view );
      break;
    case -KEYBIND_GAME_MENU:
      GameMenu();
      break;
    case -KEYBIND_UNIT_MENU: {
      Unit *u = map.GetUnit( mv->Cursor() );
      if ( u ) UnitMenu( u ); }
      break;
    case -KEYBIND_UNIT_CONTENT: {
      MapObject *mo = map.GetMapObject( mv->Cursor() );
      if ( mo ) {
        if ( mo->IsShop() )
          ContainerContent( static_cast<UnitContainer *>(
                            static_cast<Building *>(mo)) );
        else if ( static_cast<Unit *>(mo)->IsTransport() )
          ContainerContent( static_cast<UnitContainer *>(
                            static_cast<Transport *>(mo)) );
      }}
      break;
    case -KEYBIND_UNIT_INFO: {
      Unit *u = map.GetUnit( mv->Cursor() );
      if ( u ) UnitInfo( u ); }
      break;
    case -KEYBIND_UNIT_NEXT:
      SelectNextUnit();
      break;
    case -KEYBIND_UNIT_SELECT:
      // select the unit underneath the cursor
      SelectCommand( mv->Cursor() );
      break;
    case -KEYBIND_UNIT_UNDO:
      if ( undo.GetUnit() != NULL) Undo();
      break;
    case -KEYBIND_UNIT_SWEEP:
      if ( unit && MinesweeperTargets( unit ) )
        EnterSpecialMode( MODE_SWEEP );
      break;
    default:
      if ( key == keys[G_KEY_QUIT] ) Quit();
      break;
    }

  } else if ( event.type == SDL_MOUSEBUTTONDOWN ) {
    Point pos;
    if ( event.button.button == SDL_BUTTON_LEFT ) {
      if ( !mv->Pixel2Hex( event.button.x - mwin->x, event.button.y - mwin->y, pos ) ) {
        HandleLMB( pos );
      }
    } else if ( !mv->Pixel2Hex( event.button.x - mwin->x, event.button.y - mwin->y, pos ) ) {
      Unit *u = map.GetUnit( pos );
      if ( event.button.button == SDL_BUTTON_RIGHT ) {
        if ( u ) UnitMenu( u );
        else GameMenu();
      } else {    // middle mouse button
        if ( u ) {
          if ( u->IsTransport() ) ContainerContent( static_cast<Transport *>(u) );
        } else if ( map.GetBuilding( pos ) )
          ContainerContent( map.GetBuilding( pos ) );
      }
    } else GameMenu();
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::MoveCommand
// DESCRIPTION: Got a move command from the user. See what he wants to
//              do. Move the cursor or a selected unit.
// PARAMETERS : key - the key code used to give the order
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::MoveCommand( int key ) {
  Direction dir;

  switch ( key ) {
  case SDLK_KP1:  dir = SOUTHWEST; break;
  case SDLK_DOWN:
  case SDLK_KP2:  dir = SOUTH;     break;
  case SDLK_KP3:  dir = SOUTHEAST; break;
  case SDLK_KP7:  dir = NORTHWEST; break;
  case SDLK_UP:
  case SDLK_KP8:  dir = NORTH;     break;
  case SDLK_KP9:  dir = NORTHEAST; break;
  case SDLK_LEFT:
  case SDLK_KP4:  dir = WEST;      break;
  case SDLK_RIGHT:
  case SDLK_KP6:  dir = EAST;      break;
  default: return;
  }

  Point cursor = mwin->MoveCursor( dir );
  if ( cursor != mwin->GetMapView()->Cursor() ) SetCursor( cursor );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::SelectCommand
// DESCRIPTION: Got a select command from the user. See what he wants to
//              do. Select/deselect a unit, enter a building, or attack
//              an enemy unit.
// PARAMETERS : hex - selected hex
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::SelectCommand( const Point &hex ) {
  MapView *mv = mwin->GetMapView();
  Map *map = mv->GetMap();
  Point cursor = mv->Cursor();
  Player &p = mission->GetPlayer();

  Unit *u = map->GetUnit( hex );

  switch ( p.Mode() ) {
  case MODE_BUSY:
    if ( unit->Position() == hex ) DeselectUnit();
    else if ( u ) {
      if ( u->Owner() == &p ) {
        if ( shader->GetStep(hex) != -1 )
          MoveUnit( unit, hex );    // try to move into transport
        else SelectUnit( u );
      } else if ( unit->CanHit( u ) ) {   // attack the unit
        mission->RegisterBattle( unit, u );
        DeselectUnit();
        mwin->FlashUnit( u->Position(), 2 );
        undo.Disable();
      }
    } else MoveUnit( unit, hex );  // try to move there
    break;

  case MODE_IDLE:
    if ( u && (u->Owner() == &p) ) SelectUnit( u );
    else {
      Building *b;

      if ( map->IsShop( hex ) && (b = map->GetBuilding( hex )) )
        ContainerContent( b );
    }
    break;

  case MODE_SWEEP:
    if ( (shader->GetStep( hex ) != -1) &&
         (cursor != unit->Position()) )
      ClearMine( static_cast<Transport *>(unit), u );
    else DeselectUnit();
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::ScrollCommand
// DESCRIPTION: Scroll the map display in the given direction.
// PARAMETERS : key - the key code used to give the order
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::ScrollCommand( int key ) {
  MapView *mv = mwin->GetMapView();
  Point pos = mv->Cursor();
  int sw = mv->Width() * 3 / 4 / mv->TileWidth();
  int sh = mv->Height() * 3 / 4 / mv->TileHeight();

  switch ( key ) {
  case SDLK_KP1: pos.x -= sw; pos.y += sh; break;
  case SDLK_KP2:              pos.y += sh; break;
  case SDLK_KP3: pos.x += sw; pos.y += sh; break;
  case SDLK_KP4: pos.x -= sw;                break;
  case SDLK_KP6: pos.x += sw;                break;
  case SDLK_KP7: pos.x -= sw; pos.y -= sh; break;
  case SDLK_KP8:              pos.y -= sh; break;
  case SDLK_KP9: pos.x += sw; pos.y -= sh; break;
  default: return;
  }

  const Map *map = mv->GetMap();
  if ( pos.x < 0 ) pos.x = 0;
  else if ( pos.x >= map->Width() ) pos.x = map->Width() - 1;
  if ( pos.y < 0 ) pos.y = 0;
  else if ( pos.y >= map->Height() ) pos.y = map->Height() - 1;
  SetCursor( pos );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::HandleLMB
// DESCRIPTION: React to user pressing the left mouse button.
// PARAMETERS : hex - hex the user clicked on
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::HandleLMB( const Point &hex ) {

  bool move = (hex != mwin->GetMapView()->Cursor());
  Unit *u = mission->GetMap().GetUnit( hex );

  // activate selection if the user
  // - clicked the same hex twice or
  // - clicked one of her own units and has currently no unit selected
  if ( !move ||
       (u && !unit && (u->Owner() == &mission->GetPlayer()) && u->IsReady()) )
    SelectCommand( hex );

  else if ( move ) SetCursor( hex );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::ContainerContent
// DESCRIPTION: Open a window to display the content of a transport or
//              building.
// PARAMETERS : c - container to look into
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::ContainerContent( UnitContainer *c ) {
  MapObject *mo = dynamic_cast<MapObject *>(c);

  if ( unit ) DeselectUnit();

  if ( (mo->Owner() == &mission->GetPlayer()) || !mo->Owner() ) new ContainerWindow( c, view );
  else new NoteWindow( mo->Name(), MSG(MSG_ERR_NO_ACCESS), WIN_CLOSE_ESC, view );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::UnitInfo
// DESCRIPTION: Display a window with information about a unit. If the
//              current player is not authorised to peek at the unit
//              specifications (e.g. because it's a hostile unit) the
//              information request will fail.
// PARAMETERS : unit - unit to show information about
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::UnitInfo( Unit *unit ) {
  if ( unit->Owner() == &mission->GetPlayer() )
    new UnitInfoWindow( unit->Type()->ID(), mission->GetMap(), view );
  else new NoteWindow( unit->Name(), MSG(MSG_ERR_NO_ACCESS), WIN_CLOSE_ESC, view );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::ShowLevelInfo
// DESCRIPTION: Display level information supplied by the creator, if
//              any. If no info was supplied, say so.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::ShowLevelInfo( void ) const {
  const char *msg = mission->GetInfoMsg();

  if ( !msg ) msg = MSG(MSG_ERR_NO_LVL_INFO);

  new NoteWindow( MSG(MSG_LVL_INFO), msg, WIN_CLOSE_ESC, view );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::ShowBriefing
// DESCRIPTION: Display a window with the mission objectives for the
//              current player. If the mission creator did not supply a
//              briefing, pop up an error.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::ShowBriefing( void ) const {
  Player &p = mission->GetPlayer();

  if ( p.Briefing() != -1 )
    new MessageWindow( p.Name(), mission->GetMessage(p.Briefing()), view );
  else new NoteWindow( p.Name(), MSG(MSG_ERR_NO_BRIEFING), WIN_CLOSE_ESC, view );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::ShowDebriefing
// DESCRIPTION: When the mission is over, display a message telling
//              the players whether they won or lost and optionally
//              return to the main menu.
// PARAMETERS : player  - player to show debriefing for
//              restart - whether to return to the main menu or load
//                        the next map
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status Game::ShowDebriefing( Player &player, bool restart ) {
  GUI_Status rc = GUI_OK;

  if ( player.IsInteractive() ) {
    Player *winner = NULL;
    bool draw = false;
    const char *msg;
    Player &p1 = mission->GetPlayer(PLAYER_ONE);
    Player &p2 = mission->GetPlayer(PLAYER_TWO);

    if ( p1.Success( 0 ) >= 100 ) winner = &p1;
    if ( p2.Success( 0 ) >= 100 ) {
      if ( winner ) draw = true;
      else winner = &p2;
    }

    if ( draw ) msg = MSG(MSG_RESULT_DRAW);
    else if ( &player == winner ) msg = MSG(MSG_RESULT_VICTORY);
    else msg = MSG(MSG_RESULT_DEFEAT);

    NoteWindow *note = new NoteWindow( MSG(MSG_DEBRIEFING), msg, WIN_CENTER, view );

    if ( restart ) {
      const char *next_map = NULL;
      if ( mission->GetFlags() & GI_CAMPAIGN ) next_map = mission->GetSequel();
      if ( !next_map || draw || !winner->IsHuman() ) {
        note->SetButtonID( 0, G_BUTTON_SHUTDOWN );
        note->SetButtonHook( 0, this );
      } else {
        note->EventLoop();
        view->CloseWindow( note );

        CFOptions.Unlock( next_map );
        int err = SwitchMap( next_map );
        if ( err == -1 ) {
          note = new NoteWindow( MSG(MSG_ERROR), MSG(MSG_ERR_MAP_NOT_FOUND), 0, view );
          note->SetButtonID( 0, G_BUTTON_SHUTDOWN );
          note->SetButtonHook( 0, this );
        } else {
          InitWindows();
          rc = StartTurn();
        }
      }
    } else {
      note->EventLoop();
      view->CloseWindow( note );
    }
  } else if ( restart ) rc = GUI_RESTART;

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::Execute
// DESCRIPTION: Execute the events from a recorded turn history session.
//              In contrast to History::Replay() which only creates fake
//              actions for display, this function directly affects the
//              current state of the game, ie. all actions are treated
//              exactly as if they had been triggered by the current
//              player.
// PARAMETERS : history - recorded session to execute
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::Execute( const History &history ) {
  const List &events = history.GetEvents();

  // many event types can simply be ignored;
  // all we care for is movement, attacks, plus (user-triggered)
  // unit construction and repairs
  HistEvent *he = static_cast<HistEvent *>( events.Head() );
  while ( he ) {
    Unit *u, *u2;

    if ( he->type == History::HIST_MOVE ) {
      u = mission->GetUnit( he->data[0] );
      if ( u ) {
        RemoveUnit( u );

        do {
          MoveUnit( u, (Direction)he->data[1] );
          he = static_cast<HistEvent *>( he->Next() );
        } while ( he && he->type == History::HIST_MOVE && he->data[0] == u->ID() );

        EndMovement( u );
        CheckEvents();
        continue;
      }

    } else if ( he->type == History::HIST_UNIT ) {

      switch ( he->data[1] ) {
      case History::HIST_UEVENT_CREATE:
        u = history.GetDummy( he->data[0] );
        u2 = mission->GetUnit( he->data[0] );

        // units can be created by players (in factories) or by events.
        // we only want to execute those triggered by players, since we'd
        // otherwise create units twice. a cleaner solution would be
        // to clearly indicate for each event who/what caused it, but this
        // little hack does its job as well: don't build the unit if
        // another unit with the same ID already exists
        if ( u && !u2 ) {
          mission->CreateUnit( u->Type()->ID(), *u->Owner(), u->Position(),
                               (Direction)u->Facing(), u->GroupSize(), u->XP() );
          CheckEvents();
        }
        break;
      case History::HIST_UEVENT_REPAIR:
        u = mission->GetUnit( he->data[0] );
        if ( u ) {
          UnitContainer *uc = dynamic_cast<UnitContainer *>
                              (mission->GetMap().GetMapObject( u->Position() ));
          if ( uc )
            uc->SetCrystals( uc->Crystals() - CRYSTALS_REPAIR );

          if ( mission->GetHistory() )
            mission->GetHistory()->RecordUnitEvent( *u, History::HIST_UEVENT_REPAIR );

          u->Repair();
        }
        break;
      default:
        // ignore
        break;
      }

    } else if ( he->type == History::HIST_ATTACK ) {
      u = mission->GetUnit( he->data[0] );
      u2 = mission->GetMap().GetUnit( Point( he->data[1], he->data[2] ) );

      if ( u && u2 )
        mission->RegisterBattle( u, u2 );

    } else if ( he->type == History::HIST_COMBAT ) {
      u = mission->GetUnit( he->data[0] );
      u2 = mission->GetUnit( he->data[1] );

      if ( u && u2 ) {
        Combat cmb( u, u2 );
        Point casualties( he->data[3], he->data[2] );
        ResolveBattle( &cmb, &casualties );
        CheckEvents();
      }

    } else if ( he->type == History::HIST_TRANSPORT_CRYSTALS ) {
      u = mission->GetUnit( he->data[0] );

      if ( u ) {
        UnitContainer *uc = dynamic_cast<UnitContainer *>
                            (mission->GetMap().GetMapObject( u->Position() ));
        if ( uc ) {
          Transport *t = static_cast<Transport*>( u );
          uc->SetCrystals( uc->Crystals() - he->data[1] );
          u2->SetFlags( U_MOVED|U_DONE );
          t->SetCrystals( t->Crystals() + he->data[1] );
        }
      }

    } else if ( he->type == History::HIST_TRANSPORT_UNIT ) {
      u = mission->GetUnit( he->data[0] );
      u2 = mission->GetUnit( he->data[1] );

      if ( u && u2 ) {
        UnitContainer *uc = dynamic_cast<UnitContainer *>
                            (mission->GetMap().GetMapObject( u->Position() ));
        if ( uc ) {
          Transport *t = static_cast<Transport*>( u );
          uc->RemoveUnit( u2 );
          t->InsertUnit( u2 );
        }
      }
    }

    he = static_cast<HistEvent *>( he->Next() );
  }

  // make sure the battles are not rerun at EndTurn()
  mission->GetBattles().Clear();
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::GameMenu
// DESCRIPTION: Pop up a MenuWindow with general game options like
//              "End Turn" or "Quit".
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::GameMenu( void ) {
  MenuWindow *menu = new MenuWindow( PROGRAMNAME, this, view );

  menu->AddItem( 0, G_BUTTON_END_TURN, 0, MSG(MSG_B_END_TURN) );
  menu->AddItem( 0, G_BUTTON_MAP,      0, MSG(MSG_B_MAP) );
  menu->AddItem( 0, G_BUTTON_BRIEFING, 0, MSG(MSG_B_OBJECTIVES) );
  menu->AddBar( 0 );
  menu->AddItem( 0, G_BUTTON_LEV_INFO, 0, MSG(MSG_B_LEVEL_INFO) );
  menu->AddMenu( 0, 0, MSG(MSG_B_OPTIONS) );
  menu->AddItem( 1, G_BUTTON_GENERAL_OPTIONS, 0, MSG(MSG_B_OPT_GENERAL) );
  menu->AddItem( 1, G_BUTTON_LANGUAGE_OPTIONS, 0, MSG(MSG_B_OPT_LANGUAGE) );
  menu->AddItem( 1, G_BUTTON_VIDEO_OPTIONS, 0, MSG(MSG_B_OPT_VIDEO) );
#ifndef DISABLE_SOUND
  menu->AddItem( 1, G_BUTTON_SOUND_OPTIONS, 0, MSG(MSG_B_OPT_AUDIO) );
#endif
  menu->AddItem( 1, G_BUTTON_KEYBOARD_OPTIONS, 0, MSG(MSG_B_OPT_KEYBOARD) );
  menu->AddBar( 0 );
  menu->AddItem( 0, G_BUTTON_SAVE,
        mission->GetFlags() & GI_PBEM ? WIDGET_DISABLED : 0,
        MSG(MSG_B_SAVE_GAME) );

  if (view->IsFullScreen())
    menu->AddItem( 0, G_BUTTON_MINIMIZE_WINDOW, 0, MSG(MSG_OPT_KEY_MINIMIZE) );
  menu->AddItem( 0, G_BUTTON_ABORT,    0, MSG(MSG_B_MAIN_MENU) );
  menu->AddItem( 0, G_BUTTON_QUIT,     0, MSG(MSG_B_QUIT) );

  menu->Layout();
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::UnitMenu
// DESCRIPTION: Pop up a MenuWindow with possible actions for a selected
//              unit.
// PARAMETERS : u - unit to open window for
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::UnitMenu( Unit *u ) {

  if ( (u->Owner() == &mission->GetPlayer()) &&
       (u->IsTransport() || u->IsMinesweeper() || (undo.GetUnit() == u)) ) {
    MenuWindow *menu = new MenuWindow( u->Name(), this, view );

    menu->AddItem( 0, G_BUTTON_UNIT_INFO, 0, MSG(MSG_B_UNIT_INFO) );

    if ( u->IsTransport() )
      menu->AddItem( 0, G_BUTTON_UNIT_CONTENT, 0, MSG(MSG_B_UNIT_CONTENT) );

    if ( u->IsMinesweeper() )
      menu->AddItem( 0, G_BUTTON_UNIT_SWEEP,
            (MinesweeperTargets( u ) ? 0 : WIDGET_DISABLED), MSG(MSG_B_UNIT_SWEEP) );

    if ( undo.GetUnit() == u )
      menu->AddItem( 0, G_BUTTON_UNIT_UNDO, 0, MSG(MSG_B_UNIT_UNDO) );

    menu->Layout();
    g_tmp_prv_unit = u;
  } else UnitInfo( u );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::HandleNetworkError
// DESCRIPTION: Call when a network error has been detected. Ask user
//              whether current game should be saved, and shutdown
//              afterwards.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::HandleNetworkError( void ) {
  string choices( MSG(MSG_B_SAVE) );
  choices += '|';
  choices += MSG(MSG_B_CANCEL);

  DialogWindow *dw = new DialogWindow( MSG(MSG_ERROR),
      MSG(MSG_ERR_NETWORK), choices, 0, WIN_FONT_BIG|WIN_CENTER,
      view );
  dw->SetButtonHook( this );
  dw->SetButtonID( 0, G_BUTTON_SAVE_AND_SHUTDOWN );
  dw->SetButtonID( 1, G_BUTTON_SHUTDOWN );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::Shutdown
// DESCRIPTION: This method should be called when the current game ends,
//              either because one player surrendered, or the mission
//              was completed. It will not be called when quitting the
//              application altogether.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Game::Shutdown( void ) const {
  Audio::StopMusic( CF_MUSIC_FADE_TIME );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Game::WidgetActivated
// DESCRIPTION: The WidgetActivated() method gets called whenever a
//              widget from the game menu or another associated window
//              (e.g. password confirmation) is activated.
// PARAMETERS : button - pointer to the widget that called the function
//              win    - pointer to the window the widget belongs to
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status Game::WidgetActivated( Widget *button, Window *win ) {
  GUI_Status rc = GUI_OK;

  switch ( button->ID() ) {
  case G_BUTTON_END_TURN:
    view->CloseWindow( win );
    rc = EndTurn();
    if ( rc == GUI_RESTART ) Shutdown();
    break;
  case G_BUTTON_MAP:
    view->CloseWindow( win );
    new TacticalWindow( mwin->GetMapView(), *mission, view );
    break;
  case G_BUTTON_BRIEFING:
    view->CloseWindow( win );
    ShowBriefing();
    break;
  case G_BUTTON_LEV_INFO:
    view->CloseWindow( win );
    ShowLevelInfo();
    break;
  case G_BUTTON_GENERAL_OPTIONS:
    static_cast<MenuWindow *>(win)->CloseParent();
    view->CloseWindow( win );
    new GeneralOptionsWindow( mwin->GetMapView(), view );
    break;
  case G_BUTTON_LANGUAGE_OPTIONS:
    static_cast<MenuWindow *>(win)->CloseParent();
    view->CloseWindow( win );
    new LocaleOptionsWindow( this, view );
    break;
  case G_BUTTON_KEYBOARD_OPTIONS:
    static_cast<MenuWindow *>(win)->CloseParent();
    view->CloseWindow( win );
    new KeyboardOptionsWindow( view );
    break;
  case G_BUTTON_VIDEO_OPTIONS:
    static_cast<MenuWindow *>(win)->CloseParent();
    view->CloseWindow( win );
    new VideoOptionsWindow( view );
    break;
#ifndef DISABLE_SOUND
  case G_BUTTON_SOUND_OPTIONS:
    static_cast<MenuWindow *>(win)->CloseParent();
    view->CloseWindow( win );
    new SoundOptionsWindow( view );
    break;
#endif
  case G_BUTTON_ABORT: {
    view->CloseWindow( win );
    Audio::PlaySfx( Audio::SND_GUI_ASK, 0 );
    string buttons;
    buttons.append( MSG(MSG_B_YES) );
    buttons += '|';
    buttons.append( MSG(MSG_B_NO) );
    DialogWindow *req = new DialogWindow( NULL, MSG(MSG_ASK_ABORT),
                        buttons, 1, 0, view );
    req->SetButtonID( 0, G_BUTTON_SHUTDOWN );
    req->SetButtonHook( 0, this );
    break; }
  case G_BUTTON_QUIT:
    view->CloseWindow( win );
    Quit();
    break;
  case G_BUTTON_SAVE:
  case G_BUTTON_SAVE_AND_SHUTDOWN:
    view->CloseWindow( win );
    Save( NULL );
    if ( button->ID() == G_BUTTON_SAVE )
      break;
    // otherwise fall through
  case G_BUTTON_SHUTDOWN:
    Shutdown();
    rc = GUI_RESTART;
    break;

  case G_BUTTON_UNIT_INFO:
    view->CloseWindow( win );
    UnitInfo( g_tmp_prv_unit );
    break;
  case G_BUTTON_UNIT_CONTENT:
    view->CloseWindow( win );
    ContainerContent( static_cast<Transport *>(g_tmp_prv_unit) );
    break;
  case G_BUTTON_UNIT_SWEEP:
    view->CloseWindow( win );
    if ( unit != g_tmp_prv_unit ) {
      // disable display updates so that selection doesn't shade
      mwin->GetMapView()->Disable();
      SelectUnit( g_tmp_prv_unit );
      mwin->GetMapView()->Enable();
    }
    EnterSpecialMode( MODE_SWEEP );
    break;
  case G_BUTTON_UNIT_UNDO:
    view->CloseWindow( win );
    Undo();
    break;
  case G_BUTTON_MINIMIZE_WINDOW:
    view->CloseWindow( win );
    SDL_WM_IconifyWindow();
    break;
  }

  return rc;
}

