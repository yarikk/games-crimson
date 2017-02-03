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

/////////////////////////////////////////////////////////////////////////
// unit.cpp
////////////////////////////////////////////////////////////////////////

#include "unit.h"
#include "hexsup.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::Unit
// DESCRIPTION: Create a new unit.
// PARAMETERS : type - pointer to a unit type definition
//              pid  - owner identifier
//              id   - unique unit identifier
//              pos  - position on map
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Unit::Unit( const UnitType *type, unsigned char pid, unsigned short id, const Point &pos ) :
      MapObject( MO_UNIT ) {
  u_type = type;

  u_pos = pos;
  u_flags = type->Flags();
  u_id = id;

  u_facing = (pid == PLAYER_TWO) ? SOUTH : NORTH;
  u_group = MAX_GROUP_SIZE;
  u_xp = 0;

  u_pid = pid;
  u_crystals = 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::Unit
// DESCRIPTION: Load a unit from a data file.
// PARAMETERS : file - descriptor of an open data file
//              type - pointer to a unit type definition
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Unit::Unit( MemBuffer &file, const UnitType *type ) :
      MapObject( MO_UNIT ) {
  u_pid = file.Read8();
  u_pos.x = file.Read16();
  u_pos.y = file.Read16();
  u_flags = file.Read32();
  u_id = file.Read16();
  u_facing = file.Read8();
  u_group = file.Read8();
  u_xp = file.Read8();

  file.Read32();   // ignore target information

  if ( IsTransport() ) u_crystals = file.Read16();
  else u_crystals = 0;

  u_type = type;
}


////////////////////////////////////////////////////////////////////////
// NAME       : Unit::Save
// DESCRIPTION: Save the unit to a data file.
// PARAMETERS : file - descriptor of the save file
// RETURNS    : 0 on succes, non-zero on error
////////////////////////////////////////////////////////////////////////

int Unit::Save( MemBuffer &file ) const {
  file.Write8( u_type->ID() );
  file.Write8( u_pid );

  file.Write16( u_pos.x );
  file.Write16( u_pos.y );
  file.Write32( u_flags );
  file.Write16( u_id );

  file.Write8( u_facing );
  file.Write8( u_group );
  file.Write8( u_xp );

  file.Write16( (unsigned short)-1 );  // target information
  file.Write16( (unsigned short)-1 );

  if ( IsTransport() ) file.Write16( u_crystals );

  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Unit::Export
// DESCRIPTION: Save the unit to a plain text file.
// PARAMETERS : file - descriptor of the save file
// RETURNS    : 0 on succes, non-zero on error
////////////////////////////////////////////////////////////////////////

int Unit::Export( ofstream &file ) const {
  file << "[unit]\n";
  file << "type = " << Name() << '\n';
  file << "player = " << (u_pid+1)%3 << '\n';
  file << "id = " << u_id << '\n';
  file << "pos = " << u_pos.x << '/' << u_pos.y << '\n';

  if ( u_group < MAX_GROUP_SIZE )
    file << "size = " << (short)u_group << '\n';

  if ( XPLevel() != 0 )
    file << "xp = " << (short)XPLevel() << '\n';

  if ( !IsSheltered() &&
       (((u_pid == PLAYER_ONE) && (u_facing != NORTH)) ||
       ((u_pid == PLAYER_TWO) && (u_facing != SOUTH))) )
    file << "face = " << (short)u_facing << '\n';

  if ( IsTransport() && (u_crystals > 0) )
    file << "crystals = " << u_crystals << '\n';

  file << '\n';
  return 0;
}

