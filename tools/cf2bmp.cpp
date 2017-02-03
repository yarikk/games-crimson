/* cf2bmp -- create an image for a Crimson Fields level file
   Copyright (C) 2004-2007 Jens Granseuer

   Usage: cf2bmp <file>.lev <map>.bmp
   The unit and tile sets must be present in the working directory.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <iostream>
#include <string>
using namespace std;

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "SDL.h"

#include "mission.h"
#include "mapview.h"
#include "globals.h"

#ifdef _MSC_VER
// SDL_Main linkage destroys the command line in VS8
#undef main
#endif

int main( int argc, char *argv[] ) {
  string outmap, level;

  if ( argc != 3 ) {
    cout << "Usage: " << argv[0] << " <file>.lev <image>.bmp" << endl
         << "  --help     display this help and exit" << endl;
    return 1;
  } else {
    level = argv[1];
    outmap = argv[2];
  }

  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    cerr << "Error: Could not init SDL. " << SDL_GetError() << endl;
    return 1;
  }
  atexit(SDL_Quit);

  Mission m;
  if ( m.Load( level.c_str() ) ) {
    cerr << "Error: Could not load level " << level << endl;
    return 1;
  }

  // create full-size bmp image
  Surface ms;
  Map &map = m.GetMap();
  const TerrainSet &tset = m.GetTerrainSet();
  Rect size( 0, 0,
       map.Width() * (tset.TileWidth() - tset.TileShiftX()) + tset.TileShiftX(),
       map.Height() * tset.TileHeight() + tset.TileShiftY() );
  if ( ms.Create( size.w, size.h, DISPLAY_BPP, 0 ) ) {
    cerr << "Error: Could not create surface" << endl;
    return 1;
  }

  MapView mv( &ms, size, MV_DISABLE_FOG|MV_DISABLE_CURSOR );
  mv.SetMap( &map );
  mv.Enable();
  mv.Draw();

  if ( SDL_SaveBMP( ms.s_surface, outmap.c_str() ) ) {
    cerr << "Error: Could not save to " << outmap << endl;
    return 1;
  }

  return 0;
}

