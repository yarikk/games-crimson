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
// mapwindow.cpp
////////////////////////////////////////////////////////////////////////

#include "mapwindow.h"
#include "game.h"
#include "msgs.h"

////////////////////////////////////////////////////////////////////////
// NAME       : MapWindow::MapWindow
// DESCRIPTION: Set the basic map window variables.
// PARAMETERS : x     - left edge of window
//              y     - top edge of window
//              w     - window width
//              h     - window height
//              flags - window flags (see window.h for details)
//              view  - pointer to the view the window will be put on
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

MapWindow::MapWindow( short x, short y, unsigned short w, unsigned short h,
                      unsigned short flags, View *view ) :
           Window( x, y, w, h, flags, view ) {
  panel = new Panel( this, view );
  mview = new MapView( this, *this,
              MV_AUTOSCROLL|MV_DISABLE|MV_DISABLE_CURSOR|MV_DISABLE_FOG );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWindow::HandleEvent
// DESCRIPTION: React to user input.
// PARAMETERS : event - SDL_Event
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

GUI_Status MapWindow::HandleEvent( const SDL_Event &event ) {
  return Gam->HandleEvent(event);
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWindow::Draw
// DESCRIPTION: Draw the map window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapWindow::Draw( void ) {
  if ( mview->Enabled() ) mview->Draw();
  else Window::DrawBack();
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWindow::DrawBack
// DESCRIPTION: Draw the map window background into the internal buffer.
// PARAMETERS : x - left edge of area to paint
//              y - top edge of area to paint
//              w - width of area to paint
//              h - height of area to paint
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapWindow::DrawBack( short x, short y, unsigned short w, unsigned short h ) {
  FillRect( x, y, w, h, Color(CF_COLOR_SHADOW) );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWindow::MoveHex
// DESCRIPTION: Smoothly move a hex image (usually a unit or cursor)
//              from one hex to another (adjacent) one.
// PARAMETERS : img   - map tile identifier for the hex image
//              tiles - tile set containing the image
//              hex1  - source hex position
//              hex2  - destination hex position
//              speed - total time (ms) for the animation
//              blink - if TRUE make target cursor blink (default is
//                      FALSE)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapWindow::MoveHex( unsigned short img, const TileSet &tiles,
                const Point &hex1, const Point &hex2,
                unsigned short speed, bool blink /* = false */ ) {
  // allocate a new surface where we can store the "background" graphics
  Point psrc = mview->Hex2Pixel( hex1 );
  Point pdst = mview->Hex2Pixel( hex2 );

  Rect bg;
  bg.x = MIN( psrc.x, pdst.x );
  bg.y = MIN( psrc.y, pdst.y );
  bg.w = tiles.TileWidth() + ABS(psrc.x-pdst.x);
  bg.h = tiles.TileHeight() + ABS(psrc.y-pdst.y);
  bg.Clip( *this );

  Surface *bgs = new Surface;
  if ( !bgs->Create( bg.w, bg.h, DISPLAY_BPP, 0 ) ) {
    unsigned long oldticks, ticks = 0, blinkticks = 0;
    Point curp = mview->Cursor();

    Blit( bgs, bg, 0, 0 );

    if ( blink && mview->CursorEnabled() ) {
      mview->DisableCursor();
      Show( mview->UpdateHex( curp ) );
      blinkticks = speed / 2;
    }

    oldticks = SDL_GetTicks();
    do {
      short cx, cy;

      // don't modify the following three lines; some versions of gcc
      // (e.g. 2.95.3) seem to produce bogus code when all the calculations
      // are done in one line
      cx = (short)((pdst.x - psrc.x) * ticks);
      cy = (short)((pdst.y - psrc.y) * ticks);
      tiles.DrawTile( img, this,
            psrc.x + cx / speed, psrc.y + cy / speed, bg );

      Show( bg );
      bgs->Blit( this, Rect( 0, 0, bg.w, bg.h ), bg.x, bg.y );

      ticks = SDL_GetTicks() - oldticks;

      if ( blinkticks && ticks >= blinkticks ) {
        mview->EnableCursor();
        Show( mview->UpdateHex( curp ) );
        blinkticks = 0;
      }

    } while ( ticks < speed );

    Show( bg );
  }
  delete bgs;
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWindow::FadeHex
// DESCRIPTION: Fade in a hex image (a unit or terrain image).
// PARAMETERS : img  - tile identifier for the hex image
//              hex  - destination hex position
//              in   - whether to fade in or out
//              unit - if TRUE paint a unit image, otherwise the
//                     respective terrain
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapWindow::FadeHex( unsigned short img, const Point &hex, bool in, bool unit ) {
  unsigned long firstticks, oldticks, ticks;
  Point pos = mview->Hex2Pixel( hex );
  const Rect dest( pos.x, pos.y, mview->TileWidth(), mview->TileHeight() );
  const unsigned int speed = 4 * ANIM_SPEED_UNIT;
  const unsigned short terrain = mview->GetMap()->HexImage( hex );

  Surface tile;
  tile.Create( mview->TileWidth(), mview->TileHeight(), DISPLAY_BPP, SDL_HWSURFACE );
  tile.SetAlpha( SDL_ALPHA_TRANSPARENT, SDL_SRCALPHA );
  tile.SetColorKey( Color(CF_COLOR_WHITE) );
  tile.DisplayFormat();
  tile.Flood( Color(CF_COLOR_WHITE) );

  Unit *blocker = NULL;
  if ( unit ) mview->DrawUnit( img, &tile, 0, 0, tile );
  else {
    mview->DrawTerrain( img, &tile, 0, 0, tile );
    blocker = mview->GetMap()->GetUnit( hex );
  }

  firstticks = oldticks = SDL_GetTicks();
  do {
    ticks = SDL_GetTicks() - firstticks;

    int alpha = SDL_ALPHA_OPAQUE * ticks / speed;
    if ( alpha > SDL_ALPHA_OPAQUE ) alpha = SDL_ALPHA_OPAQUE;
    if ( !in ) alpha = SDL_ALPHA_OPAQUE - alpha;

    tile.SetAlpha( alpha, SDL_SRCALPHA );
    mview->DrawTerrain( terrain, this, pos.x, pos.y, dest );
    tile.Blit( this, tile, pos.x, pos.y );
    if ( blocker )
      mview->DrawUnit( blocker->Image(), this, pos.x, pos.y, dest );
    Show( dest );

    if ( ticks - oldticks < 30 ) SDL_Delay( 25 );
    oldticks = ticks;
  } while ( ticks < speed );

  Show( dest );
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWindow::FlashUnit
// DESCRIPTION: Make the unit at the target hex flash with white.
// PARAMETERS : hex   - destination hex position
//            : times - desired number of times
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapWindow::FlashUnit( const Point &hex, unsigned short times ) {
  unsigned long firstticks, oldticks, ticks;
  Point pos = mview->Hex2Pixel( hex );
  const Rect dest( pos.x, pos.y, mview->TileWidth(), mview->TileHeight() );
  const unsigned int speed = ANIM_SPEED_UNIT * 3 / 2;
  const unsigned int delay = speed / 3;
  unsigned short img;
  bool show_cursor;

  Surface tile;
  tile.Create( mview->TileWidth(), mview->TileHeight(), DISPLAY_BPP, 0 );
  tile.SetAlpha( SDL_ALPHA_OPAQUE, SDL_SRCALPHA );
  tile.SetColorKey( Color(CF_COLOR_WHITE) );
  tile.DisplayFormat();
  tile.Flood( Color(CF_COLOR_WHITE) );

  img = mview->GetMap()->GetUnit( hex )->Image();
  mview->DrawUnit( img, &tile, 0, 0, tile );
  show_cursor = mview->CursorEnabled() && (mview->Cursor() == hex);
  if ( show_cursor )
    mview->DrawTerrain( mview->GetCursorImage(), &tile, 0, 0, tile );

  Surface whitetile;
  whitetile.Create( mview->TileWidth(), mview->TileHeight(), DISPLAY_BPP, 0 );
  whitetile.SetAlpha( SDL_ALPHA_OPAQUE / 4, SDL_SRCALPHA );
  whitetile.DisplayFormat();
  whitetile.Flood( Color(CF_COLOR_WHITE) );

  for ( int i = 0; i < times; ++i ) {
    firstticks = oldticks = SDL_GetTicks();
    do {
      whitetile.Blit( &tile, whitetile, 0, 0 );
      if ( show_cursor )
        mview->DrawTerrain( mview->GetCursorImage(), &tile, 0, 0, tile );
      tile.Blit( this, tile, pos.x, pos.y );
      Show( dest );

      ticks = SDL_GetTicks() - firstticks;
      if ( ticks - oldticks < 30 ) SDL_Delay( 25 );
      oldticks = ticks;
    } while ( ticks < delay );

    SDL_Delay( delay );

    mview->DrawUnit( img, &tile, 0, 0, tile );
    if ( show_cursor )
      mview->DrawTerrain( mview->GetCursorImage(), &tile, 0, 0, tile );
    tile.Blit( this, tile, pos.x, pos.y );
    Show( dest );

    SDL_Delay( delay );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWindow::BoxAvoidHexes
// DESCRIPTION: Try to place a box on the screen so that it does not
//              cover the two given hexes.
// PARAMETERS : box  - the box; left and top edge will be modified
//              hex1 - first hex position
//              hex2 - second hex position
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapWindow::BoxAvoidHexes( Rect &box, const Point &hex1, const Point &hex2 ) const {
  // define some preferred positions; we'll try to take one of these
  Point defs[4];
  defs[0] = Point( (w - box.w) / 2, (h - box.h) / 2 );
  defs[1] = Point( (w - box.w) / 2, 40 );
  defs[2] = Point( 40, (h - box.h) / 2 );
  defs[3] = Point( w - box.w - 40, (h - box.h) / 2 );

  Point p1 = mview->Hex2Pixel( hex1 );
  Point p2 = mview->Hex2Pixel( hex2 );

  for ( int i = 0; i < 4; ++i ) {
    if ( (((defs[i].x > p1.x + mview->TileWidth()) || (defs[i].x + box.w <= p1.x)) ||
         ((defs[i].y > p1.y + mview->TileHeight()) || (defs[i].y + box.h <= p1.y))) &&
         (((defs[i].x > p2.x + mview->TileWidth()) || (defs[i].x + box.w <= p2.x)) ||
         ((defs[i].y > p2.y + mview->TileHeight()) || (defs[i].y + box.h <= p2.y))) ) {
      box.x = defs[i].x;
      box.y = defs[i].y;
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWindow::DisplayHex
// DESCRIPTION: Make sure the requested hex is visible and update the
//              display. Only call this function if the map view is
//              enabled.
// PARAMETERS : hex - hex to view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapWindow::DisplayHex( const Point &hex ) {
  if ( mview->FlagSet( MV_DIRTY ) || !mview->HexVisible( hex ) ) {
    mview->CenterOnHex( hex );
    mview->UnsetFlags( MV_DIRTY );
    Show( *mview );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWindow::MoveCursor
// DESCRIPTION: Move the cursor into the given direction if possible.
// PARAMETERS : dir - requested direction
// RETURNS    : new cursor position
////////////////////////////////////////////////////////////////////////

Point MapWindow::MoveCursor( Direction dir ) {
  Point dest, cursor = mview->Cursor();

  if ( mview->GetMap()->Dir2Hex( cursor, dir, dest ) ) return cursor;

  mview->DisableCursor();
  mview->UpdateHex( cursor );
  MoveHex( mview->GetCursorImage(),
           *mview->GetMap()->GetTerrainSet(),
           cursor, dest, ANIM_SPEED_CURSOR );
  mview->EnableCursor();
  return dest;
}

////////////////////////////////////////////////////////////////////////
// NAME       : MapWindow::VideoModeChange
// DESCRIPTION: This method is called by the view whenever the video
//              resolution changes. The window can then adjust itself
//              to the new dimensions. Afterwards a View::Refresh() is
//              performed, i. e. the window just needs to reblit its
//              contents to its internal buffer, and NOT call a view
//              update itself.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void MapWindow::VideoModeChange( void ) {
  if ( (view->Width() != w) || (view->Height() != h) ) {
    SetSize( view->Width(), view->Height() );

    mview->Resize( *this );
    Draw();
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Panel::Panel
// DESCRIPTION: Create the panel window.
// PARAMETERS : mw   - pointer to map window
//              view - pointer to view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Panel::Panel( MapWindow *mw, View *view ) :
    Window( 0, mw->Height() - DEFAULT_PANEL_HEIGHT, 0, 0, 0, view ),
    workshop_icon( view->GetSystemIcons(), 148, 32, XP_ICON_WIDTH, XP_ICON_HEIGHT ),
    factory_icon( view->GetSystemIcons(), 160, 32, XP_ICON_WIDTH, XP_ICON_HEIGHT )
{
  obj = NULL;
  mapwin = mw;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Panel::Draw
// DESCRIPTION: Draw the panel window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Panel::Draw( void ) {
  if ( obj ) {
    short namex = 5;

    Window::Draw();

    if ( obj->IsUnit() ) {
      Unit *u = static_cast<Unit *>(obj);
      char buf[4];
      sprintf( buf, "(%d)", u->GroupSize() );

      namex += 5 + XP_ICON_WIDTH;
      Images[ICON_XP_BASE + u->XPLevel()]->Draw( this, 5, (h - XP_ICON_HEIGHT)/2 );
      sfont->Write( buf, this, namex + 5 + sfont->TextWidth( u->Name() ),
                  (h - sfont->Height())/2 );
    } else {
      Building *b = static_cast<Building *>(obj);
      if ( b->IsWorkshop() ) {
        workshop_icon.Draw( this, 5, (h - XP_ICON_HEIGHT)/2 );
        namex += 5 + XP_ICON_WIDTH;
      }
      if ( b->IsFactory() ) {
        factory_icon.Draw( this, namex, (h - XP_ICON_HEIGHT)/2 );
        namex += 5 + XP_ICON_WIDTH;
      }
    }
    sfont->Write( obj->Name(), this, namex, (h - sfont->Height())/2 );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Panel::Update
// DESCRIPTION: Set the information displayed on the panel and update
//              the display.
// PARAMETERS : obj - map object to display information about (may be
//                    NULL, in which case the panel will be cleared)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Panel::Update( MapObject *obj ) {
  Rect refresh = *this;

  if ( !obj ) {
    if ( this->obj ) {
      w = h = 0;
      y = mapwin->Height() - DEFAULT_PANEL_HEIGHT;
      view->Refresh( refresh );
      this->obj = NULL;
    }
  } else {
    this->obj = obj;

    // resize and reposition if necessary
    w = sfont->TextWidth( obj->Name() ) + 10;
    if ( obj->IsUnit() ) w += 5 + XP_ICON_WIDTH + sfont->Width() * 3;
    else {
      Building *b = static_cast<Building *>(obj);
      if ( b->IsWorkshop() ) w += 5 + XP_ICON_WIDTH;
      if ( b->IsFactory() ) w += 5 + XP_ICON_WIDTH;
    }
    h = refresh.h = DEFAULT_PANEL_HEIGHT;

    MapView *mv = mapwin->GetMapView();
    Point pcursor = mv->Hex2Pixel( mv->Cursor() );
    if ( pcursor.x <= w + mv->TileWidth() ) {
      if ( y == 0 ) {
        if ( pcursor.y <= h + mv->TileHeight() ) {
          y = mapwin->Height() - h;
        }
      } else if ( pcursor.y > y - mv->TileHeight() ) y = 0;
    }
    SetSize( w, h );   // allocate surface
    Draw();

    // update the old window position and show the new
    if ( refresh.y == y ) {
      if ( w > refresh.w ) refresh.w = w;
    } else Show();
    view->Refresh( refresh );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Panel::HandleEvent
// DESCRIPTION: The window must pass all incoming events to the map
//              window which does proper processing.
// PARAMETERS : event - event as received from the event handler
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status Panel::HandleEvent( const SDL_Event &event ) {
  return mapwin->HandleEvent( event );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Panel::VideoModeChange
// DESCRIPTION: This method is called by the view whenever the video
//              resolution changes. The window can then adjust itself
//              to the new dimensions. Afterwards a View::Refresh() is
//              performed, i. e. the window just needs to reblit its
//              contents to its internal buffer, and NOT call a view
//              update itself.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Panel::VideoModeChange( void ) {
  Window::VideoModeChange();

  if ( TopEdge() + DEFAULT_PANEL_HEIGHT < view->Height() )
    y = view->Height() - DEFAULT_PANEL_HEIGHT;
}


////////////////////////////////////////////////////////////////////////
// NAME       : TacticalWindow::TacticalWindow
// DESCRIPTION: Create a window and show an overview map of the level.
// PARAMETERS : mapview - pointer to map viewport
//              m       - mission object
//              view    - pointer to the window's view
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

TacticalWindow::TacticalWindow( MapView *mapview, Mission &m, View *view ) :
        Window( WIN_CLOSE_ESC|WIN_CENTER, view ), mv(mapview), mission(m),
        p1(m.GetPlayer(PLAYER_ONE)), p2(m.GetPlayer(PLAYER_TWO)) {
  Map *pmap = mv->GetMap();

  Rect win, map, viewport;

  unsigned char magnify = MIN( (view->Width() - 20) / pmap->Width(),
                               (view->Height() * 3 / 4) / pmap->Height() );
  if ( magnify < 2 ) magnify = 2;
  else if ( magnify > 6 ) magnify = 6;

  map.w = MIN( pmap->Width() * magnify + 2, view->Width() - 10 );
  map.h = MIN( pmap->Height() * magnify + magnify/2 + 2,
               view->Height() - sfont->Height() * 3 - 40 );

  // calculate window dimensions
  const char *ulabel = MSG(MSG_UNITS);
  const char *slabel = MSG(MSG_SHOPS);
  unsigned short ulen = sfont->TextWidth( ulabel );
  unsigned short slen = sfont->TextWidth( slabel );
  unsigned short labellen = MAX( ulen, slen );

  win.w = MAX( map.w + 20, labellen + sfont->Width() * 12 + 30 );
  win.h = MIN( map.h + sfont->Height() * 3 + 40, view->h );
  win.Center( *view );
  win.Align( *view );

  map.x = (win.w - map.w) / 2;
  map.y = MAX( 5, win.h - 3 * sfont->Height() - map.h - 35 );

  SetSize( win );
  CalcViewPort( viewport );

  new ButtonWidget( GUI_CLOSE, 1, h - sfont->Height() - 9,
                    w - 2, sfont->Height() + 8,
                    WIDGET_DEFAULT, MSG(MSG_B_OK), this );

  mw = new MapWidget( 0, map.x, map.y, map.w, map.h,
                      WIDGET_HSCROLLKEY|WIDGET_VSCROLLKEY, this );
  mw->SetPlayerColors( p1.LightColor(), p2.LightColor() );
  mw->SetMap( pmap, viewport, magnify );
  mw->SetHook( this );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : TacticalWindow::WidgetActivated
// DESCRIPTION: Handle events from the MapWidget.
// PARAMETERS : widget - widget sending the message (unused)
//              win    - window the widget belongs to (unused)
// RETURNS    : GUI status
////////////////////////////////////////////////////////////////////////

GUI_Status TacticalWindow::WidgetActivated( Widget *widget, Window *win ) {
  Point hex = mw->GetLastHex();
  Rect viewport;

  view->DisableUpdates();
  if ( !mv->HexVisible( hex ) ) mv->CenterOnHex( hex );
  Gam->SetCursor( hex );

  CalcViewPort( viewport );
  mw->SetViewPort( viewport );
  mw->Draw();
  view->EnableUpdates();
  view->Refresh();

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : TacticalWindow::Draw
// DESCRIPTION: Draw the window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TacticalWindow::Draw( void ) {
  Window::Draw();

  // show unit and building count
  Color col;
  unsigned short linewidth = w - 40, lx, l1y, l2y, lh, xoff1, xoff2;
  char numbuf[4];

  unsigned short bs1 = 0, bs2 = 0, bstotal = 0;
  Building *b = static_cast<Building *>( mission.GetShops().Head() );
  while ( b ) {
    if ( b->Owner() ) {
      if ( b->Owner()->ID() == PLAYER_ONE ) ++bs1;
      else if ( b->Owner()->ID() == PLAYER_TWO ) ++bs2;
    }
    ++bstotal;
    b = static_cast<Building *>( b->Next() );
  }


  lh = (h - mw->y - mw->h - sfont->Height() * 3 - 9) / 5;
  lx = MIN( (w - linewidth - 10) / 2, mw->x + 5 );
  l1y = mw->y + mw->h + lh * 2;
  l2y = h - (sfont->Height() + lh) * 2 - 9;
  DrawBox( Rect( lx - 5, l1y - lh, w - 2*lx + 10, (sfont->Height() * 2 + lh * 3) ),
           BOX_RECESSED );

  lx = (w - linewidth - 10) / 2;
  xoff1 = sfont->Write( MSG(MSG_UNITS), this, lx, l1y );
  xoff2 = sfont->Write( MSG(MSG_SHOPS), this, lx, l2y );

  lx += MAX( xoff1, xoff2 ) + 20;
  col = p1.LightColor();
  xoff1 = sfont->Write( itoa( p1.Units(0), numbuf ), this, lx, l1y, col );
  xoff2 = sfont->Write( itoa( bs1, numbuf ), this, lx, l2y, col );

  lx += MAX( xoff1, xoff2 ) + 20;
  col = p2.LightColor();
  xoff1 = sfont->Write( itoa( p2.Units(0), numbuf ), this, lx, l1y, col );
  xoff2 = sfont->Write( itoa( bs2, numbuf ), this, lx, l2y, col );

  lx += MAX( xoff1, xoff2 ) + 20;
  sfont->Write( itoa( mission.GetUnits().CountNodes(), numbuf ), this, lx, l1y );
  sfont->Write( itoa( bstotal, numbuf ), this, lx, l2y );
}

////////////////////////////////////////////////////////////////////////
// NAME       : TacticalWindow::CalcViewPort
// DESCRIPTION: Determine which part of the map is currently displayed
//              on the map window and set the viewport rect accordingly.
// PARAMETERS : vp - Rect buffer to hold the view port coordinates
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TacticalWindow::CalcViewPort( Rect &vp ) const {
  vp.x = mv->MinXHex(-1);
  vp.y = mv->MinYHex(-1);
  vp.w = (mv->MaxXHex(-1,0) + 1) - vp.x;
  vp.h = (mv->MaxYHex(-1,0) + 1) - vp.y;
}


////////////////////////////////////////////////////////////////////////
// NAME       : CombatWindow::CombatWindow
// DESCRIPTION: The CombatWindow visualizes the fight between two units.
// PARAMETERS : combat - pointer to a combat object
//              mapwin - pointer to the map window; required to update
//                       the display when a unit is destroyed
//              view   - view the window will belong to
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

CombatWindow::CombatWindow( Combat *combat, MapWindow *mapwin, View *view ) :
  Window( 0, view ), mapwin( mapwin ), att( combat->GetAttacker() ),
    def( combat->GetDefender() ), apos( att->Position() ), dpos( def->Position() ),
    startgroup1( att->GroupSize() ), startgroup2( def->GroupSize() ) {

  MapView *mv = mapwin->GetMapView();

  // calculate window dimensions
  short wwidth = (MAX( sfont->TextWidth( att->Name() ),
                       sfont->TextWidth( def->Name() ) ) + XP_ICON_WIDTH + 25) * 2;
  wwidth = MAX( wwidth, mv->TileWidth() * 6 + 50 );
  Rect win( 0, 0, wwidth, mv->TileHeight() * 3 + sfont->Height() * 2 + 50 );

  // place window on map window
  mv->CenterOnHex( Point( (apos.x + dpos.x) / 2, (apos.y + dpos.y) / 2 ) );
  mapwin->BoxAvoidHexes( win, apos, dpos );
  SetSize( win.x, win.y, win.w, win.h );

  // highlight the fighting units
  Rect clip;
  Point punit = mv->Hex2Pixel( apos );
  clip = Rect( punit.x, punit.y, mv->TileWidth(), mv->TileHeight() );
  clip.Clip( *mv );
  mv->DrawTerrain( IMG_CURSOR_HIGHLIGHT, mapwin, punit.x, punit.y, clip );
  punit = mv->Hex2Pixel( dpos );
  clip = Rect( punit.x, punit.y, mv->TileWidth(), mv->TileHeight() );
  clip.Clip( *mv );
  mv->DrawTerrain( IMG_CURSOR_HIGHLIGHT, mapwin, punit.x, punit.y, clip );

  // create button widget, message areas, and unit display areas
  Rect brect( 1, h - sfont->Height() - 10, w - 2, sfont->Height() + 10 );

  msgbar1 = Rect( 4, h - sfont->Height() - 14 - brect.h, (w - 16) / 2, sfont->Height() + 6 );
  msgbar2 = Rect( msgbar1.x + msgbar1.w + 8, msgbar1.y, msgbar1.w, msgbar1.h );

  att_anchor = Rect( w/4 - mv->TileWidth()/2, 10 + mv->TileHeight(), mv->TileWidth(), mv->TileHeight() );
  def_anchor = Rect( (w * 3)/4 - mv->TileWidth()/2, att_anchor.y, mv->TileWidth(), mv->TileHeight() );

  clock[0][0] = Rect( att_anchor.x, att_anchor.y - mv->TileHeight(), mv->TileWidth(), mv->TileHeight() );
  clock[0][1] = Rect( att_anchor.x + mv->TileWidth() - mv->TileShiftX(), att_anchor.y - mv->TileShiftY(), mv->TileWidth(), mv->TileHeight() );
  clock[0][2] = Rect( att_anchor.x + mv->TileWidth() - mv->TileShiftX(), att_anchor.y + mv->TileShiftY(), mv->TileWidth(), mv->TileHeight() );
  clock[0][3] = Rect( att_anchor.x, att_anchor.y + mv->TileHeight(), mv->TileWidth(), mv->TileHeight() );
  clock[0][4] = Rect( att_anchor.x - mv->TileWidth() + mv->TileShiftX(), att_anchor.y + mv->TileShiftY(), mv->TileWidth(), mv->TileHeight() );
  clock[0][5] = Rect( att_anchor.x - mv->TileWidth() + mv->TileShiftX(), att_anchor.y - mv->TileShiftY(), mv->TileWidth(), mv->TileHeight() );
  clock[1][0] = Rect( def_anchor.x, def_anchor.y - mv->TileHeight(), mv->TileWidth(), mv->TileHeight() );
  clock[1][1] = Rect( def_anchor.x + mv->TileWidth() - mv->TileShiftX(), def_anchor.y - mv->TileShiftY(), mv->TileWidth(), mv->TileHeight() );
  clock[1][2] = Rect( def_anchor.x + mv->TileWidth() - mv->TileShiftX(), def_anchor.y + mv->TileShiftY(), mv->TileWidth(), mv->TileHeight() );
  clock[1][3] = Rect( def_anchor.x, def_anchor.y + mv->TileHeight(), mv->TileWidth(), mv->TileHeight() );
  clock[1][4] = Rect( def_anchor.x - mv->TileWidth() + mv->TileShiftX(), def_anchor.y + mv->TileShiftY(), mv->TileWidth(), mv->TileHeight() );
  clock[1][5] = Rect( def_anchor.x - mv->TileWidth() + mv->TileShiftX(), def_anchor.y - mv->TileShiftY(), mv->TileWidth(), mv->TileHeight() );

  button = new ButtonWidget( GUI_CLOSE, brect.x, brect.y, brect.w, brect.h, WIDGET_DEFAULT, MSG(MSG_B_OK), this );

  Draw();
  view->Refresh();
}

////////////////////////////////////////////////////////////////////////
// NAME       : CombatWindow::Draw
// DESCRIPTION: Draw the window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void CombatWindow::Draw( void ) {
  Window::Draw();
  DrawBox( msgbar1, BOX_RECESSED );
  DrawBox( msgbar2, BOX_RECESSED );

  DrawBox( Rect( msgbar1.x, 4, msgbar1.w, msgbar1.y - 8 ), BOX_CARVED );
  DrawBox( Rect( msgbar2.x, 4, msgbar2.w, msgbar2.y - 8 ), BOX_CARVED );

  Images[ICON_XP_BASE + att->XPLevel()]->Draw( this, msgbar1.x + 4, msgbar1.y + (msgbar1.h - XP_ICON_HEIGHT)/2 );
  Images[ICON_XP_BASE + def->XPLevel()]->Draw( this, msgbar2.x + 4, msgbar2.y + (msgbar2.h - XP_ICON_HEIGHT)/2 );

  sfont->Write( att->Name(), this, msgbar1.x + XP_ICON_WIDTH + 8,
                msgbar1.y + (msgbar1.h - sfont->Height())/2 );
  sfont->Write( def->Name(), this, msgbar2.x + XP_ICON_WIDTH + 8,
                msgbar2.y + (msgbar2.h - sfont->Height())/2 );

  // draw terrain and unit image to the center of the 'clock'
  MapView *mv = mapwin->GetMapView();
  Map *map = mv->GetMap();
  mv->DrawTerrain( map->HexImage( apos ), this, att_anchor.x, att_anchor.y, att_anchor );
  mv->DrawTerrain( map->HexImage( dpos ), this, def_anchor.x, def_anchor.y, def_anchor );
  mv->DrawUnit( att->Image(), this, att_anchor.x, att_anchor.y, att_anchor );
  mv->DrawUnit( def->Image(), this, def_anchor.x, def_anchor.y, def_anchor );

  DrawState();
}

////////////////////////////////////////////////////////////////////////
// NAME       : CombatWindow::DrawState
// DESCRIPTION: Update the two 'clocks' to display the current standing.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void CombatWindow::DrawState( void ) {
  short group1 = att->GroupSize(), group2 = def->GroupSize();
  MapView *mv = mapwin->GetMapView();

  for ( int i = NORTH; i <= NORTHWEST; ++i ) {

    if ( i < startgroup1 ) {
      mv->DrawTerrain( IMG_RECESSED_HEX, this, clock[0][i].x, clock[0][i].y, clock[0][i] );
      mv->DrawUnit( att->BaseImage() + i, this, clock[0][i].x, clock[0][i].y, clock[0][i] );

      if ( i >= group1 )
        mv->DrawFog( this, clock[0][i].x, clock[0][i].y, clock[0][i] );
    }

    if ( i < startgroup2 ) {
      mv->DrawTerrain( IMG_RECESSED_HEX, this, clock[1][i].x, clock[1][i].y, clock[1][i] );
      mv->DrawUnit( def->BaseImage() + i, this, clock[1][i].x, clock[1][i].y, clock[1][i] );

      if ( i >= group2 )
        mv->DrawFog( this, clock[1][i].x, clock[1][i].y, clock[1][i] );
    }
  }
}

