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
// view.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_VIEW_H
#define _INCLUDE_VIEW_H

#include "SDL.h"
#include "window.h"

typedef GUI_Status (*GUIEventFilter)( SDL_Event &event, Window *window );

class View : public Surface {
public:
  View( unsigned short w, unsigned short h, short bpp, unsigned long flags );
  ~View( void );

  void SetVideoMode( unsigned short w, unsigned short h, short bpp,
                     unsigned long flags );
  void Update( void );
  void Update( const Rect &rect );
  void Refresh( void );
  void Refresh( const Rect &refresh );
  void AddWindow( Window *window );
  void SelectWindow( Window *window );
  Window *CloseWindow( Window *window );
  void CloseAllWindows( void );

  void DisableUpdates( void ) { allow_updates = false; }
  void EnableUpdates( void ) { allow_updates = true; }

  GUI_Status HandleEvents( void );
  GUI_Status FetchEvent( SDL_Event &event );
  GUI_Status PeekEvent( SDL_Event &event );
  void SetEventFilter( GUIEventFilter efilter ) { filter = efilter; }
  int ToggleFullScreen( void );
  bool IsFullScreen( void ) const { return (s_surface->flags & SDL_FULLSCREEN) != 0; }
  unsigned char ScreenBPP( void ) const { return s_surface->format->BitsPerPixel; }

  void SetSmallFont( Font *font ) { sfont = font; }
  void SetLargeFont( Font *font ) { lfont = font; }
  Font *SmallFont( void ) const { return sfont; }
  Font *LargeFont( void ) const { return lfont; }
  void SetSystemIcons( Surface *icons ) { delete sys_icons; sys_icons = icons; }
  Surface *GetSystemIcons( void ) const { return sys_icons; }
  void SetFGPen( const Color &fg ) { colors[0] = fg; }
  void SetBGPen( const Color &bg ) { colors[1] = bg; }
  const Color &GetFGPen( void ) const { return colors[0]; }
  const Color &GetBGPen( void ) const { return colors[1]; }

private:
  GUI_Status SystemFilter( const SDL_Event &event );

  List windows;
  Font *sfont, *lfont;  // small and large fonts
  bool allow_updates;

  Color colors[2];
  Surface *sys_icons;

  GUIEventFilter filter;
};

#endif	/* _INCLUDE_VIEW_H */

