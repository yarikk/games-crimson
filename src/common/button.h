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
// button.h - button widget class
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_BUTTON_H
#define _INCLUDE_BUTTON_H

#include <string>
using namespace std;

#include "widget.h"

class ButtonWidget : public Widget {
public:
  ButtonWidget( short id, short x, short y, unsigned short w,
                unsigned short h, unsigned short flags,
                const char *title, Window *window );

  virtual void Draw( void );

  void SetImage( Surface *image, const Rect &state1, const Rect &state2 );

  virtual GUI_Status MouseDown( const SDL_MouseButtonEvent &button );
  virtual GUI_Status MouseUp( const SDL_MouseButtonEvent &button );
  virtual GUI_Status KeyDown( const SDL_keysym &key );
  virtual GUI_Status KeyUp( const SDL_keysym &key );

  virtual void Push( void );
  GUI_Status Activate( void );

protected:
  Image image[2];
};


#define DEFAULT_CBW_SIZE 15

class CheckboxWidget : public ButtonWidget {
public:
  CheckboxWidget( short id, short x, short y, unsigned short w,
                unsigned short h, bool state, unsigned short flags,
                const char *title, Window *window );

  GUI_Status MouseDown( const SDL_MouseButtonEvent &button );
  GUI_Status MouseUp( const SDL_MouseButtonEvent &button ) { return GUI_OK; }
  GUI_Status KeyDown( const SDL_keysym &key );
  GUI_Status KeyUp( const SDL_keysym &key ) { return GUI_OK; }
};


class MenuButtonWidget : public ButtonWidget {
public:
  MenuButtonWidget( short id, short x, short y, unsigned short w,
                unsigned short h, unsigned short flags,
                const char *title, Window *window );

  virtual GUI_Status MouseMove( const SDL_MouseMotionEvent &move );

  bool IsMenu( void ) const { return (flags & WIDGET_STYLE_SUBMENU) != 0; }
  bool IsCurrent( void ) const
       { return (flags & (WIDGET_STYLE_HIGHLIGHT|WIDGET_DEFAULT)) ==
                         (WIDGET_STYLE_HIGHLIGHT|WIDGET_DEFAULT); }
  void ToggleCurrent( void )
       { flags ^= (WIDGET_STYLE_HIGHLIGHT|WIDGET_DEFAULT); }
};


class CycleWidget : public ButtonWidget {
public:
  CycleWidget( short id, short x, short y, unsigned short w, unsigned short h,
               unsigned short flags, const char *title, unsigned short defval,
               const char *labels[], Window *window );
  ~CycleWidget( void ) { delete [] choices; }

  void SetValue( unsigned short value );
  unsigned short GetValue( void ) const { return val; }
  const char *GetLabel( void ) const { return choices[val].c_str(); }

  GUI_Status MouseDown( const SDL_MouseButtonEvent &button );
  GUI_Status MouseUp( const SDL_MouseButtonEvent &button );
  GUI_Status KeyUp( const SDL_keysym &key );
  void Draw( void );

private:
  void CycleValue( void );

  unsigned short val;
  unsigned short maxval;
  bool up;
  string *choices;
};


class DropWidget : public ButtonWidget {
public:
  DropWidget( short id, short x, short y, unsigned short w,
              unsigned short h, unsigned short flags,
              const char *title, Window *window );

  void Draw( void );
};

#endif	/* _INCLUDE_BUTTON_H */

