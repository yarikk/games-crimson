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
// lset.cpp -- level set handling
////////////////////////////////////////////////////////////////////////

#include "lset.h"
#include "fileio.h"
#include "misc.h"

#define FOG_ALPHA 128
#define IMG_FOG   1

////////////////////////////////////////////////////////////////////////
// NAME       : TileSet::Load
// DESCRIPTION: Load a tile set from a file. This only reads the image
//              surface(s) and associated data.
// PARAMETERS : file    - file descriptor
//              setname - name of the requested tile set. This is
//                        actually only used when saving a mission to
//                        a file
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int TileSet::Load( MemBuffer &file, const char *setname ) {

  int rc = tiles.LoadImageData( file, true );
  if ( !rc ) {
    tiles.DisplayFormat();
    name.assign( setname );
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : TileSet::DrawTile
// DESCRIPTION: Draw a tile image to a surface.
// PARAMETERS : n    - image number
//              dest - destination surface
//              px   - horizontal offset on surface
//              py   - vertical offset on surface
//              clip - clipping rectangle
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TileSet::DrawTile( unsigned short n, Surface *dest,
                        short px, short py, const Rect &clip ) const {
  // set up destination
  Rect dstrect( px, py, TileWidth(), TileHeight() );
  if ( dstrect.x + dstrect.w < clip.x ) return;
  if ( dstrect.y + dstrect.h < clip.y ) return;
  if ( dstrect.x >= clip.x + clip.w ) return;
  if ( dstrect.y >= clip.y + clip.h ) return;

  // set up source
  Rect srcrect;
  unsigned short gfx_per_line = tiles.Width() / TileWidth();
  srcrect.x = (n % gfx_per_line) * TileWidth();
  srcrect.y = (n / gfx_per_line) * TileHeight();
  srcrect.w = dstrect.w;
  srcrect.h = dstrect.h;

  // clip and blit to surface
  dstrect.ClipBlit( srcrect, clip );
  tiles.LowerBlit( dest, srcrect, dstrect.x, dstrect.y );
}


////////////////////////////////////////////////////////////////////////
// NAME       : UnitSet::UnitSet
// DESCRIPTION: Initialize a new unit set.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

UnitSet::UnitSet( void ) {
  num_sfx = 0;
  ut = NULL;
  sfx = NULL;
  portraits = NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitSet::~UnitSet
// DESCRIPTION: Destroy the currently loaded unit set.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

UnitSet::~UnitSet( void ) {
  delete [] ut;
  if ( sfx ) {
    for ( int i = 0; i < num_sfx; ++i ) delete sfx[i];
    delete [] sfx;
  }
  delete [] portraits;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitSet::GetUnitInfo
// DESCRIPTION: Retrieve information about a unit type by its ID.
// PARAMETERS : utid - unit type ID
// RETURNS    : pointer to unit info or NULL if ID is invalid
////////////////////////////////////////////////////////////////////////

const UnitType *UnitSet::GetUnitInfo( unsigned short utid ) const {
  if ( utid < num_tiles ) return &ut[utid];
  return NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitSet::GetSound
// DESCRIPTION: Get a sound effect by its ID.
// PARAMETERS : sfxid - effect ID
// RETURNS    : pointer to effect or NULL if ID is invalid
////////////////////////////////////////////////////////////////////////

SoundEffect *UnitSet::GetSound( unsigned short sfxid ) const {
  if ( sfxid < num_sfx ) return sfx[sfxid];
  return NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitSet::Load
// DESCRIPTION: Load a unit set from a file.
// PARAMETERS : file    - file descriptor
//              setname - name of the requested unit set. This is
//                        actually only used when saving a mission to
//                        a file
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int UnitSet::Load( MemBuffer &file, const char *setname ) {
  int rc = -1;

  if ( file.Read32() == FID_UNITSET ) {
    num_tiles = file.Read16();
    num_sfx = file.Read8();

    if ( !LoadSfx( file ) && !LoadUnitTypes( file ) &&
         !TileSet::Load( file, setname ) ) {
      unsigned char num_portraits = file.Read8();
      if ( num_portraits ) {
        portraits = new Surface [num_portraits];

        for ( int i = 0; i < num_portraits; ++i ) {
          if ( portraits[i].LoadImageData( file ) ) return -1;
          portraits[i].DisplayFormat();
        }
      }

      rc = 0;
    }
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitSet::LoadUnitTypes
// DESCRIPTION: Load unit type definitions from a unit set file.
// PARAMETERS : file - descriptor of the unit set file
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int UnitSet::LoadUnitTypes( MemBuffer &file ) {
  ut = new UnitType [num_tiles];
  if ( !ut ) return -1;

  int rc = 0;

  for ( int i = 0; i < num_tiles; ++i ) {
    if ( ut[i].Load( file, this ) ) {
      rc = -1;
      break;
    }
  }

  // load unit names in different languages
  if ( rc == 0 )
    rc = unit_names.Load( file );

  if ( rc == -1 ) {
      delete [] ut;
      ut = NULL;
      num_tiles = 0;
      return -1;
  }

  SetLocale( CF_LANG_DEFAULT );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitSet::LoadSfx
// DESCRIPTION: Load unit sound effects from a unit set file.
// PARAMETERS : file - descriptor of the unit set file
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int UnitSet::LoadSfx( MemBuffer &file ) {
  if ( num_sfx ) {
    string sfxd( get_sfx_dir() ), sfxname;
    unsigned char len;

    sfx = new SoundEffect * [num_sfx];
    if ( !sfx ) return -1;

    for ( int i = 0; i < num_sfx; ++i ) {
      len = file.Read8();
      sfxname = file.ReadS( len );
      sfx[i] = new SoundEffect( (sfxd + sfxname).c_str() );
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitSet::SetLocale
// DESCRIPTION: Set the language to use.
// PARAMETERS : lang - locale identifier
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void UnitSet::SetLocale( const string &lang ) {
  unit_names.SetDefaultLanguage( lang );

  for ( int i = 0; i < num_tiles; ++i )
    ut[i].SetName( unit_names.GetMsg( i ) );
}


////////////////////////////////////////////////////////////////////////
// NAME       : TerrainType::Load
// DESCRIPTION: Load a terrain type definition from a file.
// PARAMETERS : file - file descriptor
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int TerrainType::Load( MemBuffer &file ) {
  tt_type = file.Read16();
  tt_image = file.Read16();
  tt_att_mod = file.Read8();
  tt_def_mod = file.Read8();
  tt_move = file.Read8();
  tt_color = file.Read32();
  return 0;
}


////////////////////////////////////////////////////////////////////////
// NAME       : TerrainSet::~TerrainSet
// DESCRIPTION: Destroy the currently loaded terrain set.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

TerrainSet::~TerrainSet( void ) {
  delete [] tt;
}

////////////////////////////////////////////////////////////////////////
// NAME       : TerrainSet::Load
// DESCRIPTION: Load a terrain set from a file.
// PARAMETERS : file    - file descriptor
//              setname - name of the requested terrain set. This is
//                        actually only used when saving a mission to
//                        a file
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int TerrainSet::Load( MemBuffer &file, const char *setname ) {
  int rc = -1;

  if ( file.Read32() == FID_TERRAINSET ) {
    num_tiles = file.Read16();

    for ( int i = 0; i < 10; ++i )
      classid[i] = file.Read16();

    if ( !LoadTerrainTypes( file ) && !TileSet::Load( file, setname ) )
      // create the fog surface. This is kept separate from the tiles
      // surface so that we don't need to modify the alpha value each
      // time we want fog
      fog.Create( TileWidth(), TileHeight(), 16, SDL_SWSURFACE );
      fog.SetAlpha( FOG_ALPHA, SDL_SRCALPHA );
      fog.SetColorKey( Color(CF_COLOR_WHITE) );
      fog.DisplayFormat();
      fog.Flood( Color(CF_COLOR_WHITE) );
      DrawTile( IMG_FOG, &fog, 0, 0, fog );

      rc = 0;
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : TerrainSet::GetTerrainInfo
// DESCRIPTION: Retrieve information about a terrain type by its ID.
// PARAMETERS : ttid - terrain type ID
// RETURNS    : pointer to terrain info or NULL if ID is invalid
////////////////////////////////////////////////////////////////////////

const TerrainType *TerrainSet::GetTerrainInfo( unsigned short ttid ) const {
  if ( ttid < num_tiles ) return &tt[ttid];
  return NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : TerrainSet::LoadTerrainTypes
// DESCRIPTION: Load terrain type definitions from a terrain set file.
// PARAMETERS : file - descriptor of the terrain set file
// RETURNS    : 0 on success, non-zero on error
////////////////////////////////////////////////////////////////////////

int TerrainSet::LoadTerrainTypes( MemBuffer &file ) {
  tt = new TerrainType [num_tiles];
  if ( !tt ) return -1;

  for ( int i = 0; i < num_tiles; ++i ) {
    if ( tt[i].Load( file ) ) {
      delete [] tt;
      tt = NULL;
      num_tiles = 0;
      return -1;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : TerrainSet::DrawFog
// DESCRIPTION: Draw the fog hex to a surface.
// PARAMETERS : dest - destination surface
//              px   - horizontal offset on surface
//              py   - vertical offset on surface
//              clip - clipping rectangle
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TerrainSet::DrawFog( Surface *dest, short px, short py, const Rect &clip ) const {
  fog.Blit( dest, fog, px, py );
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitType::Load
// DESCRIPTION: Load a unit type definition from a file.
// PARAMETERS : file - data source descriptor
//              set  - level set
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int UnitType::Load( MemBuffer &file, const UnitSet *set ) {
  ut_snd_move = ut_snd_fire = NULL;
  ut_terrain = file.Read16();
  ut_image = file.Read16();
  ut_flags = file.Read16();
  file.Read( &ut_moves, 13 );

  // set sound effects
  unsigned char sfx[2];
  sfx[0] = file.Read8();
  sfx[1] = file.Read8();
  if ( sfx[0] != UT_NO_SOUND ) ut_snd_move = set->GetSound( sfx[0] );
  if ( sfx[1] != UT_NO_SOUND ) ut_snd_fire = set->GetSound( sfx[1] );

  file.Read( &ut_trans_slots, 5 );
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitType::Firepower
// DESCRIPTION: Get firepower of this unit type against a specific
//              target unit class.
// PARAMETERS : target_type - target class (U_*)
// RETURNS    : firepower
////////////////////////////////////////////////////////////////////////

unsigned char UnitType::Firepower( unsigned long target_type ) const {
  unsigned char fp = 0;

  if ( target_type == U_GROUND ) fp = ut_pow_ground;
  else if ( target_type == U_SHIP ) fp = ut_pow_ship;
  else if ( target_type == U_AIR ) fp = ut_pow_air;
  return fp;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitType::IsInFOF
// DESCRIPTION: Check whether a unit of a certain class at a given
//              distance is still in the field of fire.
// PARAMETERS : dist        - distance to target
//              target_type - target unit class (U_*)
// RETURNS    : TRUE if unit is in FOF, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool UnitType::IsInFOF( unsigned short dist, unsigned long target_type ) const {
  if ( target_type == U_GROUND )
    return ( (ut_min_range_ground <= dist) && (ut_max_range_ground >= dist) );

  else if ( target_type == U_SHIP )
    return ( (ut_min_range_ship <= dist) && (ut_max_range_ship >= dist) );

  else if ( target_type == U_AIR )
    return ( (ut_min_range_air <= dist) && (ut_max_range_air >= dist) );

  return false;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitType::MaxFOF
// DESCRIPTION: Get maximum weapon range against a specific unit class.
// PARAMETERS : target_type - target class (U_*)
// RETURNS    : maximum weapon range
////////////////////////////////////////////////////////////////////////

unsigned char UnitType::MaxFOF( unsigned long target_type ) const {
  unsigned char range = 0;

  if ( target_type == U_GROUND ) range = ut_max_range_ground;
  else if ( target_type == U_SHIP ) range = ut_max_range_ship;
  else if ( target_type == U_AIR ) range = ut_max_range_air;
  return range;
}

////////////////////////////////////////////////////////////////////////
// NAME       : UnitType::MinFOF
// DESCRIPTION: Get minimum weapon range against a specific unit class.
// PARAMETERS : target_type - target class (U_*)
// RETURNS    : minimum weapon range
////////////////////////////////////////////////////////////////////////

unsigned char UnitType::MinFOF( unsigned long target_type ) const {
  unsigned char range = 1;

  if ( target_type == U_GROUND ) range = ut_min_range_ground;
  else if ( target_type == U_SHIP ) range = ut_min_range_ship;
  else if ( target_type == U_AIR ) range = ut_min_range_air;
  return range;
}

