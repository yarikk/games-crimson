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

/////////////////////////////////////////////////////////////////////////
// unitwindow.cpp
/////////////////////////////////////////////////////////////////////////

#include "unitwindow.h"
#include "extwindow.h"
#include "gamewindow.h"
#include "game.h"
#include "strutil.h"
#include "msgs.h"

#define ICON_MINI_WIDTH    15
#define ICON_MINI_HEIGHT  15
#define ICON_MINI_REPAIR1_X  93
#define ICON_MINI_REPAIR1_Y  46
#define ICON_MINI_REPAIR2_X  93
#define ICON_MINI_REPAIR2_Y  46
#define ICON_MINI_BUILD1_X  108
#define ICON_MINI_BUILD1_Y  46
#define ICON_MINI_BUILD2_X  108
#define ICON_MINI_BUILD2_Y  46

#define CW_MODE_NORMAL    0
#define CW_MODE_PRODUCTION  1

#ifdef _WIN32_WCE
# define CW_EXIT_BUTTON_FLAGS  0
#else
# define CW_EXIT_BUTTON_FLAGS WIDGET_DEFAULT
#endif

// node class used by the UnitListWidget
class ULWNode : public Node {
public:
  const UnitType *type;
  Unit *unit;
  unsigned short image;
  bool ok;
  bool selected;
};

// list widget used to display units...
class UnitListWidget : public ListWidget {
public:
  UnitListWidget( short id, short x, short y, unsigned short w,
    unsigned short h, List *list, short selected,
    unsigned short flags, const UnitSet &uset,
    const TerrainSet &tset, Window *window );

  void DrawNodes( void );
  unsigned short ItemHeight( void ) const { return itemh; }

private:
  unsigned short itemh;
  const UnitSet &uset;
  const TerrainSet &tset;
  Rect hex_bg;
};


// button IDs for the container hook
#define CH_BUTTON_REPAIR  0
#define CH_BUTTON_BUILD    1

#define CH_BUTTON_REPAIR_CONFIRM 2
#define CH_LIST_UNITS    3

// widget IDs for the UnitLoadWindow
#define ULW_WIDGET_LIST    1
#define ULW_WIDGET_OK    2
#define ULW_WIDGET_CRYSTALS  3

////////////////////////////////////////////////////////////////////////
// NAME       : UnitListWidget::UnitListWidget
// DESCRIPTION: Create a new unit list widget.
// PARAMETERS : id       - widget identifier
//              x        - left edge of widget
//              y        - top edge of widget
//              w        - widget width
//              h        - widget height
//              list     - list nodes
//              selected - node highlighted by default (-1 == none)
//              flags    - widget flags (see widget.h for details)
//              uset     - unit set
//              tset     - terrain set
//              window   - widget parent window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

UnitListWidget::UnitListWidget( short id, short x, short y, unsigned short w,
    unsigned short h, List *list, short selected, unsigned short flags,
    const UnitSet &uset, const TerrainSet &tset, Window *window ) :
    ListWidget( id, x, y, w, h, list, selected, flags, NULL, window ),
    uset(uset), tset(tset), hex_bg( 32, 32, ICON_WIDTH, ICON_HEIGHT ) {
  spacing = 0;
  itemh = ICON_HEIGHT + spacing;
  Update();
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitListWidget::DrawNodes
// DESCRIPTION: Draw the list nodes.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void UnitListWidget::DrawNodes( void ) {
  Rect box( x + 4, y + 1 + spacing, listboxw - 8, h - 2 - 2 * spacing );
  Rect area( x + 1, y + 1, listboxw - 2, h - 2 );
  short num = toprow / ItemHeight();                          // number of top node
  ULWNode *n = static_cast<ULWNode *>(list->GetNode( num ));  // top node
  short xoff = box.x, yoff = box.y + (num * ItemHeight()) - toprow;
  Surface *icons = surface->GetView()->GetSystemIcons();

  surface->DrawBack( area );

  while ( n ) {
    Rect hilite( x + 2, yoff, listboxw - 4, ItemHeight() );
    hilite.Clip( area );

    if ( n->selected )
      surface->FillRectAlpha( hilite, Color(CF_COLOR_GHOSTED) );

    if ( num == current )
      surface->FillRectAlpha( hilite, surface->GetFGPen() );

    // draw unit icon
    Rect icon( hex_bg );
    Rect dest( xoff, yoff, ICON_WIDTH, ICON_HEIGHT );
    dest.ClipBlit( icon, area );

    icons->Blit( surface, icon, dest.x, dest.y );
    uset.DrawTile( n->image, surface, xoff + (ICON_WIDTH - uset.TileWidth()) / 2,
                  yoff + (ICON_HEIGHT - uset.TileHeight()) / 2, box );
    if ( !n->ok || (n->unit && !n->unit->IsReady()) )
      tset.DrawTile( IMG_NOT_AVAILABLE, surface,
                     xoff + (ICON_WIDTH - tset.TileWidth()) / 2,
                     yoff + (ICON_HEIGHT - tset.TileHeight()) / 2, box );

    char buf[4];
    if ( n->unit ) {
      icon = *Images[ICON_XP_BASE + n->unit->XPLevel()];
      dest = Rect( xoff + ICON_WIDTH + 4, yoff + 2, XP_ICON_WIDTH, XP_ICON_HEIGHT );
      dest.ClipBlit( icon, area );
      icons->Blit( surface, icon, dest.x, dest.y );
      font->Write( itoa( n->unit->GroupSize(), buf ), surface, xoff + ICON_WIDTH + 4,
                   yoff + ICON_HEIGHT - font->Height() - 2, box );
    } else
      font->Write( itoa( n->type->Cost(), buf ), surface, xoff + ICON_WIDTH + 4,
                   yoff + (ICON_HEIGHT - font->Height()) / 2, box );

    yoff += ItemHeight();
    if ( yoff >= box.y + box.h ) break;

    ++num;
    n = static_cast<ULWNode *>(n->Next());
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : ContainerWindow::ContainerWindow
// DESCRIPTION: Create a new ContainerWindow for a transport or building.
// PARAMETERS : container - unit container to show the window for; this
//                          can be either a transport or a building
//              view      - pointer to the view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

ContainerWindow::ContainerWindow( UnitContainer *container, View *view ) :
                 Window( WIN_CLOSE_ESC|WIN_CENTER, view ), c(container),
                 mode(CW_MODE_NORMAL),
                 crystals_icon( view->GetSystemIcons(), 160, 0, ICON_WIDTH, ICON_HEIGHT ),
                 last_selected(0) {
  int i;
  Building *b;
  Transport *t;
  Player *player = dynamic_cast<MapObject *>(c)->Owner();;
  MapView *mv = Gam->GetMapWindow()->GetMapView();

  if ( dynamic_cast<MapObject *>(c)->IsUnit() ) {
    unit = true;
    t = static_cast<Transport *>(c);
    b = NULL;
  } else {
    unit = false;
    b = static_cast<Building *>(c);
    t = NULL;
  }

  // calculate window dimensions
  short width = mv->TileWidth() + XP_ICON_WIDTH + lfont->Width() * 20 + 70;
  short height = 8 * mv->TileHeight() + 40 + sfont->Height();
  SetSize( MIN(width, view->Width()), MIN(height, view->Height()) );

  for ( i = 0; i < CH_NUM_BUTTONS; ++i ) buttons[i] = NULL;

  // create unit list
  for ( i = 0; i < c->UnitCount(); ++i ) {
    ULWNode *n = new ULWNode;

    n->unit = c->GetUnit( i );
    n->type = n->unit->Type();
    n->selected = false;

    if ( !player ) {
      n->image = n->unit->Type()->Image() + Gam->GetMission()->GetPlayer().ID() * 6;
      n->ok = false;
    } else {
      n->image = n->unit->BaseImage();
      n->ok = n->unit->IsReady();
    }
    normal.AddTail( n );
  }

  // create list widget
  width = mv->TileWidth() + XP_ICON_WIDTH + DEFAULT_SLIDER_SIZE + 20;
  listwidget = new UnitListWidget( CH_LIST_UNITS, 10, 10,
               width, h - sfont->Height() - 28, &normal, -1,
               WIDGET_VSCROLL|WIDGET_HSCROLLKEY|WIDGET_VSCROLLKEY,
               *mv->GetMap()->GetUnitSet(), *mv->GetMap()->GetTerrainSet(),
               this );
  listwidget->SetHook( this );

  // create buttons
  Surface *icons = view->GetSystemIcons();
  new ButtonWidget( GUI_CLOSE, 10, h - sfont->Height() - 18,
      width, sfont->Height() + 8, CW_EXIT_BUTTON_FLAGS,
      MSG(MSG_B_EXIT), this );

  if ( player == &Gam->GetMission()->GetPlayer() ) {
    ButtonWidget *btn;
    unsigned short xoff = width + 15, yoff = lfont->Height() + 35;

    if ( (unit && t->IsMedic()) ||
         (!unit && b->IsWorkshop()) ) {
      btn = new ButtonWidget( CH_BUTTON_REPAIR, xoff, yoff,
            ICON_MINI_WIDTH + 8 + sfont->Width(), ICON_MINI_HEIGHT + 4,
            WIDGET_STYLE_GFX|WIDGET_ALIGN_RIGHT|WIDGET_ALIGN_WITHIN,
            "_1", this );
      btn->SetImage( icons, Rect( ICON_MINI_REPAIR1_X, ICON_MINI_REPAIR1_Y,
                                  ICON_MINI_WIDTH, ICON_MINI_HEIGHT ),
                            Rect( ICON_MINI_REPAIR2_X, ICON_MINI_REPAIR2_Y,
                                  ICON_MINI_WIDTH, ICON_MINI_HEIGHT ) );
      btn->SetHook( this );
      xoff += btn->Width();
      buttons[CH_BUTTON_REPAIR] = btn;
    }

    if ( !unit ) {
      if ( b->IsFactory() && (b->UnitProduction() != 0) ) {
        btn = new ButtonWidget( CH_BUTTON_BUILD, xoff, yoff,
              ICON_MINI_WIDTH + 8 + sfont->Width(), ICON_MINI_HEIGHT + 4,
              WIDGET_STYLE_GFX|WIDGET_ALIGN_RIGHT|WIDGET_ALIGN_WITHIN,
              "_2", this );
        btn->SetImage( icons, Rect( ICON_MINI_BUILD1_X, ICON_MINI_BUILD1_Y,
                                    ICON_MINI_WIDTH, ICON_MINI_HEIGHT ),
                              Rect( ICON_MINI_BUILD2_X, ICON_MINI_BUILD2_Y,
                                    ICON_MINI_WIDTH, ICON_MINI_HEIGHT ) );
        btn->SetHook( this );
        xoff += btn->Width();
        buttons[CH_BUTTON_BUILD] = btn;
      }
    }
  }

  height = h - lfont->Height() - ICON_MINI_HEIGHT - 55;
  unitinfo = Rect( width + 15, h - height - 10, w - width - 25, height );

  Draw();
  if ( !normal.IsEmpty() ) listwidget->Select( 0 );
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : ContainerWindow::Draw
// DESCRIPTION: Draw the container window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void ContainerWindow::Draw( void ) {
  Window::Draw();
  UnitInfoWindow::DrawUnitInfo( -1, NULL, NULL, this, unitinfo );

  unsigned short xoff;
  Rect boxrect( unitinfo.x, 10, w - unitinfo.x - 10, lfont->Height() + 20 );

  // show the name of the unit/building
  DrawBox( boxrect, BOX_RAISED );

  boxrect.x += 5;
  boxrect.w -= 10;

  const char *name = dynamic_cast<MapObject *>(c)->Name();
  unsigned short nlen = lfont->FitText( name, boxrect.w, false );
  if ( nlen < strlen(name) ) xoff = 0;
  else xoff = (boxrect.w - lfont->TextWidth( name )) / 2;

  Color col = lfont->GetColor();
  lfont->SetColor( view->GetBGPen() );
  lfont->WriteEllipsis( name, this, boxrect.x + xoff + 3, 23, boxrect );
  lfont->SetColor( view->GetFGPen() );
  lfont->WriteEllipsis( name, this, boxrect.x + xoff, 20, boxrect );
  lfont->SetColor( col );

  // show crystal storage
  crystals_icon.Draw( this, w - 5 - ICON_WIDTH, lfont->Height() + 30 );
  DrawCrystals();
}

////////////////////////////////////////////////////////////////////////
// NAME       : ContainerWindow::DrawCrystals
// DESCRIPTION: Show the crystals storage of the container and its
//              change per turn.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void ContainerWindow::DrawCrystals( void ) {
  char buf[16];
  unsigned short stock, change;

  stock = c->Crystals();
  if ( unit ) change = 0;
  else change = static_cast<Building *>(c)->CrystalProduction();

  if ( change > 0 ) sprintf( buf, "%d (+%d)", stock, change );
  else itoa( stock, buf );

  unsigned short tlen = sfont->TextWidth( buf );
  Rect rect( w - ICON_WIDTH - tlen - 5 - 30,
             lfont->Height() + 30 + (ICON_HEIGHT - sfont->Height()) / 2,
             tlen + ICON_WIDTH + 5, sfont->Height() );
  DrawBack( rect );
  sfont->Write( buf, this, rect.x + 30, rect.y );
}

////////////////////////////////////////////////////////////////////////
// NAME       : ContainerWindow::HandleEvent
// DESCRIPTION: React to system events.
// PARAMETERS : event - event received by the event handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status ContainerWindow::HandleEvent( const SDL_Event &event ) {
  GUI_Status rc;

  if ( (event.type == SDL_KEYDOWN) && (event.key.keysym.sym == SDLK_SPACE) &&
       listwidget->Selected() ) {
    // activate the currently selected node in the list widget
    // => either select the unit for movement (NORMAL mode) or
    //    build the selected unit type (PRODUCTION mode)
    rc = SelectNode( static_cast<ULWNode *>( listwidget->Selected() ) );
  } else rc = Window::HandleEvent( event );
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : ContainerWindow::SwitchMode
// DESCRIPTION: Enter or leave build mode. This function creates or
//              destroys the unit production list for the unit list
//              widget. This function may only be used if the object
//              is a building (more specifically: a factory)
// PARAMETERS : newmode - mode to switch to
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void ContainerWindow::SwitchMode( unsigned short newmode ) {
  Mission *m = Gam->GetMission();

  if ( newmode == CW_MODE_PRODUCTION ) {             // only buildings, current
                                                     // mode must be CW_MODE_NORMAL
    // create production list
    Building *b = static_cast<Building *>(c);
    unsigned long prod = b->UnitProduction();

    if ( prod ) {
      unsigned short crystals = b->Crystals();

      for ( int i = 0; i < 32; ++i ) {
        if ( prod & (1 << i) ) {        // this unit type can be built
          ULWNode *n = new ULWNode;
          n->type = m->GetUnitSet().GetUnitInfo( i );
          n->unit = NULL;
          n->image = n->type->Image() + b->Owner()->ID() * 6;
          n->ok = (n->type->Cost() <= crystals);
          n->selected = false;
          build.AddTail( n );
        }
      }

      // disable repair button
      if ( buttons[CH_BUTTON_REPAIR] ) buttons[CH_BUTTON_REPAIR]->Disable();

      // display new list in list widget
      DrawBack( unitinfo );
      listwidget->SwitchList( &build, 0 );
      UnitInfoWindow::DrawUnitInfo(
          static_cast<ULWNode *>(build.Head())->type->ID(),
          &m->GetUnitSet(), &m->GetTerrainSet(), this, unitinfo );
      Show();
    }
  } else {       // switch back to CW_MODE_NORMAL from CW_MODE_PRODUCTION
    // destroy production list
    while ( !build.IsEmpty() ) delete build.RemHead();

    // re-enable repair and button
    if ( buttons[CH_BUTTON_REPAIR] ) buttons[CH_BUTTON_REPAIR]->Enable();

    // display normal list in widget
    DrawBack( unitinfo );
    listwidget->SwitchList( &normal, -1 );
    UnitInfoWindow::DrawUnitInfo( -1, NULL, NULL, this, unitinfo );
    Show();
  }

  mode = newmode;
  last_selected = NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : ContainerWindow::SelectNode
// DESCRIPTION: When a node from the list widget is selected, either try
//              to activate that unit (if in normal mode) or try to
//              build it (if in production mode).
// PARAMETERS : node - selected node from the list widget
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status ContainerWindow::SelectNode( ULWNode *node ) {
  GUI_Status rc = GUI_OK;

  if ( node ) {
    if ( mode == CW_MODE_NORMAL ) {          // normal mode
      if ( node->ok ) {                      // unit is ready
        Gam->SelectUnit( node->unit );
        rc = GUI_CLOSE;
      }
    } else {                        // production mode
      if ( node->ok ) {             // sufficient amount of crystals
        Building *b = static_cast<Building *>(c);
        Unit *u = Gam->GetMission()->CreateUnit( node->type->ID(), *b->Owner(), b->Position() );

        // add unit to the widget's list
        ULWNode *ulw = new ULWNode;
        ulw->unit = u;
        ulw->type = u->Type();
        ulw->image = u->BaseImage();
        ulw->ok = false;
        ulw->selected = false;
        normal.AddTail( ulw );

        b->SetCrystals( b->Crystals() - u->BuildCost() );
        DrawCrystals();
        Show();
        SwitchMode( CW_MODE_NORMAL );
      } else new NoteWindow( MSG(MSG_ERR_NO_CRYSTALS), MSG(MSG_ERR_NO_PRODUCTION), 0, view );
    }
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : ContainerWindow::WidgetActivated
// DESCRIPTION: Call the appropriate functions when widgets are pressed
//              in the ContainerWindow or associated windows.
// PARAMETERS : widget - widget that called the function
//              win    - pointer to the window the widget belongs to
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status ContainerWindow::WidgetActivated( Widget *widget, Window *win ) {
  GUI_Status rc = GUI_OK;

  switch ( widget->ID() ) {
  case CH_BUTTON_REPAIR: {
    ULWNode *n = static_cast<ULWNode *>(listwidget->Selected());
    if ( n ) {
      Unit *u = n->unit;
      if ( u->GroupSize() < MAX_GROUP_SIZE ) {
        string repmsg;
        repmsg = StringUtil::strprintf( MSG(MSG_ASK_REPAIR), u->Name() );
        repmsg = StringUtil::strprintf( repmsg, CRYSTALS_REPAIR );

        if ( c->Crystals() >= CRYSTALS_REPAIR ) {
          string buttons;
          buttons.append( MSG(MSG_B_REPAIR) );
          buttons += '|';
          buttons.append( MSG(MSG_B_CANCEL) );
          DialogWindow *reqwin =
            new DialogWindow( NULL, repmsg, buttons, 0, 0, view );
          reqwin->SetButtonID( 0, CH_BUTTON_REPAIR_CONFIRM );
          reqwin->SetButtonHook( 0, this );
        } else new NoteWindow( MSG(MSG_ERR_NO_CRYSTALS), repmsg, WIN_CLOSE_ESC, view );
      }
    }
    break; }
  case CH_BUTTON_BUILD:
    SwitchMode( (Mode() == CW_MODE_NORMAL) ? CW_MODE_PRODUCTION : CW_MODE_NORMAL );
    break;

  case CH_BUTTON_REPAIR_CONFIRM: {
    Audio::PlaySfx( Audio::SND_GAM_REPAIR, 0 );
    Unit *u = static_cast<ULWNode *>(listwidget->Selected())->unit;
    c->SetCrystals( c->Crystals() - CRYSTALS_REPAIR );
    if ( Gam->GetMission()->GetHistory() )
      Gam->GetMission()->GetHistory()->RecordUnitEvent( *u, History::HIST_UEVENT_REPAIR );

    u->Repair();
    listwidget->Draw();      // update list display
    DrawCrystals();          // update crystals
    Show();
    rc = GUI_CLOSE;          // close the request window
    break; }

  case CH_LIST_UNITS: {
    // Display information about the currently selected unit
    // or, if the node has been selected twice (e.g. by double
    // clicking), select the respective unit.
    ULWNode *node = static_cast<ULWNode *>(
                    static_cast<ListWidget *>(widget)->Selected()
                    );

    if ( node != last_selected ) {
      short uid = -1;
      if ( node ) uid = node->type->ID();
      DrawBack( unitinfo );
      UnitInfoWindow::DrawUnitInfo( uid, &Gam->GetMission()->GetUnitSet(),
                      &Gam->GetMission()->GetTerrainSet(), this, unitinfo );
      win->Show( unitinfo );
      last_selected = node;
    } else if ( node ) rc = SelectNode( node );
    break; }
  }

  return rc;
}


////////////////////////////////////////////////////////////////////////
// NAME       : UnitLoadWindow::UnitLoadWindow
// DESCRIPTION: This window is used to ask the player which units to
//              load when he moves a transport out of a container.
// PARAMETERS : t       - transport to be moved
//              c       - unit container the transport is moving out of
//              uset    - unit set used
//              tset    - terrain set used
//              history - for event recording
//              view    - pointer to the view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

UnitLoadWindow::UnitLoadWindow( Transport &t, UnitContainer &c,
        const UnitSet &uset, const TerrainSet &tset,
        History *history, View *view ) :
        Window( WIN_CENTER, view ), t(t), c(c), last_selected(0),
        fullslots(t.FullSlots()), crystals(t.Crystals()),
        have_units(false), history(history) {
  // calculate window dimensions
  short width = uset.TileWidth() + XP_ICON_WIDTH + DEFAULT_SLIDER_SIZE +
                MAX(sfont->Width() * 24, tset.TileWidth() * 4) + 20;
  short height = tset.TileHeight() + tset.TileShiftY() + ICON_HEIGHT * 3 +
                 sfont->Height() * 3 + 70;
  SetSize( MIN(width, view->Width()), MIN(height, view->Height()) );

  // create unit list
  for ( int i = 0; i < c.UnitCount(); ++i ) {
    Unit *u = c.GetUnit( i );
    if ( (u != &t) && static_cast<UnitContainer &>(t).Allow( u->Type() ) ) {
      ULWNode *n = new ULWNode;
      n->unit = u;
      n->type = u->Type();
      n->image = u->BaseImage();
      n->ok = u->IsReady() && t.Allow( u );
      n->selected = false;
      units.AddTail( n );
      have_units |= n->ok;
    }
  }

  if ( !Opened() ) return;

  // create list widget
  wd_list = new UnitListWidget(
          ULW_WIDGET_LIST, 10, sfont->Height() + 10,
          uset.TileWidth() + XP_ICON_WIDTH + DEFAULT_SLIDER_SIZE + 20,
          h - sfont->Height() * 2 - 28, &units, -1,
          WIDGET_VSCROLL|WIDGET_HSCROLLKEY|WIDGET_VSCROLLKEY,
          uset, tset, this );
  wd_list->SetKey( SDLK_SPACE );
  wd_list->SetHook( this );

  // create buttons
  Widget *wd = new ButtonWidget( ULW_WIDGET_OK,
               1, h - sfont->Height() - 9, (w - 2) / 2, sfont->Height() + 8,
               WIDGET_DEFAULT, MSG(MSG_B_OK), this );
  wd->SetHook( this );

  new ButtonWidget( GUI_CLOSE, wd->LeftEdge() + wd->Width(), wd->TopEdge(),
               wd->Width(), wd->Height(), 0, MSG(MSG_B_CANCEL), this );

  unitinfo = Rect( wd_list->LeftEdge() + wd_list->Width() + 5,
             wd_list->TopEdge(),
             w - wd_list->LeftEdge() - wd_list->Width() - 15,
             wd_list->Height() - ICON_HEIGHT );

  char buf[10];
  sprintf( buf, "(%d)", c.Crystals() + t.Crystals() );
  cstrw = sfont->TextWidth( buf );
  wd_crystals = new StringWidget( 0,
           unitinfo.x + unitinfo.w - cstrw - sfont->Width() * 4,
           unitinfo.y + unitinfo.h + (ICON_HEIGHT - sfont->Height() - 8)/2,
           sfont->Width() * 4, sfont->Height() + 8,
           itoa( crystals, buf ), 4,
           WIDGET_ALIGN_CENTER|WIDGET_STR_CONST|WIDGET_STYLE_NOBORDER,
           NULL, this );

  unsigned short max = MIN( t.MaxCrystals() - ((fullslots - (crystals+9)/10) * 10),
                            c.Crystals() + t.Crystals() );
  wd_slider = new SliderWidget( ULW_WIDGET_CRYSTALS, unitinfo.x + ICON_WIDTH,
           wd_crystals->TopEdge() + (wd_crystals->Height() - DEFAULT_SLIDER_SIZE) / 2,
           wd_crystals->LeftEdge() - unitinfo.x - ICON_WIDTH - 5, DEFAULT_SLIDER_SIZE,
           0, max, crystals, max/5, WIDGET_HSCROLL|WIDGET_HSCROLLKEY, NULL, this );
  wd_slider->SetHook( this );

  Draw();
  wd_list->Select( 0 );
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitLoadWindow::Draw
// DESCRIPTION: Draw the window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void UnitLoadWindow::Draw( void ) {
  Window::Draw();
  UnitInfoWindow::DrawUnitInfo( -1, NULL, NULL, this, unitinfo );

  const char *msg = MSG(MSG_TRANSFER_UNITS);
  unsigned short txtw = sfont->TextWidth(msg);
  sfont->WriteEllipsis( msg, this, MAX( 10, (w - txtw) / 2 ), 5,
                        Rect( 10, 5, w-20, sfont->Height() ) );

  Image crystals_icon( view->GetSystemIcons(), 160, 0, ICON_WIDTH, ICON_HEIGHT );
  crystals_icon.Draw( this, unitinfo.x, unitinfo.y + unitinfo.h );

  char buf[10];
  sprintf( buf, "(%d)", c.Crystals() + t.Crystals() );
  sfont->Write( buf, this, unitinfo.x + unitinfo.w - cstrw,
                unitinfo.y + unitinfo.h + (ICON_HEIGHT - sfont->Height() - 2)/2,
                *this );
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitLoadWindow::WidgetActivated
// DESCRIPTION: Call the appropriate functions when widgets are pressed
//              in the window.
// PARAMETERS : widget - widget that called the function
//              win    - pointer to the window the widget belongs to
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status UnitLoadWindow::WidgetActivated( Widget *widget, Window *win ) {
  GUI_Status rc = GUI_OK;
  ULWNode *node;

  switch ( widget->ID() ) {
  case ULW_WIDGET_LIST: {
    // Display information about the currently selected unit
    // or, if the node has been selected twice (e.g. by double
    // clicking), select the respective unit.
    node = static_cast<ULWNode *>( wd_list->Selected() );

    if ( node != last_selected ) {
      short uid = -1;
      if ( node ) uid = node->type->ID();
      DrawBack( unitinfo );
      UnitInfoWindow::DrawUnitInfo( uid, &Gam->GetMission()->GetUnitSet(),
                      &Gam->GetMission()->GetTerrainSet(), this, unitinfo );
      win->Show( unitinfo );
      last_selected = node;
    } else if ( node ) {
      if ( node->ok ) {
        node->selected ^= 1;
        if ( node->selected ) fullslots += node->unit->Weight();
        else fullslots -= node->unit->Weight();

        // update the ok flag for all nodes
        for ( node = static_cast<ULWNode *>(units.Head()); node;
              node = static_cast<ULWNode *>(node->Next()) ) {
          node->ok = node->selected || (node->unit->IsReady() &&
                     (t.Slots() >= fullslots + node->unit->Weight()));
        }

        wd_list->Draw();
        wd_list->Show();

        unsigned short max =
          MIN( t.MaxCrystals() - ((fullslots - (crystals+9)/10) * 10),
               c.Crystals() + t.Crystals() );
        wd_slider->Adjust( 0, max, max/5 );
        wd_slider->Draw();
        wd_slider->Show();
      } else Audio::PlaySfx( Audio::SND_GUI_ERROR, 0 );
    }
    break; }
  case ULW_WIDGET_CRYSTALS: {
    char buf[8];
    fullslots -= (crystals + 9)/10 - (wd_slider->Level() + 9)/10;
    crystals = wd_slider->Level();
    wd_crystals->SetString( itoa( crystals, buf ), true );

    // update the ok flag for all nodes
    for ( node = static_cast<ULWNode *>(units.Head()); node;
          node = static_cast<ULWNode *>(node->Next()) ) {
      node->ok = node->selected || (node->unit->IsReady() &&
                 (t.Slots() >= fullslots + node->unit->Weight()));
    }
    wd_list->Draw();
    wd_list->Show();
    break; }
  case ULW_WIDGET_OK:
    // move selected units into the transport
    for ( node = static_cast<ULWNode *>(units.Head()); node;
          node = static_cast<ULWNode *>(node->Next()) ) {
      if ( node->selected ) {
        c.RemoveUnit( node->unit );
        node->unit->SetFlags( U_MOVED|U_DONE );
        t.InsertUnit( node->unit );

        if ( history )
          history->RecordTransportEvent( c, t, *node->unit );

      }
    }

    if ( history && (t.Crystals() != crystals) )
      history->RecordTransportEvent( c, t, crystals - t.Crystals() );

    c.SetCrystals( c.Crystals() - (crystals - t.Crystals()) );
    t.SetCrystals( crystals );
    rc = (GUI_Status)ULW_WIDGET_OK;
    break;
  }

  return rc;
}

