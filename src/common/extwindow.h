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
// extwindow.h - extended window classes
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_EXTWINDOW_H
#define _INCLUDE_EXTWINDOW_H

#include <string>

#include "window.h"
#include "button.h"
#include "textbox.h"
#include "misc.h"

// window to display some text and an arbitrary number of buttons
// to get a response from the user
class DialogWindow : public Window {
public:
  DialogWindow( const char *title, const string &msg,
                const string &buttons, short def,
                unsigned short flags, View *view );
  ~DialogWindow( void );

  void Draw( void );
  void SetButtonID( unsigned short button, short id ) const
            { btns[button]->SetID( id ); }
  void SetButtonHook( unsigned short button, WidgetHook *hook ) const
            { btns[button]->SetHook( hook ); }
  void SetButtonHook( WidgetHook *hook ) const;

private:
  ButtonWidget **btns;
  unsigned short btncnt;
  string text;
  const char *head;
  char *bcaptions;
};


// simple window with a short message (used e.g. at the start of each turn)
class NoteWindow : public DialogWindow {
public:
  NoteWindow( const char *title, const string &note,
              unsigned short flags, View *view );
};


// message window with a textscroll widget and a button
class MessageWindow : public Window {
public:
  MessageWindow( const char *title, const char *msg, View *view );
  virtual void Draw( void );
  void SetTitle( const char *title ) { this->title = title; }

  ButtonWidget *button;
  TextScrollWidget *textscroll;

private:
  const char *title;
};


// password window
class PasswordWindow : public Window {
public:
  PasswordWindow( const char *title, const char *msg,
                  const char *pass, bool abort, View *view );
  ~PasswordWindow( void ) { delete [] password; }
  void Draw( void );

  bool PasswordOk( void ) const;
  void NewPassword( const char *pass );

  StringWidget *string;

private:
  char *password;
  const char *title;
};


// progress window
class ProgressWindow : public Window, public UserActionHook {
public:
  ProgressWindow( short x, short y, unsigned short w,
                  unsigned short h, short pmin, short pmax,
                  const char *msg, unsigned short flags, View *view );

  virtual bool Cancelled( void );

  void Set( short pos ) const { progress->SetLevel( pos ); }
  short Get( void ) const { return progress->Level(); }
  void Advance( short pdelta ) const { progress->Advance( pdelta ); }

private:
  ProgressWidget *progress;
};


#define MENU_TITLE     1
#define MENU_SUBMENU   2
#define MENU_ITEM      3
#define MENU_SEPARATOR 4

class MenuWindow : public Window, private WidgetHook {
public:
  MenuWindow( const char *title, WidgetHook *hook, View *view );

  void AddItem( unsigned char level, short id, unsigned short flags,
                const char *title );
  void AddMenu( unsigned char level, unsigned short flags,
                const char *title );
  void AddBar( unsigned char level );

  void SetPosition( const Point &pos ) { position = pos; }
  void SetMinWidth( unsigned short w ) { minwidth = w; }
  void Layout( void );
  void Draw( void );

  void SetParent( MenuWindow *parent ) { parent_menu = parent; }
  void CloseParent( void );

  GUI_Status HandleEvent( const SDL_Event &event );

private:
  GUI_Status WidgetActivated( Widget *widget, Window *win );

  MenuButtonWidget *NextItem( void ) const;
  MenuButtonWidget *PrevItem( void ) const;
  MenuButtonWidget *NextItem( Widget *current ) const;
  MenuButtonWidget *CurrentItem( void ) const;

  class MenuItem : public Node {
  public:
    MenuItem( unsigned char type, unsigned char lev, short id,
              unsigned short flags, const string &title );

    unsigned char mi_type;
    unsigned char mi_level;
    short mi_id;
    unsigned short mi_flags;
    string mi_title;
  };

  List items;

  unsigned short vspacing;
  unsigned short barheight;
  bool layout;
  Point position;
  unsigned short minwidth;

  WidgetHook *item_hook;
  MenuWindow *parent_menu;
};

#endif  /* _INCLUDE_EXTWINDOW_H */

