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

////////////////////////////////////////////////////////////////////////
// building.cpp
////////////////////////////////////////////////////////////////////////

#include "building.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Building::Building
// DESCRIPTION: Create a new building.
// PARAMETERS : pos  - position on map
//              id   - building indentifier
//              pid  - identifier of controlling player
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Building::Building( const Point &pos, unsigned short id,
                    unsigned char pid ) :
          MapObject( MO_BUILDING ) {
  b_id = id;
  b_pos = pos;
  b_flags = BLD_WORKSHOP;
  b_crystals = 0;
  b_maxcrystals = 1000;
  b_uprod = 0;
  b_cprod = 0;
  b_pid = pid;
  b_minweight = 0;
  b_maxweight = 99;
  b_name_id = -1;
  b_name = 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Building::Load
// DESCRIPTION: Load a building definition from a file.
// PARAMETERS : file - data file descriptor
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Building::Load( MemBuffer &file ) {
  b_id = file.Read16();
  b_pos.x = file.Read16();
  b_pos.y = file.Read16();
  b_flags = file.Read16();
  b_crystals = file.Read16();
  b_maxcrystals = file.Read16();
  b_uprod = file.Read32();

  b_cprod = file.Read8();
  b_pid = file.Read8();
  b_minweight = file.Read8();
  b_maxweight = file.Read8();
  b_name_id = file.Read8();
  b_name = 0;
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Building::Save
// DESCRIPTION: Save the building to a file.
// PARAMETERS : file - save file descriptor
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Building::Save( MemBuffer &file ) const {
  file.Write16( b_id );
  file.Write16( b_pos.x );
  file.Write16( b_pos.y );
  file.Write16( b_flags );
  file.Write16( b_crystals );
  file.Write16( b_maxcrystals );
  file.Write32( b_uprod );

  file.Write8( b_cprod );
  file.Write8( b_pid );
  file.Write8( b_minweight );
  file.Write8( b_maxweight );
  file.Write8( b_name_id );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Building::Export
// DESCRIPTION: Save the building to a plain text file.
// PARAMETERS : file - save file descriptor
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Building::Export( ofstream &file, const UnitSet *uset ) const {
  file << "[building]\n";
  file << "name = " << (short)b_name_id << '\n';
  file << "player = " << (b_pid+1)%3 << '\n';
  file << "id = " << b_id << '\n';
  file << "pos = " << b_pos.x << '/' << b_pos.y << '\n';

  if ( (b_flags & BLD_WORKSHOP) != 0 ) file << "type = workshop\n";
  if ( (b_flags & BLD_FACTORY) != 0 ) {
    file << "type = factory\n";

    for ( int i = 0; i < uset->NumTiles(); ++i ) {
      if ( ((1<<i) & UnitProduction()) != 0 )
        file << "factory = " << uset->GetUnitInfo(i)->Name() << '\n';
    }
  }
  if ( IsMine() )
    file << "mining = " << (short)CrystalProduction() << '\n';

  file << "crystals = " << b_crystals << '\n';
  file << "capacity = " << b_maxcrystals << '\n';
  file << "minweight = " << (short)b_minweight << '\n';
  file << "maxweight = " << (short)b_maxweight << '\n' << '\n';

  return 0;
}

