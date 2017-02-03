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

//////////////////////////////////////////////////////////////////////////
// unitwindow.h - the window classes used for getting unit specifications
//                and showing contents of buildings and transports
//////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDE_UNITWINDOW_H
#define _INCLUDE_UNITWINDOW_H

#include "window.h"
#include "button.h"
#include "listselect.h"
#include "textbox.h"
#include "map.h"
#include "history.h"

class UnitListWidget;
class ULWNode;

// the same window class is used for buildings and transports

#define CH_NUM_BUTTONS    3

class ContainerWindow : public Window, public WidgetHook {
public:
  ContainerWindow( UnitContainer *container, View *view );
  ~ContainerWindow( void ) {}

  GUI_Status HandleEvent( const SDL_Event &event );
  void SwitchMode( unsigned short newmode );
  GUI_Status SelectNode( ULWNode *node );

  unsigned short Mode( void ) const { return mode; }

  void Draw( void );
  void DrawCrystals( void );

  GUI_Status WidgetActivated( Widget *widget, Window *win );

private:
  UnitContainer *c;

  bool unit;                          // true if transport, false if building
  unsigned short mode;

  List normal;
  List build;
  Rect unitinfo;
  Image crystals_icon;

  UnitListWidget *listwidget;
  ButtonWidget *buttons[CH_NUM_BUTTONS];

  Node *last_selected;
};

class UnitLoadWindow : public Window, public WidgetHook {
public:
  UnitLoadWindow( Transport &t, UnitContainer &c,
                  const UnitSet &uset, const TerrainSet &tset,
                  History *history, View *view );

  void Draw( void );
  GUI_Status WidgetActivated( Widget *widget, Window *win );

  bool Opened( void ) const { return have_units || (c.Crystals() > 0); }

private:
  Transport &t;
  UnitContainer &c;
  List units;
  ULWNode *last_selected;
  unsigned short fullslots; // we can't change these in the transport
  unsigned short crystals;  // because the user may cancel
  bool have_units;

  StringWidget *wd_crystals;
  SliderWidget *wd_slider;
  ListWidget *wd_list;
  Rect unitinfo;
  unsigned short cstrw;

  History *history;
};

#endif  /* _INCLUDE_UNITWINDOW_H */

