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
// eventwindow.cpp
///////////////////////////////////////////////////////////////////////

#include "eventwindow.h"
#include "extwindow.h"
#include "extwindow2.h"
#include "uiaux.h"

extern const char *player2_labels[];
extern const char *dir_labels[];
extern const char *size_labels[];
extern const char *xp_labels[];
static const char *uselect_labels[] = { "Single Unit", "Unit Type", 0 };

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventGenericWindow::EdEventGenericWindow
// DESCRIPTION: This window allows modification of the fields common to
//              all event types, and also offers links to event-specific
//              options.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdEventGenericWindow::EdEventGenericWindow( Event &event,
       Mission &mission, View *view ) :
       Window(WIN_CENTER, view), event(event), mission(mission) {
  // calculate dimensions
  SetSize( MIN(sfont->Width() * 35 + 20, view->Width()),
           MIN(sfont->Height() * 12 + 125, view->Height()) );

  unsigned short wdh = sfont->Height() + 8, wdx, wdy = 5;
  Widget *wd;
  char buf[48] = "";

  wd = new NumberWidget( 0, sfont->Width() * 2 + 10, wdy,
       sfont->Width() * 6, wdh, event.ID(),
       0, 32767, WIDGET_STR_CONST|WIDGET_ALIGN_LEFT, "ID", this );

  wdx = wd->LeftEdge() + wd->Width() + sfont->TextWidth("Player") + 10;

  wd = new CheckboxWidget( B_ID_DISABLED,
       w - 5 - DEFAULT_CBW_SIZE, wdy + (wdh - DEFAULT_CBW_SIZE)/2,
       DEFAULT_CBW_SIZE, DEFAULT_CBW_SIZE, event.Flags() & EFLAG_DISABLED,
       WIDGET_STYLE_GFX|WIDGET_STYLE_NOBORDER|WIDGET_ALIGN_LEFT,
       "_Disabled", this );
  wd->SetHook( this );

  wd = new CycleWidget( B_ID_PLAYER, wdx, wdy,
       wd->LeftEdge() - wdx - 15 - sfont->TextWidth("Disabled"), wdh,
       0, "_Player", event.Player(), player2_labels, this );
  wd->SetHook( this );

  wdy += wd->Height() + 5;
  wd = new ButtonWidget( B_ID_TITLE, 5, wdy,
       sfont->Width() * 9 + 10, wdh, 0, "Msg _Title", this );
  wd->SetHook( this );

  wdx = wd->LeftEdge() + wd->Width() + 5;
  e_title = new StringWidget( 0, wdx, wdy, w - wdx - 5, wdh,
            (mission.GetMessage(event.Title()) ?
             mission.GetMessage(event.Title()) : ""),
            30, WIDGET_STR_CONST, NULL, this );

  wdy += wd->Height() + 5;
  wd = new ButtonWidget( B_ID_MSG, 5, wdy, w - 10, wdh,
       0, "Set Event _Message", this );
  wd->SetHook( this );

  wdy += wd->Height() + 5;
  e_msg = new TextScrollWidget( 0, 5, wdy, w - 10, sfont->Height() * 5,
          (mission.GetMessage(event.Message()) ?
           mission.GetMessage(event.Message()) : ""),
          0, NULL, this );

  wdy += e_msg->Height() + 5;
  wd = new ButtonWidget( B_ID_TRIGGER, 5, wdy,
       sfont->Width() * 12 + 10, wdh, 0,
       "Edit T_rigger", this );
  wd->SetHook( this );

  wdy += wd->Height() + 5;
  wd = new ButtonWidget( B_ID_EVENT, 5, wdy, wd->Width(), wdh,
       0, "Edit _Event", this );
  wd->SetHook( this );

  wdy += wd->Height() + 5;
  wd = new ButtonWidget( B_ID_DEPEND, 5, wdy, wd->Width(), wdh,
       0, "Depends o_n", this );
  wd->SetHook( this );

  wdx = wd->LeftEdge() + wd->Width() + 5;
  if ( event.Dependency() >= 0 ) {
    Event *e = mission.GetEventByID( event.Dependency() );
    if ( e ) sprintf( buf, "%s (%d)", e->Name(), e->ID() );
  }
  e_depends = new StringWidget( 0, wdx, wdy, w - wdx - 5, wdh,
              buf, 47, WIDGET_STR_CONST, NULL, this );

  wdy += wd->Height() + 5;
  wd = new ButtonWidget( B_ID_DISCARD, 5, wdy, wd->Width(), wdh,
       0, "D_iscards", this );
  wd->SetHook( this );

  buf[0] = '\0';
  if ( event.Discard() >= 0 ) {
    Event *e = mission.GetEventByID( event.Discard() );
    if ( e ) sprintf( buf, "%s (%d)", e->Name(), e->ID() );
  }
  e_discards = new StringWidget( 0, wdx, wdy,
               e_depends->Width(), wdh, buf, 47,
               WIDGET_STR_CONST, NULL, this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventGenericWindow::Draw
// DESCRIPTION: Draw the window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdEventGenericWindow::Draw( void ) {
  Window::Draw();

  unsigned short lblx = sfont->Width() * 12 + 20,
                 lbly = e_msg->TopEdge() + e_msg->Height() + 9;

  sfont->Write( ':', this, lblx, lbly );
  sfont->Write( event.TriggerName(), this, lblx + sfont->Width() * 2, lbly );

  lbly += sfont->Height() + 13;
  sfont->Write( ':', this, lblx, lbly );
  sfont->Write( event.Name(), this, lblx + sfont->Width() * 2, lbly );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventGenericWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the button
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdEventGenericWindow::WidgetActivated( Widget *widget, Window *win ) {
  TLWNode *n;

  switch ( widget->ID() ) {
  case B_ID_TITLE:
    TLWListBuilder::BuildMsgList( mission.GetMessages(), tlist );
    new ListSelectWindow( "Select event title", tlist, event.Title(),
                          true, this, B_ID_TITLE_OK, view );
    break;

  case B_ID_TITLE_OK:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetTitle( n ? n->ID() : -1 );
    e_title->SetString( n ? n->Name() : NULL, true );
    tlist.Clear();
    break;

  case B_ID_MSG:
    TLWListBuilder::BuildMsgList( mission.GetMessages(), tlist );
    new ListSelectWindow( "Select event message", tlist, event.Message(),
                          true, this, B_ID_MSG_OK, view );
    break;

  case B_ID_MSG_OK:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetMessage( n ? n->ID() : -1 );
    e_msg->SetText( n ? mission.GetMessage(n->ID()) : NULL );
    e_msg->Draw();
    e_msg->Show();
    tlist.Clear();
    break;

  case B_ID_DEPEND:
    TLWListBuilder::BuildEventList( mission.GetEvents(), tlist );
    new ListSelectWindow( "Select event", tlist, event.Dependency(),
                          true, this, B_ID_DEPEND_OK, view );
    break;

  case B_ID_DEPEND_OK:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetDependency( n ? n->ID() : -1 );
    e_depends->SetString( n ? n->Name() : NULL );
    tlist.Clear();
    break;

  case B_ID_DISCARD:
    TLWListBuilder::BuildEventList( mission.GetEvents(), tlist );
    new ListSelectWindow( "Select event", tlist, event.Discard(),
                          true, this, B_ID_DISCARD_OK, view );
    break;

  case B_ID_DISCARD_OK:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetDiscard( n ? n->ID() : -1 );
    e_discards->SetString( n ? n->Name() : NULL );
    tlist.Clear();
    break;

  case B_ID_TRIGGER:
    switch ( event.Trigger() ) {
    case ETRIGGER_HAVE_BUILDING:
      new EdTrigHaveShopWindow( event, mission, view ); break;
    case ETRIGGER_HAVE_CRYSTALS:
      new EdTrigHaveCrystalsWindow( event, mission, view ); break;
    case ETRIGGER_HAVE_UNIT:
      new EdTrigHaveUnitWindow( event, mission, view ); break;
    case ETRIGGER_TIMER:
      new EdTrigTimerWindow( event, view ); break;
    case ETRIGGER_UNIT_DESTROYED:
      new EdTrigUnitDestroyedWindow( event, mission, view ); break;
    case ETRIGGER_UNIT_POSITION:
      new EdTrigUnitPositionWindow( event, mission, view ); break;
    case ETRIGGER_HANDICAP:
      new EdTrigHandicapWindow( event, view ); break;
    default:
      new NoteWindow("Sorry", "Not yet implemented", 0, view);
    }
    break;

  case B_ID_EVENT:
    switch ( event.Type() ) {
    case EVENT_CONFIGURE:
      new EdEventConfigureWindow( event, mission, view ); break;
    case EVENT_CREATE_UNIT:
      new EdEventCreateUnitWindow( event, mission, view ); break;
    case EVENT_DESTROY_UNIT:
      new EdEventDestroyUnitWindow( event, mission, view ); break;
    case EVENT_MANIPULATE_EVENT:
      new EdEventManipEventWindow( event, mission, view ); break;
    case EVENT_MESSAGE:
      new EdEventMessageWindow( event, mission, view ); break;
    case EVENT_MINING:
      new EdEventMiningWindow( event, mission, view ); break;
    case EVENT_RESEARCH:
      new EdEventResearchWindow( event, mission, view ); break;
    case EVENT_SCORE:
      new EdEventScoreWindow( event, mission, view ); break;
    case EVENT_SET_HEX:
      new EdEventSetHexWindow( event, mission, view ); break;
    case EVENT_SET_TIMER:
      new EdEventSetTimerWindow( event, mission, view ); break;
    default:
      new NoteWindow("Sorry", "Not yet implemented", 0, view);
    }
    break;
  case B_ID_PLAYER:
    event.SetPlayer( static_cast<CycleWidget *>(widget)->GetValue() );
    break;
  case B_ID_DISABLED:
    event.ToggleFlags( EFLAG_DISABLED );
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventConfigureWindow::EdEventConfigureWindow
// DESCRIPTION: This window allows configuration of a NEXT_MAP event.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdEventConfigureWindow::EdEventConfigureWindow( Event &event,
                      Mission &mission, View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  static const char *setting_labels[] = {
         "Briefing Player 1", "Briefing Player 2", "Next Map", 0 };

  SetSize( MIN(sfont->Width() * 20 + 15, view->Width()),
           sfont->Height() * 3 + 40 );

  unsigned short wdh = sfont->Height() + 8, wdx;
  Widget *wd;

  wdx = 10 + sfont->TextWidth( "Setting" );
  wd = new CycleWidget( B_ID_SETTING, wdx, 5, w - 5 - wdx, wdh,
       0, "_Setting", event.GetData(0), setting_labels, this );
  wd->SetHook( this );

  e_msg_but = new ButtonWidget( B_ID_MSG,
       5, wd->TopEdge() + wd->Height() + 5,
       sfont->Width() * 7 + 10, wdh,
       event.GetData(0) == 2 ? WIDGET_DISABLED : 0,
       "_Message", this );
  e_msg_but->SetHook( this );

  wdx = e_msg_but->LeftEdge() + e_msg_but->Width() + 5;
  e_msg = new StringWidget( S_ID_VALUE, wdx, e_msg_but->TopEdge(),
          w - wdx - 5, wdh, GetValueString(), 30,
          event.GetData(0) == 2 ? 0 : WIDGET_STR_CONST,
          NULL, this );
  e_msg->SetHook( this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventConfigureWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdEventConfigureWindow::WidgetActivated( Widget *widget, Window *win ) {
  switch( widget->ID() ) {
  case B_ID_SETTING:
    event.SetData( 0, static_cast<CycleWidget *>(widget)->GetValue() );
    if ( event.GetData(0) == 2 ) {
      e_msg_but->Disable();
      e_msg->UnsetFlags( WIDGET_STR_CONST );
    } else {
      e_msg_but->Enable();
      e_msg->SetFlags( WIDGET_STR_CONST );
    }
    e_msg->SetString( GetValueString() );
    Draw();
    Show();
    break;
  case B_ID_MSG:
    TLWListBuilder::BuildMsgList( mission.GetMessages(), tlist );
    new ListSelectWindow( "Select message", tlist,
        event.GetData(1), false, this, B_ID_MSG_OK, view );
    break;
  case B_ID_MSG_OK: {
    TLWNode *n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetData( 1, n ? n->ID() : -1 );
    e_msg->SetString( n ? n->Name() : "<None>" );
    tlist.Clear(); }
    break;
  case S_ID_VALUE:
    event.SetTmpBuf( e_msg->String() ? e_msg->String() : "" );
    break;
  }
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventConfigureWindow::GetValueString
// DESCRIPTION: Get the string representation of the "value" parameter
//              that should be shown to the user.
// PARAMETERS : -
// RETURNS    : display string
////////////////////////////////////////////////////////////////////////

const char *EdEventConfigureWindow::GetValueString( void ) const {
  const char *ret = NULL;

  switch (event.GetData(0) ) {
  case 0:
  case 1:
    ret = mission.GetMessage(event.GetData(1));
    break;
  case 2:
    if ( event.GetTmpBuf() != "" ) ret = event.GetTmpBuf().c_str();
    break;
  }
  return ( ret == NULL ) ? "<None>" : ret;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventCreateUnitWindow::EdEventCreateUnitWindow
// DESCRIPTION: This window allows configuration of a CREATE_UNIT
//              event.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdEventCreateUnitWindow::EdEventCreateUnitWindow( Event &event,
                         Mission &mission, View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  SetSize( MIN(sfont->Width() * 30 + 15, view->Width()),
           sfont->Height() * 4 + 50 );

  Widget *wd;
  unsigned short wdx, wdh = sfont->Height() + 8;
  Point loc = mission.GetMap().Index2Hex( event.GetData(1) );

  wd = new ButtonWidget( B_ID_UNIT, 5, 5,
       sfont->Width() * 4 + 10, wdh, 0, "_Unit", this );
  wd->SetHook( this );

  wdx = wd->LeftEdge() + wd->Width() + 5;
  e_unit = new StringWidget( 0, wdx, 5, w - wdx - 5, wdh,
           mission.GetUnitSet().GetUnitInfo(event.GetData(0))->Name(),
           30, WIDGET_STR_CONST, NULL, this );

  wdx = 10 + sfont->TextWidth( "X" );
  wd = new NumberWidget( N_ID_XPOS, wdx, wd->TopEdge() + wdh + 5,
       sfont->Width() * 4 + 10, wdh, loc.x, 0,
       mission.GetMap().Width() - 1, WIDGET_ALIGN_LEFT, "_X", this );
  wd->SetHook( this );

  wdx += wd->Width() + 10 + sfont->TextWidth( "Y" );
  wd = new NumberWidget( N_ID_YPOS, wdx, wd->TopEdge(),
       wd->Width(), wdh, loc.y, 0,
       mission.GetMap().Height() - 1, WIDGET_ALIGN_LEFT, "_Y", this );
  wd->SetHook( this );

  wdx += wd->Width() + 10 + sfont->TextWidth( "Direction" );
  wd = new CycleWidget( B_ID_DIRECTION,
       wdx, wd->TopEdge(), w - 5 - wdx, wdh,
       0, "_Direction", event.GetData(2) & 0x07, dir_labels, this );
  wd->SetHook( this );

  wdx = 10 + sfont->TextWidth("Squad Size");
  wd = new CycleWidget( B_ID_SIZE, wdx, wd->TopEdge() + wd->Height() + 5,
       w/2 - 5 - wdx, wdh, 0, "Squad _Size",
       ((event.GetData(2) & 0x38) >> 3) - 1, size_labels, this );
  wd->SetHook( this );

  wdx += wd->Width() + sfont->TextWidth("XP Level") + 10;
  wd = new CycleWidget( B_ID_XP, wdx,
       wd->TopEdge(), w - 5 - wdx, wdh, 0, "XP _Level",
       (event.GetData(2) & 0x1C0) >> 6, xp_labels, this );
  wd->SetHook( this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventCreateUnitWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdEventCreateUnitWindow::WidgetActivated( Widget *widget, Window *win ) {
  TLWNode *n;

  switch( widget->ID() ) {
  case B_ID_UNIT:
    TLWListBuilder::BuildUnitTypesList( mission.GetUnitSet(), tlist );
    new ListSelectWindow( "Select unit", tlist,
        event.GetData(0), false, this, B_ID_UNIT_OK, view );
    break;
  case B_ID_UNIT_OK:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetData( 0, n ? n->ID() : -1 );
    e_unit->SetString( n ? n->Name() : "Error" );
    tlist.Clear();
    break;
  case N_ID_XPOS:
  case N_ID_YPOS: {
    Point loc = mission.GetMap().Index2Hex( event.GetData(1) );
    if ( widget->ID() == N_ID_XPOS )
      loc.x = static_cast<NumberWidget *>(widget)->Number();
    else
      loc.y = static_cast<NumberWidget *>(widget)->Number();
    event.SetData( 1, mission.GetMap().Hex2Index(loc) );
    break; }
  case B_ID_DIRECTION:
    event.SetData( 2, (event.GetData(2) & ~0x0007) |
          static_cast<CycleWidget *>(widget)->GetValue() );
    break;
  case B_ID_SIZE:
    event.SetData( 2, (event.GetData(2) & ~0x0038) |
          ((static_cast<CycleWidget *>(widget)->GetValue() + 1) << 3) );
    break;
  case B_ID_XP:
    event.SetData( 2, (event.GetData(2) & ~0x01C0) |
          (static_cast<CycleWidget *>(widget)->GetValue() << 6) );
    break;
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventDestroyUnitWindow::EdEventDestroyUnitWindow
// DESCRIPTION: This window allows configuration of a DESTROY_UNIT
//              event.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdEventDestroyUnitWindow::EdEventDestroyUnitWindow( Event &event,
                      Mission &mission, View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  SetSize( MIN(sfont->Width() * 30 + 15, view->Width()),
           sfont->Height() * 5 + 60 );

  Widget *wd;
  unsigned short wdx, wdh = sfont->Height() + 8;
  char buf[32] = "Any";
  static const char *player_labels[] = { "Any", "One", "Two", 0 };
  Point loc;

  wd = new ButtonWidget( B_ID_UNIT, 5, 5,
       sfont->Width() * 4 + 10, wdh, 0, "_Unit", this );
  wd->SetHook( this );

  Unit *u = mission.GetUnitByID( event.GetData(0) );
  if ( u ) {
    sprintf( buf, "%s (%d)", u->Name(), u->ID() );
    loc = Point(0, 0);
  } else {
    loc = mission.GetMap().Index2Hex( event.GetData(2) );
  }

  wdx = wd->LeftEdge() + wd->Width() + 5;
  e_unit = new StringWidget( 0, wdx, 5, w - wdx - 5, wdh,
           buf, 30, WIDGET_STR_CONST, NULL, this );

  wdx = 10 + sfont->TextWidth( "X" );
  e_xpos = new NumberWidget( N_ID_XPOS, wdx, wd->TopEdge() + wdh + 5,
           sfont->Width() * 4 + 10, wdh, loc.x, 0,
           mission.GetMap().Width() - 1,
           WIDGET_ALIGN_LEFT|(u ? WIDGET_DISABLED : 0), "_X", this );
  e_xpos->SetHook( this );

  wdx += wd->Width() + 10 + sfont->TextWidth( "Y" );
  e_ypos = new NumberWidget( N_ID_YPOS, wdx, e_xpos->TopEdge(),
       e_xpos->Width(), wdh, loc.y, 0,
       mission.GetMap().Height() - 1,
       WIDGET_ALIGN_LEFT|(u ? WIDGET_DISABLED : 0), "_Y", this );
  e_ypos->SetHook( this );

  wdx = sfont->Width() * 20 + 10;
  wd = new CycleWidget( B_ID_PLAYER, wdx, e_ypos->TopEdge() + wdh + 10,
       w - wdx - 5, wdh, 0, "Controlled by _Player",
       event.GetData(1) + 1, player_labels, this );
  wd->SetHook( this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventDestroyUnitWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdEventDestroyUnitWindow::WidgetActivated( Widget *widget, Window *win ) {

  switch( widget->ID() ) {
  case B_ID_UNIT:
    TLWListBuilder::BuildUnitList( mission.GetUnits(), tlist );
    new ListSelectWindow( "Select unit", tlist,
        event.GetData(0), true, this, B_ID_UNIT_OK, view );
    break;
  case B_ID_UNIT_OK: {
    TLWNode *n = static_cast<ListSelectWindow *>(win)->Selected();
    if ( n ) {
      event.SetData( 0, n->ID() );
      event.SetData( 2, 0 );
      e_unit->SetString( n->Name() );
      e_xpos->SetNumber( 0 );
      e_ypos->SetNumber( 0 );
      e_xpos->Disable();
      e_ypos->Disable();
    } else {
      event.SetData( 0, -1 );
      e_unit->SetString( "Any" );
      e_xpos->Enable();
      e_ypos->Enable();
    }
    e_xpos->Draw();
    e_ypos->Draw();
    e_xpos->Show();
    e_ypos->Show();
    tlist.Clear();
    break; }
  case B_ID_PLAYER:
    event.SetData( 1, static_cast<CycleWidget *>(widget)->GetValue() - 1 );
    break;
  case N_ID_XPOS:
  case N_ID_YPOS: {
    Point loc = mission.GetMap().Index2Hex( event.GetData(2) );
    if ( widget->ID() == N_ID_XPOS )
      loc.x = static_cast<NumberWidget *>(widget)->Number();
    else
      loc.y = static_cast<NumberWidget *>(widget)->Number();
    event.SetData( 2, mission.GetMap().Hex2Index(loc) );
    break; }
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventManipEventWindow::EdEventManipEventWindow
// DESCRIPTION: This window allows configuration of a MANIPULATE_EVENT
//              event.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdEventManipEventWindow::EdEventManipEventWindow( Event &event,
                         Mission &mission, View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  static const char *action_labels[] = { "Set flags",
                                         "Clear flags",
                                         "Toggle flags", 0 };

  SetSize( MIN(sfont->Width() * 25 + 15, view->Width()),
           sfont->Height() * 4 + 50 );

  Widget *wd;
  unsigned short wdx, wdy, wdh = sfont->Height() + 8;
  char buf[48] = "Error";

  wd = new ButtonWidget( B_ID_EVENT, 5, 5,
       sfont->Width() * 5 + 10, wdh, 0, "_Event", this );
  wd->SetHook( this );

  Event *e = mission.GetEventByID( event.GetData(0) );
  if ( e ) sprintf( buf, "%s (%d)", e->Name(), e->ID() );
  wdx = wd->LeftEdge() + wd->Width() + 5;
  e_event = new StringWidget( 0, wdx, 5, w - wdx - 5, wdh,
            buf, 30, WIDGET_STR_CONST, NULL, this );

  wdx = sfont->Width() * 6 + 10;
  wdy = wdh + 10;
  wd = new CycleWidget( B_ID_ACTION, wdx, wdy,
       w - wdx - 5, wdh, 0, "_Action",
       event.GetData(2), action_labels, this );
  wd->SetHook( this );

  wdx = w - 10 - DEFAULT_CBW_SIZE;
  wdy += wdh + 5;
  wd = new CheckboxWidget( B_ID_DISABLED, wdx, wdy,
       DEFAULT_CBW_SIZE, DEFAULT_CBW_SIZE,
       event.GetData(1) & EFLAG_DISABLED,
       WIDGET_STYLE_GFX|WIDGET_STYLE_NOBORDER|WIDGET_ALIGN_LEFT,
       "_Disabled", this );
  wd->SetHook( this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );
  Draw();
  Show();

  TLWListBuilder::BuildEventList( mission.GetEvents(), tlist );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventManipEventWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdEventManipEventWindow::WidgetActivated( Widget *widget, Window *win ) {

  switch( widget->ID() ) {
  case B_ID_EVENT:
    new ListSelectWindow( "Select event", tlist,
        event.GetData(0), false, this, B_ID_EVENT_OK, view );
    break;
  case B_ID_EVENT_OK: {
    TLWNode *n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetData( 0, n ? n->ID() : -1 );
    e_event->SetString( n ? n->Name() : "Error" );
    break; }
  case B_ID_ACTION:
    event.SetData( 2, static_cast<CycleWidget *>(widget)->GetValue() );
    break;
  case B_ID_DISABLED:
    event.SetData( 1, event.GetData(1)^EFLAG_DISABLED );
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventMessageWindow::EdEventMessageWindow
// DESCRIPTION: This window allows configuration of a MESSAGE event.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdEventMessageWindow::EdEventMessageWindow( Event &event, Mission &mission,
                                          View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  SetSize( MIN(sfont->Width() * 15 + 15, view->Width()),
           sfont->Height() * 2 + 25 );

  unsigned short wdx, wdh = sfont->Height() + 8;
  Point loc = mission.GetMap().Index2Hex( event.GetData(0) );

  wdx = 10 + sfont->TextWidth( "X" );
  e_xpos = new NumberWidget( 0,
       wdx, 5, sfont->Width() * 4 + 10, wdh,
       loc.x, -1, mission.GetMap().Width() - 1,
       WIDGET_ALIGN_LEFT, "_X", this );

  wdx += e_xpos->Width() + 10 + sfont->TextWidth( "Y" );
  e_ypos = new NumberWidget( 0,
       wdx, e_xpos->TopEdge(), e_xpos->Width(), wdh,
       loc.y, -1, mission.GetMap().Height() - 1,
       WIDGET_ALIGN_LEFT, "_Y", this );

  ButtonWidget *wd = new ButtonWidget( GUI_CLOSE,
                     1, h - wdh - 1, w - 2, wdh,
                     WIDGET_DEFAULT, "_OK", this );
  wd->SetHook( this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventMessageWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdEventMessageWindow::WidgetActivated( Widget *widget, Window *win ) {
  Point loc( e_xpos->Number(), e_ypos->Number() );
  short idx = ((loc.x >= 0) && (loc.y >= 0)) ? mission.GetMap().Hex2Index(loc) : -1;
  event.SetData( 0, idx );
  return GUI_CLOSE;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventMiningWindow::EdEventMiningWindow
// DESCRIPTION: This window allows configuration of a MINING event.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdEventMiningWindow::EdEventMiningWindow( Event &event,
                         Mission &mission, View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  static const char *action_labels[] = { "Set supplies",
                                         "Modify supplies",
                                         "Set production",
                                         "Modify production", 0 };

  SetSize( MIN(sfont->Width() * 30 + 15, view->Width()),
           sfont->Height() * 4 + 50 );

  Widget *wd;
  unsigned short wdx, wdy, wdh = sfont->Height() + 8;

  wd = new ButtonWidget( B_ID_MINE, 5, 5,
       sfont->Width() * 6, wdh, 0, "_Mine", this );
  wd->SetHook( this );

  wdx = wd->LeftEdge() + wd->Width() + 5;
  Building *b = mission.GetBuildingByID( event.GetData(0) );
  e_mine = new StringWidget( 0, wdx, wd->TopEdge(),
           w - wdx - 5, wdh, b ? b->Name() : "Error",
           30, WIDGET_STR_CONST, NULL, this );

  wdy = wd->Height() + 10;
  wd = new CycleWidget( B_ID_ACTION, wdx, wdy,
       e_mine->Width(), wdh, 0, "_Action",
       event.GetData(2), action_labels, this );
  wd->SetHook( this );

  wdy += wdh + 5;
  e_crystals = new NumberWidget( N_ID_CRYSTALS,
       wdx, wdy, wd->Width(), wdh,
       event.GetData(1), -1000, 10000,
       WIDGET_ALIGN_LEFT, "Amou_nt", this );
  e_crystals->SetHook( this );

  // this is kind of a hack: it's not (yet) possible to
  // change the maximum string length after creation of
  // the widget, so we assumed the maximum above and set
  // the real values now
  SetMiningRange( event.GetData(2) );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventMiningWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdEventMiningWindow::WidgetActivated( Widget *widget, Window *win ) {
  TLWNode *n;

  switch( widget->ID() ) {
  case B_ID_MINE:
    TLWListBuilder::BuildShopList( mission.GetBuildings(), tlist );
    new ListSelectWindow( "Select mine", tlist,
        event.GetData(0), false, this, B_ID_MINE_OK, view );
    break;
  case B_ID_MINE_OK:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetData( 0, n ? n->ID() : -1 );
    e_mine->SetString( n ? n->Name() : "Error" );
    tlist.Clear();
    break;
  case B_ID_ACTION:
    event.SetData( 2, static_cast<CycleWidget *>(widget)->GetValue() );
    SetMiningRange( static_cast<CycleWidget *>(widget)->GetValue() );
    // fall through in case range has changed the number
  case N_ID_CRYSTALS:
    event.SetData( 1, e_crystals->Number() );
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventMiningWindow::SetMiningRange
// DESCRIPTION: The minimum and maximum numbers allowed in the "amount"
//              widget are subject of the "action" chosen. We set the
//              correct range here.
// PARAMETERS : action - action to set the range for
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdEventMiningWindow::SetMiningRange( short action ) const {
  short min = 0, max = 0;
  switch ( action ) {
  case 0: max = 1000; break;              // set supplies
  case 1: min = -1000; max = 1000; break; // modify supplies
  case 2: max = 200; break;               // set production
  case 3: min = -200; max = 200; break;   // modify production;
  }
  e_crystals->SetMin( min );
  e_crystals->SetMax( max );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventResearchWindow::EdEventResearchWindow
// DESCRIPTION: This window allows configuration of a RESEARCH event.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdEventResearchWindow::EdEventResearchWindow( Event &event,
                       Mission &mission, View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  SetSize( MIN(sfont->Width() * 25, view->Width()),
           sfont->Height() * 4 + 55 );

  static const char *action_labels[] = { "Allow", "Disallow", 0 };
  Widget *wd;
  unsigned short wdx, wdh = sfont->Height() + 8;

  wdx = sfont->TextWidth("Action") + 10;
  wd = new CycleWidget( B_ID_ACTION, wdx, 5, w - wdx - 5, wdh,
       0, "_Action", event.GetData(2), action_labels, this );
  wd->SetHook( this );

  wd = new ButtonWidget( B_ID_SHOP, 5, wd->TopEdge() + wdh + 5,
       sfont->Width() * 4 + 10, wdh, 0, "_Shop", this );
  wd->SetHook( this );

  Building *b = mission.GetBuildingByID( event.GetData(0) );
  wdx = wd->LeftEdge() + wd->Width() + 5;
  e_shop = new StringWidget( 0, wdx, wd->TopEdge(), w - wdx - 5, wdh,
           b ? b->Name() : "Error", 30, WIDGET_STR_CONST,
           NULL, this );

  wd = new ButtonWidget( B_ID_UNIT, 5, wd->TopEdge() + wdh + 5,
       wd->Width(), wdh, 0, "_Unit", this );
  wd->SetHook( this );

  e_unit = new StringWidget( 0, wdx, wd->TopEdge(),
           e_shop->Width(), wdh,
           mission.GetUnitSet().GetUnitInfo(event.GetData(1))->Name(),
           30, WIDGET_STR_CONST, NULL, this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventResearchWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdEventResearchWindow::WidgetActivated( Widget *widget, Window *win ) {
  TLWNode *n;

  switch( widget->ID() ) {
  case B_ID_UNIT:
    TLWListBuilder::BuildUnitTypesList( mission.GetUnitSet(), tlist );
    new ListSelectWindow( "Select researched unit", tlist,
        event.GetData(1), false, this, B_ID_UNIT_OK, view );
    break;
  case B_ID_UNIT_OK:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetData( 1, n ? n->ID() : -1 );
    e_unit->SetString( n ? n->Name() : "Error" );
    tlist.Clear();
    break;
  case B_ID_SHOP:
    TLWListBuilder::BuildShopList( mission.GetBuildings(), tlist );
    new ListSelectWindow( "Select factory", tlist,
        event.GetData(0), false, this, B_ID_SHOP_OK, view );
    break;
  case B_ID_SHOP_OK:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetData( 0, n ? n->ID() : -1 );
    e_shop->SetString( n ? n->Name() : "Error" );
    tlist.Clear();
    break;
  case B_ID_ACTION:
    event.SetData( 2, static_cast<CycleWidget *>(widget)->GetValue() );
    break;
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventScoreWindow::EdEventScoreWindow
// DESCRIPTION: This window allows configuration of a SCORE event.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdEventScoreWindow::EdEventScoreWindow( Event &event, Mission &mission, View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  SetSize( MIN(sfont->Width() * 20 + 15, view->Width()),
           sfont->Height() * 4 + 50 );

  unsigned short wdx = sfont->Width() * 5 + 10;
  Widget *wd = new NumberWidget( N_ID_SCORE, wdx, 5,
               w - wdx - 5, sfont->Height() + 8,
               event.GetData(0), 1, 100,
               WIDGET_ALIGN_LEFT, "_Score", this );
  wd->SetHook( this );

  wd = new ButtonWidget( B_ID_TITLE, 5, wd->TopEdge() + wd->Height() + 5,
                    sfont->Width() * 10, wd->Height(), 0, "Lose _Title", this );
  wd->SetHook( this );

  wdx = wd->LeftEdge() + wd->Width() + 5;
  const char *str = mission.GetMessage(event.GetData(2));
  other_title = new StringWidget( 0, wdx, wd->TopEdge(), w - wdx - 5, wd->Height(),
           (str || event.GetData(2) == -1) ? str : "Error", 30, WIDGET_STR_CONST,
           NULL, this );

  wd = new ButtonWidget( B_ID_MSG, 5, wd->TopEdge() + wd->Height() + 5,
                         wd->Width(), wd->Height(), 0, "Lose _Msg", this );
  wd->SetHook( this );

  str = mission.GetMessage(event.GetData(1));
  other_msg = new StringWidget( 0, wdx, wd->TopEdge(), other_title->Width(), wd->Height(),
           (str || event.GetData(1) == -1) ? str : "Error", 30, WIDGET_STR_CONST,
           NULL, this );

  new ButtonWidget( GUI_CLOSE, 1, h - wd->Height() - 1, w - 2, wd->Height(),
                    WIDGET_DEFAULT, "_OK", this );
  Draw();
  Show();

  TLWListBuilder::BuildMsgList( mission.GetMessages(), msglist );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventScoreWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdEventScoreWindow::WidgetActivated( Widget *widget, Window *win ) {
  TLWNode *n;

  switch ( widget->ID() ) {
  case N_ID_SCORE:
    event.SetData( 0, static_cast<NumberWidget *>(widget)->Number() );
    break;
  case B_ID_MSG:
    new ListSelectWindow( "Select lose message", msglist, event.GetData(1),
                          true, this, B_ID_MSG_OK, view );
    break;
  case B_ID_MSG_OK:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetData( 1, n ? n->ID() : -1 );
    other_msg->SetString( n ? mission.GetMessage(n->ID()) : NULL );
    break;
  case B_ID_TITLE:
    new ListSelectWindow( "Select lose title", msglist, event.GetData(2),
                          true, this, B_ID_TITLE_OK, view );
    break;
  case B_ID_TITLE_OK:
    n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetData( 2, n ? n->ID() : -1 );
    other_title->SetString( n ? mission.GetMessage(n->ID()) : NULL );
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventSetHexWindow::EdEventSetHexWindow
// DESCRIPTION: This window allows configuration of a SET_HEX event.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdEventSetHexWindow::EdEventSetHexWindow( Event &event, Mission &mission,
                                          View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  const TerrainSet &set = mission.GetTerrainSet();

  SetSize( MIN(sfont->Width() * 15 + 15, view->Width()),
           sfont->Height() * 2 + set.TileHeight() + 30 );

  tilepos = Rect( (w - set.TileWidth()) / 2, 5,
                  set.TileWidth(), set.TileHeight() );

  Widget *wd;
  unsigned short wdx, wdh = sfont->Height() + 8;
  Point loc = mission.GetMap().Index2Hex( event.GetData(1) );

  wd = new ButtonWidget( B_ID_TILE_PREV,
       tilepos.x - sfont->Width() - 10, 5,
       sfont->Width() + 10, tilepos.h, 0, "_<", this );
  wd->SetHook( this );

  wd = new ButtonWidget( B_ID_TILE_NEXT, tilepos.x + tilepos.w, 5,
       wd->Width(), tilepos.h, 0, "_>", this );
  wd->SetHook( this );

  wdx = 10 + sfont->TextWidth( "X" );
  wd = new NumberWidget( N_ID_XPOS, wdx, wd->TopEdge() + wd->Height() + 5,
       sfont->Width() * 4 + 10, wdh, loc.x, 0,
       mission.GetMap().Width() - 1, WIDGET_ALIGN_LEFT, "_X", this );
  wd->SetHook( this );

  wdx += wd->Width() + 10 + sfont->TextWidth( "Y" );
  wd = new NumberWidget( N_ID_YPOS, wdx, wd->TopEdge(),
       wd->Width(), wdh, loc.y, 0,
       mission.GetMap().Height() - 1, WIDGET_ALIGN_LEFT, "_Y", this );
  wd->SetHook( this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventSetHexWindow::Draw
// DESCRIPTION: Draw the window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdEventSetHexWindow::Draw( void ) {
  Window::Draw();
  DrawTerrain( event.GetData(0), false );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventSetHexWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdEventSetHexWindow::WidgetActivated( Widget *widget, Window *win ) {
  unsigned short num;

  switch( widget->ID() ) {
  case B_ID_TILE_PREV:
    num = event.GetData( 0 );
    if ( num > 0 ) --num;
    else num = mission.GetTerrainSet().NumTiles() - 1;
    event.SetData( 0, num );
    DrawTerrain( num, true );
    Show( tilepos );
    break;
  case B_ID_TILE_NEXT:
    num = event.GetData( 0 );
    if ( ++num == mission.GetTerrainSet().NumTiles() ) num = 0;
    event.SetData( 0, num );
    DrawTerrain( num, true );
    Show( tilepos );
    break;
  case N_ID_XPOS:
  case N_ID_YPOS: {
    Point loc = mission.GetMap().Index2Hex( event.GetData(1) );
    if ( widget->ID() == N_ID_XPOS )
      loc.x = static_cast<NumberWidget *>(widget)->Number();
    else
      loc.y = static_cast<NumberWidget *>(widget)->Number();
    event.SetData( 1, mission.GetMap().Hex2Index(loc) );
    break; }
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventSetHexWindow::DrawTerrain
// DESCRIPTION: Draw a terrain image to the window surface.
// PARAMETERS : terrain - terrain identifier
//              update  - whether to update the display
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void EdEventSetHexWindow::DrawTerrain( unsigned short terrain, bool update ) {
  const TerrainSet &set = mission.GetTerrainSet();
  const TerrainType *tt = set.GetTerrainInfo( terrain );

  if ( tt ) {
    set.DrawTile( tt->tt_image, this, tilepos.x, tilepos.y, tilepos );

    if ( update ) Show( tilepos );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventSetTimerWindow::EdEventSetTimerWindow
// DESCRIPTION: This window allows configuration of a SET_TIMER event.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdEventSetTimerWindow::EdEventSetTimerWindow( Event &event,
                         Mission &mission, View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  static const char *offset_labels[] = { "None (absolute)",
                                         "Execution time",
                                         "Trigger configuration", 0 };

  SetSize( MIN(sfont->Width() * 25 + 15, view->Width()),
           sfont->Height() * 4 + 50 );

  Widget *wd;
  unsigned short wdx, wdy, wdh = sfont->Height() + 8;
  char buf[48] = "Error";

  wd = new ButtonWidget( B_ID_EVENT, 5, 5,
       sfont->Width() * 5 + 10, wdh, 0, "_Event", this );
  wd->SetHook( this );

  Event *e = mission.GetEventByID( event.GetData(0) );
  if ( e ) sprintf( buf, "%s (%d)", e->Name(), e->ID() );
  wdx = wd->LeftEdge() + wd->Width() + 5;
  e_event = new StringWidget( 0, wdx, 5, w - wdx - 5, wdh,
            buf, 30, WIDGET_STR_CONST, NULL, this );

  wdy = wdh + 10;
  wd = new CycleWidget( B_ID_OFFSET, e_event->LeftEdge(), wdy,
       w - e_event->LeftEdge() - 5, wdh, 0, "O_ffset",
       event.GetData(2), offset_labels, this );
  wd->SetHook( this );

  wdx = sfont->TextWidth("Time") + 10;
  wdy += wdh + 5;
  wd = new NumberWidget( N_ID_TIME,
       wdx, wdy, sfont->Width() * 5, wdh,
       event.GetData(1), 0, 100,
       WIDGET_ALIGN_LEFT, "_Time", this );
  wd->SetHook( this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );
  Draw();
  Show();

  TLWListBuilder::BuildEventList( mission.GetEvents(), tlist );
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdEventSetTimerWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdEventSetTimerWindow::WidgetActivated( Widget *widget, Window *win ) {

  switch( widget->ID() ) {
  case B_ID_EVENT:
    new ListSelectWindow( "Select event", tlist,
        event.GetData(0), false, this, B_ID_EVENT_OK, view );
    break;
  case B_ID_EVENT_OK: {
    TLWNode *n = static_cast<ListSelectWindow *>(win)->Selected();
    if ( n ) {
      Event *e = static_cast<Event *>( n->UserData() );
      if ( e->Trigger() != ETRIGGER_TIMER ) {
        n = NULL;
        new NoteWindow( "Error", "Selected event has no 'Timer' trigger.",
            0, view );
      }
    }
    event.SetData( 0, n ? n->ID() : -1 );
    e_event->SetString( n ? n->Name() : "Error" );
    break; }
  case B_ID_OFFSET:
    event.SetData( 2, static_cast<CycleWidget *>(widget)->GetValue() );
    break;
  case N_ID_TIME:
    event.SetData( 1, static_cast<NumberWidget *>(widget)->Number() );
    break;
  }

  return GUI_OK;
}


////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigHaveCrystalsWindow::EdTrigHaveCrystalsWindow
// DESCRIPTION: This window allows configuration of a HAVE_CRYSTALS
//              trigger.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdTrigHaveCrystalsWindow::EdTrigHaveCrystalsWindow( Event &event,
                      Mission &mission, View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  SetSize( MIN(sfont->Width() * 30 + 15, view->Width()),
           sfont->Height() * 4 + 50 );

  Widget *wd;
  unsigned short wdx, wdh = sfont->Height() + 8;
  static const char *moreless_labels[] = { ">=", "<", 0 };

  wd = new ButtonWidget( B_ID_SHOP, 5, 5,
       sfont->Width() * 4 + 10, wdh, 0, "_Shop", this );
  wd->SetHook( this );

  Building *b = mission.GetBuildingByID( event.GetTData(2) );
  wdx = wd->LeftEdge() + wd->Width() + 5;
  t_shop = new StringWidget( 0, wdx, 5, w - wdx - 5, wdh,
           b ? b->Name() : "All", 30, WIDGET_STR_CONST,
           NULL, this );

  wdx = sfont->TextWidth("Player") + 10;
  wd = new CycleWidget( B_ID_PLAYER, wdx, wd->Height() + 10,
       sfont->Width() * 3 + 20, wdh, 0, "_Player",
       event.GetTData(1), player2_labels, this );
  wd->SetHook( this );

  wdx = w - 5 - sfont->Width() * 4 - 20;
  wd = new CycleWidget( B_ID_MOREORLESS, wdx, wd->TopEdge(),
       sfont->Width() * 4 + 20, wdh, 0, NULL,
       event.GetTData(0) < 0 ? 1 : 0, moreless_labels, this );
  wd->SetHook( this );

  wdx = wd->LeftEdge() - 5 - sfont->Width() * 6;
  wd = new NumberWidget( N_ID_CRYSTALS, wdx, wd->TopEdge(),
       sfont->Width() * 6, wdh, ABS(event.GetTData(0)),
       1, 5000, WIDGET_ALIGN_LEFT, "_Crystals", this );
  wd->SetHook( this );

  t_transports = new CheckboxWidget( B_ID_TRANSPORTS,
       5, wd->TopEdge() + wd->Height() + 5,
       DEFAULT_CBW_SIZE, DEFAULT_CBW_SIZE,
       event.GetTData(2) == -2,
       WIDGET_STYLE_GFX|WIDGET_STYLE_NOBORDER|WIDGET_ALIGN_RIGHT|
       (event.GetTData(2) >= 0 ? WIDGET_DISABLED : 0),
       "_Include Transports", this );
  t_transports->SetHook( this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigHaveCrystalsWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdTrigHaveCrystalsWindow::WidgetActivated( Widget *widget, Window *win ) {

  switch( widget->ID() ) {
  case B_ID_SHOP:
    TLWListBuilder::BuildShopList( mission.GetBuildings(), tlist );
    new ListSelectWindow( "Select shop", tlist,
        event.GetTData(2), true, this, B_ID_SHOP_OK, view );
    break;
  case B_ID_SHOP_OK: {
    TLWNode *n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetTData( 2, n ? n->ID() : -1 );
    t_shop->SetString( n ? n->Name() : "All" );
    tlist.Clear();

    if ( !n ) {
      t_transports->Enable();
      if ( t_transports->Clicked() ) event.SetTData( 2, -2 );
    } else t_transports->Disable();
    t_transports->Draw();
    Show();
    break; }
  case B_ID_PLAYER:
    event.SetTData( 1, static_cast<CycleWidget *>(widget)->GetValue() );
    break;
  case N_ID_CRYSTALS:
    event.SetTData( 0, SIGN(event.GetTData(0)) * static_cast<NumberWidget *>(widget)->Number() );
    break;
  case B_ID_MOREORLESS:
    event.SetTData( 0, event.GetTData( 0 ) * -1 );
    break;
  case B_ID_TRANSPORTS:
    event.SetTData( 2, widget->Clicked() ? -2 : -1 );
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigHaveShopWindow::EdTrigHaveShopWindow
// DESCRIPTION: This window allows configuration of a HAVE_BUILDING
//              trigger.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdTrigHaveShopWindow::EdTrigHaveShopWindow( Event &event,
                      Mission &mission, View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  SetSize( MIN(sfont->Width() * 30 + 15, view->Width()),
           sfont->Height() * 4 + 50 );

  Widget *wd;
  unsigned short wdx, wdh = sfont->Height() + 8;

  wd = new ButtonWidget( B_ID_SHOP, 5, 5,
       sfont->Width() * 4 + 10, wdh, 0, "_Shop", this );
  wd->SetHook( this );

  Building *b = mission.GetBuildingByID( event.GetTData(0) );
  wdx = wd->LeftEdge() + wd->Width() + 5;
  t_shop = new StringWidget( 0, wdx, 5, w - wdx - 5, wdh,
           b ? b->Name() : "Error", 30, WIDGET_STR_CONST,
           NULL, this );

  wdx = sfont->Width() * 20 + 10;
  wd = new CycleWidget( B_ID_PLAYER, wdx, wd->Height() + 10,
       w - wdx - 5, wdh, 0, "Controlled by _Player",
       event.GetTData(1), player2_labels, this );
  wd->SetHook( this );

  wd = new NumberWidget( N_ID_TIMER,
       sfont->Width() * 7 + 10, wd->TopEdge() + wd->Height() + 5,
       sfont->Width() * 6, wdh, event.GetTData(2),
       -1, 999, WIDGET_ALIGN_LEFT, "At _Time", this );
  wd->SetHook( this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigHaveShopWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdTrigHaveShopWindow::WidgetActivated( Widget *widget, Window *win ) {

  switch( widget->ID() ) {
  case B_ID_SHOP:
    TLWListBuilder::BuildShopList( mission.GetBuildings(), tlist );
    new ListSelectWindow( "Select shop", tlist,
        event.GetTData(0), false, this, B_ID_SHOP_OK, view );
    break;
  case B_ID_SHOP_OK: {
    TLWNode *n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetTData( 0, n ? n->ID() : -1 );
    t_shop->SetString( n ? n->Name() : "Error" );
    tlist.Clear();
    break; }
  case B_ID_PLAYER:
    event.SetTData( 1, static_cast<CycleWidget *>(widget)->GetValue() );
    break;
  case N_ID_TIMER:
    event.SetTData( 2, static_cast<NumberWidget *>(widget)->Number() );
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigHaveUnitWindow::EdTrigHaveUnitWindow
// DESCRIPTION: This window allows configuration of a HAVE_UNIT trigger.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdTrigHaveUnitWindow::EdTrigHaveUnitWindow( Event &event,
                      Mission &mission, View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  SetSize( MIN(sfont->Width() * 30 + 15, view->Width()),
           sfont->Height() * 4 + 50 );

  Widget *wd;
  unsigned short wdx, wdh = sfont->Height() + 8;
  char buf[32] = "Error";

  wd = new ButtonWidget( B_ID_UNIT, 5, 5,
       sfont->Width() * 4 + 10, wdh, 0, "_Unit", this );
  wd->SetHook( this );

  Unit *u = mission.GetUnitByID( event.GetTData(0) );
  if ( u ) sprintf( buf, "%s (%d)", u->Name(), u->ID() );

  wdx = wd->LeftEdge() + wd->Width() + 5;
  t_unit = new StringWidget( 0, wdx, 5, w - wdx - 5, wdh,
           buf, 30, WIDGET_STR_CONST, NULL, this );

  wdx = sfont->Width() * 20 + 10;
  wd = new CycleWidget( B_ID_PLAYER, wdx, wd->Height() + 10,
       w - wdx - 5, wdh, 0, "Controlled by _Player",
       event.GetTData(1), player2_labels, this );
  wd->SetHook( this );

  wd = new NumberWidget( N_ID_TIMER,
       sfont->Width() * 8 + 10, wd->TopEdge() + wd->Height() + 5,
       sfont->Width() * 6, wdh, event.GetTData(2),
       -1, 999, WIDGET_ALIGN_LEFT, "At _Time", this );
  wd->SetHook( this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigHaveUnitWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdTrigHaveUnitWindow::WidgetActivated( Widget *widget, Window *win ) {

  switch( widget->ID() ) {
  case B_ID_UNIT:
    TLWListBuilder::BuildUnitList( mission.GetUnits(), tlist );
    new ListSelectWindow( "Select unit", tlist,
        event.GetTData(0), false, this, B_ID_UNIT_OK, view );
    break;
  case B_ID_UNIT_OK: {
    TLWNode *n = static_cast<ListSelectWindow *>(win)->Selected();
    event.SetTData( 0, n ? n->ID() : -1 );
    t_unit->SetString( n ? n->Name() : "Error" );
    tlist.Clear();
    break; }
  case B_ID_PLAYER:
    event.SetTData( 1, static_cast<CycleWidget *>(widget)->GetValue() );
    break;
  case N_ID_TIMER:
    event.SetTData( 2, static_cast<NumberWidget *>(widget)->Number() );
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigTimerWindow::EdTrigTimerWindow
// DESCRIPTION: This window allows configuration of a TIMER trigger.
// PARAMETERS : event - event to edit
//              view  - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdTrigTimerWindow::EdTrigTimerWindow( Event &event, View *view ) :
                  Window(WIN_CENTER, view), event(event) {
  SetSize( MIN(sfont->Width() * 21 + 15, view->Width()),
           sfont->Height() * 2 + 30 );

  unsigned short wdx = sfont->Width() * 15 + 10,
                 wdh = sfont->Height() + 8;
  Widget *wd = new NumberWidget( 0, wdx, 5, w - wdx - 5, wdh,
       event.GetTData(0), 0, 999, WIDGET_ALIGN_LEFT,
       "Execute at _time", this );
  wd->SetHook( this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigTimerWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdTrigTimerWindow::WidgetActivated( Widget *widget, Window *win ) {
  event.SetTData( 0, static_cast<NumberWidget *>(widget)->Number() );
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigUnitDestroyedWindow::EdTrigUnitDestroyedWindow
// DESCRIPTION: This window allows configuration of a UNIT_DESTROYED
//              trigger.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdTrigUnitDestroyedWindow::EdTrigUnitDestroyedWindow( Event &event,
                           Mission &mission, View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  SetSize( MIN(sfont->Width() * 30 + 15, view->Width()),
           sfont->Height() * 4 + 50 );

  Widget *wd;
  unsigned short wdx, wdh = sfont->Height() + 8;
  char buf[32] = "All";

  wdx = 10 + sfont->TextWidth("Select");
  t_uselect = new CycleWidget( B_ID_USELECT, wdx, 5, w/2 - wdx - 5, wdh,
              0, "_Select", (event.GetTData(0) >= -1) ? 0 : 1,
              uselect_labels, this );
  t_uselect->SetHook( this );

  wd = new ButtonWidget( B_ID_UNIT,
       5, t_uselect->TopEdge() + wdh + 5,
       sfont->Width() * 4 + 10, wdh, 0, "_Unit", this );
  wd->SetHook( this );

  if ( event.GetTData(0) >= 0 ) {
    Unit *u = mission.GetUnitByID( event.GetTData(0) );
    if ( u ) sprintf( buf, "%s (%d)", u->Name(), u->ID() );
  } else if ( event.GetTData(0) < -1 ) {
    strcpy( buf, mission.GetUnitSet().GetUnitInfo(-event.GetTData(0) - 2)->Name() );
  }
  wdx = wd->LeftEdge() + wd->Width() + 5;
  t_unit = new StringWidget( 0, wdx, wd->TopEdge(), w - wdx - 5, wdh,
           buf, 30, WIDGET_STR_CONST, NULL, this );

  wdx = sfont->Width() * 20 + 10;
  t_player = new CycleWidget( B_ID_PLAYER,
             wdx, wd->TopEdge() + wd->Height() + 5,
             w - wdx - 5, wdh,
             (event.GetTData(0) >= 0) ? WIDGET_DISABLED : 0,
             "Controlled by _Player", event.GetTData(1),
             player2_labels, this );
  t_player->SetHook( this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigUnitDestroyedWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdTrigUnitDestroyedWindow::WidgetActivated( Widget *widget, Window *win ) {

  switch( widget->ID() ) {
  case B_ID_UNIT:
    if ( t_uselect->GetValue() == 0 ) {
      TLWListBuilder::BuildUnitList( mission.GetUnits(), tlist );
      new ListSelectWindow( "Select target unit", tlist,
          event.GetTData(0), true, this, B_ID_UNIT_OK, view );
    } else {
      TLWListBuilder::BuildUnitTypesList( mission.GetUnitSet(), tlist );
      new ListSelectWindow( "Select unit type", tlist,
          -event.GetTData(0) - 2, true, this, B_ID_UNIT_OK, view );
    }
    break;
  case B_ID_UNIT_OK: {
    TLWNode *n = static_cast<ListSelectWindow *>(win)->Selected();
    if ( t_uselect->GetValue() == 0 ) {
      event.SetTData( 0, n ? n->ID() : -1 );
      if ( n ) {
        event.SetTData( 1, static_cast<Unit *>(n->UserData())->Owner() );
        t_player->Disable();
      } else {
        event.SetTData( 1, event.Player()^1 );
        t_player->Enable();
        t_player->SetValue( event.GetTData(1) );
      }
    } else {
      event.SetTData( 0, n ? -n->ID() - 2 : -1);
    }
    t_unit->SetString( n ? n->Name() : "All", true );

    Draw();
    Show();
    tlist.Clear();
    break; }
  case B_ID_USELECT:
    if ( t_uselect->GetValue() == 0 ) {
      event.SetTData( 0, -1 );
      t_unit->SetString( "All", true );
    } else if ( event.GetTData(0) != -1 ) {
      Unit *u = mission.GetUnitByID( event.GetTData(0) );
      if ( u ) {
        event.SetTData( 0, -u->Type()->ID() - 2 );
        t_unit->SetString( u->Name(), true );
      } else {
        event.SetTData( 0, -1 );
        t_unit->SetString( "All", true );
      }
    }
    t_player->Enable();
    t_player->SetValue( event.GetTData(1) );
    break;
  case B_ID_PLAYER:
    event.SetTData( 1, static_cast<CycleWidget *>(widget)->GetValue() );
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigUnitPositionWindow::EdTrigUnitPositionWindow
// DESCRIPTION: This window allows configuration of a UNIT_POSITION
//              trigger.
// PARAMETERS : event   - event to edit
//              mission - current mission
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdTrigUnitPositionWindow::EdTrigUnitPositionWindow( Event &event,
                           Mission &mission, View *view ) :
      Window(WIN_CENTER, view), event(event), mission(mission) {
  SetSize( MIN(sfont->Width() * 30 + 15, view->Width()),
           sfont->Height() * 4 + 50 );

  Widget *wd;
  unsigned short wdx, wdh = sfont->Height() + 8;
  char buf[32] = "Any";
  Point loc = mission.GetMap().Index2Hex( event.GetTData(1) );

  wdx = 10 + sfont->TextWidth("Select");
  t_uselect = new CycleWidget( B_ID_USELECT, wdx, 5, w/2 - wdx - 5, wdh,
              0, "_Select", (event.GetTData(0) >= -1) ? 0 : 1,
              uselect_labels, this );
  t_uselect->SetHook( this );

  wdx = w/2 + 10 + sfont->TextWidth("Player");
  wd = new CycleWidget( B_ID_PLAYER, wdx, t_uselect->TopEdge(),
           w - wdx - 5, wdh, 0, "_Player", event.GetTData(2),
           player2_labels, this );
  wd->SetHook( this );

  wd = new ButtonWidget( B_ID_UNIT, 5, t_uselect->TopEdge() + wdh + 5,
       sfont->Width() * 4 + 10, wdh, 0, "_Unit", this );
  wd->SetHook( this );

  if ( event.GetTData(0) >= 0 ) {
    Unit *u = mission.GetUnitByID( event.GetTData(0) );
    if ( u ) sprintf( buf, "%s (%d)", u->Name(), u->ID() );
  } else if ( event.GetTData(0) < -1 ) {
    strcpy( buf, mission.GetUnitSet().GetUnitInfo(-event.GetTData(0) - 2)->Name() );
  }

  wdx = wd->LeftEdge() + wd->Width() + 5;
  t_unit = new StringWidget( 0, wdx, wd->TopEdge(), w - wdx - 5, wdh,
           buf, 30, WIDGET_STR_CONST, NULL, this );

  wdx = sfont->Width() * 10 + 10;
  wd = new NumberWidget( N_ID_POSX,
       wdx, wd->TopEdge() + wd->Height() + 5,
       (w - wdx - sfont->Width() - 15) / 2, wdh,
       loc.x, 0, mission.GetMap().Width() - 1,
       WIDGET_ALIGN_LEFT, "Position _X", this );
  wd->SetHook( this );

  wdx += wd->Width() + sfont->Width() + 10;
  wd = new NumberWidget( N_ID_POSY,
       wdx, wd->TopEdge(), w - wdx - 5, wdh,
       loc.y, 0, mission.GetMap().Height() - 1,
       WIDGET_ALIGN_LEFT, "_Y", this );
  wd->SetHook( this );

  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigUnitPositionWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdTrigUnitPositionWindow::WidgetActivated( Widget *widget, Window *win ) {
  switch( widget->ID() ) {
  case B_ID_UNIT:
    if ( t_uselect->GetValue() == 0 ) {
      TLWListBuilder::BuildUnitList( mission.GetUnits(), tlist );
      new ListSelectWindow( "Select unit", tlist,
          event.GetTData(0), true, this, B_ID_UNIT_OK, view );
    } else {
      TLWListBuilder::BuildUnitTypesList( mission.GetUnitSet(), tlist );
      new ListSelectWindow( "Select unit type", tlist,
          -event.GetTData(0) - 2, true, this, B_ID_UNIT_OK, view );
    }
    break;
  case B_ID_UNIT_OK: {
    TLWNode *n = static_cast<ListSelectWindow *>(win)->Selected();
    if ( t_uselect->GetValue() == 0 )
      event.SetTData( 0, n ? n->ID() : -1 );
    else
      event.SetTData( 0, n ? -n->ID() - 2 : -1);
    t_unit->SetString( n ? n->Name() : "Any", true );
    tlist.Clear();
    break; }
  case N_ID_POSX:
  case N_ID_POSY: {
    Point loc = mission.GetMap().Index2Hex( event.GetTData(1) );
    if ( widget->ID() == N_ID_POSX )
      loc.x = static_cast<NumberWidget *>(widget)->Number();
    else
      loc.y = static_cast<NumberWidget *>(widget)->Number();
    event.SetTData( 1, mission.GetMap().Hex2Index(loc) );
    break; }
  case B_ID_USELECT:
    if ( t_uselect->GetValue() == 0 ) {
      event.SetTData( 0, -1 );
      t_unit->SetString( "Any", true );
    } else if ( event.GetTData(0) != -1 ) {
      Unit *u = mission.GetUnitByID( event.GetTData(0) );
      if ( u ) {
        event.SetTData( 0, -u->Type()->ID() - 2 );
        t_unit->SetString( u->Name(), true );
      } else {
        event.SetTData( 0, -1 );
        t_unit->SetString( "Any", true );
      }
    }
    break;
  case B_ID_PLAYER:
    event.SetTData( 2,
          static_cast<CycleWidget *>(widget)->GetValue() );
    break;
  }

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigHandicapWindow::EdTrigHandicapWindow
// DESCRIPTION: This window allows configuration of a HANDICAP trigger.
// PARAMETERS : event - event to edit
//              view  - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

EdTrigHandicapWindow::EdTrigHandicapWindow( Event &event, View *view ) :
                  Window(WIN_CENTER, view), event(event) {
  SetSize( MIN(sfont->Width() * 20 + DEFAULT_CBW_SIZE + 15, view->Width()),
           sfont->Height() * 4 + 40 );

  Widget *wd = new CheckboxWidget( HANDICAP_NONE, 5, 5,
               DEFAULT_CBW_SIZE, DEFAULT_CBW_SIZE,
               event.GetTData(0) & HANDICAP_NONE,
               WIDGET_STYLE_GFX|WIDGET_STYLE_NOBORDER|WIDGET_ALIGN_RIGHT,
               "_No handicap", this );
  wd->SetHook( this );

  wd = new CheckboxWidget( HANDICAP_P1, 5, wd->TopEdge() + wd->Height() + 5,
       DEFAULT_CBW_SIZE, DEFAULT_CBW_SIZE,
       (event.GetTData(0) & HANDICAP_P1) != 0,
       WIDGET_STYLE_GFX|WIDGET_STYLE_NOBORDER|WIDGET_ALIGN_RIGHT,
       "Player _1 handicapped", this );
  wd->SetHook( this );

  wd = new CheckboxWidget( HANDICAP_P2, 5, wd->TopEdge() + wd->Height() + 5,
       DEFAULT_CBW_SIZE, DEFAULT_CBW_SIZE,
       (event.GetTData(0) & HANDICAP_P2) != 0,
       WIDGET_STYLE_GFX|WIDGET_STYLE_NOBORDER|WIDGET_ALIGN_RIGHT,
       "Player _2 handicapped", this );
  wd->SetHook( this );

  unsigned short wdh = sfont->Height() + 8;
  new ButtonWidget( GUI_CLOSE, 1, h - wdh - 1, w - 2, wdh,
                    WIDGET_DEFAULT, "_OK", this );
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : EdTrigHandicapWindow::WidgetActivated
// DESCRIPTION: React to user actions.
// PARAMETERS : widget - activated widget
//              win    - window containing the widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status EdTrigHandicapWindow::WidgetActivated( Widget *widget, Window *win ) {
  short handicap = event.GetTData( 0 );
  event.SetTData( 0, handicap ^ widget->ID() );
  return GUI_OK;
}

