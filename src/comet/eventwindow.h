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
// eventwindow.h - event editing window classes for CoMET
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_ED_EVENTWINDOW_H
#define _INCLUDE_ED_EVENTWINDOW_H

#include "window.h"
#include "button.h"
#include "mission.h"

class EdEventGenericWindow : public Window, public WidgetHook {
public:
  EdEventGenericWindow( Event &event, Mission &mission, View *view );
  void Draw( void );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_TITLE = 1,
    B_ID_MSG,
    B_ID_PLAYER,
    B_ID_DEPEND,
    B_ID_DISCARD,
    B_ID_TRIGGER,
    B_ID_EVENT,
    B_ID_TITLE_OK,
    B_ID_MSG_OK,
    B_ID_DEPEND_OK,
    B_ID_DISCARD_OK,
    B_ID_DISABLED
  };

  TextScrollWidget *e_msg;
  StringWidget *e_title;
  StringWidget *e_depends;
  StringWidget *e_discards;
  TLWList tlist;

  Event &event;
  Mission &mission;
};

class EdEventCreateUnitWindow : public Window, public WidgetHook {
public:
  EdEventCreateUnitWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_UNIT = 1,
    B_ID_UNIT_OK,
    N_ID_XPOS,
    N_ID_YPOS,
    B_ID_DIRECTION,
    B_ID_SIZE,
    B_ID_XP
  };

  StringWidget *e_unit;
  TLWList tlist;

  Event &event;
  Mission &mission;
};

class EdEventDestroyUnitWindow : public Window, public WidgetHook {
public:
  EdEventDestroyUnitWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_UNIT = 1,
    B_ID_UNIT_OK,
    B_ID_PLAYER,
    N_ID_XPOS,
    N_ID_YPOS
  };

  StringWidget *e_unit;
  NumberWidget *e_xpos;
  NumberWidget *e_ypos;
  TLWList tlist;

  Event &event;
  Mission &mission;
};

class EdEventManipEventWindow : public Window, public WidgetHook {
public:
  EdEventManipEventWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_EVENT = 1,
    B_ID_EVENT_OK,
    B_ID_ACTION,
    B_ID_DISABLED
  };

  StringWidget *e_event;
  TLWList tlist;

  Event &event;
  Mission &mission;
};

class EdEventMessageWindow : public Window, public WidgetHook {
public:
  EdEventMessageWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  NumberWidget *e_xpos;
  NumberWidget *e_ypos;

  Event &event;
  Mission &mission;
};

class EdEventMiningWindow : public Window, public WidgetHook {
public:
  EdEventMiningWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  void SetMiningRange( short action ) const;

  enum {
    B_ID_MINE = 1,
    B_ID_MINE_OK,
    B_ID_ACTION,
    N_ID_CRYSTALS
  };

  StringWidget *e_mine;
  NumberWidget *e_crystals;
  TLWList tlist;

  Event &event;
  Mission &mission;
};

class EdEventConfigureWindow : public Window, public WidgetHook {
public:
  EdEventConfigureWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );
  const char *GetValueString( void ) const;

  enum {
    B_ID_SETTING = 1,
    B_ID_MSG,
    B_ID_MSG_OK,
    S_ID_VALUE
  };

  ButtonWidget *e_msg_but;
  StringWidget *e_msg;
  TLWList tlist;

  Event &event;
  Mission &mission;
};

class EdEventResearchWindow : public Window, public WidgetHook {
public:
  EdEventResearchWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_UNIT = 1,
    B_ID_UNIT_OK,
    B_ID_SHOP,
    B_ID_SHOP_OK,
    B_ID_ACTION
  };

  StringWidget *e_unit;
  StringWidget *e_shop;
  TLWList tlist;

  Event &event;
  Mission &mission;
};

class EdEventScoreWindow : public Window, public WidgetHook {
public:
  EdEventScoreWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_MSG = 1,
    B_ID_MSG_OK,
    B_ID_TITLE,
    B_ID_TITLE_OK,
    N_ID_SCORE
  };

  Event &event;
  Mission &mission;

  TLWList msglist;
  StringWidget *other_msg;
  StringWidget *other_title;
};

class EdEventSetHexWindow : public Window, public WidgetHook {
public:
  EdEventSetHexWindow( Event &event, Mission &mission, View *view );

private:
  void Draw( void );
  GUI_Status WidgetActivated( Widget *button, Window *win );
  void DrawTerrain( unsigned short terrain, bool update );

  enum {
    B_ID_TILE_PREV = 1,
    B_ID_TILE_NEXT,
    N_ID_XPOS,
    N_ID_YPOS
  };

  Rect tilepos;
  Event &event;
  Mission &mission;
};

class EdEventSetTimerWindow : public Window, public WidgetHook {
public:
  EdEventSetTimerWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_EVENT = 1,
    B_ID_EVENT_OK,
    B_ID_OFFSET,
    N_ID_TIME
  };

  StringWidget *e_event;
  TLWList tlist;

  Event &event;
  Mission &mission;
};

class EdTrigHaveCrystalsWindow : public Window, public WidgetHook {
public:
  EdTrigHaveCrystalsWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_SHOP = 1,
    B_ID_SHOP_OK,
    B_ID_PLAYER,
    N_ID_CRYSTALS,
    B_ID_MOREORLESS,
    B_ID_TRANSPORTS
  };

  StringWidget *t_shop;
  CheckboxWidget *t_transports;
  TLWList tlist;

  Event &event;
  Mission &mission;
};

class EdTrigHaveShopWindow : public Window, public WidgetHook {
public:
  EdTrigHaveShopWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_SHOP = 1,
    B_ID_SHOP_OK,
    B_ID_PLAYER,
    N_ID_TIMER
  };

  StringWidget *t_shop;
  TLWList tlist;

  Event &event;
  Mission &mission;
};

class EdTrigHaveUnitWindow : public Window, public WidgetHook {
public:
  EdTrigHaveUnitWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_UNIT = 1,
    B_ID_UNIT_OK,
    B_ID_PLAYER,
    N_ID_TIMER
  };

  StringWidget *t_unit;
  TLWList tlist;

  Event &event;
  Mission &mission;
};

class EdTrigTimerWindow : public Window, public WidgetHook {
public:
  EdTrigTimerWindow( Event &event, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  Event &event;
};

class EdTrigUnitDestroyedWindow : public Window, public WidgetHook {
public:
  EdTrigUnitDestroyedWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_UNIT = 1,
    B_ID_UNIT_OK,
    B_ID_PLAYER,
    B_ID_USELECT
  };

  StringWidget *t_unit;
  CycleWidget *t_player;
  CycleWidget *t_uselect;
  TLWList tlist;

  Event &event;
  Mission &mission;
};

class EdTrigUnitPositionWindow : public Window, public WidgetHook {
public:
  EdTrigUnitPositionWindow( Event &event, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_UNIT = 1,
    B_ID_UNIT_OK,
    N_ID_POSX,
    N_ID_POSY,
    B_ID_PLAYER,
    B_ID_USELECT
  };

  StringWidget *t_unit;
  CycleWidget *t_uselect;
  TLWList tlist;

  Event &event;
  Mission &mission;
};

class EdTrigHandicapWindow : public Window, public WidgetHook {
public:
  EdTrigHandicapWindow( Event &event, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  Event &event;
};

#endif	// _INCLUDE_ED_EVENTWINDOW_H

