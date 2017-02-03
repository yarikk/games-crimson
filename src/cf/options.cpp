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
// options.cpp
////////////////////////////////////////////////////////////////////////

#include "options.h"
#include "lang.h"

#define CF_DEFAULT_PORT   9999
#define CF_DEFAULT_SERVER "localhost"

////////////////////////////////////////////////////////////////////////
// NAME       : Options::Options
// DESCRIPTION: Initialize default options.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Options::Options( void ) : gametype(GTYPE_AI), show_damage(true), replay(true),
         quick_replay(false), campaign(false), language(CF_LANG_DEFAULT),
         server(CF_DEFAULT_SERVER), server_port(CF_DEFAULT_PORT),
         local_port(CF_DEFAULT_PORT) {
  for (int i = 0; i < KEYBIND_COUNT; ++i)
    keymap[i] = SDLK_UNKNOWN;

#ifdef V43
  keymap[KEYBIND_GAME_MENU] = SDLK_TAB;
  keymap[KEYBIND_UNIT_MENU] = SLK_SPACE;
  keymap[KEYBIND_UNIT_SELECT] = SDLK_RETURN;
#else
  // some default keys
  keymap[KEYBIND_GAME_MENU] = SDLK_F1;
  keymap[KEYBIND_UNIT_NEXT] = SDLK_F2;
  keymap[KEYBIND_UNIT_SELECT] = SDLK_SPACE;
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Options::SetRemoteName
// DESCRIPTION: Set name of server to connect to.
// PARAMETERS : name - server name or IP address (may be NULL)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Options::SetRemoteName( const char *name ) {
  if ( name )
    server.assign( name );
  else
    server.erase();
}

////////////////////////////////////////////////////////////////////////
// NAME       : Options::GetRemoteName
// DESCRIPTION: Get name of server to connect to.
// PARAMETERS : -
// RETURNS    : name or IP address of server or NULL if not available
////////////////////////////////////////////////////////////////////////

const char *Options::GetRemoteName( void ) const {
  if ( server.empty() )
    return NULL;
 return server.c_str();
}
