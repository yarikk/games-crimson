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
// mapwidget.h - tactical overview map display
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_MAPWIDGET_H
#define _INCLUDE_MAPWIDGET_H

#include "widget.h"
#include "map.h"

class MapWidget : public Widget {
public:
  MapWidget( short id, short x, short y, unsigned short w,
             unsigned short h, unsigned short flags,
             Window *window );

  void Draw( void );

  void SetMap( const Map *map, const Rect &viewport, unsigned char magnify );
  void SetPlayerColors( const Color &p1, const Color &p2 )
                      { player_col[0] = p1; player_col[1] = p2; }
  void SetViewPort( const Rect &viewport );
  const Point &GetLastHex( void ) const { return last_hex; }

  GUI_Status MouseDown( const SDL_MouseButtonEvent &button );
  GUI_Status KeyDown( const SDL_keysym &key );

private:
  void DrawMap( const Map *map ) const;

  Surface mapbuffer;
  unsigned char magnify;
  Point mapsize;    // map width and height
  Rect vp;
  Rect mp;
  bool draw_vp;
  Point last_hex;   // where the user last clicked
  Color player_col[2];
  Image bumper[4];
};

#endif	/* _INCLUDE_MAPWIDGET_H */

