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
// player.cpp
////////////////////////////////////////////////////////////////////////

#include <string.h>

#include "player.h"
#include "strutil.h"
#include "gamedefs.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Player::Load
// DESCRIPTION: Load player information from a file.
// PARAMETERS : file - data file descriptor
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

int Player::Load( MemBuffer &file ) {
  p_id = file.Read8();
  p_type = file.Read8();
  p_success = file.Read8();
  p_briefing = file.Read8();
  p_name_id = file.Read8();

  unsigned char len = file.Read8();
  p_password = StringUtil::crypt( file.ReadS(len) );

  p_col_light.r = file.Read8();
  p_col_light.g = file.Read8();
  p_col_light.b = file.Read8();
  p_col_dark.r = file.Read8();
  p_col_dark.g = file.Read8();
  p_col_dark.b = file.Read8();

  p_mode = MODE_IDLE;
  p_remote = false;
  p_units = 0;
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Player::Save
// DESCRIPTION: Save player information to a file.
// PARAMETERS : file - data file descriptor
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int Player::Save( MemBuffer &file ) const {
  file.Write8( p_id );
  file.Write8( p_type );
  file.Write8( p_success );
  file.Write8( p_briefing );
  file.Write8( p_name_id );
  file.Write8( p_password.size() );
  file.WriteS( StringUtil::crypt( p_password ) );

  file.Write8( p_col_light.r );
  file.Write8( p_col_light.g );
  file.Write8( p_col_light.b );
  file.Write8( p_col_dark.r );
  file.Write8( p_col_dark.g );
  file.Write8( p_col_dark.b );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Player::Password
// DESCRIPTION: Get the player's password.
// PARAMETERS : -
// RETURNS    : pointer to password, or NULL if no password set
////////////////////////////////////////////////////////////////////////

const char *Player::Password( void ) const {
  if ( !p_password.empty() ) return p_password.c_str();
  return NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Player::SetPassword
// DESCRIPTION: Set the player's password.
// PARAMETERS : password - new player password; the function does not
//                         check whether it fits into the password
//                         buffer; may be NULL to erase the player's
//                         current password
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Player::SetPassword( const char *password ) {
  if ( password == NULL ) p_password.erase();
  else p_password.assign( password );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Player::Units
// DESCRIPTION: Increase or decrease the player's unit count.
// PARAMETERS : delta - change in number of units
// RETURNS    : number of units after change
////////////////////////////////////////////////////////////////////////

unsigned short Player::Units( short delta ) {
  p_units += delta;
  return p_units;
}

