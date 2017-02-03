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
// widget.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_WIDGET_H
#define _INCLUDE_WIDGET_H

#include <string>
#include <vector>
using namespace std;

#include "surface.h"
#include "font.h"

class Window;

// GUI return codes
enum GUI_Status { GUI_ERROR = -6, GUI_RESTART, GUI_QUIT, GUI_CLOSE, GUI_NONE, GUI_OK };

// widget flags
#define WIDGET_DEFAULT        0x0001  // widget responds to RETURN key
#define WIDGET_HIDDEN         0x0002  // widget is not shown and does not receive event messages
#define WIDGET_DISABLED       0x0004  // widget is shown, but does not receive event messages
#define WIDGET_COMPOSITE      0x0008  // widget is part of a composite widget (not the
                                      // CompositeWidget itself)
#define WIDGET_ALIGN_CENTER   0x0010
#define WIDGET_ALIGN_LEFT     0x0020
#define WIDGET_ALIGN_RIGHT    0x0040
#define WIDGET_ALIGN_TOP      0x0080
#define WIDGET_ALIGN_WITHIN   0x0100  // must be combined with _LEFT or _RIGHT to be useful

// for buttons mostly; some combinations are not allowed (e.g. SUBMENU and GFX)
#define WIDGET_STYLE_STD       0x0000 // standard bevelled box
#define WIDGET_STYLE_NOBORDER  0x0200 // don't draw widget borders
#define WIDGET_STYLE_GFX       0x0400 // use images
#define WIDGET_STYLE_MENU      0x0800 // kind of combination of NOBORDER and STD
#define WIDGET_STYLE_SUBMENU   0x1000 // display sub-menu indicator
#define WIDGET_STYLE_HIGHLIGHT 0x2000 // use highlight instead of background color

// for string widgets
#define WIDGET_STR_CONST      0x4000  // string in widget cannot be modified
#define WIDGET_STR_PASSWORD   0x8000  // display string as asterisks

// for scrollers
#define WIDGET_HSCROLL        0x1000  // horizontal slider
#define WIDGET_VSCROLL        0x2000  // vertical slider
#define WIDGET_HSCROLLKEY     0x4000  // scroller supports left/right cursor keys
#define WIDGET_VSCROLLKEY     0x8000  // scroller supports up/down cursor keys


class WidgetHook;

class Widget : public Rect {
public:
  Widget( short id, short x, short y, unsigned short w, unsigned short h,
     unsigned short flags, const char *title, Window *window );
  virtual ~Widget( void ) {}

  short ID( void ) const { return id; }
  const char *Title( void ) const { return title.c_str(); }
  Window *GetWindow( void ) const { return surface; }
  void SetFont( Font *font ) { this->font = font; }
  void SetID( short id ) { this->id = id; }
  void SetHook( WidgetHook *hook ) { this->hook = hook; }
  void SetKey( short key ) { this->key = key; }
  void SetTitle( const char *title );

  void Show( void ) const;
  virtual void Draw( void ) = 0;
  void PrintTitle( short xoff, short yoff, const Color &hcol ) const;
  void PrintTitle( const Color &hcol ) const;

  virtual void Push( void );
  virtual void Release( void );
  void Disable( void ) { SetFlags( WIDGET_DISABLED ); }
  void Enable( void ) { UnsetFlags( WIDGET_DISABLED ); }
  void Hide( void ) { SetFlags( WIDGET_HIDDEN ); }
  void Unhide( void ) { UnsetFlags( WIDGET_HIDDEN ); }
  bool Clicked( void ) const { return clicked; }
  bool Hidden( void ) const { return (flags & WIDGET_HIDDEN) != 0; }
  bool Composite( void ) const { return (flags & WIDGET_COMPOSITE) != 0; }
  bool Disabled( void ) const { return (flags & WIDGET_DISABLED) != 0; }
  void SetFlags( unsigned short f ) { flags |= f; }
  void UnsetFlags( unsigned short f ) { flags &= ~f; }

  GUI_Status HandleEvent( const SDL_Event &event );
  virtual GUI_Status MouseDown( const SDL_MouseButtonEvent &button ) { return GUI_OK; }
  virtual GUI_Status MouseUp( const SDL_MouseButtonEvent &button ) { return GUI_OK; }
  virtual GUI_Status KeyDown( const SDL_keysym &key ) { return GUI_OK; }
  virtual GUI_Status KeyUp( const SDL_keysym &key ) { return GUI_OK; }
  virtual GUI_Status MouseMove( const SDL_MouseMotionEvent &motion ) { return GUI_OK; }

  void SetSize( short x, short y, unsigned short w, unsigned short h )
              { this->x = x; this->y = y; this->w = w; this->h = h; }

  Widget *next;

protected:
  Window *surface;
  unsigned short flags;
  short id;
  short key;      // keystroke to activate widget
  short keypos;

  Font *font;
  string title;
  WidgetHook *hook;

  bool clicked;
};

// Hook class for widgets. The activate method will be called
// whenever a widget is "activated". What this means is up to
// the widget
class WidgetHook {
public:
  WidgetHook( void ) {}
  virtual ~WidgetHook( void ) {}

  virtual GUI_Status WidgetActivated( Widget *widget, Window *win ) = 0;
};

// Hook class used to signal aborting in synchronous operations
class UserActionHook {
public:
  virtual bool Cancelled( void ) = 0;
};

// composite widgets encapsulate several widgets but look and act like
// a single widget to the outside
class CompositeWidget : public Widget {
public:
  CompositeWidget( short id, short x, short y,  unsigned short w, unsigned short h,
    unsigned short flags, const char *title, Window *window );
  ~CompositeWidget( void );

  virtual void Draw( void );

  virtual GUI_Status MouseDown( const SDL_MouseButtonEvent &button );
  virtual GUI_Status MouseUp( const SDL_MouseButtonEvent &button );
  virtual GUI_Status KeyDown( const SDL_keysym &key );
  virtual GUI_Status KeyUp( const SDL_keysym &key );
  virtual GUI_Status MouseMove( const SDL_MouseMotionEvent &motion );

protected:
  void AddWidget( Widget *w ) { components.push_back( w ); }
  void RemoveWidget( Widget *w );
  Widget *GetWidget( short id ) const;

private:
  vector<Widget *> components;
};

#include "window.h"

#endif  /* _INCLUDE_WIDGET_H */

