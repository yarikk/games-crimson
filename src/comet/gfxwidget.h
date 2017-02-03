// CoMET - The Crimson Fields Map Editing Tool
// Copyright (C) 2002-2007 Jens Granseuer
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
// gfxwidget.h - list widget class for gfx selection
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_ED_GFXWIDGET_H
#define _INCLUDE_ED_GFXWIDGET_H

#include "slider.h"
#include "mapview.h"

class GfxWidget : public CompositeWidget, public WidgetHook {
public:
  GfxWidget( short id, short x, short y, unsigned short w,
    unsigned short h, unsigned short flags,
    const char *title, Window *window );

  void Draw( void );

  GUI_Status Select( short item );      // highlight a list item

  GUI_Status MouseDown( const SDL_MouseButtonEvent &button );
  GUI_Status KeyDown( const SDL_keysym &key );

  short Selected( void ) const { return current; }
  unsigned short ItemHeight( void ) const { return gfx_h + spacing; }

protected:
  GUI_Status WidgetActivated( Widget *widget, Window *win );

  void DrawItems( void );

  virtual void DrawItem( short image, short x, short y ) = 0;
  virtual short GetImage( unsigned short item ) const = 0;
  void Init( unsigned short gfxw, unsigned short gfxh,
             unsigned short items );

  short current;
  unsigned short items;
  unsigned short rows;
  unsigned short toprow;
  unsigned short visrows;
  short spacing;
  unsigned short gfx_w;
  unsigned short gfx_h;
  unsigned short gfx_per_row;

  Rect bounds;

  bool init;
};


class TileWidget : public GfxWidget {
public:
  TileWidget( short id, short x, short y, unsigned short w,
    unsigned short h, unsigned short flags,
    const char *title, Window *window, TerrainSet *ts );

  void SetTerrainSet( TerrainSet *ts );

protected:
  void DrawItem( short image, short x, short y )
       { tset->DrawTile( image, surface, x, y, bounds ); }
  short GetImage( unsigned short type ) const;

private:
  TerrainSet *tset;
};

class UnitWidget : public GfxWidget {
public:
  UnitWidget( short id, short x, short y, unsigned short w,
    unsigned short h, unsigned short flags,
    const char *title, Window *window, UnitSet *us );

  void SetUnitSet( UnitSet *us );

protected:
  void DrawItem( short image, short x, short y )
       { uset->DrawTile( image, surface, x, y, bounds ); }
  short GetImage( unsigned short type ) const;

private:
  UnitSet *uset;
};

#endif  // _INCLUDE_ED_GFXWIDGET_H

