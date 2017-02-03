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
// mapwindow.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_MAPWINDOW_H
#define _INCLUDE_MAPWINDOW_H

#include "window.h"
#include "mapview.h"
#include "mapwidget.h"
#include "mission.h"
#include "misc.h"

#define ANIM_SPEED_UNIT		150  // time it takes for a unit to move one hex (ms)
#define ANIM_SPEED_CURSOR	40   // speed when moving cursor

#define DEFAULT_PANEL_HEIGHT	(20)

// the small info panel at the bottom of the map window
class Panel : public Window {
public:
  Panel( class MapWindow *mw, View *view );

  void Draw( void );
  void Update( MapObject *obj );
  virtual GUI_Status HandleEvent( const SDL_Event &event );
  virtual void VideoModeChange( void );

private:
  MapObject *obj;
  MapWindow *mapwin;
  Image workshop_icon;
  Image factory_icon;
};


class MapWindow : public Window {
public:
  MapWindow( short x, short y, unsigned short w, unsigned short h,
             unsigned short flags, View *view );
  ~MapWindow( void ) { delete mview; }

  virtual GUI_Status HandleEvent( const SDL_Event &event );
  virtual void Draw( void );
  virtual void Close( void ) { Window::Close(); view->CloseWindow(panel); }
  virtual void DrawBack( short x, short y, unsigned short w, unsigned short h );
  virtual void VideoModeChange( void );

  MapView *GetMapView( void ) const { return mview; }
  Panel *GetPanel( void ) const { return panel; }

  void MoveHex( unsigned short img, const TileSet &tiles, const Point &hex1,
                const Point &hex2, unsigned short speed, bool blink = false );
  Point MoveCursor( Direction dir );
  void FadeOutUnit( unsigned short img, const Point &hex )
       { FadeHex( img, hex, false, true ); }
  void FadeInUnit( unsigned short img, const Point &hex )
       { FadeHex( img, hex, true, true ); }
  void FadeInTerrain( unsigned short img, const Point &hex )
       { FadeHex( img, hex, true, false ); }
  void FlashUnit( const Point &hex, unsigned short times );

  void DisplayHex( const Point &hex );
  void BoxAvoidHexes( Rect &rect, const Point &hex1, const Point &hex2 ) const;

private:
  void FadeHex( unsigned short img, const Point &hex, bool in, bool unit );

  MapView *mview;
  Panel *panel;
};


class TacticalWindow : public Window, public WidgetHook {
public:
  TacticalWindow( MapView *mapview, Mission &m, View *view );

  void Draw( void );
  GUI_Status WidgetActivated( Widget *widget, Window *win );

private:
  void CalcViewPort( Rect &vp ) const;

  MapView *mv;
  MapWidget *mw;
  Mission &mission;
  Player &p1;
  Player &p2;
};


// window displaying the results of a battle
class CombatWindow : public Window {
public:
  CombatWindow( Combat *combat, MapWindow *mapwin, View *view );

  virtual void Draw( void );

  ButtonWidget *button;

private:
  void DrawState( void );

  MapWindow *mapwin;

  Unit *att;
  Unit *def;
  Point apos;
  Point dpos;
  short startgroup1;
  short startgroup2;

  Rect msgbar1;
  Rect msgbar2;
  Rect att_anchor;
  Rect def_anchor;
  Rect clock[2][6];
};

#endif	/* _INCLUDE_MAPWINDOW_H */

