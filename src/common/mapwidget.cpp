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
// mapwidget.cpp
////////////////////////////////////////////////////////////////////////

#include "mapwidget.h"
#include "globals.h"

////////////////////////////////////////////////////////////////////////
// NAME       : MapWidget::MapWidget
// DESCRIPTION: Create a new map widget. This widget displays a small
//              image of the current map and allows the player to move
//              the viewport around in which case any listeners are
//              notified.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              flags  - widget flags (see widget.h for details)
//              window - window to add this widget to
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

MapWidget::MapWidget( short id, short x, short y, unsigned short w,
           unsigned short h, unsigned short flags, Window *window ) :
   Widget( id, x, y, w, h, flags, NULL, window ), magnify(0), draw_vp(false) {
  Surface *icons = window->GetView()->GetSystemIcons();
  bumper[0] = Image( icons, 157, 46, 11, 7 ); // up
  bumper[1] = Image( icons, 157, 53, 11, 7 ); // down
  bumper[2] = Image( icons, 168, 46, 7, 11 ); // left
  bumper[3] = Image( icons, 175, 46, 7, 11 ); // right
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWidget::Draw
// DESCRIPTION: Draw the widget to the display surface.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapWidget::Draw( void ) {
  surface->DrawBox( *this, BOX_RECESSED );

  if ( magnify != 0 ) {
    // blit mapbuffer to window surface
    mapbuffer.Blit( surface, mp, x + 1, y + 1 );

    if ( draw_vp ) {
      // darken the entire map, then light up the visible part
      surface->FillRectAlpha( Rect(x+1,y+1,w-2,h-2), Color(CF_COLOR_BLACK), 96 );
      mapbuffer.Blit( surface, vp, x + 1 + (vp.x - mp.x),
                                   y + 1 + (vp.y - mp.y) );
    }

    if ( mp.x > 0 )
      bumper[2].Draw( surface, x + 3, y + (h - bumper[2].Height()) / 2 );
    if ( mp.x + mp.w < mapbuffer.w )
      bumper[3].Draw( surface, x + w - 3 - bumper[3].Width(), y + (h - bumper[3].Height()) / 2 );

    if ( mp.y > 0 )
      bumper[0].Draw( surface, x + (w - bumper[0].Width()) / 2, y + 3 );
    if ( mp.y + mp.h < mapbuffer.h )
      bumper[1].Draw( surface, x + (w - bumper[1].Width()) / 2, y + h - 3 - bumper[1].Height() );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWidget::SetMap
// DESCRIPTION: Assign a map to be displayed to the widget.
// PARAMETERS : map      - map to display (may be NULL)
//              viewport - visible part of the map in hexes. A box will
//                         be drawn around this area if the viewport is
//                         smaller than the map.
//              magnify  - magnification to use for the small map (> 0)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapWidget::SetMap( const Map *map, const Rect &viewport, unsigned char magnify ) {
  this->magnify = 0;

  if ( map ) {
    mapsize.x = map->Width();
    mapsize.y = map->Height();

    if ( !mapbuffer.Create( map->Width() * magnify,
                            map->Height() * magnify + magnify/2,
                            DISPLAY_BPP, 0 ) ) {
      this->magnify = magnify;
      SetViewPort( viewport );
      DrawMap( map );
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWidget::SetViewPort
// DESCRIPTION: Set the visible area. The widget will draw a box around
//              it if it is smaller than the widget.
// PARAMETERS : viewport - visible part of the map in hexes
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapWidget::SetViewPort( const Rect &viewport ) {
  if ( magnify > 0 ) {
    vp.x = viewport.x * magnify;
    vp.y = viewport.y * magnify;
    vp.w = viewport.w * magnify;
    vp.h = viewport.h * magnify + magnify/2;

    mp.x = mp.y = 0;
    mp.w = MIN( w - 2, mapsize.x * magnify );
    mp.h = MIN( h - 2, mapsize.y * magnify + magnify/2 );

    draw_vp = (mp >= vp);

    vp.w = MIN( vp.w, mp.w );
    vp.h = MIN( vp.h, mp.h );

    if ( draw_vp ) {
      mp.Center( vp );
      mp.Align( mapbuffer );
    }

    last_hex.x = viewport.x + viewport.w/2;
    last_hex.y = viewport.y + viewport.h/2;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWidget::DrawMap
// DESCRIPTION: Draw the map into the internal buffer. From there it can
//              be blitted as often as needed.
// PARAMETERS : map - map to draw
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapWidget::DrawMap( const Map *map ) const {
  Color color;
  unsigned short index = 0;
  short yoff;

  mapbuffer.Flood( Color(CF_COLOR_BACKGROUND) );

  for ( int py = 0; py < map->Height(); ++py ) {
    for ( int px = 0; px < map->Width(); ++px ) {
      if ( px & 1 ) yoff = magnify / 2;
      else yoff = 0;

      MapObject *obj = map->GetMapObject( Point(px, py) );
      if ( obj && obj->Owner() ) color = player_col[obj->Owner()->ID()];
      else color = Color( map->HexColor( index ) );
      ++index;
      mapbuffer.FillRect(
                magnify * px, magnify * py + yoff,
                magnify, magnify, color );
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWidget::MouseDown
// DESCRIPTION: Report the hex the user clicked on to the hook and let
//              it decide what to do next (probably update the view
//              port).
// PARAMETERS : button - SDL_MouseButtonEvent received from the event
//                       handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status MapWidget::MouseDown( const SDL_MouseButtonEvent &button ) {
  if ( magnify > 0 ) {
    short mx = button.x - surface->LeftEdge();
    short my = button.y - surface->TopEdge();

    Rect mdsp( x + 1, y + 1, w - 2, h - 2 );

    if ( mdsp.Contains( mx, my ) ) {
      if ( button.button == SDL_BUTTON_LEFT ) {
        last_hex.x = (mx - LeftEdge() - 1 + mp.x) / magnify;
        last_hex.y = (my - TopEdge() - 1 -
                     (last_hex.x & 1 ? magnify/2 : 0) + mp.y) / magnify;
        if ( last_hex.y == mapsize.y ) --last_hex.y;

        if ( hook ) hook->WidgetActivated( this, surface );
      }
    }
  }
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWidget::KeyDown
// DESCRIPTION: If WIDGET_[HV]SCROLLKEY were set for the widget, you can
//              move the view port via the cursor keys.
// PARAMETERS : key - SDL_keysym received from the event handler
// RETURNS    : GUI_OK
////////////////////////////////////////////////////////////////////////

GUI_Status MapWidget::KeyDown( const SDL_keysym &key ) {
  if ( draw_vp ) {
    bool call_hook = false;

    switch ( key.sym ) {
    case SDLK_LEFT:
      if ( flags & WIDGET_HSCROLLKEY ) {
        // make sure we always scroll an even number of columns
        // to avoid strong vertical drift
        last_hex.x -= (vp.w / magnify / 2) & 0xFFFE;
        call_hook = true;
      }
      break;
    case SDLK_RIGHT:
      if ( flags & WIDGET_HSCROLLKEY ) {
        last_hex.x += (vp.w / magnify / 2) & 0xFFFE;
        call_hook = true;
      }
      break;
    case SDLK_UP:
      if ( flags & WIDGET_VSCROLLKEY ) {
        last_hex.y -= (vp.h / magnify / 2);
        call_hook = true;
      }
      break;
    case SDLK_DOWN:
      if ( flags & WIDGET_VSCROLLKEY ) {
        last_hex.y += (vp.h / magnify / 2);
        call_hook = true;
      }
      break;
    default:
      break;
    }

    if ( call_hook ) {
      if ( last_hex.x < 0 ) last_hex.x = 0;
      else if ( last_hex.x >= mapsize.x ) last_hex.x = mapsize.x - 1;

      if ( last_hex.y < 0 ) last_hex.y = 0;
      else if ( last_hex.y >= mapsize.y ) last_hex.y = mapsize.y - 1;

      if ( hook ) hook->WidgetActivated( this, surface );
    }
  }
  return GUI_OK;
}

