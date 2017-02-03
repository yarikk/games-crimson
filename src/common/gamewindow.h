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
// gamewindow.h - gameplay window classes
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_GAMEWINDOW_H
#define _INCLUDE_GAMEWINDOW_H

#include "window.h"
#include "button.h"
#include "map.h"

// unit type information window
class UnitInfoWindow : public Window {
public:
  UnitInfoWindow( unsigned short utype, const Map &map,
                  View *view );

  void Draw( void );
  GUI_Status HandleEvent( const SDL_Event &event );

  static void DrawUnitInfo( short uid, const UnitSet *uset,
                            const TerrainSet *tset,
                            Window *dest, const Rect &rect );
private:
  Rect image;
  Rect info;
  unsigned short unit;
  const UnitSet *uset;
  const TerrainSet *tset;
  Surface *portrait;
};

#endif	/* _INCLUDE_GAMEWINDOW_H */

