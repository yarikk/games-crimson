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
// view.cpp
////////////////////////////////////////////////////////////////////////

#include "view.h"

////////////////////////////////////////////////////////////////////////
// NAME       : View::View
// DESCRIPTION: Initialize the view structure. It will also create a
//              display surface.
// PARAMETERS : w     - display surface width
//              h     - display surface height
//              bpp   - display depth
//              flags - display surface flags (see SDL/SDL.h)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

View::View( unsigned short w, unsigned short h, short bpp,
            unsigned long flags ) {
  sfont = lfont = NULL;
  sys_icons = NULL;
  s_surface = NULL;

  SetVideoMode( w, h, bpp, flags );
  if ( !s_surface ) return;

  x = y = 0;
  allow_updates = true;
  filter = NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::~View
// DESCRIPTION: Destroy the view and associated data structures.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

View::~View( void ) {
  CloseAllWindows();
  delete sfont;
  delete lfont;
  delete sys_icons;
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::SetVideoMode
// DESCRIPTION: Set the view's video mode. If the view already has a
//              video surface (i.e. the resultion is changed) all
//              windows are adjusted to the new size.
// PARAMETERS : w     - resolution width
//              h     - resolution height
//              bpp   - color depth
//              flags - video flags
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void View::SetVideoMode( unsigned short w, unsigned short h, short bpp,
                         unsigned long flags ) {
  bool change = (s_surface != NULL);

  if ( change ) SDL_FreeSurface( s_surface );

  s_surface = SDL_SetVideoMode( w, h, bpp, flags );

  if ( s_surface ) {
    this->w = s_surface->w;
    this->h = s_surface->h;

    if ( change ) {
      // adjust window positions and sizes
      Window *win = static_cast<Window *>( windows.Tail() );

      while ( win ) {
        win->VideoModeChange();
        win = static_cast<Window *>( win->Prev() );
      }

      Refresh();
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::Update
// DESCRIPTION: Update the display surface without reblitting the
//              particular window surfaces.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void View::Update( void ) {
  Update( *this );
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::Update
// DESCRIPTION: Update part of the display surface without reblitting
//              the particular window surfaces.
// PARAMETERS : rect - area to be updated
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void View::Update( const Rect &rect ) {
  if ( allow_updates )
    SDL_UpdateRect( s_surface, rect.x, rect.y, rect.w, rect.h );
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::Refresh
// DESCRIPTION: Update the entire display surface.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void View::Refresh( void ) {
  Window *win = static_cast<Window *>( windows.Tail() );

  Flood( Color(CF_COLOR_WHITE) );
  while ( win ) {
    if ( !win->Closed() ) {
      win->Blit( this, Rect( 0, 0, win->Width(), win->Height() ),
                 win->LeftEdge(), win->TopEdge() );
    }
    win = static_cast<Window *>( win->Prev() );
  }

  // now update the display
  Update();
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::Refresh
// DESCRIPTION: Update parts of the display surface only.
// PARAMETERS : refresh - area to refresh
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void View::Refresh( const Rect &refresh ) {
  Window *window = static_cast<Window *>( windows.Tail() );
  Rect v( refresh );
  v.Clip( *this );

  FillRect( v, Color(CF_COLOR_WHITE) );
  while ( window ) {
    if ( !window->Closed() ) {
      // clip the window to the refresh area
      Rect win = *window;
      Rect src( 0, 0, window->Width(), window->Height() );
      win.ClipBlit( src, v );
      if ( win.Width() && win.Height() )
        window->Blit( this, src, win.LeftEdge(), win.TopEdge() );
    }
    window = static_cast<Window *>( window->Prev() );
  }
  Update( v );
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::AddWindow
// DESCRIPTION: Attach a window to the view. The window will then
//              receive event messages and can be shown. This
//              function does not automatically update the display.
//              Call Window::Refresh to do so manually.
// PARAMETERS : window - window to add
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void View::AddWindow( Window *window ) {
  windows.AddHead( window );
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::CloseWindow
// DESCRIPTION: Close a window. If there are other windows left, update
//              the display.
//              This function only marks the window closed. The actual
//              cleaning up is done in the event handling function.
// PARAMETERS : window - window to close
// RETURNS    : active window after closing, NULL if no windows left
////////////////////////////////////////////////////////////////////////

Window *View::CloseWindow( Window *window ) {
  Window *focus;

  window->Remove();                     // remove from windows list

  focus = static_cast<Window *>( windows.Head() );
  if ( !windows.IsEmpty() ) Refresh( *window );

  window->Close();
  windows.AddTail( window );            // put it back to the end of the list

  return focus;
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::SelectWindow
// DESCRIPTION: Redirect all incoming event messages to another window.
// PARAMETERS : window - window to receive event messages
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void View::SelectWindow( Window *window ) {
  if ( window != static_cast<Window *>( windows.Head() ) ) {
    window->Remove();
    windows.AddHead( window );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Window::CloseAllWindows
// DESCRIPTION: Close all windows. This function only marks the window
//              to be closed. The actual cleaning is done in the event
//              handling function.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void View::CloseAllWindows( void ) {
  for ( Window *w = static_cast<Window *>(windows.Head());
        w && !w->Closed(); w = static_cast<Window *>(w->Next()) )
    w->Close();
  Refresh();
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::HandleEvents
// DESCRIPTION: Distribute event messages to the windows and react
//              appropriately.
// PARAMETERS : -
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status View::HandleEvents( void ) {
  SDL_Event event;
  GUI_Status rc = FetchEvent( event );

  if ( (rc != GUI_ERROR) && (rc != GUI_QUIT) && (rc != GUI_NONE) ) {
    Window *win = static_cast<Window *>( windows.Head() );
    if ( win ) {
      rc = win->HandleEvent( event );
      if ( rc == GUI_CLOSE ) win = CloseWindow( win );
    }

    // collect destroyed windows
    while ( (win = static_cast<Window *>(windows.Tail())) && win->Closed() ) {
      win->Remove();
      delete win;
    }
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::FetchEvent
// DESCRIPTION: Get the next event from the event queue.
// PARAMETERS : event - buffer to hold the event information
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status View::FetchEvent( SDL_Event &event ) {
  GUI_Status rc;

  do {
    if ( SDL_WaitEvent( &event ) ) {

      // try to aggregate several mouse motion events to prevent getting flooded
      if ( event.type == SDL_MOUSEMOTION ) {
        SDL_Event next;
        Uint32 start = SDL_GetTicks();

        do {
          int got = SDL_PeepEvents( &next, 1, SDL_GETEVENT, SDL_MOUSEMOTIONMASK );

          if ( got > 0 ) {
            event.motion.x = next.motion.x;
            event.motion.y = next.motion.y;
            event.motion.xrel += next.motion.xrel;
            event.motion.yrel += next.motion.yrel;
          } else break;

        } while ( SDL_GetTicks() - start < 10 );
      }

      rc = SystemFilter( event );

      if ( rc != GUI_NONE ) {
        if ( filter ) {
          rc = filter( event, static_cast<Window *>(windows.Head()) );
        } else rc = GUI_OK;
      }
    } else rc = GUI_ERROR;
  } while ( rc == GUI_NONE );

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::PeekEvent
// DESCRIPTION: Get the next event from the event queue if there are any
//              events pending. If there are no events waiting, return
//              immediately.
// PARAMETERS : event - buffer to hold the event information
// RETURNS    : GUI status; GUI_NONE is returned if no events waiting
////////////////////////////////////////////////////////////////////////

GUI_Status View::PeekEvent( SDL_Event &event ) {
  GUI_Status rc = GUI_NONE;

  while ( (rc == GUI_NONE) && SDL_PollEvent( &event ) ) {
    rc = SystemFilter( event );

    if ( rc != GUI_NONE ) {
      if ( filter ) {
        rc = filter( event, static_cast<Window *>(windows.Head()) );
      } else rc = GUI_OK;
    }
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::ToggleFullScreen
// DESCRIPTION: Switch from windowed to fullscreen mode or vice versa.
// PARAMETERS : -
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int View::ToggleFullScreen( void ) {
  return !SDL_WM_ToggleFullScreen( s_surface );
}

////////////////////////////////////////////////////////////////////////
// NAME       : View::SystemFilter
// DESCRIPTION: This is the global system event filter. All events
//              received by the event handler are first run through
//              this filter, afterwards through the user filter (if
//              present), and only then passed on to the windows. The
//              system filter is the only one allowed to modify event
//              information.
// PARAMETERS : event - event to be checked
// RETURNS    : GUI_NONE to prevent this event from being processed by
//              anyone. For any other return code the event is
//              distributed as usual.
////////////////////////////////////////////////////////////////////////

GUI_Status View::SystemFilter( const SDL_Event &event ) {
  GUI_Status rc;

  // <Alt><Return> toggles fullscreen
  if ( (event.type == SDL_KEYDOWN) &&
       (event.key.keysym.sym == SDLK_RETURN) &&
       (event.key.keysym.mod & KMOD_ALT) != 0) {
    ToggleFullScreen();
    rc = GUI_NONE;
  } else rc = GUI_OK;

  return rc;
}

