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
// mapview.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_MAPVIEW_H
#define _INCLUDE_MAPVIEW_H

#include "rect.h"
#include "map.h"
#include "surface.h"

// special hex graphics
#define IMG_CURSOR_IDLE         4
#define IMG_CURSOR_SELECT       3
#define IMG_CURSOR_ATTACK       5
#define IMG_CURSOR_SPECIAL	7     // e.g. pioneers, depot builders
#define IMG_CURSOR_HIGHLIGHT    0
#define IMG_RECESSED_HEX        2
#define IMG_NOT_AVAILABLE       6


#define MV_AUTOSCROLL		0x0001	// autoscroll when cursor reaches edge of viewport
#define MV_DISABLE		0x0002	// disable viewport entirely (don't update)
#define MV_DISABLE_FOG		0x0004	// don't show fog
#define MV_DISABLE_CURSOR	0x0008	// don't show cursor
#define MV_ENABLE_UNIT_STATS	0x0010  // overlay units with stats (health, xp)
#define MV_DIRTY		0x8000  // must be redrawn

class MapView : public Rect {
public:
  MapView( Surface *display, const Rect &bounds, unsigned short flags );
  ~MapView( void ) { delete [] shader_map; }

  void Resize( const Rect &bounds );
  void SetMap( Map *map );
  Map *GetMap( void ) const { return map; }
  signed char *GetFogBuffer( void ) const { return shader_map; }

  void Enable( void );
  bool Enabled( void ) const { return !FlagSet(MV_DISABLE); }
  void Disable( void );
  void EnableFog( void ) { UnsetFlags(MV_DISABLE_FOG); }
  void DisableFog( void ) { SetFlags(MV_DISABLE_FOG); }
  bool FogEnabled( void ) const { return !FlagSet(MV_DISABLE_FOG); }
  void EnableCursor( void ) { UnsetFlags(MV_DISABLE_CURSOR); }
  void DisableCursor( void ) { SetFlags(MV_DISABLE_CURSOR); }
  bool CursorEnabled( void ) const { return !FlagSet(MV_DISABLE_CURSOR); }
  void EnableUnitStats( void ) { SetFlags(MV_ENABLE_UNIT_STATS); }
  void DisableUnitStats( void ) { UnsetFlags(MV_ENABLE_UNIT_STATS); }
  bool UnitStatsEnabled( void ) const { return FlagSet(MV_ENABLE_UNIT_STATS); }

  void SetFlags( unsigned short flags ) { this->flags |= flags; }
  void UnsetFlags( unsigned short flags ) { this->flags &= (~flags); }
  bool FlagSet( unsigned short flag ) const { return (flags & flag) == flag; }

  void Draw( void ) const { Draw( 0, 0, w, h ); }
  void Draw( short x, short y, unsigned short w, unsigned short h ) const;
  void DrawMap( short x, short y, unsigned short w, unsigned short h,
                Surface *dest, short dx, short dy ) const;

  void DrawUnit( unsigned short n, Surface *dest,
                 short px, short py, const Rect &clip ) const
               { map->GetUnitSet()->DrawTile( n, dest, px, py, clip ); }
  void DrawTerrain( unsigned short n, Surface *dest,
                 short px, short py, const Rect &clip ) const
               { map->GetTerrainSet()->DrawTile( n, dest, px, py, clip ); }
  void DrawFog( Surface *dest, short px, short py, const Rect &clip ) const
               { map->GetTerrainSet()->DrawFog( dest, px, py, clip ); }
  void DrawUnitHealth( unsigned char health, Surface *dest,
                       short px, short py, const Rect &clip ) const;

  int Pixel2Hex( short px, short py, Point &hex ) const;
  Point Hex2Pixel( const Point &hex ) const;
  bool HexVisible( const Point &hex ) const;
  void Scroll( short px, short py );
  Point GetOffsets( void ) const { return Point(curx,cury); }

  Rect UpdateHex( const Point &hex );
  void CenterOnHex( const Point &hex );
  Rect SetCursor( const Point &hex );
  Point Cursor( void ) const { return cursor; }
  void SetCursorImage( unsigned short cur ) { cursor_image = cur; }
  unsigned short GetCursorImage( void ) const { return cursor_image; }

  unsigned short MinXHex( short x ) const;
  unsigned short MinYHex( short y ) const;
  unsigned short MaxXHex( short x, unsigned short w ) const;
  unsigned short MaxYHex( short y, unsigned short h ) const;

  unsigned short TileWidth( void ) const { return map->GetTerrainSet()->TileWidth(); }
  unsigned short TileHeight( void ) const { return map->GetTerrainSet()->TileHeight(); }
  unsigned short TileShiftX( void ) const { return map->GetTerrainSet()->TileShiftX(); }
  unsigned short TileShiftY( void ) const { return map->GetTerrainSet()->TileShiftY(); }

private:
  void InitOffsets( void );
  bool CheckScroll( void );

  Point cursor;
  unsigned short cursor_image;

  unsigned short curx;		// current map offsets
  unsigned short cury;
  unsigned short maxx;		// maximum map offsets
  unsigned short maxy;

  Map *map;
  Surface *surface;

  unsigned short flags;
  signed char *shader_map;
};

#endif	/* _INCLUDE_MAPVIEW_H */

