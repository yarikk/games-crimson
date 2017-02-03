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
// extwindow2.h - extended window classes for CoMET
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_ED_EXTWINDOW2_H
#define _INCLUDE_ED_EXTWINDOW2_H

#include "window.h"
#include "button.h"
#include "misc.h"
#include "mission.h"
#include "mapview.h"

class ListExchangeWindow : public Window, public WidgetHook {
public:
  ListExchangeWindow( TLWList &l1, TLWList &l2, const char *label1,
                      const char *label2, View *view );
  void SetHook( WidgetHook *hook ) const { ok->SetHook(hook); }

private:
  GUI_Status WidgetActivated( Widget *widget, Window *win );

  enum {
    B_ID_OK = 1,
    B_ID_RIGHT,
    B_ID_LEFT
  };

  ButtonWidget *ok;
  TextListWidget *wd_l1;
  TextListWidget *wd_l2;
  TLWList &list1;
  TLWList &list2;
};

class ListSelectWindow : public Window, public WidgetHook {
public:
  ListSelectWindow( const char *title, TLWList &list,
            short defid, bool nullok, WidgetHook *hook,
            short button, View *view );

  TLWNode *Selected( void ) const { return current; }

private:
  GUI_Status WidgetActivated( Widget *widget, Window *win );

  short button;
  bool nullok;
  TextListWidget *lw;
  const TLWList &list;
  WidgetHook *client;
  TLWNode *current;
};

// create a new mission
class NewMissionWindow : public Window, public WidgetHook {
public:
  NewMissionWindow( View *view );

  void SetHook( WidgetHook *hook, short id )
    { this->hook = hook; ok_id = id; }

  Mission *GetMission( void ) const { return mission; }

private:
  Mission *NewMission( const Point &size, const string &tset,
                   const string &uset );
  Point GetMapSize( void ) const;
  const char *GetTileSet( void ) const { return tileset->String(); }
  const char *GetUnitSet( void ) const { return unitset->String(); }

  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_UNITSET = 1,
    B_ID_TILESET,
    B_ID_UNITSET_OK,
    B_ID_TILESET_OK,
    B_ID_RANDOM,
    B_ID_OK
  };

  Mission *mission;

  NumberWidget *m_width;
  NumberWidget *m_height;
  StringWidget *tileset;
  StringWidget *unitset;
  CheckboxWidget *gen_random;
  SliderWidget *gen_water;
  SliderWidget *gen_roughness;
  WidgetHook *hook;
  short ok_id;
};

class EdMissionSetupWindow : public Window, public WidgetHook {
public:
  EdMissionSetupWindow( Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *widget, Window *win );

  enum {
    B_ID_NAME,
    B_ID_INFO,
    B_ID_PLAYER1,
    B_ID_PLAYER2,
    B_ID_BRIEF1,
    B_ID_BRIEF2,
    B_ID_CAMPAIGN_NAME,
    B_ID_CAMPAIGN_INFO,
    B_ID_SHOWMSG,
    B_ID_SET,
    S_ID_SEQUEL,
    B_ID_PLAYERS,
    S_ID_MUSIC,
    B_ID_CAMPAIGN,
    B_ID_SKIRMISH
  };

  Mission &mission;

  StringWidget *nextmap;
  TextScrollWidget *msg;
  CycleWidget *showmsg;

  TLWList tlist;
};

class EdBuildingWindow : public Window, public WidgetHook {
public:
  EdBuildingWindow( Building &bld, Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *widget, Window *win );

  enum {
    B_ID_PLAYER = 1,
    B_ID_WORKSHOP,
    B_ID_FACTORY,
    B_ID_CRYSTALS,
    B_ID_OK,
    B_ID_NAME,
    B_ID_NAME_OK,
    B_ID_UNITS,
    N_ID_MAXCRYSTALS,
    B_ID_FACTORY_UNITS
  };

  Building &b;
  Mission &mission;

  StringWidget *b_name;
  CycleWidget *b_player;
  NumberWidget *b_crystals;
  NumberWidget *b_maxcrystals;
  NumberWidget *b_mining;
  ButtonWidget *b_factory_units;

  TLWList fact_list_ok;
  TLWList fact_list_na;
  TLWList msg_list;
};


class EdUnitWindow : public Window, public WidgetHook {
public:
  EdUnitWindow( Unit &unit, Mission &mission, View *view );

  void SetMapView( MapView &mapview ) { mv = &mapview; }

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_PLAYER = 1,
    B_ID_DIRECTION,
    B_ID_SIZE,
    B_ID_XP,
    N_ID_CRYSTALS,
    B_ID_UNITS,
    B_ID_OK
  };

  Unit &u;
  Mission &mission;
  MapView *mv;           // we need this to update the map display
                         // if e.g. owner or direction changes
  CycleWidget *u_player;
};


class EdEventsWindow : public Window, public WidgetHook {
public:
  EdEventsWindow( Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_NEW = 1,
    B_ID_EDIT,
    B_ID_DELETE,
    B_ID_OK
  };

  TextListWidget *e_list;
  TLWList nodes;
  Mission &mission;
};


class SelectEventWindow : public Window, public WidgetHook {
public:
  SelectEventWindow( Mission &mission, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );
  void BuildTLWList( TLWList &list ) const;

  enum {
    B_ID_TRIGGER = 1,
    B_ID_OK,
    B_ID_CANCEL
  };

  TextListWidget *e_list;
  CycleWidget *e_trigger;
  TLWList nodes;
  Mission &mission;
};

class EdUnitsWindow : public Window, public WidgetHook {
public:
  EdUnitsWindow( Mission &mission, MapObject &container, View *view );

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );
  void BuildTLWList( TLWList &list ) const;

  enum {
    B_ID_NEW = 1,
    B_ID_EDIT,
    B_ID_DELETE,
    B_ID_OK
  };

  TextListWidget *e_list;
  TLWList nodes;
  TLWList utypes;
  Mission &mission;
  MapObject &container;
};

class SelectUnitWindow : public Window, public WidgetHook {
public:
  SelectUnitWindow( Mission &mission, WidgetHook *hook, View *view );
  SelectUnitWindow( Mission &mission, TLWList &list,
                    WidgetHook *hook, View *view );

  const UnitType *Selected( void ) const;

private:
  GUI_Status WidgetActivated( Widget *button, Window *win );
  void Init( TLWList &list, WidgetHook *hook );

  enum {
    B_ID_CANCEL = 1
  };

  TextListWidget *e_list;
  TLWList nodes;
  Mission &mission;
};

class EdMsgWindow : public Window, public WidgetHook {
public:
  EdMsgWindow( Locale &locale, View *view );

private:
  void Draw( void );
  void BuildLangsList( void );
  void BuildMsgsList( void );
  GUI_Status WidgetActivated( Widget *button, Window *win );

  enum {
    B_ID_LANG_NEW = 1,
    B_ID_LANG_DELETE,
    B_ID_MSG_ADD,
    B_ID_MSG_MODIFY,
    L_ID_LANGS,
    L_ID_MSGS
  };

  TextListWidget *w_langs;
  TextListWidget *w_msgs;
  StringWidget *w_msg;
  StringWidget *w_lid;
  Locale &locale;

  TLWList l_langs;
  TLWList l_msgs;
  Language *cur_lang;
  int cur_msg_id;
};

#endif  // _INCLUDE_ED_EXTWINDOW2_H

