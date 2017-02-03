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
// game.h
////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDE_GAME_H
#define _INCLUDE_GAME_H

#include "mission.h"
#include "mapwindow.h"
#include "extwindow.h"
#include "list.h"
#include "path.h"
#include "options.h"
#include "network.h"
#include "globals.h"

#define PROGRAMNAME "Crimson Fields"

// global vars
extern class Game *Gam;
extern class Image *Images[];

#define DEFAULT_DELAY  (5 * ANIM_SPEED_UNIT)

class UndoCache {
public:
  UndoCache( void ) : unit(NULL) {}

  void Disable( void ) { unit = NULL; }
  void Register( Unit *u )
    { unit = u; pos = u->Position(); dir = u->Facing(); }

  bool Disabled( void ) const { return unit == NULL; }
  Unit *GetUnit( void ) const { return unit; }
  const Point &GetPosition( void ) const { return pos; }
  unsigned char GetDirection( void ) const { return dir; }

private:
  Unit *unit;
  Point pos;
  unsigned char dir;
};

class Game : public WidgetHook {
public:
  Game( View *view );
  virtual ~Game( void );

  int Load( MemBuffer &buffer );
  int Load( const char *file );
  int Save( MemBuffer &buffer ) { return mission->Save( buffer ); }
  int Save( const char *file );

  void InitWindows( void );
  void InitKeys( void );
  void Quit( void ) const;
  void Shutdown( void ) const;
  GUI_Status HandleEvent( const SDL_Event &event );

  GUI_Status StartTurn( void );
  GUI_Status EndTurn( void );

  void SetCursor( const Point &cursor ) const;
  Unit *MoveUnit( Unit *u, const Point &dest );
  int MoveUnit( Unit *u, Direction dir, bool blink = false );
  void SelectUnit( Unit *u );
  void DeselectUnit( bool update = true );

  MapWindow *GetMapWindow( void ) const { return mwin; }
  Mission *GetMission( void ) const { return mission; }
  void UnitInfo( Unit *unit );
  void ResolveBattle( Combat *com, const Point *result = NULL );

#ifndef DISABLE_NETWORK
  void SetNetworkConnection( TCPConnection *c ) { peer = c; }
#endif

private:
  string CreateSaveFileName( const char *filename ) const;
  void ClearMine( Transport *sweeper, Unit *mine );
  bool HaveWinner( void );
  GUI_Status CheckEvents( void );
  void ExecPreStartEvents( void );
  void Execute( const History &history );

  void MoveCommand( int key );
  void SelectCommand( const Point &hex );
  void ScrollCommand( int key );
  void HandleLMB( const Point &hex );
  void EnterSpecialMode( unsigned char mode );
  void SelectNextUnit( void );
  void RemoveUnit( Unit *u );
  void Undo( void );

  void EndMovement( Unit *u ) const;
  bool UnitTargets( Unit *u ) const;
  bool MinesweeperTargets( Unit *u ) const;

  void ShowBriefing( void ) const;
  GUI_Status ShowDebriefing( Player &player, bool restart );
  void ShowLevelInfo( void ) const;
  void ContainerContent( UnitContainer *c );
  void GameMenu( void );
  void UnitMenu( Unit *unit );
  int SwitchMap( const char *mission );
  void HandleNetworkError( void );

  bool SetPlayerPassword( Player *player ) const;
  bool CheckPassword( const char *title, const char *msg, const char *pw, short retry ) const;

  GUI_Status WidgetActivated( Widget *button, Window *win );

  Mission *mission;
  MapWindow *mwin;

  Unit *unit;    // selected unit

  short keys[6];  // keyboard shortcuts

  MoveShader *shader;
  View *view;

  UndoCache undo;

  string last_file_name; // remember save file names
  Unit *g_tmp_prv_unit;

#ifndef DISABLE_NETWORK
  TCPConnection *peer;
#endif
};

#endif  /* _INCLUDE_GAME_H */

