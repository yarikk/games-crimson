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
// window.cpp
////////////////////////////////////////////////////////////////////////

#include "window.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Window::Window
// DESCRIPTION: Initialize the window.
// PARAMETERS : x     - left edge of window
//              y     - top edge of window
//              w     - window width
//              h     - window height
//              flags - window flags (see window.h for details)
//              view  - pointer to the view the window will be put on
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Window::Window( short x, short y, unsigned short w, unsigned short h,
                unsigned short flags, View *view ) {
  Init( flags, view );
  SetSize( x, y, w, h );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::Window
// DESCRIPTION: Initialize the window without giving a specific size.
//              If you use this function, you MUST call SetSize()
//              before trying to draw the window, as this function will
//              not allocate a surface for it.
// PARAMETERS : flags - window flags (see window.h for details)
//              view  - pointer to the view the window will be put on
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Window::Window( unsigned short flags, View *view ) {
  Init( flags, view );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::~Window
// DESCRIPTION: Destroy the window and release its resources.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Window::~Window( void ) {
  RemoveAllWidgets();
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::Init
// DESCRIPTION: Common initializing function for both constructors.
// PARAMETERS : flags - window flags (see window.h for details)
//              view  - pointer to the view the window will be put on
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Window::Init( unsigned short flags, View *view ) {
  widgets = NULL;
  SetSmallFont( view->SmallFont() );
  SetLargeFont( view->LargeFont() );

  this->view = view;
  this->flags = flags;
  view->AddWindow( this );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::SetSize
// DESCRIPTION: Set window size and position. These values must be
//              initialized before trying to draw the window.
// PARAMETERS : x - left edge of window
//              y - top edge of window
//              w - window width
//              h - window height
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int Window::SetSize( short x, short y, unsigned short w, unsigned short h ) {
  int err;

  this->x = x;
  this->y = y;
  err = Create( w, h, view->ScreenBPP(), 0 );       // allocate surface

  if ( !err && (flags & WIN_CENTER) ) Center( *view );
  return err;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::SetSize
// DESCRIPTION: Same as above, but set window size only and leave the
//              window position alone. If the WIN_CENTER flag is set,
//              the window will be recentered, however.
// PARAMETERS : w - window width
//              h - window height
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int Window::SetSize( unsigned short w, unsigned short h ) {
  return SetSize( x, y, w, h );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::Show
// DESCRIPTION: Copy the window surface to the display surface.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Window::Show( void ) {
  Show( Rect( 0, 0, w, h) );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::Show
// DESCRIPTION: Copy part of the window surface to the display surface.
// PARAMETERS : rect - rectangle to copy
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Window::Show( const Rect &rect ) {
  view->Refresh( Rect(x + rect.x, y + rect.y, rect.w, rect.h) );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::Draw
// DESCRIPTION: Draw the window surface into the internal buffer. To
//              actually display the window, call Window::Show()
//              afterwards.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Window::Draw( void ) {
  DrawBack();

  Widget *wd = widgets;
  while ( wd ) {
    if ( !wd->Hidden() ) wd->Draw();
    wd = wd->next;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::DrawBack
// DESCRIPTION: Draw the window background into the internal buffer.
// PARAMETERS : rect - area to paint
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Window::DrawBack( const Rect &rect ) {
  const static Image bg( view->GetSystemIcons(), 0, 34, 30, 30 );
  Rect clip;
  FillPattern( rect, bg, 0, 0 );
  GetClipRect( clip );
  SetClipRect( rect );
  DrawBox( Rect(0, 0, w, h), BOX_RAISED );
  SetClipRect( clip );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::AddWidget
// DESCRIPTION: Attach a widget to the window. This function will set
//              the widget surface and font.
// PARAMETERS : widget - widget to add
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Window::AddWidget( Widget *widget ) {
  widget->SetFont( sfont );

  if ( !widget->Composite() ) {
    widget->next = widgets;
    widgets = widget;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::RemoveWidget
// DESCRIPTION: Remove a widget from the window.
// PARAMETERS : widget - widget to remove
// RETURNS    : pointer to the widget removed
////////////////////////////////////////////////////////////////////////

Widget *Window::RemoveWidget( Widget *widget ) {
  Widget *wd = widgets, **wdptr = &widgets;

  while ( wd && wd != widget ) {
    wdptr = &wd->next;
    wd = wd->next;
  }

  if ( wd ) *wdptr = wd->next;
  return wd;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::RemoveAllWidgets
// DESCRIPTION: Remove all widgets from the window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Window::RemoveAllWidgets( void ) {
  Widget *wd = widgets;

  while ( wd ) {
    Widget *wd2 = wd->next;
    delete wd;
    wd = wd2;
  }
  widgets = NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::HandleEvent
// DESCRIPTION: Distribute system events to the window widgets.
// PARAMETERS : event - event received by the event handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status Window::HandleEvent( const SDL_Event &event ) {

  if ( ((flags & WIN_CLOSE_ESC) && (event.type == SDL_KEYDOWN) &&
        (event.key.keysym.sym == SDLK_ESCAPE)) ||
       ((flags & WIN_CLOSE_UNFOCUS) && (event.type == SDL_MOUSEBUTTONDOWN) &&
        !Contains( event.button.x, event.button.y )) ) return GUI_CLOSE;

  GUI_Status rc = GUI_OK;
  Widget *wd = widgets;
  while ( wd ) {
    if ( !wd->Hidden() && !wd->Disabled() ) {
      GUI_Status wrc = wd->HandleEvent( event );
      if ( wrc != GUI_OK ) rc = wrc;
    }
    wd = wd->next;
  }

  return rc;
} 

////////////////////////////////////////////////////////////////////////
// NAME       : Window::EventLoop
// DESCRIPTION: Wait for events and distribute them to the widgets.
//              This function does not return until either an SDL_QUIT
//              event is received or a widget returns a value other than
//              GUI_OK.
// PARAMETERS : -
// RETURNS    : GUI status
//
// NOTE       : You MUST make sure that the return value of this
//              function is handed back to the main event loop.
//              Otherwise things like the user trying to close the
//              program window will get lost! If the function returns
//              GUI_QUIT, GUI_RESTART, or GUI_ERROR do not do any
//              further processing.
//              DEPRECATED. Don't use this if you don't have to.
//              Event propagation doesn't work the way it should.
////////////////////////////////////////////////////////////////////////

GUI_Status Window::EventLoop( void ) {
  SDL_Event event;
  GUI_Status rc;

  do {
    rc = view->FetchEvent( event );

    if ( (rc != GUI_QUIT) && (rc != GUI_ERROR) )
      rc = HandleEvent( event );
  } while ( rc == GUI_OK );

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::VideoModeChange
// DESCRIPTION: This method is called by the view whenever the video
//              resolution changes. The window can then adjust itself
//              to the new dimensions. Afterwards a View::Refresh() is
//              performed, i. e. the window just needs to reblit its
//              contents to its internal buffer, and NOT call a view
//              update itself.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Window::VideoModeChange( void ) {
  Rect pos = *this;

  pos.Align( *view );
  pos.Clip( *view );
  if ( flags & WIN_CENTER ) pos.Center( *view );

  if ( (pos.w != w) || (pos.h != h) ) {
    SetSize( pos.x, pos.y, pos.w, pos.h );
    Draw();
  } else {
    x = pos.x;
    y = pos.y;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::GetFGPen
// DESCRIPTION: Get foreground color.
// PARAMETERS : -
// RETURNS    : foreground color
////////////////////////////////////////////////////////////////////////

const Color &Window::GetFGPen( void ) const {
  return view->GetFGPen();
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::GetBGPen
// DESCRIPTION: Get background color.
// PARAMETERS : -
// RETURNS    : background color
////////////////////////////////////////////////////////////////////////

const Color &Window::GetBGPen( void ) const {
  return view->GetBGPen();
}

