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
// event.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_EVENT_H
#define _INCLUDE_EVENT_H

#include "player.h"
#include "gamedefs.h"
#include "textbox.h"

class Event : public Node {
public:
  short Load( MemBuffer &file );
  int Save( MemBuffer &file ) const;

  bool Check( void );
  void Execute( class View *view );
  void Discard( void );
  bool Discarded( void ) const { return (e_flags & EFLAG_DISCARDED) != 0; }

  unsigned char ID( void ) const { return e_id; }
  unsigned char Trigger( void ) const { return e_trigger; }
  void SetPlayer( Player &p ) { e_player = &p; }

  short GetData( unsigned short index ) const { return e_data[index]; }
  void SetData( unsigned short index, short value )
       { e_data[index] = value; }
  short GetTData( unsigned short index ) const { return e_tdata[index]; }
  void SetTData( unsigned short index, short value )
       { e_tdata[index] = value; }

private:
  bool Disabled( void ) const { return (e_flags & EFLAG_DISABLED) != 0; }
  void SetFlags( unsigned short flags ) { e_flags |= flags; }
  void UnsetFlags( unsigned short flags ) { e_flags &= (~flags); }
  void ToggleFlags( unsigned short flags ) { e_flags ^= flags; }

  void DisplayMessage( Player *p, short msg, short title, class Mission *m,
                       bool show, class View *view ) const;

  bool CheckTrigger( void );
  bool CheckDependencies( TLWList &deps );
  struct Point GetFocus( void ) const;

  unsigned char e_id;
  unsigned char e_type;
  unsigned char e_trigger;
  signed char e_depend;
  signed char e_discard;

  short e_tdata[3];             // trigger data
  short e_data[3];              // event data

  short e_title;
  short e_message;
  unsigned short e_flags;

  Player *e_player;
};

#endif	/* _INCLUDE_EVENT_H */

