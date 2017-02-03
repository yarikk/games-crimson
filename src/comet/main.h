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

//////////////////////////////////////////////////////////////
// main.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_ED_MAIN_H
#define _INCLUDE_ED_MAIN_H

#define PROGRAMNAME "CoMET"

#ifdef WIN32
# define CFEDRC "cfed.ini"
#else
# define CFEDRC "cfedrc"
#endif


struct EdOptions {
  short width;
  short height;
  short bpp;
  bool sound;
  unsigned long flags;
  const char *level;
};

class Editor {
public:
  Editor( const char *mapdir, const EdOptions &opts );
  ~Editor( void );

  View *GetView( void ) const { return view; }

private:
  int Init( const EdOptions &opts );

  View *view;
};

#endif  // _INCLUDE_ED_MAIN_H

