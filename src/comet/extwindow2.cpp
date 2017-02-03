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
// extwindow2.cpp
///////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "extwindow2.h"
#include "extwindow.h"
#include "eventwindow.h"
#include "filewindow.h"
#include "fileio.h"
#include "uiaux.h"
#include "mapgen.h"

// static vars
const char *player2_labels[] = { "One", "Two", 0 };
const char *player3_labels[] = { "One", "Two", "Neutral", 0 };

const char *dir_labels[] = { "North", "Northeast", "Southeast",
                             "South", "Southwest", "Northwest", 0 };
const char *xp_labels[] = { "0", "1", "2", "3", "4", "5", "6", 0 };
const char *size_labels[] = { "1", "2", "3", "4", "5", "6", 0 };

extern const char *event_labels[];
extern const char *etrigger_labels[];

// non-private widget ID for the SelectUnitWindow
#define B_ID_SUW_OK      102


////////////////////////////////////////////////////////////////////////
// NAME       : ListExchangeWindow::ListExchangeWindow
// DESCRIPTION: This window displays two TLWList objects. The user can
//              move nodes from one list to the other and vice versa.
// PARAMETERS : l1     - first TLWList
//              l2     - second TLWList
//              label1 - label to be shown for l1
//              label2 - label to be shown for l2
//              view   - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

ListExchangeWindow::ListExchangeWindow( TLWList &l1, TLWList &l2,
            const char *label1, const char *label2, View *view )
          : Window(WIN_CENTER, view), list1(l1), list2(l2) {
  // calculate window size
  unsigned short maxlen = sfont->TextWidth(label1), nodes1 = 0, nodes2 = 0;
  TLWNode *n;

  for ( n = static_cast<TLWNode *>(l1.Head()); n;
        n = static_cast<TLWNode *>(n->Next()) ) {
    maxlen = MAX( maxlen, sfont->TextWidth(n->Name()) );
    ++nodes1;
  }
  for ( n = static_cast<TLWNode *>(l2.Head()); n;
        n = static_cast<TLWNode *>(n->Next()) ) {
    maxlen = MAX( maxlen, sfont->TextWidth(n->Name()) );
    ++nodes2;
  }

  Rect win( 0, 0, (maxlen * 2 + 10) + sfont->Width() + 30,
            (MAX(nodes1, nodes2) + 4) * (sfont->Height() + 4) + 30 );
  win.Clip( *view );
  SetSize( win );

  wd_l1 = new TextListWidget( 0, 5, sfont->Height() + 10,
          w/2 - sfont->Width() - 10, h - sfont->Height() * 2 - 20,
          &l1, -1, WIDGET_ALIGN_TOP, label1, this );

  wd_l2 = new TextListWidget( 0, w - 5 - wd_l1->Width(),
          wd_l1->TopEdge(), wd_l1->Width(), wd_l1->Height(),
          &l2, -1, WIDGET_ALIGN_TOP, label2, this );

  Widget *wd;
  wd = new ButtonWidget( B_ID_RIGHT,
       wd_l1->LeftEdge() + wd_l1->Width() + 5,
       wd_l1->TopEdge() + wd_l1->Height()/2 - sfont->Height() - 15,
       wd_l2->LeftEdge() - wd_l1->LeftEdge() - wd_l1->Width() - 10,
       sfont->Height() + 8, 0, "_>", this );
  wd->SetHook( this );

  wd = new ButtonWidget( B_ID_LEFT, wd->LeftEdge(),
       wd->TopEdge() + wd->Height() + 7, wd->Width(), wd->Height(),
       0, "_<", this );
  wd->SetHook( this );

  ok = new ButtonWidget( B_ID_OK, 1, h - sfont->Height() - 9,
                         w - 2, sfont->Height() + 8,
                         WIDGET_DEFAULT, "_OK", this );
  ok->SetHook( this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : ListExchangeWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status ListExchangeWindow::WidgetActivated( Widget *widget, Window *win ) {
  TLWNode *n;

  switch ( widget->ID() ) {
  case B_ID_RIGHT:
    n = static_cast<TLWNode *>(wd_l1->Selected());
    if ( n ) {
      n->Remove();
      list2.InsertNodeSorted( n );
      wd_l1->Update(); wd_l1->Draw(); wd_l1->Show();
      wd_l2->Update(); wd_l2->Draw(); wd_l2->Show();
    }
    break;
  case B_ID_LEFT:
    n = static_cast<TLWNode *>(wd_l2->Selected());
    if ( n ) {
      n->Remove();
      list1.InsertNodeSorted( n );
      wd_l1->Update(); wd_l1->Draw(); wd_l1->Show();
      wd_l2->Update(); wd_l2->Draw(); wd_l2->Show();
    }
    break;
  case B_ID_OK:
    view->CloseWindow( this );
    break;
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : ListSelectWindow::ListSelectWindow
// DESCRIPTION: This window displays the contents of a TLWList object.
//              The user can select a single node (or none) and click
//              OK.
// PARAMETERS : title  - window title
//              list   - TLWList object to display
//              defid  - identifier of the node to be highlighted by
//                       default (-1 means no node selected)
//              nullok - if true, accept user hitting OK without a node
//                       selected and deselect a node if clicked twice;
//                       if false, refuse to close the window until the
//                       user has made a choice. If the list is empty,
//                       it will close even if nullok is false.
//              hook   - WidgetHook to notify when the user hits OK
//              button - button identifier passed to the hook
//              view   - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

ListSelectWindow::ListSelectWindow( const char *title, TLWList &list,
                  short defid, bool nullok, WidgetHook *hook,
                  short button, View *view ) :
          Window(WIN_CENTER, view), button(button), nullok(nullok),
          list(list), client(hook), current(0) {
  // calculate window size
  unsigned short maxlen = sfont->TextWidth(title) + 10, nodes = 0;
  short defnode = -1;
  TLWNode *n;

  for ( n = static_cast<TLWNode *>(list.Head()); n;
        n = static_cast<TLWNode *>(n->Next()) ) {
    maxlen = MAX( maxlen, sfont->TextWidth(n->Name()) );

    if ( n->ID() == defid ) {
      defnode = nodes;
      current = n;
    }
    ++nodes;
  }

  Rect win( 0, 0, maxlen + 20, (nodes + 2) * (sfont->Height() + 4) + 25 );
  win.Clip( *view );
  SetSize( win );

  lw = new TextListWidget( button + 1, 5, sfont->Height() + 10,
           w - 10, h - sfont->Height() * 2 - 20, &list, defnode,
           WIDGET_ALIGN_TOP|WIDGET_HSCROLLKEY|WIDGET_VSCROLLKEY,
           title, this );
  lw->SetHook( this );

  Widget *wd = new ButtonWidget( button, 1, h - sfont->Height() - 9,
                   w - 2, sfont->Height() + 8, WIDGET_DEFAULT,
                   "_OK", this );
  wd->SetHook( this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : ListSelectWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status ListSelectWindow::WidgetActivated( Widget *widget, Window *win ) {

  if ( widget->ID() == button ) {

    if ( nullok || current || (list.CountNodes() == 0) ) {
      view->CloseWindow( this );
      client->WidgetActivated( widget, this );
    } else new NoteWindow( "Error", "You must make a selection", 0, view );

  } else {

    TLWNode *n = static_cast<TLWNode *>(lw->Selected());
    if ( (n == current) && nullok ) {
      lw->Select( -1 );
      current = NULL;
    } else current = n;
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : NewMissionWindow::NewMissionWindow
// DESCRIPTION: This window pops up when the user wants to create a new
//              mission. He can adjust a few basic settings like map
//              size or tileset and create the mission.
// PARAMETERS : view - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

NewMissionWindow::NewMissionWindow( View *view ) :
   Window(WIN_CENTER, view), mission(0), hook(0), ok_id(B_ID_OK) {
  // calculate window dimensions
  Font *f = SmallFont();
  unsigned short width = MIN( f->Width() * 38 + 20, view->Width() );
  unsigned short height = f->Height() * 6 + 60;
  unsigned short xoff, yoff = 5;
  xoff = MAX(f->TextWidth("Map Width:"), f->TextWidth("Map Height:")) + 10;

  SetSize( width, height );

  m_width = new NumberWidget( 1, xoff, yoff, f->Width() * 4 + 10,
            f->Height() + 6, 40, 20, 180, WIDGET_ALIGN_LEFT,
            "Map _Width:", this );

  width = MIN( f->Width() * 11 + 10, w - xoff - m_width->w - 24 - f->Width() * 12 );
  tileset = new StringWidget( 3, xoff + m_width->w + 10 + f->Width() * 9,
            yoff, width, m_width->h, "default", 10,
            WIDGET_ALIGN_LEFT|WIDGET_STR_CONST, "_Tile Set:", this );

  ButtonWidget *btn = new ButtonWidget( B_ID_TILESET,
             tileset->x + tileset->w, tileset->y,
             f->Width() * 3 + 4, tileset->h, 0, "...", this );
  btn->SetKey( 't' );
  btn->SetHook( this );

  yoff += m_width->Height() + 5;

  m_height = new NumberWidget( 2, xoff, yoff, m_width->w, m_width->h,
            40, 20, 180, WIDGET_ALIGN_LEFT, "Map _Height:", this );

  unitset = new StringWidget( 4, tileset->x, yoff, tileset->w, tileset->h,
            "default", 10, WIDGET_ALIGN_LEFT|WIDGET_STR_CONST,
            "_Unit Set:", this );

  btn = new ButtonWidget( B_ID_UNITSET,
             btn->x, unitset->y, btn->w, unitset->h, 0, "...", this );
  btn->SetKey( 'u' );
  btn->SetHook( this );

  yoff += btn->Height() + 5;
  gen_random = new CheckboxWidget( B_ID_RANDOM, 5, yoff,
               DEFAULT_CBW_SIZE, DEFAULT_CBW_SIZE, true,
               WIDGET_STYLE_GFX|WIDGET_STYLE_NOBORDER|WIDGET_ALIGN_RIGHT,
               "_Generate random terrain", this );
  gen_random->SetHook( this );

  yoff += gen_random->Height() + 5;
  xoff = gen_random->LeftEdge() + 10 + f->TextWidth("Water Level");
  gen_water = new SliderWidget( 0, xoff, yoff,
              w - xoff - 5, DEFAULT_SLIDER_SIZE, 0, 10, 4, 1,
              WIDGET_HSCROLL|WIDGET_ALIGN_LEFT, "W_ater Level", this );

  yoff += gen_water->Height() + 5;
  gen_roughness = new SliderWidget( 0, xoff, yoff,
                  w - xoff - 5, DEFAULT_SLIDER_SIZE, 1, 20, 6, 2,
                  WIDGET_HSCROLL|WIDGET_ALIGN_LEFT, "_Roughness", this );

  yoff = h - f->Height() - 15;
  btn = new ButtonWidget( B_ID_OK, 10, yoff,
        f->Width() * 10, f->Height() + 10, 0, "_OK", this );
  btn->SetHook( this );

  new ButtonWidget( GUI_CLOSE,
      w - 10 - btn->w, yoff, btn->w, btn->h, 0, "_Cancel", this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : NewMissionWindow::GetMapSize
// DESCRIPTION: Get the current slider settings.
// PARAMETERS : -
// RETURNS    : selected size as a Point
////////////////////////////////////////////////////////////////////////

Point NewMissionWindow::GetMapSize( void ) const {
  return Point( m_width->Number(), m_height->Number() );
}

////////////////////////////////////////////////////////////////////////
// NAME       : NewMissionWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : button - activated button widget
//              win    - window containing the button
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status NewMissionWindow::WidgetActivated( Widget *button, Window *win ) {
  FileWindow *fw;

  switch ( button->ID() ) {
  case B_ID_TILESET:
    fw = new FileWindow( get_data_dir().c_str(),
                         GetTileSet(), ".tiles", 0, view );
    fw->ok->SetID( B_ID_TILESET_OK );
    fw->ok->SetHook( this );
    break;
  case B_ID_TILESET_OK: {
    string file = static_cast<FileWindow *>(win)->GetFile();
    if ( file.length() > 0 ) {
      file = file_part( file );
      file.erase( file.length() - 6 );   // remove file name suffix
      tileset->SetString( file.c_str() );
      view->CloseWindow( win );

      if ( file == "default" ) gen_random->Enable();
      else gen_random->Disable();
      gen_random->Draw();
      WidgetActivated( gen_random, this );
    }
    break; }
  case B_ID_UNITSET:
    fw = new FileWindow( get_data_dir().c_str(),
                         GetUnitSet(), ".units", 0, view );
    fw->ok->SetID( B_ID_UNITSET_OK );
    fw->ok->SetHook( this );
    break;
  case B_ID_UNITSET_OK: {
    string file = static_cast<FileWindow *>(win)->GetFile();
    if ( file.length() > 0 ) {
      file = file_part( file );
      file.erase( file.length() - 6 );   // remove file name suffix
      unitset->SetString( file.c_str() );
      view->CloseWindow( win );
    }
    break; }
  case B_ID_RANDOM:
    if ( button->Disabled() || !button->Clicked() ) {
      gen_water->Disable();
      gen_roughness->Disable();
    } else {
      gen_water->Enable();
      gen_roughness->Enable();
    }
    gen_water->Draw();
    gen_roughness->Draw();
    Show();
    break;
  case B_ID_OK:
    if ( hook ) {
      mission = NewMission( GetMapSize(),
                GetTileSet(), GetUnitSet() );
      if ( mission ) {
        if ( !gen_random->Disabled() && gen_random->Clicked() ) {
          MapGenerator gen;
          gen.Generate( mission->GetMap(),
              gen_water->Level(), gen_roughness->Level() );
        }

        // add the most basic messages
        Language en;
        en.SetID( CF_LANG_DEFAULT );
        en.AddMsg( "Unnamed" );   // map name
        en.AddMsg( "Player 1" );  // name player 1
        en.AddMsg( "Player 2" );  // name player 2
        en.AddMsg( "HQ 1" );      // HQ player 1
        en.AddMsg( "HQ 2" );      // HQ player 2
        mission->GetMessages().AddLanguage( en );
        mission->SetName( 0 );
        mission->GetPlayer( PLAYER_ONE ).SetNameID( 1 );
        mission->GetPlayer( PLAYER_TWO ).SetNameID( 2 );

        // use the button ID supplied by the caller
        button->SetID( ok_id );
        hook->WidgetActivated( button, this );
        button->SetID( B_ID_OK );
      }
    }
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : NewMissionWindow::NewMission
// DESCRIPTION: Create a new mission.
// PARAMETERS : size - map size
//              tset - tile set name
//              uset - unit set name
// RETURNS    : pointer to new mission or NULL on error
////////////////////////////////////////////////////////////////////////

Mission *NewMissionWindow::NewMission( const Point &size,
         const string &tset, const string &uset ) {
  Mission *ms = NULL;
  TerrainSet *ts = new TerrainSet;
  UnitSet *us = new UnitSet;

  string tname( get_data_dir() + tset + ".tiles" );
  File tfile( tname );

  string uname( get_data_dir() + uset + ".units" );
  File ufile( uname );

  if ( !tfile.Open("rb") || ts->Load( tfile, tset.c_str() ) ) {
    new NoteWindow( "Error", "Tile set not available", 0, view );
  } else if ( !ufile.Open("rb") || us->Load( ufile, uset.c_str() ) ) {
    new NoteWindow( "Error", "Unit set not available", 0, view );
  } else {
    ms = new Mission( size, ts, us );
  }

  if ( !ms ) {
    delete ts;
    delete us;
  }

  tfile.Close();
  ufile.Close();
  return ms;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdMissionSetupWindow::EdMissionSetupWindow
// DESCRIPTION: This window allows modification of the basic mission
//              settings like player names.
// PARAMETERS : mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdMissionSetupWindow::EdMissionSetupWindow( Mission &mission, View *view ) :
       Window(WIN_CENTER, view), mission(mission) {
  // calculate dimensions
  SetSize( MIN(sfont->Width() * 40 + 20, view->Width()),
           MIN(sfont->Height() * 11 + 110, view->Height()) );

  static const char *msg_labels[] = { "Level Name", "Level Info",
                                      "Name 1", "Name 2",
                                      "Briefing 1", "Briefing 2",
                                      "Campaign Name", "Campaign Info", 0 };
  static const char *type_labels[] = { "Solo", "Duel", 0 };

  unsigned short wdh = sfont->Height() + 8, wdx, wdy = 5;
  Widget *wd;

  wdx = sfont->Width() * 4 + 10;
  wd = new CycleWidget( B_ID_PLAYERS, wdx, wdy,
            w/2 - wdx - 5, wdh, WIDGET_ALIGN_LEFT,
            "_Type", mission.GetNumPlayers()-1, type_labels, this );
  wd->SetHook( this );

  wdx = w/2 + sfont->Width() * 8 + 10;
  wd = new StringWidget( S_ID_SEQUEL, wdx, wdy, w - wdx - 5, wdh,
       mission.GetSequel(), 20, WIDGET_ALIGN_LEFT, "_Next Map", this );
  wd->SetHook( this );

  wdy += wdh + 5;
  wd = new StringWidget( S_ID_MUSIC, wdx, wdy, w - wdx - 5, wdh,
       mission.GetMusic(), 20, WIDGET_ALIGN_LEFT, "M_usic", this );
  wd->SetHook( this );

  wd = new CheckboxWidget( B_ID_CAMPAIGN, 5, wdy + (wdh - DEFAULT_CBW_SIZE)/2,
       DEFAULT_CBW_SIZE, DEFAULT_CBW_SIZE, mission.IsCampaign(),
       WIDGET_STYLE_NOBORDER|WIDGET_STYLE_GFX|WIDGET_ALIGN_RIGHT,
       "_Campaign Map", this );
  wd->SetHook( this );

  wdx = wd->LeftEdge() + wd->Width() + sfont->TextWidth("Campaign Map") + 15;
  wd = new CheckboxWidget( B_ID_SKIRMISH, wdx, wd->TopEdge(),
       DEFAULT_CBW_SIZE, DEFAULT_CBW_SIZE, mission.IsSkirmish(),
       WIDGET_STYLE_NOBORDER|WIDGET_STYLE_GFX|WIDGET_ALIGN_RIGHT,
       "S_kirmish Map", this );
  wd->SetHook( this );

  wdy += wdh + 10;
  wd = new ButtonWidget( B_ID_SET, w - 15 - sfont->Width() * 3, wdy,
       sfont->Width() * 3 + 10, wdh, 0, "_Set", this );
  wd->SetHook( this );

  // the user can use the following widget to choose which message to
  // display in the textbox below
  wdx = sfont->Width() * 7 + 10;
  showmsg = new CycleWidget( B_ID_SHOWMSG, wdx, wdy,
            wd->LeftEdge() - wdx - 5, wdh, WIDGET_ALIGN_LEFT,
            "_Message", 0, msg_labels, this );
  showmsg->SetHook( this );

  wdy += wdh + 5;
  msg = new TextScrollWidget( 0, 5, wdy,
        w - 10, h - wdh - 5 - wdy, mission.GetMessage(mission.GetName()),
        0, NULL, this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );

  Draw();
  Show();
  TLWListBuilder::BuildMsgList( mission.GetMessages(), tlist );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdMissionSetupWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the button
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdMissionSetupWindow::WidgetActivated( Widget *widget, Window *win ) {
  TLWNode *n;
  unsigned char pid;

  switch ( widget->ID() ) {
  case B_ID_BRIEF1:
  case B_ID_BRIEF2:
    pid = widget->ID() - B_ID_BRIEF1;
    n = static_cast<ListSelectWindow *>(win)->Selected();

    mission.GetPlayer(pid).SetBriefing( n ? n->ID() : -1 );
    msg->SetText( n ? mission.GetMessage(n->ID()) : NULL );
    break;

  case B_ID_NAME:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    mission.SetName( n ? n->ID() : -1 );
    msg->SetText( n ? mission.GetMessage(n->ID()) : NULL );
    break;

  case B_ID_INFO:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    mission.SetLevelInfoMsg( n ? n->ID() : -1 );
    msg->SetText( n ? mission.GetMessage(n->ID()) : NULL );
    break;

  case B_ID_CAMPAIGN:
    mission.SetCampaign( widget->Clicked() );
    break;

  case B_ID_CAMPAIGN_INFO:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    mission.SetCampaignInfo( n ? n->ID() : -1 );
    msg->SetText( n ? mission.GetMessage(n->ID()) : NULL );
    break;

  case B_ID_CAMPAIGN_NAME:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    mission.SetCampaignName( n ? n->ID() : -1 );
    msg->SetText( n ? mission.GetMessage(n->ID()) : NULL );
    break;

  case B_ID_SKIRMISH:
    mission.SetSkirmish( widget->Clicked() );
    break;

  case S_ID_SEQUEL:
    mission.SetSequel( static_cast<StringWidget *>(widget)->String() );
    break;

  case S_ID_MUSIC:
    mission.SetMusic( static_cast<StringWidget *>(widget)->String() );
    break;

  case B_ID_PLAYER1:
  case B_ID_PLAYER2:
    pid = widget->ID() - B_ID_PLAYER1;
    n = static_cast<ListSelectWindow *>(win)->Selected();

    if ( n ) {
      const char *name = mission.GetMessage(n->ID());
      if ( name && strlen(name) > 30 ) {
        new NoteWindow( "Error",
                        "Player names must not be longer than 30 characters.",
                        0, view );
      } else {
        mission.GetPlayer(pid).SetNameID( n->ID() );
        msg->SetText( name );
      }
    } else {
      mission.GetPlayer(pid).SetNameID( -1 );
      msg->SetText( NULL );
    }
    break;

  case B_ID_SET: {
    short msgid;
    bool optional = true;
    const char *label;

    switch ( showmsg->GetValue() ) {
    case B_ID_NAME: msgid = mission.GetName(); optional = false;
                    label = "Select name"; break;
    case B_ID_INFO: msgid = mission.GetLevelInfoMsg();
                    label = "Select info"; break;
    case B_ID_PLAYER1: msgid = mission.GetPlayer(PLAYER_ONE).NameID(); optional = false;
                    label = "Select name"; break;
    case B_ID_PLAYER2: msgid = mission.GetPlayer(PLAYER_TWO).NameID(); optional = false;
                    label = "Select name"; break;
    case B_ID_BRIEF1: msgid = mission.GetPlayer(PLAYER_ONE).Briefing();
                    label = "Select briefing"; break;
    case B_ID_BRIEF2: msgid = mission.GetPlayer(PLAYER_TWO).Briefing();
                    label = "Select briefing"; break;
    case B_ID_CAMPAIGN_NAME: msgid = mission.GetCampaignName();
                    label = "Select campaign name"; break;
    case B_ID_CAMPAIGN_INFO: msgid = mission.GetCampaignInfo();
                    label = "Select campaign info"; break;
    default: return GUI_OK;
    }

    new ListSelectWindow( label, tlist, msgid, optional,
        this, showmsg->GetValue(), view );
    break; }

  case B_ID_SHOWMSG: {
    short msgid;

    switch ( showmsg->GetValue() ) {
    case B_ID_NAME: msgid = mission.GetName(); break;
    case B_ID_INFO: msgid = mission.GetLevelInfoMsg(); break;
    case B_ID_PLAYER1: msgid = mission.GetPlayer(PLAYER_ONE).NameID(); break;
    case B_ID_PLAYER2: msgid = mission.GetPlayer(PLAYER_TWO).NameID(); break;
    case B_ID_BRIEF1: msgid = mission.GetPlayer(PLAYER_ONE).Briefing(); break;
    case B_ID_BRIEF2: msgid = mission.GetPlayer(PLAYER_TWO).Briefing(); break;
    case B_ID_CAMPAIGN_INFO: msgid = mission.GetCampaignInfo(); break;
    case B_ID_CAMPAIGN_NAME: msgid = mission.GetCampaignName(); break;
    default: msgid = -1;
    }

    msg->SetText( (msgid == -1) ? NULL : mission.GetMessage(msgid) );
    break; }

  case B_ID_PLAYERS:
    mission.SetNumPlayers( static_cast<CycleWidget *>(widget)->GetValue()+1 );
    break;
  }

  msg->Draw();
  msg->Show();
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdBuildingWindow::EdBuildingWindow
// DESCRIPTION: This window contains the settings for a building.
// PARAMETERS : bld     - building to be modified
//              mission - currently active mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdBuildingWindow::EdBuildingWindow( Building &bld, Mission &mission, View *view ) :
                  Window(WIN_CENTER, view), b(bld), mission(mission) {
  // calculate dimensions
  SetSize( MIN(sfont->Width() * 50, view->Width()),
           MIN(sfont->Height() * 12, view->Height()) );

  unsigned short xoff, yoff = 5, wdh = sfont->Height() + 10;

  ButtonWidget *btn;
  btn = new ButtonWidget( B_ID_NAME, 5, yoff, sfont->Width() * 4 + 10, wdh,
                          0, "_Name", this ),
  btn->SetHook( this );

  xoff = 10 + btn->Width();
  const char *name = b.Name();
  b_name = new StringWidget( 0, xoff, yoff, w/2 - xoff - 5, wdh,
                             name ? name : "Error", 30, WIDGET_STR_CONST,
                             NULL, this );

  xoff = w/2 + 10 + sfont->TextWidth("Player");
  b_player = new CycleWidget( B_ID_PLAYER, xoff, yoff, w - 5 - xoff, wdh,
                              0, "_Player", b.Owner(), player3_labels, this );
  yoff += 5 + wdh;

  btn = new CheckboxWidget( B_ID_WORKSHOP, 5, yoff, DEFAULT_CBW_SIZE,
                            DEFAULT_CBW_SIZE, b.IsWorkshop(),
                            WIDGET_STYLE_NOBORDER|WIDGET_STYLE_GFX|WIDGET_ALIGN_RIGHT,
                            "_Workshop", this );
  btn->SetHook( this );

  btn = new CheckboxWidget( B_ID_FACTORY, 5, yoff + DEFAULT_CBW_SIZE + 5,
                            DEFAULT_CBW_SIZE, DEFAULT_CBW_SIZE, b.IsFactory(),
                            WIDGET_STYLE_NOBORDER|WIDGET_STYLE_GFX|WIDGET_ALIGN_RIGHT,
                            "_Factory", this );
  btn->SetHook( this );

  btn = new ButtonWidget( B_ID_UNITS, w/2 + 5, yoff, (w - 20)/2, wdh,
                          0, "_Units...", this ),
  btn->SetHook( this );
  yoff += 5 + wdh;

  b_factory_units = new ButtonWidget( B_ID_FACTORY_UNITS, btn->LeftEdge(), yoff,
                    btn->Width(), wdh, (b.IsFactory() ? 0 : WIDGET_DISABLED),
                    "F_actory Settings...", this );
  b_factory_units->SetHook( this );
  yoff += 5 + wdh;

  xoff = 10 + sfont->TextWidth("Crystals");
  b_crystals = new NumberWidget( 0, xoff, yoff, sfont->Width() * 4 + 10, wdh,
                                 b.Crystals(), 0, b.MaxCrystals(),
                                 WIDGET_ALIGN_LEFT, "_Crystals", this );

  xoff += b_crystals->Width() + 10 + sfont->TextWidth("of");
  b_maxcrystals = new NumberWidget( N_ID_MAXCRYSTALS, xoff, yoff,
                                  b_crystals->Width(), wdh,
                                  b.MaxCrystals(), 0, 10000,
                                  WIDGET_ALIGN_LEFT, "of", this );
  b_maxcrystals->SetHook( this );

  xoff = w/2 + 5 + sfont->Width() * 7;
  b_mining = new NumberWidget( 0, xoff, yoff,
                               b_crystals->Width(), wdh,
                               b.CrystalProduction(), 0, 1000,
                               WIDGET_ALIGN_LEFT, "O_utput", this );

  btn = new ButtonWidget( B_ID_OK, 1, h - 1 - wdh, w - 2, wdh,
                          WIDGET_DEFAULT, "_OK", this ),
  btn->SetHook( this );

  Draw();
  Show();

  TLWListBuilder::BuildUnitTypesList( mission.GetUnitSet(), fact_list_na );
  if ( b.IsFactory() ) {
    TLWNode *n = static_cast<TLWNode *>(fact_list_na.Head()), *n2;

    while ( n ) {
      n2 = static_cast<TLWNode *>(n->Next());
      if ( b.CanProduce( n->ID() ) ) {
        n->Remove();
        fact_list_ok.InsertNodeSorted( n );
      }
      n = n2;
    }
  }
  TLWListBuilder::BuildMsgList( mission.GetMessages(), msg_list );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdBuildingWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdBuildingWindow::WidgetActivated( Widget *widget, Window *win ) {

  switch ( widget->ID() ) {
  case B_ID_WORKSHOP:
    if ( widget->Clicked() ) b.SetFlags( BLD_WORKSHOP );
    else b.UnsetFlags( BLD_WORKSHOP );
    break;
  case B_ID_FACTORY:
    if ( widget->Clicked() ) {
      b.SetFlags( BLD_FACTORY );
      b_factory_units->Enable();
    } else {
      b.UnsetFlags( BLD_FACTORY );
      b_factory_units->Disable();
    }
    b_factory_units->Draw();
    b_factory_units->Show();
    break;
  case B_ID_UNITS:
    new EdUnitsWindow( mission, b, view );
    break;
  case B_ID_FACTORY_UNITS:
    new ListExchangeWindow( fact_list_na, fact_list_ok,
                            "Not Available", "Available", view );
    break;
  case B_ID_OK:
    view->CloseWindow( win );
    b.SetOwner( b_player->GetValue() );
    b.SetCrystals( b_crystals->Number() );
    b.SetMaxCrystals( b_maxcrystals->Number() );
    b.SetCrystalProduction( b_mining->Number() );

    for ( Unit *u = static_cast<Unit *>(mission.GetUnits().Head()); u;
          u = static_cast<Unit *>(u->Next()) ) {
      if ( u->Position() == b.Position() ) u->SetOwner( b.Owner() );
    }

    // fix factory settings
    if ( b.IsFactory() ) {
      unsigned long unitmask = 0;

      for ( TLWNode *n = static_cast<TLWNode *>(fact_list_ok.Head());
            n; n = static_cast<TLWNode *>(n->Next()) ) {
        unitmask |= (1 << n->ID());
      }
      b.SetUnitProduction( unitmask );
    } else b.SetUnitProduction( 0 );
    break;
  case N_ID_MAXCRYSTALS:
    b_crystals->SetMax( b_maxcrystals->Number() );
    break;
  case B_ID_NAME:
    new ListSelectWindow( "Select name", msg_list,
        b.NameID(), false, this, B_ID_NAME_OK, view );
    break;
  case B_ID_NAME_OK: {
    TLWNode *mn = static_cast<ListSelectWindow *>(win)->Selected();

    const char *name = (mn ? mission.GetMessage(mn->ID()) : NULL);
    if ( name && strlen(name) > 30 ) {
      new NoteWindow( "Error",
                      "Shop names must not be longer than 30 characters.",
                      0, view );
    } else {
      b.SetNameID( mn ? mn->ID() : -1 );
      b.SetName( name );
      b_name->SetString( name ? name : "Error" );
    }
    break; }
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdUnitWindow::EdUnitWindow
// DESCRIPTION: This window contains the settings for a unit.
// PARAMETERS : unit    - unit to be modified
//              mission - currently active mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdUnitWindow::EdUnitWindow( Unit &unit, Mission &mission, View *view ) :
              Window(WIN_CENTER, view), u(unit), mission(mission), mv(0) {
  // calculate dimensions
  SetSize( MIN(sfont->Width() * 50, view->Width()),
           MIN(sfont->Height() * 12, view->Height()) );

  unsigned short xoff, yoff = 5, wdh = sfont->Height() + 10;

  Widget *wd = new StringWidget( 0, 5, yoff, w/2 - 10, wdh,
                   u.Name(), 19,
                   WIDGET_STYLE_NOBORDER|WIDGET_STR_CONST,
                   NULL, this );
  xoff = w/2 + 10 + sfont->TextWidth("Player");
  u_player = new CycleWidget( B_ID_PLAYER, xoff, yoff, w - 5 - xoff, wdh,
                              (u.IsSheltered() ? WIDGET_DISABLED : 0),
                              "_Player", u.Owner(),
                              (u.IsSheltered() ? player3_labels : player2_labels),
                              this );
  u_player->SetHook( this );
  yoff += 5 + wdh;

  new NumberWidget( 0, wd->LeftEdge() + 10 + sfont->Width() * 2,
                    yoff, sfont->Width() * 6, wdh, u.ID(), 0, 32000,
                    WIDGET_ALIGN_LEFT|WIDGET_STYLE_NOBORDER|WIDGET_STR_CONST,
                    "ID", this );

  xoff = w/2 + 10 + sfont->TextWidth("Direction");
  wd = new CycleWidget( B_ID_DIRECTION, xoff, yoff, w - 5 - xoff, wdh,
                        0, "_Direction", u.GetDirection(), dir_labels, this );
  wd->SetHook( this );
  yoff += 5 + wdh;

  xoff = 10 + sfont->TextWidth("Squad Size");
  wd = new CycleWidget( B_ID_SIZE, xoff, yoff, w/2 - 5 - xoff, wdh,
                        0, "Squad _Size", u.GroupSize() - 1, size_labels, this );
  wd->SetHook( this );

  xoff = w/2 + 10 + sfont->TextWidth("XP Level");
  wd = new CycleWidget( B_ID_XP, xoff, yoff, w - 5 - xoff, wdh,
                        0, "XP _Level", u.XPLevel(), xp_labels, this );
  wd->SetHook( this );

  yoff += 5 + wdh;
  if ( u.IsTransport() && !u.IsSheltered() ) {
    xoff = 10 + sfont->TextWidth("Crystals");
    // maximum number of crystals should be
    // (mission.StorageLeft(u) + (u.Crystals() + 9)/10) * 10,
    // but we can't update that maxium when the list of contained units
    // changes, so we allow the maximum here and update manually when the
    // widget is activated if necessary
    wd = new NumberWidget( N_ID_CRYSTALS, xoff, yoff, sfont->Width() * 6, wdh,
                           u.Crystals(), 0, u.Type()->Slots() * 10,
                           WIDGET_ALIGN_LEFT, "_Crystals", this );
    wd->SetHook( this );

    wd = new ButtonWidget( B_ID_UNITS, w/2 + 5, yoff, (w - 20)/2, wdh,
                           0, "_Units...", this ),
    wd->SetHook( this );
  }

  wd = new ButtonWidget( B_ID_OK, 1, h - 1 - wdh, w - 2, wdh,
                         WIDGET_DEFAULT, "_OK", this ),
  wd->SetHook( this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdUnitWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdUnitWindow::WidgetActivated( Widget *widget, Window *win ) {
  GUI_Status rc = GUI_OK;

  switch ( widget->ID() ) {
  case B_ID_DIRECTION:
    u.SetDirection( static_cast<CycleWidget *>(widget)->GetValue() );
    break;
  case B_ID_SIZE:
    u.SetGroupSize( static_cast<CycleWidget *>(widget)->GetValue() + 1 );
    break;
  case B_ID_XP:
    u.SetXP( static_cast<CycleWidget *>(widget)->GetValue() * XP_PER_LEVEL );
    break;
  case B_ID_UNITS:
    new EdUnitsWindow( mission, u, view );
    break;
  case N_ID_CRYSTALS: {
    NumberWidget *cwidget = static_cast<NumberWidget *>(widget);
    unsigned short crystals = MIN( cwidget->Number(),
                  (mission.StorageLeft(u) + (u.Crystals() + 9)/10) * 10 );
    if ( cwidget->Number() != crystals ) cwidget->SetNumber( crystals );
    u.SetCrystals( crystals );
    break; }
  case B_ID_OK:
    view->CloseWindow( win );
    u.SetOwner( u_player->GetValue() );

    if ( u.IsTransport() && !u.IsSheltered() ) {
      for ( Unit *walk = static_cast<Unit *>(mission.GetUnits().Head());
            walk; walk = static_cast<Unit *>(walk->Next()) ) {
        if ( walk->Position() == u.Position() ) walk->SetOwner( u.Owner() );
      }
    }

    if ( mv ) view->Refresh( mv->UpdateHex(u.Position()) );
    break;
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventsWindow::EdEventsWindow
// DESCRIPTION: This window presents the list of events for selection.
// PARAMETERS : mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdEventsWindow::EdEventsWindow( Mission &mission, View *view ) :
              Window(WIN_CENTER, view), mission(mission) {
  // calculate dimensions
  SetSize( MIN(sfont->Width() * 20 + 20, view->Width()),
           MIN( MAX(mission.GetEvents().CountNodes() + 2, 6) *
                   (sfont->Height() + 2) + 40,
                view->Height()) );

  TLWListBuilder::BuildEventList( mission.GetEvents(), nodes );

  e_list = new TextListWidget( 0, 5, 5, w - 10, h - sfont->Height() * 2 - 30,
               &nodes, -1, WIDGET_HSCROLLKEY|WIDGET_VSCROLLKEY, NULL, this );

  ButtonWidget *btn = new ButtonWidget( B_ID_NEW,
               1, h - sfont->Height() * 2 - 17, (w - 2)/2, sfont->Height() + 8,
               0, "_New", this );
  btn->SetHook( this );

  btn = new ButtonWidget( B_ID_EDIT,
              btn->LeftEdge() + btn->Width(), btn->TopEdge(),
              btn->Width(), btn->Height(), 0, "_Edit", this );
  btn->SetHook( this );

  btn = new ButtonWidget( B_ID_DELETE,
              btn->LeftEdge() - btn->Width(), btn->TopEdge() + btn->Height(),
              btn->Width(), btn->Height(), 0, "_Delete", this );
  btn->SetHook( this );

  btn = new ButtonWidget( B_ID_OK,
              btn->LeftEdge() + btn->Width(), btn->TopEdge(),
              btn->Width(), btn->Height(), 0, "_OK", this );
  btn->SetHook( this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventsWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : button - activated button widget
//              win    - window containing the button
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdEventsWindow::WidgetActivated( Widget *button, Window *win ) {

  switch ( button->ID() ) {
  case B_ID_EDIT: {
    TLWNode *n = static_cast<TLWNode *>( e_list->Selected() );
    if ( n ) {
      Event *e = (Event *)n->UserData();
      new EdEventGenericWindow( *e, mission, view );
    }
    break; }
  case B_ID_NEW:
    view->CloseWindow( win );
    new SelectEventWindow( mission, view );
    break;
  case B_ID_DELETE: {
    TLWNode *n = static_cast<TLWNode *>( e_list->Selected() );
    if ( n ) {
      Event *e = (Event *)n->UserData();

      n->Remove();
      delete n;
      e_list->Update();
      e_list->Draw();
      e_list->Show();

      mission.DeleteEvent( e );
    }
    break; }
  case B_ID_OK:
    view->CloseWindow( win );
    break;
  }

  return GUI_OK;
}


////////////////////////////////////////////////////////////////////////
// NAME       : SelectEventWindow::SelectEventWindow
// DESCRIPTION: This window presents the list of event types for the
//              purpose of creating a new event.
// PARAMETERS : mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

SelectEventWindow::SelectEventWindow( Mission &mission, View *view ) :
                   Window(WIN_CENTER, view), mission(mission) {
  // calculate dimensions
  SetSize( MIN(sfont->Width() * 18 + 20, view->Width()),
           MIN( 10 * (sfont->Height() + 2) + 42, view->Height()) );

  BuildTLWList( nodes );

  e_list = new TextListWidget( 0, 5, 5, w - 10, h - sfont->Height() * 3 - 36,
               &nodes, -1, WIDGET_HSCROLLKEY|WIDGET_VSCROLLKEY, NULL, this );

  e_trigger = new CycleWidget( B_ID_TRIGGER, e_list->LeftEdge(),
                  e_list->TopEdge() + e_list->Height() + sfont->Height() + 10,
                  e_list->Width(), sfont->Height() + 8, WIDGET_ALIGN_TOP,
                  "_Triggered by", 0, etrigger_labels, this );

  ButtonWidget *btn = new ButtonWidget( B_ID_OK,
               1, h - sfont->Height() - 9, (w - 2)/2, sfont->Height() + 8,
               WIDGET_DEFAULT, "_OK", this );
  btn->SetHook( this );

  btn = new ButtonWidget( B_ID_CANCEL,
              btn->LeftEdge() + btn->Width(), btn->TopEdge(),
              btn->Width(), btn->Height(), 0, "_Cancel", this );
  btn->SetHook( this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : SelectEventWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : button - activated button widget
//              win    - window containing the button
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status SelectEventWindow::WidgetActivated( Widget *button, Window *win ) {

  switch ( button->ID() ) {
  case B_ID_OK:
    if ( e_list->Selected() ) {
      view->CloseWindow( win );

      Event *e = mission.CreateEvent(
                     static_cast<TLWNode *>(e_list->Selected())->ID(),
                     e_trigger->GetValue() );
      if ( e ) new EdEventGenericWindow( *e, mission, view );
    }
    break;
  case B_ID_CANCEL:
    view->CloseWindow( win );
    break;
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : SelectEventWindow::BuildTLWList
// DESCRIPTION: Create a list for a TextListWidget containing all
//              available event types.
// PARAMETERS : list - list of TLWNodes to be filled
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void SelectEventWindow::BuildTLWList( TLWList &events ) const {
  for ( int i = 0; event_labels[i]; ++i )
    events.InsertNodeSorted( new TLWNode(event_labels[i], 0, i+1) );
}


////////////////////////////////////////////////////////////////////////
// NAME       : EdUnitsWindow::EdUnitsWindow
// DESCRIPTION: This window presents the list of units in a unit
//              container (building or transporter) for selection.
// PARAMETERS : mission   - current mission
//              container - selected unit container
//              view      - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdUnitsWindow::EdUnitsWindow( Mission &mission, MapObject &container, View *view ) :
              Window(WIN_CENTER, view), mission(mission), container(container) {
  TLWListBuilder::BuildUnitList( mission.GetUnits(), nodes, container.Position() );

  // calculate dimensions
  SetSize( MIN(sfont->Width() * 30 + 20, view->Width()),
           MIN( MAX(nodes.CountNodes() + 2, 6) *
                   (sfont->Height() + 2) + 40,
                view->Height()) );

  e_list = new TextListWidget( 0, 5, 5, w - 10, h - sfont->Height() * 2 - 30,
               &nodes, -1, WIDGET_HSCROLLKEY|WIDGET_VSCROLLKEY, NULL, this );

  ButtonWidget *btn = new ButtonWidget( B_ID_NEW,
               1, h - sfont->Height() * 2 - 17, (w - 2)/2, sfont->Height() + 8,
               0, "_New", this );
  btn->SetHook( this );

  btn = new ButtonWidget( B_ID_EDIT,
              btn->LeftEdge() + btn->Width(), btn->TopEdge(),
              btn->Width(), btn->Height(), 0, "_Edit", this );
  btn->SetHook( this );

  btn = new ButtonWidget( B_ID_DELETE,
              btn->LeftEdge() - btn->Width(), btn->TopEdge() + btn->Height(),
              btn->Width(), btn->Height(), 0, "_Delete", this );
  btn->SetHook( this );

  btn = new ButtonWidget( B_ID_OK,
              btn->LeftEdge() + btn->Width(), btn->TopEdge(),
              btn->Width(), btn->Height(), 0, "_OK", this );
  btn->SetHook( this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdUnitsWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : button - activated button widget
//              win    - window containing the button
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdUnitsWindow::WidgetActivated( Widget *button, Window *win ) {
  TLWNode *n;

  switch ( button->ID() ) {
  case B_ID_EDIT:
    n = static_cast<TLWNode *>( e_list->Selected() );
    if ( n ) new EdUnitWindow( *((Unit *)n->UserData()), mission, view );
    break;
  case B_ID_NEW:
    utypes.Clear();
    BuildTLWList( utypes );
    new SelectUnitWindow( mission, utypes, this, view );
    break;
  case B_ID_DELETE:
    n = static_cast<TLWNode *>( e_list->Selected() );
    if ( n ) {
      Unit *u = (Unit *)n->UserData();

      n->Remove();
      delete n;
      e_list->Update();
      e_list->Draw();
      e_list->Show();

      mission.DeleteUnit( u );
    }
    break;
  case B_ID_OK:
    view->CloseWindow( win );
    break;

  case B_ID_SUW_OK: {
    const UnitType *ut = static_cast<SelectUnitWindow *>(win)->Selected();
    view->CloseWindow( win );

    if ( ut ) {
      Unit *u = mission.CreateUnit( ut, container.Owner(), container.Position() );
      if ( u ) {
        char buf[28];
        sprintf( buf, "%s (%d)", u->Name(), u->ID() );
        nodes.InsertNodeSorted( new TLWNode( buf, u ) );
        e_list->Update();
        e_list->Draw();
        e_list->Show();
      }
    }
    break; }
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdUnitsWindow::BuildTLWList
// DESCRIPTION: Construct a list with all legal unit types for the
//              current container.
// PARAMETERS : list - list to add nodes to
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdUnitsWindow::BuildTLWList( TLWList &list ) const {
  const UnitSet &us = mission.GetUnitSet();
  TLWListBuilder::BuildUnitTypesList( us, list );

  // now remove all unit types which are not allowed by the container
  unsigned short maxw = container.MaxWeight();
  if ( container.IsUnit() )
    maxw = MIN( maxw, mission.StorageLeft(static_cast<Unit &>(container)) );

  TLWNode *n = static_cast<TLWNode *>(list.Head()), *n2;
  while ( n ) {

    const UnitType *ut = us.GetUnitInfo( n->ID() );
    if ( (ut->Weight() < container.MinWeight()) ||
         (ut->Weight() > maxw) ) {
      n2 = static_cast<TLWNode *>(n->Next());
      n->Remove();
      delete n;
      n = n2;
    } else n = static_cast<TLWNode *>(n->Next());
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : SelectUnitWindow::SelectUnitWindow
// DESCRIPTION: This window presents the list of unit types.
// PARAMETERS : mission - current mission
//              hook    - widget hook to activate when OK is pressed
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

SelectUnitWindow::SelectUnitWindow( Mission &mission,
                  WidgetHook *hook, View *view ) :
                   Window(WIN_CENTER, view), mission(mission) {
  TLWListBuilder::BuildUnitTypesList( mission.GetUnitSet(), nodes );

  Init( nodes, hook );
}


////////////////////////////////////////////////////////////////////////
// NAME       : SelectUnitWindow::SelectUnitWindow
// DESCRIPTION: This window presents the list of unit types.
// PARAMETERS : mission - current mission
//              list    - use this list instead of constructing our own
//              hook    - widget hook to activate when OK is pressed
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

SelectUnitWindow::SelectUnitWindow( Mission &mission, TLWList &list,
                  WidgetHook *hook, View *view ) :
                  Window(WIN_CENTER, view), mission(mission) {
  Init( list, hook );
}

////////////////////////////////////////////////////////////////////////
// NAME       : SelectUnitWindow::Init
// DESCRIPTION: Set up the window properties and create the widgets.
// PARAMETERS : list - list to present
//              hook - widget hook to activate when OK is pressed
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void SelectUnitWindow::Init( TLWList &list, WidgetHook *hook ) {
  // calculate dimensions
  SetSize( MIN(sfont->Width() * 24 + 20, view->Width()),
           MIN( MAX(list.CountNodes() + 1, 6) *
                   (sfont->Height() + 2) + 25,
                view->Height()) );

  e_list = new TextListWidget( 0, 5, 5, w - 10, h - sfont->Height() - 15,
               &list, -1, WIDGET_HSCROLLKEY|WIDGET_VSCROLLKEY, NULL, this );

  ButtonWidget *btn = new ButtonWidget( B_ID_SUW_OK,
               1, h - sfont->Height() - 9, (w - 2)/2, sfont->Height() + 8,
               WIDGET_DEFAULT, "_OK", this );
  btn->SetHook( hook );

  btn = new ButtonWidget( B_ID_CANCEL,
              btn->LeftEdge() + btn->Width(), btn->TopEdge(),
              btn->Width(), btn->Height(), 0, "_Cancel", this );
  btn->SetHook( this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : SelectUnitWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : button - activated button widget
//              win    - window containing the button
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status SelectUnitWindow::WidgetActivated( Widget *button, Window *win ) {

  if ( button->ID() == B_ID_CANCEL ) view->CloseWindow( win );

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : SelectUnitWindow::Selected
// DESCRIPTION: Get the selected unit type.
// PARAMETERS : -
// RETURNS    : selected unit type or NULL if none selected
////////////////////////////////////////////////////////////////////////

const UnitType *SelectUnitWindow::Selected( void ) const {
  TLWNode *n = static_cast<TLWNode *>(e_list->Selected());

  if ( n ) return mission.GetUnitSet().GetUnitInfo( n->ID() );
  return NULL;
}


////////////////////////////////////////////////////////////////////////
// NAME       : EdMsgWindow::EdMsgWindow
// DESCRIPTION: This window offers a way to create new language catalogs
//              and messages or modify existing ones.
// PARAMETERS : locale - locale of the current mission
//              view   - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdMsgWindow::EdMsgWindow( Locale &locale, View *view ) :
     Window(WIN_CENTER, view), locale(locale), cur_lang(0), cur_msg_id(-1) {
  // calculate dimensions
  SetSize( MIN(sfont->Width() * 35 + 20, view->Width()),
           MIN((sfont->Height() + 2) * 10 + 50, view->Height()) );

  BuildLangsList();
  if ( !l_langs.IsEmpty() ) {
    cur_lang = static_cast<Language *>(static_cast<TLWNode *>(l_langs.Head())->UserData());
    BuildMsgsList();
    if ( !l_msgs.IsEmpty() ) cur_msg_id = 0;
  }

  ButtonWidget *btn;
  unsigned short wdh = sfont->Height() + 8, wdw, wdy;

  wdw = sfont->TextWidth("Languages") + 10;

  w_langs = new TextListWidget( L_ID_LANGS,
            wdw * 2 + 20, 10, w - 30 - wdw * 2, sfont->Height() + 10 + 2 * wdh,
            &l_langs, l_langs.IsEmpty() ? -1 : 0, WIDGET_HSCROLLKEY, NULL, this );
  w_langs->SetHook( this );

  btn = new ButtonWidget( B_ID_LANG_NEW,
            10, w_langs->TopEdge() + sfont->Height() + 5, wdw, wdh,
            0, "_New", this );
  btn->SetHook( this );

  wdw = sfont->TextWidth("ID") + 5;
  w_lid = new StringWidget( 0,
          btn->LeftEdge() + wdw, btn->TopEdge() + wdh + 5,
          btn->Width() - wdw, wdh,
          NULL, 2, WIDGET_ALIGN_LEFT, "_ID", this );

  btn = new ButtonWidget( B_ID_LANG_DELETE,
            btn->LeftEdge() + btn->Width() + 5, btn->TopEdge(),
            btn->Width(), wdh, 0, "_Delete", this );
  btn->SetHook( this );

  wdy = w_lid->TopEdge() + wdh + 10;
  w_msgs = new TextListWidget( L_ID_MSGS,
           5, wdy, w - 10, h - wdy - 2 * wdh - 15,
           &l_msgs, cur_msg_id, WIDGET_VSCROLLKEY, NULL, this );
  w_msgs->SetHook( this );

  wdw = sfont->TextWidth("Text") + 10;
  w_msg = new StringWidget( 0,
          wdw, w_msgs->TopEdge() + w_msgs->Height() + 5, w - wdw - 5, wdh,
          cur_msg_id == -1 ? NULL : cur_lang->GetMsg(cur_msg_id), 4096,
          WIDGET_ALIGN_LEFT, "_Text", this );

  btn = new ButtonWidget( B_ID_MSG_ADD,
            w_msgs->LeftEdge(), w_msg->TopEdge() + wdh + 5, (w - 20) / 4, wdh,
            0, "_Add", this );
  btn->SetHook( this );
  btn = new ButtonWidget( B_ID_MSG_MODIFY,
            btn->LeftEdge() + btn->Width() + 5, btn->TopEdge(), btn->Width(), wdh,
            0, "_Modify", this );
  btn->SetHook( this );

  btn = new ButtonWidget( GUI_CLOSE,
            w - 5 - btn->Width(), btn->TopEdge(), btn->Width(), wdh,
            0, "_OK", this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdMsgWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdMsgWindow::WidgetActivated( Widget *widget, Window *win ) {
  TLWNode *n;

  switch ( widget->ID() ) {
  case L_ID_LANGS:
    n = static_cast<TLWNode *>( w_langs->Selected() );
    if ( n ) {
      cur_lang = static_cast<Language *>( n->UserData() );
      BuildMsgsList();
      w_msgs->Update();
      if ( cur_msg_id != -1 ) {
        w_msgs->Select( cur_msg_id );
        w_msg->SetString( cur_lang->GetMsg( cur_msg_id ) );
      }
      w_msgs->Draw();
      w_msgs->Show();
    }
    break;
  case L_ID_MSGS:
    n = static_cast<TLWNode *>( w_msgs->Selected() );
    if ( n ) {
      w_msg->SetString( (char *)n->UserData() );
      cur_msg_id = n->ID();
    }
    break;

  case B_ID_LANG_NEW: {
    Language l;

    if ( l_langs.CountNodes() == 0 ) {
      // add empty English catalog
      l.SetID( CF_LANG_DEFAULT );
    } else {
      // copy English catalog to new language
      const char *id = w_lid->String();
      if ( id ) {
        if ( locale.GetLanguage( id ) ) {
          new NoteWindow( "Error", "This ID is already taken.", 0, view );
          break;
        } else {
          const Language *def = locale.GetLanguage( CF_LANG_DEFAULT );
          l.SetID( id );
          for ( int i = 0; i < def->Size(); ++i )
            l.AddMsg( def->GetMsg( i ) );
        }
      } else {
        new NoteWindow( "Error", "Please enter an ID first.", 0, view );
        break;
      }
    }
    locale.AddLanguage( l );
    BuildLangsList();
    w_langs->Update();
    w_langs->Draw();
    w_langs->Show();
    break; }
  case B_ID_LANG_DELETE:
    if ( cur_lang ) {
      if ( (string(cur_lang->ID()) == CF_LANG_DEFAULT) && (l_langs.CountNodes() > 1) ) {
        new NoteWindow( "Error", "You cannot delete the default catalog", 0, view );
      } else {
        locale.RemoveLanguage( *cur_lang );
        BuildLangsList();
        w_langs->Update();
        w_langs->Draw();
        w_langs->Show();

        if ( w_langs->Selected() ) {
          cur_lang = static_cast<Language *>(
                     static_cast<TLWNode *>(w_langs->Selected())->UserData()
                     );
        } else cur_lang = NULL;
        BuildMsgsList();
        cur_msg_id = l_msgs.IsEmpty() ? -1 : 0;
        w_msgs->Update();
        w_msgs->Select( cur_msg_id );
        w_msgs->Draw();
        w_msgs->Show();
        w_msg->SetString( cur_lang ? cur_lang->GetMsg( cur_msg_id ) : NULL );
      }
    }
    break;
  case B_ID_MSG_ADD:
    if ( w_msg->String() ) {
      // append the message to _all_ languages
      for ( TLWNode *n = static_cast<TLWNode *>( l_langs.Head() );
            n; n = static_cast<TLWNode *>( n->Next() ) ) {
        Language *l = static_cast<Language *>( n->UserData() );
        l->AddMsg( w_msg->String() );
      }
      BuildMsgsList();
      w_msgs->Update();
      if ( cur_msg_id == -1 ) w_msg->SetString( NULL );
      else w_msgs->Select( cur_lang->Size() - 1 );
      w_msgs->Draw();
      w_msgs->Show();
    }
    break;
  case B_ID_MSG_MODIFY:
    if ( (cur_msg_id != -1) && w_msg->String() ) {
      cur_lang->SetMsg( w_msg->String(), cur_msg_id );
      BuildMsgsList();
      w_msgs->Draw();
      w_msgs->Show();
    }
    break;
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdMsgWindow::Draw
// DESCRIPTION: Draw the window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdMsgWindow::Draw( void ) {
  Window::Draw();
  DrawBox( Rect(5, 5, w - 10, w_langs->Height() + 10), BOX_CARVED );
  sfont->Write( "Languages", this, 10, 10 );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdMsgWindow::BuildLangsList
// DESCRIPTION: Compile the list of available languages.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdMsgWindow::BuildLangsList( void ) {
  const std::map<const string, Language> &lib = locale.GetLibrary();
  l_langs.Clear();

  for ( map<const string, Language>::const_iterator iter = lib.begin();
        iter != lib.end(); ++iter ) {
    l_langs.InsertNodeSorted( new TLWNode( iter->second.ID(),
            const_cast<Language *>(&iter->second) ) );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdMsgWindow::BuildMsgsList
// DESCRIPTION: Compile the list of messages in the current language.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdMsgWindow::BuildMsgsList( void ) {
  l_msgs.Clear();

  if ( cur_lang ) {
    const char *m;

    for ( int i = 0; (m = cur_lang->GetMsg(i)) != 0; ++i ) {
      l_msgs.AddTail( new TLWNode(string(m).substr(0, 30).c_str(), const_cast<char *>(m), i) );
    }
  }
}

