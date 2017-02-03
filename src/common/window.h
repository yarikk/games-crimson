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
// window.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_WINDOW_H
#define _INCLUDE_WINDOW_H

#include "surface.h"
#include "list.h"
#include "font.h"
#include "widget.h"

// window flags
#define WIN_CLOSE_ESC      0x0001   // close window on 'ESC'
#define WIN_CLOSE_UNFOCUS  0x0002   // close window on clicks outside
                                    // the window surface
#define WIN_CENTER         0x0004   // center window on screen

// flags used by the extended window classes
#define WIN_FONT_BIG       0x0010  // use large window font

#define WIN_FILE_SAVE      0x0020  // save flag for file window
#define WIN_FILE_LOAD      0x0040  // load flag for file window

#define WIN_PROG_ABORT     0x0020  // create abort button for progress window
#define WIN_PROG_DEFAULT   0x0040  // set WIDGET_DEFAULT for abort button

#define WIN_DESTROYED      0x8000  // window has been closed, clean me up!

class View;

class Window : public Surface, public Node {
public:
  Window( short x, short y, unsigned short w, unsigned short h,
          unsigned short flags, View *view );
  virtual ~Window( void );

  virtual void Draw( void );
  void DrawBack( const Rect &rect );
  virtual void DrawBack( short x, short y, unsigned short w, unsigned short h )
               { DrawBack( Rect(x, y, w, h) ); }
  void DrawBack( void ) { DrawBack( 0, 0, w, h ); }
  void Show( void );
  void Show( const Rect &rect );
  int SetSize( unsigned short w, unsigned short h );
  int SetSize( short x, short y, unsigned short w, unsigned short h );
  int SetSize( const Rect &s ) { return SetSize(s.x, s.y, s.w, s.h); }

  void AddWidget( Widget *widget );
  Widget *RemoveWidget( Widget *widget );
  void RemoveAllWidgets( void );

  virtual GUI_Status HandleEvent( const SDL_Event &event );
  virtual GUI_Status EventLoop( void );
  virtual void VideoModeChange( void );

  View *GetView( void ) const { return view; }
  Font *SmallFont( void ) const { return sfont; }
  Font *LargeFont( void ) const { return lfont; }
  void SetSmallFont( Font *font ) { sfont = font; }
  void SetLargeFont( Font *font ) { lfont = font; }
  const Color &GetFGPen( void ) const;
  const Color &GetBGPen( void ) const;

  virtual void Close( void ) { flags |= WIN_DESTROYED; }
  bool Closed( void ) const { return (flags & WIN_DESTROYED) != 0; }

protected:
  Window( unsigned short flags, View *view );

  Widget *widgets;
  View *view;
  Font *sfont, *lfont;

  unsigned short flags;

private:
  void Init( unsigned short flags, View *view );
};

#include "view.h"

#endif  /* _INCLUDE_WINDOW_H */

