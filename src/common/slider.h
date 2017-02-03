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
// slider.h - slider widget class
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_SLIDER_H
#define _INCLUDE_SLIDER_H

#include "widget.h"

// mouse wheel events are only available with SDL 1.2.5 or later
#ifndef SDL_BUTTON_WHEELUP
# define SDL_BUTTON_WHEELUP 4
#endif
#ifndef SDL_BUTTON_WHEELDOWN
# define SDL_BUTTON_WHEELDOWN 5
#endif

#define DEFAULT_SLIDER_SIZE  12

class SliderWidget : public Widget {
public:
  SliderWidget( short id, short x, short y, unsigned short w,
    unsigned short h, short vmin, short vmax, short start,
    short ksize, unsigned short flags, const char *title,
    Window *window );

  void Adjust( short newmin, short newmax, short newsize );
  virtual void Draw( void );
  void ScrollTo( short level );

  virtual GUI_Status MouseMove( const SDL_MouseMotionEvent &motion );
  virtual GUI_Status MouseDown( const SDL_MouseButtonEvent &button );
  virtual GUI_Status MouseUp( const SDL_MouseButtonEvent &button );
  virtual GUI_Status KeyDown( const SDL_keysym &key );
  virtual GUI_Status KeyUp( const SDL_keysym &key );

  short Level( void ) const { return current; }
  void SetKeyStep( unsigned short step ) { keystep = step; }

private:
  Rect knob;              // the slider knob

  short min_level;
  short max_level;
  short current;

  short size;
  float step;             // number of pixels to move knob per value
  bool mousehit;          // status var used for mouse dragging
  unsigned short keystep; // level adjustment when scrolling via keys
};


class ProgressWidget : public Widget {
public:
  ProgressWidget( short id, short x, short y, unsigned short w,
               unsigned short h, short vmin, short vmax,
               unsigned short flags, const char *title, Window *window );
  void SetColor( const Color &col ) { this->col = col; }
  short Level( void ) const { return level + min_level; }
  void SetLevel( short lev );
  void Advance( short diff ) { SetLevel( Level() + diff ); }

  void Draw( void );

private:
  unsigned short level;
  short min_level;
  short max_level;

  Color col;

  short title_x;
  short title_y;
};

#endif  /* _INCLUDE_SLIDER_H */

