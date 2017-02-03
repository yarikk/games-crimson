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
// mapview.cpp
////////////////////////////////////////////////////////////////////////

#include "mapview.h"

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::MapView
// DESCRIPTION: Partly set up a map viewport. To be usable, SetMap() and
//              Enable() must be called afterwards.
// PARAMETERS : display - surface on which to display the map
//              bounds  - viewport position and size on surface
//              flags   - viewport flags (see mapview.h for details)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

MapView::MapView( Surface *display, const Rect &bounds, unsigned short flags ) :
         Rect( bounds ) {
  surface = display;

  map = NULL;
  shader_map = NULL;

  SetFlags( flags );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::InitOffsets
// DESCRIPTION: Set offsets for map display and scrolling. Call after
//              having set a new map for the viewport.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapView::InitOffsets( void ) {
  maxx = MAX( 0, map->Width() * (TileWidth() - TileShiftX()) + TileShiftX() - w );
  maxy = MAX( 0, map->Height() * TileHeight() + TileShiftY() - h );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::Resize
// DESCRIPTION: Set a new viewport size and position.
// PARAMETERS : bounds - new viewport size and position
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapView::Resize( const Rect &bounds ) {
  x = bounds.x;
  y = bounds.y;
  w = bounds.w;
  h = bounds.h;
  InitOffsets();

  if ( curx > maxx ) curx = maxx;
  if ( cury > maxy ) cury = maxy;

  if ( HexVisible( cursor ) ) Draw();
  else CenterOnHex( cursor );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::SetMap
// DESCRIPTION: Assign another map to this viewport.
// PARAMETERS : map - map to display; the map must have already had
//                    a level set assigned
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapView::SetMap( Map *map ) {
  if ( shader_map ) delete [] shader_map;

  cursor = Point( -1, -1 );
  cursor_image = IMG_CURSOR_IDLE;
  this->map = map;

  InitOffsets();
  shader_map = new signed char [map->Width() * map->Height()];
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::Enable
// DESCRIPTION: Set map offsets back to 0 and enable map display.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapView::Enable( void ) {
  curx = cury = 0;
  UnsetFlags( MV_DISABLE );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::Disable
// DESCRIPTION: Hide the actual map and display just blackness.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapView::Disable( void ) {
  SetFlags( MV_DISABLE );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::Draw
// DESCRIPTION: Draw a part of the map onto the window surface.
// PARAMETERS : x - of the viewport (!) area to update
//              y - of the viewport area to update
//              w - width of the update area
//              h - height of the update area
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapView::Draw( short x, short y, unsigned short w, unsigned short h ) const {
  surface->FillRect( *this, Color(CF_COLOR_SHADOW) );
  if ( Enabled() ) {
    DrawMap( curx + x, cury + y, w, h, surface, x + this->x, y + this->y );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::DrawMap
// DESCRIPTION: Draw a part of the map onto a surface.
// PARAMETERS : x    - leftmost pixel of the map (!) to paint
//              y    - topmost pixel to paint
//              w    - width of the map part to draw
//              h    - height of the map part
//              dest - destination surface
//              dx   - where to start drawing on the surface
//              dy   - where to start drawing vertically
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapView::DrawMap( short x, short y, unsigned short w,
              unsigned short h, Surface *dest, short dx, short dy ) const {
  int hx1 = MinXHex( x );
  int hy1 = MinYHex( y );
  int hx2 = MaxXHex( x, w );
  int hy2 = MaxYHex( y, h );

  Rect clip( dx, dy, w, h );
  int sx, sy, tx, ty, yoff;
  Point hex;

  for ( tx = hx1; tx <= hx2; ++tx ) {
    sx = tx * (TileWidth() - TileShiftX()) - x + this->x;

    if ( tx & 1 ) yoff = TileShiftY() - y;
    else yoff = -y;
    yoff += this->y;

    for ( ty = hy1; ty <= hy2; ++ty ) {
      sy = ty * TileHeight() + yoff;
      hex = Point( tx, ty );

      // draw the terrain image
      DrawTerrain( map->HexImage( hex ), dest, sx, sy, clip );

      // draw unit
      if ( Unit *u = map->GetUnit( hex ) ) {
        DrawUnit( u->Image(), dest, sx, sy, clip );
        if ( !u->IsReady() ) DrawTerrain( IMG_NOT_AVAILABLE, dest, sx, sy, clip );

        // draw unit health
        if ( UnitStatsEnabled() )
          DrawUnitHealth( u->GroupSize(), dest, sx, sy, clip );
      }

      // draw fog
      if ( FogEnabled() && (shader_map[map->Hex2Index(hex)] == -1) )
        DrawFog( dest, sx, sy, clip );

      // draw cursor
      if ( CursorEnabled() && (hex == cursor) )
        DrawTerrain( cursor_image, dest, sx, sy, clip );
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::Pixel2Hex
// DESCRIPTION: Convert viewport coordinates to hex coordinates.
// PARAMETERS : px  - pixel x relative to viewport border
//              py  - pixel y relative to viewport border
//              hex - buffer to hold the resulting hex
// RETURNS    : 0 on success, -1 on error (invalid pixels);
//              hex will contain -1, -1 then
////////////////////////////////////////////////////////////////////////

int MapView::Pixel2Hex( short px, short py, Point &hex ) const {

  if ( Contains( px, py ) ) {
    short hx = px + curx - x, hy = py + cury - y;

    hex.x = hx / (TileWidth() - TileShiftX());
    hex.y = (hy + (hex.x & 1) * TileShiftY()) / TileHeight() - (hex.x & 1);

    // calculate pixel position relative to selected hex
    hx %= TileWidth() - TileShiftX();
    hy -= hex.y * TileHeight() + (hex.x & 1) * TileShiftY();

    const Surface &mask = map->GetTerrainSet()->HexMask();
    if ( mask.GetPixel( hx, hy ) == mask.GetColorKey() ) {
      if ( hx < (TileWidth() / 2) ) --hex.x;
      else ++hex.x;

      if ( hy < (TileHeight() / 2) ) {
        if ( hex.x & 1 ) --hex.y; // odd columns
      } else {
        if ( !(hex.x & 1) ) ++hex.y; // even columns
      }
    }

    if ( map->Contains( hex ) ) return 0;
  }

  hex = Point( -1, -1 );
  return -1;
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::Hex2Pixel
// DESCRIPTION: Get the pixel coordinates (top left edge) of a hex
//              relative to the display surface offsets.
// PARAMETERS : hex - hex coordinates
// RETURNS    : Point containing the position of the hex in pixels
////////////////////////////////////////////////////////////////////////

Point MapView::Hex2Pixel( const Point &hex ) const {
  return Point( hex.x * (TileWidth() - TileShiftX()) - curx + x,
                hex.y * TileHeight() + (hex.x & 1) * TileShiftY() - cury + y );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::HexVisible
// DESCRIPTION: Check whether a given hex is currently visible in the
//              map viewport area.
// PARAMETERS : hex - hex position
// RETURNS    : true if hex (or part of it) on screen, false otherwise
////////////////////////////////////////////////////////////////////////

bool MapView::HexVisible( const Point &hex ) const {
  Point p = Hex2Pixel( hex );  // get absolute pixel values

  return( (p.x >= x) && (p.x <= x + w - TileWidth()) &&
          (p.y >= y) && (p.y <= y + h - TileHeight()) );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::UpdateHex
// DESCRIPTION: Redraw a single hex.
// PARAMETERS : hex - hex to update
// RETURNS    : Rect describing the surface area that has been updated
//              and needs to be refreshed
////////////////////////////////////////////////////////////////////////

Rect MapView::UpdateHex( const Point &hex ) {
  if ( !Enabled() ) return Rect(0,0,0,0);

  Point p = Hex2Pixel( hex );  // get absolute position

  Rect clip( p.x, p.y, TileWidth(), TileHeight() );   // create clip rect
  clip.Clip( *this );

  // draw the terrain image
  DrawTerrain( map->HexImage( hex ), surface, p.x, p.y, clip );

  // draw unit
  if ( Unit *u = map->GetUnit( hex ) ) {
    DrawUnit( u->Image(), surface, p.x, p.y, clip );
    if ( !u->IsReady() ) DrawTerrain( IMG_NOT_AVAILABLE, surface, p.x, p.y, clip );

    // draw unit health
    if ( UnitStatsEnabled() )
      DrawUnitHealth( u->GroupSize(), surface, p.x, p.y, clip );
  }

  // draw fog
  if ( FogEnabled() && (shader_map[map->Hex2Index(hex)] == -1) )
    DrawFog( surface, p.x, p.y, clip );

  // draw cursor
  if ( CursorEnabled() && (hex == cursor) )
    DrawTerrain( cursor_image, surface, p.x, p.y, clip );

  return clip;
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::DrawUnitHealth
// DESCRIPTION: Draw a unit health bar.
// PARAMETERS : health - unit health indicator
//              dest   - destination surface
//              px     - horizontal offset on surface
//              py     - vertical offset on surface
//              clip   - clipping rectangle
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapView::DrawUnitHealth( unsigned char health, Surface *dest,
              short px, short py, const Rect &clip ) const {
  Color hcol;
  if ( health >= 5 ) hcol = Color(0x62,0xAA,0x46);      // green
  else if ( health >= 3 ) hcol = Color(0xFE,0xAA,0x04); // yellow
  else hcol = Color(0xFE,0x26,0x26);                    // red

  unsigned short hp = (TileWidth() - 2 * TileShiftX() - 4) /
                      (MAX_GROUP_SIZE + 2);
  unsigned short barw = hp * MAX_GROUP_SIZE + 2;

  Rect outer( px + (TileWidth() - barw) / 2,
              py + TileHeight() - hp - 4, barw, hp + 3 );
  Rect inner( outer.x + 1, outer.y + 1, health * hp, hp + 1 );

  outer.Clip( clip );
  inner.Clip( clip );

  dest->FillRect( outer, Color(CF_COLOR_BLACK) );
  dest->FillRect( inner, hcol );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::CenterOnHex
// DESCRIPTION: Center the display on a given hex if possible.
// PARAMETERS : hex - hex position
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapView::CenterOnHex( const Point &hex ) {
  Point p = Hex2Pixel( hex );

  short off = p.x + (TileWidth() - w) / 2 + curx;
  if ( off > maxx ) curx = maxx;
  else if ( off < 0 ) curx = 0;
  else curx = off;

  off = p.y + (TileHeight() - h) / 2 + cury;
  if ( off > maxy ) cury = maxy;
  else if ( off < 0 ) cury = 0;
  else cury = off;
  Draw();
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::MinXHex
// DESCRIPTION: Get the leftmost visible hex number.
// PARAMETERS : x - leftmost pixel. If a value of -1 is given, use the
//                  current viewport settings (curx) to determine the
//                  hex.
// RETURNS    : leftmost visible hex column
////////////////////////////////////////////////////////////////////////

unsigned short MapView::MinXHex( short x ) const {
  if ( x == -1 ) x = curx;
  return MAX( (x - TileShiftX()) / (TileWidth() - TileShiftX()), 0 );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::MinYHex
// DESCRIPTION: Get the topmost visible hex number.
// PARAMETERS : y - topmost pixel. If a value of -1 is given, use the
//                  current viewport settings (cury) to determine the
//                  hex.
// RETURNS    : topmost visible hex row
////////////////////////////////////////////////////////////////////////

unsigned short MapView::MinYHex( short y ) const {
  if ( y == -1 ) y = cury;
  return MAX( (y - TileShiftY()) / TileHeight(), 0 );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::MaxXHex
// DESCRIPTION: Get the rightmost visible hex number.
// PARAMETERS : x - leftmost pixel. If a value of -1 is given, use the
//                  current viewport settings (curx/MapView::Width())
//                  to determine the hex.
//              w - display width
// RETURNS    : rightmost visible hex column
////////////////////////////////////////////////////////////////////////

unsigned short MapView::MaxXHex( short x, unsigned short w ) const {
  if ( x == -1 ) {
    x = curx;
    w = Width();
  }
  return MIN( (x + w) / (TileWidth() - TileShiftX()), map->Width() - 1 );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::MaxYHex
// DESCRIPTION: Get the lowest visible hex number.
// PARAMETERS : y - topmost pixel. If a value of -1 is given, use the
//                  current viewport settings (cury/MapView::Width())
//                  to determine the hex.
//              h - display height
// RETURNS    : lowest visible hex row
////////////////////////////////////////////////////////////////////////

unsigned short MapView::MaxYHex( short y, unsigned short h ) const {
  if ( y == -1 ) {
    y = cury;
    h = Height();
  }
  return MIN( (y + h) / TileHeight(), map->Height() - 1 );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::CheckScroll
// DESCRIPTION: Look at the current cursor position and map offsets and
//              decide whether we need to scroll the display. If so, do
//              it.
// PARAMETERS : -
// RETURNS    : TRUE if display was scrolled, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool MapView::CheckScroll( void ) {
  if ( cursor.x == -1 ) return false;

  Point p = Hex2Pixel( cursor );

  if ( (curx > 0) && (p.x <= x + TileWidth()) ) p.x = -w / 2;
  else if ( (curx < maxx) && (w + x - p.x - TileWidth() <= TileWidth()) )
    p.x = w / 2;
  else p.x = 0;

  if ( (cury > 0) && (p.y <= y + TileHeight()) ) p.y = -h / 2;
  else if ( (cury < maxy) && (h + y - p.y - TileHeight() <= TileHeight()) )
    p.y = h / 2;
  else p.y = 0;

  if ( p.x || p.y ) {
    Scroll( p.x, p.y );
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::Scroll
// DESCRIPTION: Scroll the currently visible map area.
// PARAMETERS : px - pixels to scroll horizontally
//              py - pixels to scroll vertically
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapView::Scroll( short px, short py ) {
  if ( curx + px < 0 ) px = -curx;
  else if ( curx + px > maxx ) px = maxx - curx;
  curx += px;

  if ( cury + py < 0 ) py = -cury;
  else if ( cury + py > maxy ) py = maxy - cury;
  cury += py;

#ifdef CF_SDL_LOCAL_BLIT
  // right now SDL cannot blit parts of a surface to another
  // place on the same surface if both areas overlap

  // calculate dirty rectangles
  Rect d1( 0, 0, 0, 0 );  // dirty 1
  Rect d2( 0, 0, 0, 0 );  // dirty 2
  Rect c( 0, 0, w, h );   // copy, still valid

  if ( px > 0 ) {
    d1 = Rect( w - px, 0, px, h );
    c.x = px;
    c.w = w - px;
  } else if ( px < 0 ) {
    d1 = Rect( 0, 0, -px, h );
    c.w = w + px;
  }

  if ( py > 0 ) {
    d2 = Rect( 0, h - py, w, py );
    c.y = py;
    c.h = h - py;
  } else if ( py < 0 ) {
    d2 = Rect( 0, 0, w, -py );
    c.h = h + py;
  }

  // eliminate overlapping parts
  if ( (d1.w > 0) && (d2.w > 0) ) {
    if ( d1.x == d2.x ) d2.x += d1.w;
    d2.w = MAX( 0, d2.w - d1.w );
  }

  // copy valid part to current position
  if ( c.w && c.h )
    Blit( this, c, (c.x == 0) ? -px : 0, (c.y == 0) ? -py : 0 );

  // update dirty parts
  if ( d1.w && d1.h ) Draw( d1.x, d1.y, d1.w, d1.h );
  if ( d2.w && d2.h ) Draw( d2.x, d2.y, d2.w, d2.h );
#else
  Draw();
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapView::SetCursor
// DESCRIPTION: Move the cursor onto another hex or remove it from the
//              viewport.
// PARAMETERS : hex - hex to set the cursor to. A hex of (-1, -1) will
//                    remove the cursor entirely. If the cursor was
//                    disabled and hex denotes a valid hex, this will
//                    reenable the cursor.
// RETURNS    : Rect describing the surface area that was updated
////////////////////////////////////////////////////////////////////////

Rect MapView::SetCursor( const Point &hex ) {
  Rect update( 0, 0, 0, 0 );

  if ( (hex.x == -1) && (hex.y == -1) ) {  // remove cursor
    if ( CursorEnabled() ) {
      Point old = cursor;
      cursor = Point( -1, -1 );

      // update old cursor position
      update = UpdateHex( old );
      SetFlags(MV_DISABLE_CURSOR);
    }
  } else {
    UnsetFlags(MV_DISABLE_CURSOR);
    cursor = hex;
    update = UpdateHex( hex );
    if ( FlagSet(MV_AUTOSCROLL) && CheckScroll() ) update = *this;
  }
  return update;
}

