/* mkdatafile -- create a Crimson Fields data file
   Copyright (C) 2000-2007 Jens Granseuer

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

/* takes two file names: icons BMP and output filename

   The data file is written in little-endian format.

   Creates a data file for Crimson Fields containing icons image data
*/

#include <iostream>
using namespace std;

#include "SDL.h"

#include "fileio.h"
#include "mksurface.h"

#ifdef _MSC_VER
// SDL_Main linkage destroys the command line in VS8
#undef main
#endif

int main( int argc, char *argv[] ) {
  int status;
 
  if ( argc != 3 ) {
    cerr << "Invalid number of arguments" << endl
         << "Usage: " << argv[0] << "<icons.bmp> <outfile>" << endl;
    exit(-1);
  }


  if ( SDL_Init(0) < 0 ) {
    cerr << "Couldn't init SDL: " << SDL_GetError() << endl;
    exit(-1);
  }
  atexit(SDL_Quit);

  File out( argv[2] );
  if ( !out.Open( "wb" ) ) {
    cerr << "Couldn't open output file " << argv[5] << endl;
    exit(-1);
  }

  // icons
  MkSurface img;
  status = img.SaveImageData( argv[1], out, true );
  out.Close();
  return status;
}

