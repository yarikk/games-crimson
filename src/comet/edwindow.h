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

//////////////////////////////////////////////////////////////
// edwindow.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_ED_EDWINDOW_H
#define _INCLUDE_ED_EDWINDOW_H

#include "window.h"
#include "mapview.h"
#include "mission.h"
#include "gfxwidget.h"
#include "button.h"

#define ED_MODE_TERRAIN 1
#define ED_MODE_UNIT    2
#define ED_MODE_VIEW    3


class EdWindow : public Window, public WidgetHook {
public:
  EdWindow( const string &mapdir, View *view );
  ~EdWindow( void );

  MapView &GetMapView( void ) { return mv; }

  void Draw( void );
  GUI_Status HandleEvent( const SDL_Event &event );
  void Quit( void ) const;
  void MoveCursor( int key );
  void SetCursor( const Point &cursor );

  Mission *LoadMission( const char *file, bool filereq );
  GUI_Status SaveMission( const char *file, bool filereq );
  GUI_Status ExportMission( const char *file, bool filereq );
  string GetFileName( const char *filename, const char *suffix,
                      const string &dirname, int flag ) const;

private:
  GUI_Status WidgetActivated( Widget *widget, Window *win );
  void LeftMouseButton( const Point &hex );
  void ShowContextMenu( const Point &clicked );
  void PrintCursorPos( void );

  void SwapTiles( unsigned short t1, unsigned short t2 );
  void ValidateMission( void ) const;
  void SetNewMission( Mission *ms );

  enum {
    L_ID_UNITS = 0,
    L_ID_TILES,
    S_ID_VERT,
    S_ID_HORIZ,
    B_ID_NEW_MISSION_OK,
    // button ids for the context menu below
    B_ID_TILE_GRAB,
    B_ID_TILE_SWAP,
    B_ID_UNIT_INFO,
    B_ID_UNIT_EDIT,
    B_ID_UNIT_DEL,
    B_ID_BLD_CREATE,
    B_ID_BLD_EDIT,
    B_ID_BLD_DEL,
    B_ID_EVENTS,
    B_ID_MESSAGES,
    B_ID_NEW,
    B_ID_LOAD,
    B_ID_SAVE,
    B_ID_EXPORT,
    B_ID_VALIDATE,
    B_ID_SETTINGS,
    B_ID_QUIT
  };

  Mission *mission;
  Rect panel;
  MapView mv;

  unsigned char mode;
  short selected;
  Point selected_hex;

  TileWidget *tile_wd;
  UnitWidget *unit_wd;
  SliderWidget *ud_wd;  // up/down
  SliderWidget *lr_wd;  // left/right

  string mapdir;
};

#endif  // _INCLUDE_ED_EDWINDOW_H

