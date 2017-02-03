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

/////////////////////////////////////////////////////////////////////
// player.h
/////////////////////////////////////////////////////////////////////

#ifndef _INCLUDE_PLAYER_H
#define _INCLUDE_PLAYER_H

#include "fileio.h"
#include "gamedefs.h"
#include "color.h"

#define MODE_IDLE	1   // no unit selected
#define MODE_BUSY	2   // unit selected
#define MODE_DIG	3   // for pioneers
#define MODE_CONSTRUCTION	4   // for depot builders
#define MODE_SWEEP	5   // for mine-sweepers

class Player {
public:
  Player( void ) : p_name(0) {}
  int Load( MemBuffer &file );
  int Save( MemBuffer &file ) const;

  signed char Briefing( void ) const { return p_briefing; }
  unsigned char ID( void ) const { return p_id; }
  unsigned char Mode( void ) const { return p_mode; }
  const char *Name( void ) const { return p_name; }
  signed char NameID( void ) const { return p_name_id; }
  const char *Password( void ) const;
  unsigned char Type( void ) const { return p_type; }
  bool IsHuman( void ) const { return p_type == HUMAN; }
  bool IsRemote( void ) const { return p_remote; }
  bool IsInteractive( void ) const { return IsHuman() && !IsRemote(); }

  void SetBriefing( signed char brief ) { p_briefing = brief; }
  void SetMode( unsigned char mode ) { p_mode = mode; }
  void SetType( unsigned char type ) { p_type = type; }
  void SetRemote( bool remote ) { p_remote = remote; }
  void SetPassword( const char *password );
  void SetName( const char *name ) { p_name = name; }

  unsigned char Success( signed char success ) { p_success += success; return p_success; }
  unsigned short Units( short delta );

  const Color &LightColor( void ) const { return p_col_light; }
  const Color &DarkColor( void ) const { return p_col_dark; }

private:
  unsigned char p_id;
  unsigned char p_mode;
  unsigned char p_type;       // COMPUTER or HUMAN
  bool p_remote;              // remote player in network game

  unsigned short p_units;

  unsigned char p_success;    // if p_success == 100 the level is completed
  signed char p_briefing;

  signed char p_name_id;
  const char *p_name;
  string p_password;

  Color p_col_light;
  Color p_col_dark;
};

#endif	/* _INCLUDE_PLAYER_H */

