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

////////////////////////////////////////////////////////////////////////
// gfxwidget.cpp
////////////////////////////////////////////////////////////////////////

#include "gfxwidget.h"
#include "misc.h"

////////////////////////////////////////////////////////////////////////
// NAME       : GfxWidget::GfxWidget
// DESCRIPTION: Create a new gfx widget.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              flags  - widget flags (see widget.h for details)
//              title  - widget title (currently unused)
//              window - widget parent window
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

GfxWidget::GfxWidget( short id, short x, short y, unsigned short w,
    unsigned short h, unsigned short flags, const char *title,
    Window *window ) :
    CompositeWidget( id, x, y, w, h, flags, title, window ),
    bounds( x + 4, y + 4, w - 8 - DEFAULT_SLIDER_SIZE, h - 8 ) {
  spacing = 0;
  init = false;
}

////////////////////////////////////////////////////////////////////////
// NAME       : GfxWidget::Init
// DESCRIPTION: Initialize a gfx widget. This needs to be called when a
//              new set of images is to be bound to the widget.
// PARAMETERS : gfxw  - width of a gfx item in pixels
//              gfxh  - height of a gfx item in pixels
//              items - number of gfx items to be displayed
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void GfxWidget::Init( unsigned short gfxw, unsigned short gfxh, unsigned short items ) {
  this->items = items;
  gfx_w = gfxw;
  gfx_h = gfxh;
  gfx_per_row = MAX( bounds.w/gfxw, 1 );

  toprow = 0;
  rows = (items + gfx_per_row - 1) / gfx_per_row * ItemHeight();
  visrows = MIN( bounds.h, rows );
  current = -1;

  SliderWidget *slider = static_cast<SliderWidget *>( GetWidget(0) );
  if ( slider ) {
    slider->Adjust( 0, rows - visrows, visrows );
    slider->ScrollTo( 0 );
  } else {
    // create the corresponding slider widget
    slider = new SliderWidget( 0, x + w - DEFAULT_SLIDER_SIZE, y,
       DEFAULT_SLIDER_SIZE, h, 0, rows - visrows, toprow, visrows,
       WIDGET_VSCROLL|WIDGET_COMPOSITE, NULL, surface );
    slider->SetHook( this );
    AddWidget( slider );
  }
  init = true;
}

////////////////////////////////////////////////////////////////////////
// NAME       : GfxWidget::Draw
// DESCRIPTION: Draw the widget.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void GfxWidget::Draw( void ) {
  CompositeWidget::Draw();
  surface->DrawBox( Rect(x,y,bounds.w+8,h), BOX_RECESSED );
  if ( init ) DrawItems();
}

////////////////////////////////////////////////////////////////////////
// NAME       : GfxWidget::DrawItems
// DESCRIPTION: Draw widget content.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void GfxWidget::DrawItems( void ) {
  unsigned short row = toprow / ItemHeight();
  unsigned short num = row * gfx_per_row;      // first visible item

  short xoff = bounds.x, yoff = bounds.y + row * ItemHeight() - toprow, img;
  surface->DrawBack( bounds );

  while ( (img = GetImage(num)) != -1 ) {

    if ( num == current ) {
      Rect hilite( xoff, yoff, gfx_w, ItemHeight() );
      hilite.Clip( bounds );
      surface->FillRectAlpha( hilite, surface->GetFGPen() );
    }

    DrawItem( img, xoff, yoff );

    xoff += gfx_w;
    if ( xoff + gfx_w >= bounds.x + bounds.w ) {
      xoff = bounds.x;
      yoff += ItemHeight();
      if ( yoff >= bounds.y + bounds.h ) break;
    }

    ++num;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : GfxWidget::Select
// DESCRIPTION: Adjust the widget so that the selected item is visible
//              and highlight it.
// PARAMETERS : item - number of the item to select
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status GfxWidget::Select( short item ) {
  if ( item < 0 ) item = -1;
  else if ( item >= items ) item = items - 1;

  if ( item != current ) {
    short newtop = toprow;
    current = item;

    if ( current >= 0 ) {
      short selrow = (current/gfx_per_row) * ItemHeight();
      if ( selrow < toprow ) newtop = selrow;
      else if ( selrow + ItemHeight() >= newtop + visrows )
        newtop = selrow - visrows + ItemHeight();
    }

    if ( toprow != newtop ) {
      static_cast<SliderWidget *>( GetWidget(0) )->ScrollTo( newtop );
    } else {
      DrawItems();
      Show();
    }
  }
  if ( hook ) return hook->WidgetActivated( this, surface );
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : GfxWidget::WidgetActivated
// DESCRIPTION: Adjust the widget so that it matches the current slider
//              setting.
// PARAMETERS : widget - activated widget (slider)
//              win    - window the widget is attached to
// RETURNS    : GUI_OK
////////////////////////////////////////////////////////////////////////

GUI_Status GfxWidget::WidgetActivated( Widget *widget, Window *win ) {
  if ( widget->ID() == 0 ) {
    toprow = static_cast<SliderWidget *>(widget)->Level();
    DrawItems();
    Show();
  }
  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : GfxWidget::MouseDown
// DESCRIPTION: Select an image or scroll the list.
// PARAMETERS : button - SDL_MouseButtonEvent received from the event
//                       handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status GfxWidget::MouseDown( const SDL_MouseButtonEvent &button ) {
  SDL_MouseButtonEvent mybutton = button;
  if ( ((button.button == SDL_BUTTON_WHEELUP) ||
       (button.button == SDL_BUTTON_WHEELDOWN)) &&
       Contains( button.x - surface->LeftEdge(), button.y - surface->TopEdge() ) ) {
    // reroute all wheel events to the scroller
    SliderWidget *s = static_cast<SliderWidget *>( GetWidget(0) );
    if ( s ) {
      mybutton.x = s->LeftEdge() + surface->LeftEdge();
      mybutton.y = s->TopEdge() + surface->TopEdge();
    }
  }

  GUI_Status rc = CompositeWidget::MouseDown( mybutton );

  if ( (rc == GUI_OK) && (button.button == SDL_BUTTON_LEFT) ) {
    short mx = button.x - surface->LeftEdge();
    short my = button.y - surface->TopEdge();

    if ( bounds.Contains( mx, my ) && (mx < bounds.x + gfx_per_row * gfx_w) ) {
      short col = (mx - bounds.x) / gfx_w;
      short row = (my - bounds.y + toprow) / ItemHeight();
      short item = row * gfx_per_row + col;

      if ( item < items ) return Select( item );
    }
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : GfxWidget::KeyDown
// DESCRIPTION: If the widget has WIDGET_[HV]SCROLLKEY set, the user can
//              shuffle through the items with the cursor keys.
// PARAMETERS : key - SDL_keysym received from the event handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status GfxWidget::KeyDown( const SDL_keysym &key ) {
  GUI_Status rc = CompositeWidget::KeyDown( key );
  if ( rc != GUI_OK ) return rc;

  if ( flags & (WIDGET_HSCROLLKEY|WIDGET_VSCROLLKEY) ) {
    switch ( key.sym ) {
    case SDLK_UP:
      if ( (current > 0) && (flags & WIDGET_VSCROLLKEY) )
        return Select( current - 1 );
    case SDLK_LEFT:
      if ( (current > 0) && (flags & WIDGET_HSCROLLKEY) )
        return Select( current - 1 );
      break;
    case SDLK_DOWN:
      if ( (current < items - 1) && (flags & WIDGET_VSCROLLKEY) )
        return Select( current + 1 );
    case SDLK_RIGHT:
      if ( (current < items - 1) && (flags & WIDGET_HSCROLLKEY) )
        return Select( current + 1 );
      break;
    default:
      break;
    }
  }

  return GUI_OK;
}


////////////////////////////////////////////////////////////////////////
// NAME       : TileWidget::TileWidget
// DESCRIPTION: Create a new tile widget.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              flags  - widget flags (see widget.h for details)
//              title  - widget title (currently unused)
//              window - widget parent window
//              ts     - terrain set used (may be NULL)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

TileWidget::TileWidget( short id, short x, short y, unsigned short w,
    unsigned short h, unsigned short flags, const char *title,
    Window *window, TerrainSet *ts ) :
    GfxWidget( id, x, y, w, h, flags, title, window ) {
  SetTerrainSet( ts );
}

////////////////////////////////////////////////////////////////////////
// NAME       : TileWidget::SetTerrainSet
// DESCRIPTION: Attach a different terrain set to this widget.
// PARAMETERS : ts - terrain set to be used
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TileWidget::SetTerrainSet( TerrainSet *ts ) {
  tset = ts;
  if ( ts ) Init( ts->TileWidth(), ts->TileHeight(), ts->NumTiles() );
}

////////////////////////////////////////////////////////////////////////
// NAME       : TileWidget::GetImage
// DESCRIPTION: Get the image identifier of a terrain type.
// PARAMETERS : type - terrain type
// RETURNS    : -1 on error, image identifier otherwise
////////////////////////////////////////////////////////////////////////

short TileWidget::GetImage( unsigned short type ) const {
  short rc = -1;
  if ( tset ) {
    const TerrainType *tt = tset->GetTerrainInfo(type);
    if ( tt ) rc = tt->tt_image;
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitWidget::UnitWidget
// DESCRIPTION: Create a new unit widget.
// PARAMETERS : id     - widget identifier
//              x      - left edge of widget
//              y      - top edge of widget
//              w      - widget width
//              h      - widget height
//              flags  - widget flags (see widget.h for details)
//              title  - widget title (currently unused)
//              window - widget parent window
//              us     - unit set used (may be NULL)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

UnitWidget::UnitWidget( short id, short x, short y, unsigned short w,
    unsigned short h, unsigned short flags, const char *title,
    Window *window, UnitSet *us ) :
    GfxWidget( id, x, y, w, h, flags, title, window ) {
  SetUnitSet( us );
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitWidget::SetUnitSet
// DESCRIPTION: Attach a different unit set to this widget.
// PARAMETERS : us - unit set to be used
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void UnitWidget::SetUnitSet( UnitSet *us ) {
  uset = us;
  if ( us ) Init( us->TileWidth(), us->TileHeight(), us->NumTiles() * 2 );
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitWidget::GetImage
// DESCRIPTION: Get the image identifier of a unit type.
// PARAMETERS : type - unit type
// RETURNS    : -1 on error, image identifier otherwise
////////////////////////////////////////////////////////////////////////

short UnitWidget::GetImage( unsigned short type ) const {
  short rc = -1;
  if ( uset ) {
    const UnitType *ut = uset->GetUnitInfo(type % uset->NumTiles());
    if ( ut ) {
      if ( type < uset->NumTiles() ) rc = ut->Image();
      else rc = ut->Image() + 9;      /* player 2, facing southwards */
    }
  }
  return rc;
}

