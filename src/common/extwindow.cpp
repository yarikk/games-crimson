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
// extwindow.cpp
///////////////////////////////////////////////////////////////////////

#include <string.h>

#include "extwindow.h"
#include "misc.h"
#include "sound.h"
#include "msgs.h"
#include "globals.h"  // for MIN_XRES, MIN_YRES

////////////////////////////////////////////////////////////////////////
// NAME       : DialogWindow::DialogWindow
// DESCRIPTION: Display a window containing a short message and an
//              arbitrary number of button widgets.
// PARAMETERS : title   - window title (may be NULL)
//              msg     - message
//              buttons - captions of the button widgets, separated by
//                        '|'. The first underscore in each caption (if
//                        any) will be used as the keyboard shortcut for
//                        that button.
//              def     - identifier of the default button (starting from
//                        0; -1 for none)
//              flags   - window flags (see window.h for details)
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

DialogWindow::DialogWindow( const char *title, const string &msg,
      const string &buttons, short def, unsigned short flags, View *view ) :
      Window(flags, view), btncnt(1), text(msg), head(title) {
  Font *tfont = (flags & WIN_FONT_BIG) ? LargeFont() : SmallFont();

  unsigned short txtw = sfont->TextWidth( msg.c_str() );
  unsigned short defw = MAX( view->Width() * 3/4, 300 ),
                 prefw = MIN( defw, txtw + 25 ),
                 btnw = 0;

  // count buttons
  bcaptions = new char [buttons.length() + 1];
  strcpy( bcaptions, buttons.c_str() );
  char *start = bcaptions;
  for ( char *ptr = bcaptions; *ptr != '\0'; ++ptr ) {
    if ( *ptr == '|' ) {
      *ptr = '\0';
      btnw = MAX( btnw, sfont->TextWidth(start) + 15 );
      start = ptr+1;
      ++btncnt;
    }
  }
  btnw = MAX( btnw, sfont->TextWidth(start) + 15 );

  if ( btnw * btncnt + 10 > prefw ) prefw = btnw * btncnt + 10;
  if ( title ) prefw = MAX( prefw, tfont->TextWidth( title ) + 20 );
  if ( prefw > view->Width() ) prefw = view->Width();

  Rect win( 0, 0, prefw,
            sfont->TextHeight(msg.c_str(), prefw-20, 2) +
            sfont->Height() +
            (title ? tfont->Height() + 10 : 5) + 20 );
  win.Clip( *view );

  unsigned short bspacing = (win.w - btncnt * btnw) / (btncnt + 1),
                 bx = bspacing, by = win.h - sfont->Height() - 10,
                 txty = (title ? tfont->Height() + 10 : 5);

  if ( !(flags & WIN_CENTER) ) {
    // get mouse pointer position
    int mx, my;
    SDL_GetMouseState( &mx, &my );
    win.x = mx - bspacing - (bspacing + btnw) * MAX(0,def) - btnw/2;
    win.y = my - win.h + (sfont->Height() + 6)/2 + 4;
    win.Align( *view );
  }

  SetSize( win );

  new TextWidget( 0, 5, txty, w - 10, by - txty - 5,
                  msg.c_str(), WIDGET_ALIGN_CENTER, NULL, this );

  btns = new ButtonWidget * [btncnt];
  char *cap = bcaptions;
  for ( int i = 0; i < btncnt; ++i ) {
    btns[i] = new ButtonWidget( GUI_CLOSE, bx, by,
                  btnw, sfont->Height() + 6,
                  (def == i) ? WIDGET_DEFAULT : 0,
                  cap, this );

    bx += btnw + bspacing;
    do { ++cap; } while ( *cap != '\0' );
    ++cap;
  }

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : DialogWindow::~DialogWindow
// DESCRIPTION: Delete the resources used by the window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

DialogWindow::~DialogWindow( void ) {
  delete [] btns;
  delete [] bcaptions;
}

////////////////////////////////////////////////////////////////////////
// NAME       : DialogWindow::Draw
// DESCRIPTION: Draw the window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void DialogWindow::Draw( void ) {
  Window::Draw();

  if ( head ) {
    Font *tf = (flags & WIN_FONT_BIG) ? LargeFont() : SmallFont();
    short xoff = (w - tf->TextWidth(head)) / 2;
    tf->Write( head, this, xoff + 3, 8, GetBGPen() );
    tf->Write( head, this, xoff, 5, GetFGPen() );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : DialogWindow::SetButtonHook
// DESCRIPTION: Set a WidgetHook for all buttons of this window.
// PARAMETERS : hook - WidgetHook
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void DialogWindow::SetButtonHook( WidgetHook *hook ) const {
  for ( int i = 0; i < btncnt; ++i ) btns[i]->SetHook( hook );
}


////////////////////////////////////////////////////////////////////////
// NAME       : NoteWindow::NoteWindow
// DESCRIPTION: Create a window with a title, a short message, and an
//              "OK" button.
// PARAMETERS : title - window title (may be NULL)
//              note  - the message to be displayed
//              flags - see window.h for details
//              view  - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

NoteWindow::NoteWindow( const char *title, const string &note,
                        unsigned short flags, View *view ) :
            DialogWindow( title, note, MSG(MSG_B_OK), 0, flags, view ) {}

////////////////////////////////////////////////////////////////////////
// NAME       : MessageWindow::MessageWindow
// DESCRIPTION: Create a message window. It will contain a TextScroll
//              and a Button widget.
// PARAMETERS : title - window title (may be NULL)
//              msg   - the message to be displayed in the text widget
//                      (may be NULL)
//              view  - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

MessageWindow::MessageWindow( const char *title, const char *msg, View *view ) :
            Window( WIN_CENTER|WIN_CLOSE_ESC, view ) {
  this->title = title;

  // calculate window dimensions
  SetSize( MAX(view->Width()/2, MIN_XRES), MAX(view->Height()*3/4, MIN_YRES) );

  button = new ButtonWidget( GUI_CLOSE, 1, h - sfont->Height() - 11, w - 2, sfont->Height() + 10,
                             WIDGET_DEFAULT, MSG(MSG_B_OK), this );

  short boxy = 5;
  if ( title ) boxy += lfont->Height() * 2;
  textscroll = new TextScrollWidget( 1, 5, boxy, w - 10, h - boxy - button->Height() - 15,
                                     msg, 0, NULL, this );
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : MessageWindow::Draw
// DESCRIPTION: Draw the window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MessageWindow::Draw( void ) {
  Window::Draw();
  if ( title ) {
    int xoff = (w - lfont->TextWidth( title )) / 2;
    int yoff = (5 + lfont->Height()) / 2;

    lfont->Write( title, this, xoff+3, yoff+3, GetBGPen() );
    lfont->Write( title, this, xoff, yoff, GetFGPen() );
  }
}


////////////////////////////////////////////////////////////////////////
// NAME       : PasswordWindow::PasswordWindow
// DESCRIPTION: Create a window containing a string widget for asking
//              the user for a password.
// PARAMETERS : title - window title
//              msg   - single-line message to be printed (may be NULL)
//              pass  - password to check the user input against; if
//                      this is NULL any input will be accepted
//              abort - whether dialog can be cancelled
//              view  - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

PasswordWindow::PasswordWindow( const char *title, const char *msg,
                        const char *pass, bool abort, View *view ) :
            Window( WIN_CENTER, view ) {
  this->title = title;
  password = NULL;
  NewPassword( pass );

  // calculate window dimensions
  unsigned short width = MAX( sfont->TextWidth(title) + 20, sfont->Width() * 9 + 25 ),
                 height = sfont->Height() * 3 + 45;
  if ( msg ) {
    width = MAX( width, sfont->TextWidth(msg) + 40 );
    height += sfont->Height() + 10;
  }
  SetSize( width, height );

  string = new StringWidget( 0, (w - sfont->Width() * 9 - 4) / 2,
                             h - sfont->Height() * 2 - 30,
                             sfont->Width() * 9 + 4, sfont->Height() + 4,
                             NULL, 7, WIDGET_ALIGN_TOP|WIDGET_STR_PASSWORD,
                             msg, this );

  if ( abort ) width = w/2 - 2;
  else width = w - 2;

  Widget *wd = new ButtonWidget( GUI_CLOSE, 1, h - sfont->Height() - 9,
                                 width, sfont->Height() + 8, WIDGET_DEFAULT,
                                 MSG(MSG_B_OK), this );
  if ( abort ) {
    new ButtonWidget( GUI_RESTART,
                      wd->LeftEdge() + wd->Width() + 1, wd->TopEdge(),
                      wd->Width(), wd->Height(), 0, MSG(MSG_B_CANCEL), this );
  }

  Draw();
  Show();
  string->SetFocus();
}

////////////////////////////////////////////////////////////////////////
// NAME       : PasswordWindow::Draw
// DESCRIPTION: Draw the password window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void PasswordWindow::Draw( void ) {
  short xoff = (w - sfont->TextWidth( title )) / 2;

  Window::Draw();
  sfont->Write( title, this, xoff + 3, 10, GetBGPen() );
  sfont->Write( title, this, xoff, 7, GetFGPen() );
  DrawBox( Rect( 5, 15 + sfont->Height(),
                 w - 10, h - 2 * sfont->Height() - 30 ), BOX_RECESSED );
}

////////////////////////////////////////////////////////////////////////
// NAME       : PasswordWindow::PasswordOk
// DESCRIPTION: Check the user input against the given password string.
// PARAMETERS : -
// RETURNS    : TRUE on match, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool PasswordWindow::PasswordOk( void ) const {
  if ( !password ) return true;
  else if ( !string->String() ) return false;
  return !strcmp( password, string->String() );
}

////////////////////////////////////////////////////////////////////////
// NAME       : PasswordWindow::NewPassword
// DESCRIPTION: Set new password to check the user input against.
// PARAMETERS : pass - new password; if this is NULL any user input will
//                     be accepted
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void PasswordWindow::NewPassword( const char *pass ) {
  delete [] password;

  if ( pass ) {
    password = new char [strlen(pass) + 1];
    strcpy( password, pass );
  } else password = NULL;
}


////////////////////////////////////////////////////////////////////////
// NAME       : ProgressWindow::ProgressWindow
// DESCRIPTION: Create a window containing a progress bar display and
//              an 'abort' button.
// PARAMETERS : x     - left edge of window
//              y     - top edge of window
//              w     - window width
//              h     - window height
//              pmin  - value indicating an empty progress bar
//              pmax  - value indicating a filled progress bar
//              msg   - message to display (may be NULL)
//              flags - window flags (see window.h for details)
//              view  - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

ProgressWindow::ProgressWindow( short x, short y, unsigned short w,
                unsigned short h, short pmin, short pmax,
                const char *msg, unsigned short flags, View *view ) :
                Window( x, y, w, h, flags, view ) {
  short bwidth = 0;

  if ( flags & WIN_PROG_ABORT ) {
    const char *lbl = MSG(MSG_B_SKIP);
    bwidth = sfont->TextWidth( lbl ) + 15;
    new ButtonWidget( GUI_CLOSE, w - bwidth - 5, 5, bwidth, h - 10,
                      (flags & WIN_PROG_DEFAULT ? WIDGET_DEFAULT : 0),
                      lbl, this );
  }

  progress = new ProgressWidget( GUI_OK, 5, 5, w - 10 - bwidth, h - 10,
                                 pmin, pmax, 0, msg, this );
  progress->SetColor( GetFGPen() );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : ProgressWindow::Cancelled
// DESCRIPTION: Check whether the user clicked the 'Cancel' button.
// PARAMETERS : -
// RETURNS    : TRUE if button was activated, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool ProgressWindow::Cancelled( void ) {
  GUI_Status peek;
  bool abort = false;

  do {
    SDL_Event event;
    peek = view->PeekEvent( event );
    if ( (peek != GUI_NONE) &&
         (HandleEvent( event ) == GUI_CLOSE) ) abort = true;
  } while ( peek != GUI_NONE );

  return abort;
}


////////////////////////////////////////////////////////////////////////
// NAME       : MenuWindow::MenuWindow
// DESCRIPTION: Create a new menu window.
// PARAMETERS : title - menu title (may be NULL)
//              hook  - widget hook to be notified when one of the menu
//                      items is activated (may be NULL); sub-menus
//                      are handled internally, so you won't get
//                      messages for those
//              view  - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

MenuWindow::MenuWindow( const char *title, WidgetHook *hook, View *view ) :
            Window( WIN_CLOSE_ESC|WIN_CLOSE_UNFOCUS, view ),
            layout(false), position(-1,-1), item_hook(hook), parent_menu(0) {
  minwidth = 0;
  vspacing = 4;
  barheight = vspacing * 2 + 2;    // height of the separator bar

  if ( title ) {
    items.AddHead( new MenuItem( MENU_TITLE, 0, 0, 0, title ) );
    AddBar( 0 );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : MenuWindow::AddItem
// DESCRIPTION: Add a menu item to the menu.
// PARAMETERS : level - menu level the item will be added to (root is 0)
//              id    - menu button identifier for this item
//              flags - button flags (see widget.h for details)
//              title - menu item caption
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MenuWindow::AddItem( unsigned char level, short id, unsigned short flags,
                          const char *title ) {
  items.AddTail(
    new MenuItem( MENU_ITEM, level, id, flags, title )
  );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MenuWindow::AddMenu
// DESCRIPTION: Add a sub-menu to the menu.
// PARAMETERS : level - menu level the menu will be added to (root is 0)
//              flags - button flags (see widget.h for details)
//              title - sub-menu caption
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MenuWindow::AddMenu( unsigned char level, unsigned short flags,
                          const char *title ) {
  items.AddTail(
    // sub-menus get their position in the items list as identifier;
    // that way the internal button handler can easily get the data
    // from the list
    new MenuItem( MENU_SUBMENU, level, items.CountNodes(),
                  flags|WIDGET_STYLE_SUBMENU, title )
  );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MenuWindow::AddBar
// DESCRIPTION: Add a separator bar to the menu.
// PARAMETERS : level - menu level the bar will be added to (root is 0)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MenuWindow::AddBar( unsigned char level ) {
  items.AddTail(
    new MenuItem( MENU_SEPARATOR, level, 0, 0, "" )
  );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MenuWindow::Layout
// DESCRIPTION: Once the entire menu has been specified (all items
//              added etc.) we need to calculate the window size and
//              position and create the buttons.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MenuWindow::Layout( void ) {
  if ( items.IsEmpty() ) return;

  // calculate window size
  MenuItem *mi = static_cast<MenuItem *>( items.Head() );
  unsigned char lev = mi->mi_level;
  unsigned short maxwidth = 0, width, height = 10;
  bool havetitle = false;

  do {
    if ( mi->mi_level == lev ) {
      if ( mi->mi_type == MENU_SEPARATOR ) {
        width = 0;
        height += barheight;
      } else {
        width = sfont->TextWidth( mi->mi_title.c_str() );

        // reserve space for the submenu arrow
        if ( mi->mi_type == MENU_SUBMENU ) width += 30;

        height += sfont->Height() + vspacing;

        if ( mi->mi_type == MENU_TITLE ) havetitle = true;
      }

      if ( width > maxwidth ) maxwidth = width;
    }

    mi = static_cast<MenuItem *>( mi->Next() );
  } while ( mi );


  // if position is -1,-1 (the default setting) the menu will open
  // close to the mouse pointer
  Rect win;
  if ( position == Point(-1,-1) ) {
    int mx, my;
    SDL_GetMouseState( &mx, &my );

    win.x = mx - (maxwidth - 20) / 2;
    win.y = my - sfont->Height() / 2 - (havetitle ? sfont->Height() + vspacing + barheight : 0) - 5;
  } else {
    win.x = position.x;
    win.y = position.y;
  }
  win.w = MAX( maxwidth + 20, minwidth );
  win.h = height;

  win.Align( *view );
  SetSize( win );

  // create the buttons
  mi = static_cast<MenuItem *>( items.Head() );
  unsigned short ypos = 5;

  do {
    if ( mi->mi_level == lev ) {
      if ( (mi->mi_type == MENU_ITEM) || (mi->mi_type == MENU_SUBMENU) ) {

        MenuButtonWidget *mbw = new MenuButtonWidget( mi->mi_id, 5, ypos,
            w-10, sfont->Height() + vspacing,
            WIDGET_STYLE_MENU|WIDGET_STYLE_NOBORDER|mi->mi_flags,
            mi->mi_title.c_str(), this );
        mbw->SetHook( (mi->mi_type == MENU_ITEM ? item_hook : this) );
        ypos += mbw->Height();
      } else if ( mi->mi_type == MENU_TITLE ) ypos += sfont->Height() + vspacing;
      else ypos += barheight;
    }

    mi = static_cast<MenuItem *>( mi->Next() );
  } while ( mi && (mi->mi_level >= lev) );

  layout = true;
  if ( lev == 0 ) Audio::PlaySfx( Audio::SND_GUI_MENU_SHOW, 0 );
  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : MenuWindow::Draw
// DESCRIPTION: Draw the window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MenuWindow::Draw( void ) {
  if ( !layout ) return;

  Window::Draw();

  MenuItem *mi = static_cast<MenuItem *>( items.Head() );
  unsigned char lev = mi->mi_level;
  unsigned short ypos = 5;

  do {
    if ( mi->mi_level == lev ) {

      switch ( mi->mi_type ) {
      case MENU_TITLE:
        sfont->Write( mi->mi_title.c_str(), this,
        (w - sfont->TextWidth(mi->mi_title.c_str()))/2, ypos, GetFGPen() );
        ypos += sfont->Height() + vspacing;
        break;
      case MENU_SEPARATOR:
        FillRect( 5, ypos + vspacing, w - 10, 1, Color(CF_COLOR_SHADOW) );
        FillRect( 5, ypos + vspacing + 1, w - 10, 1, Color(CF_COLOR_HIGHLIGHT) );
        ypos += barheight;
        break;
      default: // MENU_ITEM and MENU_SUBMENU
        ypos += sfont->Height() + vspacing;
      }
    }

    mi = static_cast<MenuItem *>( mi->Next() );
  } while ( mi && (mi->mi_level >= lev) );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MenuWindow::HandleEvent
// DESCRIPTION: In addition to the standard Window::HandleEvent method,
//              we support menu navigation by keyboard here.
// PARAMETERS : event - event received from the event handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status MenuWindow::HandleEvent( const SDL_Event &event ) {
  GUI_Status rc = GUI_OK;

  if ( (event.type == SDL_KEYDOWN) &&
       (event.key.keysym.sym >= SDLK_UP) &&
       (event.key.keysym.sym <= SDLK_LEFT) ) {
    MenuButtonWidget *cur, *sel;

    switch( event.key.keysym.sym ) {
    case SDLK_LEFT:             // close window
      view->CloseWindow( this );
      break;
    case SDLK_RIGHT:            // descend into submenu (same as RETURN)
      cur = CurrentItem();
      if ( cur && cur->IsMenu() ) cur->Push();
      break;
    case SDLK_UP:
    case SDLK_DOWN:
      cur = CurrentItem();
      sel = (event.key.keysym.sym == SDLK_UP) ? PrevItem() : NextItem();

      if ( sel ) {
        sel->ToggleCurrent();
        sel->Draw();
        sel->Show();

        if ( cur ) {
          cur->ToggleCurrent();
          cur->Draw();
          cur->Show();
        }
      }
      break;
    default: break;
    }

  } else if ( (event.type == SDL_KEYUP) &&
       (event.key.keysym.sym == SDLK_RIGHT) ) {
    MenuButtonWidget *cur = CurrentItem();

    if ( cur && cur->IsMenu() && cur->Clicked() ) {
      cur->Release();
      rc = cur->Activate();
    }

  } else rc = Window::HandleEvent( event );

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : MenuWindow::WidgetActivated
// DESCRIPTION: When one of the sub-menu buttons is selected, we need
//              to create the appropriate sub-menu.
// PARAMETERS : widget - selected button; its ID corresponds to its
//                       position in the items list.
//              win    - window (unused)
// RETURNS    : GUI_OK
////////////////////////////////////////////////////////////////////////

GUI_Status MenuWindow::WidgetActivated( Widget *button, Window *win ) {
  MenuWindow *sub = new MenuWindow( NULL, item_hook, view );
  MenuItem *mi = static_cast<MenuItem *>( items.GetNode(button->ID()) );
  unsigned char lev = mi->mi_level + 1;

  for ( mi = static_cast<MenuItem *>( mi->Next() );
        mi && (mi->mi_level >= lev);
        mi = static_cast<MenuItem *>( mi->Next() ) ) {

    switch ( mi->mi_type ) {
    case MENU_ITEM:
      sub->AddItem( mi->mi_level, mi->mi_id, mi->mi_flags,
                    mi->mi_title.c_str() );
      break;
    case MENU_SUBMENU:
      sub->AddMenu( mi->mi_level, mi->mi_flags,
                    mi->mi_title.c_str() );
      break;
   case MENU_SEPARATOR:
      sub->AddBar( mi->mi_level );
      break;
    }
  }

  sub->SetPosition( Point( x + w, y + button->TopEdge() - 5 ) );
  sub->SetParent( this );
  sub->Layout();

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : MenuWindow::CloseParent
// DESCRIPTION: When an item from the deepest sub-menu is selected the
//              higher menues are still open. This method can be used
//              to close the next higher menu.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MenuWindow::CloseParent( void ) {
  if ( parent_menu ) {
    parent_menu->CloseParent();
    view->CloseWindow( parent_menu );
    parent_menu = NULL;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : MenuWindow::NextItem
// DESCRIPTION: Get a pointer to the next (enabled) menu button. Widgets
//              are added to the window in reverse order, so this
//              actually looks more like PrevItem...
// PARAMETERS : -
// RETURNS    : pointer to the next menu item. If current is the last
//              valid item in the menu the search will continue at the
//              beginning. If there is no (other) valid item, the method
//              returns NULL.
////////////////////////////////////////////////////////////////////////

MenuButtonWidget *MenuWindow::NextItem( void ) const {
  Widget *wd, *prev = NULL, *current = CurrentItem();

  wd = NextItem( current );

  while ( wd && (wd != current) ) {
    if ( !wd->Disabled() ) {
      prev = wd;
      break;
    }
    wd = NextItem( wd );
  }

  return static_cast<MenuButtonWidget *>(prev);
}

////////////////////////////////////////////////////////////////////////
// NAME       : MenuWindow::PrevItem
// DESCRIPTION: Get a pointer to the previous (enabled) menu button.
// PARAMETERS : -
// RETURNS    : pointer to the previous menu item. If current is the
//              first valid item in the menu the search will continue at
//              the end. If there is no (other) valid item, the method
//              returns NULL.
////////////////////////////////////////////////////////////////////////

MenuButtonWidget *MenuWindow::PrevItem( void ) const {
  Widget *wd, *next = NULL, *current = CurrentItem();

  if ( current ) wd = current->next;
  else wd = widgets;

  do {

    while ( wd && (wd != current) ) {
      if ( !wd->Disabled() ) {
        next = wd;
        wd = current;
        break;
      }
      wd = wd->next;
    }

    if ( wd != current ) wd = widgets;

  } while ( wd != current );

  return static_cast<MenuButtonWidget *>(next);
}

////////////////////////////////////////////////////////////////////////
// NAME       : MenuWindow::NextItem
// DESCRIPTION: Get a pointer to the next menu button of an arbitrary
//              menu item, regardless of state.
// PARAMETERS : current - item to get the successor of
// RETURNS    : pointer to the next menu item or NULL if there is no
//              other item
////////////////////////////////////////////////////////////////////////

MenuButtonWidget *MenuWindow::NextItem( Widget *current ) const {
  Widget *wd = widgets, *prev = NULL;

  if ( !current || (current == widgets) ) {
    while ( wd ) {
      prev = wd;
      wd = wd->next;
    }
  } else {
    while ( wd != current ) {
      prev = wd;
      wd = wd->next;
    }
  }
  return static_cast<MenuButtonWidget *>(prev);
}

////////////////////////////////////////////////////////////////////////
// NAME       : MenuWindow::CurrentItem
// DESCRIPTION: Get a pointer to the currently selected menu button.
// PARAMETERS : -
// RETURNS    : pointer to the selected menu item or NULL if none is
//              selected
////////////////////////////////////////////////////////////////////////

MenuButtonWidget *MenuWindow::CurrentItem( void ) const {
  MenuButtonWidget *wd = static_cast<MenuButtonWidget *>(widgets);

  while ( wd ) {
    if ( wd->IsCurrent() ) break;

    wd = static_cast<MenuButtonWidget *>(wd->next);
  }

  return wd;
}


////////////////////////////////////////////////////////////////////////
// NAME       : MenuItem::MenuItem
// DESCRIPTION: Create a new menu item.
// PARAMETERS : type  - item type (see extwindow.h for details)
//              lev   - menu level this item belongs to
//              id    - identifier for this item
//              flags - button flags (see widget.h for details)
//              title - item caption
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

MenuWindow::MenuItem::MenuItem( unsigned char type, unsigned char lev, short id,
                    unsigned short flags, const string &title ) :
          mi_type(type), mi_level(lev), mi_id(id), mi_flags(flags),
          mi_title(title) {}

