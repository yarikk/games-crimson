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
// gamewindow.cpp
///////////////////////////////////////////////////////////////////////

#include "gamewindow.h"
#include "mapview.h"

////////////////////////////////////////////////////////////////////////
// NAME       : UnitInfoWindow::UnitInfoWindow
// DESCRIPTION: Pop up a window with information about the given unit
//              type.
// PARAMETERS : utype - unit type ID
//              map   - map
//              view  - view the window will be attached to
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

UnitInfoWindow::UnitInfoWindow( unsigned short utype, const Map &map,
        View *view ) : Window(WIN_CENTER, view), unit(utype),
                       uset(map.GetUnitSet()), tset(map.GetTerrainSet()) {
  const UnitType *type = uset->GetUnitInfo( utype );
  unsigned short width = MAX( sfont->Width() * 24, uset->TileWidth() * 4) + 20,
                 height = uset->TileHeight() + uset->TileShiftY() +
                          ICON_HEIGHT * 2 + sfont->Height() + 30;

  // don't show portrait on small screens
  short ptnum = type->Portrait();
  if ( (view->Width() < 640) || (view->Height() < 480) ) ptnum = -1;

  if ( ptnum < 0 ) width = MAX( width, uset->TileWidth() * 7 + 40 );
  info.x = info.y = 5;
  info.w = width - 10;
  info.h = height - 10;

  if ( ptnum >= 0 ) {
    portrait = uset->GetPortrait(ptnum);
    image.w = portrait->Width() + 2;
    image.h = portrait->Height() + 2;

    if ( image.w >= image.h ) {     // put image below info area
      width = MAX( width, image.w + 10 );
      height += image.h + 5;
      image.x = (width - image.w) / 2;
      image.y = height - 5 - image.h;
      info.w = width - 10;
    } else {                        // draw image right of info area
      width += image.w + 5;
      height = MAX( height, image.h + 10 );
      image.x = width - image.w - 5;
      image.y = (height - image.h) / 2;
      info.h = height - 10;
    }
  } else portrait = NULL;

  SetSize( width, height );

  Draw();
  Show();
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitInfoWindow::Draw
// DESCRIPTION: Draw the window.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void UnitInfoWindow::Draw( void ) {
  Window::Draw();
  DrawUnitInfo( unit, uset, tset, this, info );
  if ( portrait ) {
    DrawBox( image, BOX_RECESSED );
    portrait->Blit( this, *portrait, image.x + 1, image.y + 1 );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitInfoWindow::HandleEvent
// DESCRIPTION: Close the window on any button or key pressed.
// PARAMETERS : event - event received by the event handler
// RETURNS    : GUI_CLOSE if the window should be closed,
//              GUI_OK otherwise
////////////////////////////////////////////////////////////////////////

GUI_Status UnitInfoWindow::HandleEvent( const SDL_Event &event ) {

  if ( (event.type == SDL_KEYDOWN) || (event.type == SDL_MOUSEBUTTONDOWN) )
    return GUI_CLOSE;

  return GUI_OK;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitInfoWindow::DrawUnitInfo
// DESCRIPTION: Display unit type information.
// PARAMETERS : uid  - unit ID (may be <0 to clear the rect area)
//              uset - unit set (may be NULL if uid < 0)
//              tset - terrain set (may be NULL if uid < 0)
//              dest - destination window
//              rect - rectangle in which to display
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void UnitInfoWindow::DrawUnitInfo( short uid, const UnitSet *uset,
     const TerrainSet *tset, Window *dest, const Rect &rect ) {
  const UnitType *type;

  dest->DrawBox( rect, BOX_CARVED );
  if ( (uid < 0) || ((type = uset->GetUnitInfo(uid)) == NULL) ) return;

  char buf[12];
  short xpos, ypos, xspacing, yspacing, terbreak;

  View *view = dest->GetView();
  Font *sfont = dest->SmallFont();
  Image speed_icon( view->GetSystemIcons(), 0, 0, ICON_WIDTH, ICON_HEIGHT );
  Image def_icon( view->GetSystemIcons(), 32, 0, ICON_WIDTH, ICON_HEIGHT );
  Image ground_icon( view->GetSystemIcons(), 64, 0, ICON_WIDTH, ICON_HEIGHT );
  Image air_icon( view->GetSystemIcons(), 96, 0, ICON_WIDTH, ICON_HEIGHT );
  Image ship_icon( view->GetSystemIcons(), 128, 0, ICON_WIDTH, ICON_HEIGHT );

  yspacing = rect.h - tset->TileHeight() - (ICON_HEIGHT + sfont->Width()) * 2;
  if ( rect.w >= rect.h ) {        // draw all terrain on the same level
    terbreak = 7;
  } else {                         // draw two terrain levels
    terbreak = 4;
    yspacing -= tset->TileHeight();
  }
  yspacing /= 4;

  sfont->Write( type->Name(), dest,
                rect.x + (rect.w - sfont->TextWidth(type->Name())) / 2,
                rect.y + yspacing, rect );

  // show unit specs
  xspacing = (rect.w - 2 * ICON_WIDTH - 3 * sfont->Width()) / 3;
  xpos = rect.x + xspacing;
  ypos = rect.y + sfont->Height() + 2 * yspacing;
  itoa( type->Speed(), buf );
  sfont->Write( buf, dest, xpos + ICON_WIDTH + 4,
                ypos + (ICON_HEIGHT - sfont->Height())/2 );
  speed_icon.Draw( dest, xpos, ypos );

  xpos += ICON_WIDTH + xspacing;
  itoa( type->Armour(), buf );
  sfont->Write( buf, dest, xpos + ICON_WIDTH + 4,
                ypos + (ICON_HEIGHT - sfont->Height())/2 );
  def_icon.Draw( dest, xpos, ypos );

  // show unit combat strength
  xspacing = (rect.w - 3 * ICON_WIDTH) / 4;
  xpos = rect.x + xspacing;
  ypos += ICON_HEIGHT - 5;
  ground_icon.Draw( dest, xpos, ypos );
  if ( type->Firepower(U_GROUND) == 0 ) strcpy( buf, "-" );
  else if ( (type->MinFOF(U_GROUND) == type->MaxFOF(U_GROUND))
         && (type->MaxFOF(U_GROUND) <= 1) ) itoa( type->Firepower(U_GROUND), buf );
  else sprintf( buf, "%2d (%d-%d)", type->Firepower(U_GROUND),
                type->MinFOF(U_GROUND), type->MaxFOF(U_GROUND) );
  sfont->Write( buf, dest, xpos + (ICON_WIDTH - sfont->TextWidth(buf))/2,
                           ypos + ICON_HEIGHT );

  xpos += ICON_WIDTH + xspacing;
  air_icon.Draw( dest, xpos, ypos );
  if ( type->Firepower(U_AIR) == 0 ) strcpy( buf, "-" );
  else if ( (type->MinFOF(U_AIR) == type->MaxFOF(U_AIR))
         && (type->MaxFOF(U_AIR) <= 1) ) itoa( type->Firepower(U_AIR), buf );
  else sprintf( buf, "%2d (%d-%d)", type->Firepower(U_AIR),
                type->MinFOF(U_AIR), type->MaxFOF(U_AIR) );
  sfont->Write( buf, dest, xpos + (ICON_WIDTH - sfont->TextWidth(buf))/2,
                           ypos + ICON_HEIGHT );

  xpos += ICON_WIDTH + xspacing;
  ship_icon.Draw( dest, xpos, ypos );
  if ( type->Firepower(U_SHIP) == 0 ) strcpy( buf, "-" );
  else if ( (type->MinFOF(U_SHIP) == type->MaxFOF(U_SHIP))
         && (type->MaxFOF(U_SHIP) <= 1) ) itoa( type->Firepower(U_SHIP), buf );
  else sprintf( buf, "%2d (%d-%d)", type->Firepower(U_SHIP),
                type->MinFOF(U_SHIP), type->MaxFOF(U_SHIP) );
  sfont->Write( buf, dest, xpos + (ICON_WIDTH - sfont->TextWidth(buf))/2,
                           ypos + ICON_HEIGHT );


  ypos += ICON_HEIGHT + sfont->Height() + yspacing;
  // show terrain access
  Rect terborder( rect.x + 10, ypos, rect.w - 20, rect.h - ypos );
  dest->FillRect( terborder.x, terborder.y - yspacing/2, terborder.w, 1, Color(CF_COLOR_SHADOW) );
  dest->FillRect( terborder.x, terborder.y + 1 - yspacing/2, terborder.w, 1, Color(CF_COLOR_HIGHLIGHT) );

  xpos = terborder.x + (terborder.w - tset->TileWidth() * terbreak) / 2;
  ypos = terborder.y;
  for ( int i = 0; i < 7; ++i ) {
    if ( i == terbreak ) {
      xpos = terborder.x + (terborder.w - tset->TileWidth() * (7 - terbreak)) / 2;
      ypos += tset->TileHeight();
    }

    const TerrainType *tt = tset->GetTerrainInfo( tset->GetTerrainClassID(i) );
    tset->DrawTile( tt->tt_image, dest, xpos, ypos, rect );
    if ( !(type->Terrain() & tt->tt_type) )
      tset->DrawTile( IMG_NOT_AVAILABLE, dest, xpos, ypos, rect );
    xpos += tset->TileWidth();
  }
}

