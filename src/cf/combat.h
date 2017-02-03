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
// combat.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_COMBAT_H
#define _INCLUDE_COMBAT_H

#include "map.h"

#include "list.h"
#include "fileio.h"

#include "window.h"
#include "button.h"

class Mission;

class Combat : public Node {
public:
  Combat( void ) {}
  Combat( Unit *att, Unit *def );
  int Load( MemBuffer &file, Mission &mission );
  int Save( MemBuffer &file ) const;

  Unit *GetAttacker( void ) const { return c_att; }
  Unit *GetDefender( void ) const { return c_def; }

  void CalcModifiers( const Map &map );
  Point CalcResults( void );
  Point CalcResults( unsigned char atthits, unsigned char defhist ) const;

private:
  Unit *c_att;       // attacker
  Unit *c_def;       // defender
  signed char aamod;
  signed char admod;
  signed char damod; // defender only gets terrain bonus for attacks
  signed char ddmod;
};

#endif	/* _INCLUDE_COMBAT_H */

