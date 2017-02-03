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

/////////////////////////////////////////////////////////////////////////
// lset.h - level set handling
// A level set contains the definitions of terrain and unit types as
// well as the corresponding graphics and sound data.
/////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDE_LSET_H
#define _INCLUDE_LSET_H

#include <string>

#include "gamedefs.h"
#include "sound.h"
#include "surface.h"
#include "lang.h"

#define FID_UNITSET       MakeID('U','S','E','T')
#define FID_TERRAINSET    MakeID('T','S','E','T')

// terrain type definitions
class TerrainType {
public:
  int Load( MemBuffer &file );

  unsigned short tt_type;       // one (or more) of the TT_ defined above
  unsigned short tt_image;
  signed char tt_att_mod;       // attack modifier (-20% to +20%)
  signed char tt_def_mod;       // defence modifier (-20% to +20%)
  signed char tt_move;          // cost to cross hex type
  char reserved;
  unsigned long tt_color;       // color shown on the overview map display
};

class UnitSet;

class UnitType {
public:
  UnitType( void ) {}
  int Load( MemBuffer &file, const UnitSet *set );
  void SetName( const char *name ) { ut_name = name; }

  unsigned short Terrain( void ) const { return ut_terrain; }
  unsigned short Image( void ) const { return ut_image; }
  unsigned short Flags( void ) const { return ut_flags; }
  unsigned char Speed( void ) const { return ut_moves; }
  unsigned char Weight( void ) const { return ut_weight; }
  unsigned char Armour( void ) const { return ut_defence; }
  unsigned char Firepower( unsigned long target_type ) const;
  bool IsInFOF( unsigned short dist, unsigned long target_type ) const;
  unsigned char MaxFOF( unsigned long target_type ) const;
  unsigned char MinFOF( unsigned long target_type ) const;
  unsigned char Cost( void ) const { return ut_build; }
  unsigned char Slots( void ) const { return ut_trans_slots; }
  unsigned char MinWeight( void ) const { return ut_trans_min_weight; }
  unsigned char MaxWeight( void ) const { return ut_trans_max_weight; }
  const char *Name( void ) const { return ut_name; }
  SoundEffect *MoveSound( void ) const { return ut_snd_move; }
  SoundEffect *FireSound( void ) const { return ut_snd_fire; }
  unsigned char ID( void ) const { return ut_typeid; }
  signed char Portrait( void ) const { return ut_portrait; }

protected:
  unsigned short ut_terrain;		// wherever it may roam...
  unsigned short ut_image;
  unsigned short ut_flags;
  unsigned char ut_moves;
  unsigned char ut_weight;		// used by transports
  unsigned char ut_defence;		// defensive strength
  unsigned char ut_pow_ground;		// offensive strengths
  unsigned char ut_pow_ship;
  unsigned char ut_pow_air;
  unsigned char ut_min_range_ground;
  unsigned char ut_max_range_ground;
  unsigned char ut_min_range_ship;
  unsigned char ut_max_range_ship;
  unsigned char ut_min_range_air;
  unsigned char ut_max_range_air;
  unsigned char ut_build;		// resources needed to build this unit type
  unsigned char ut_trans_slots;		// max number of units the transport can carry
  unsigned char ut_trans_min_weight;	// min weight of units the transport can carry
  unsigned char ut_trans_max_weight;	// max weight of units the transport can carry
  unsigned char ut_typeid;
  signed char ut_portrait;		// portrait gfx index
  SoundEffect *ut_snd_move;
  SoundEffect *ut_snd_fire;
  const char *ut_name;
};


#define DEFAULT_TILE_WIDTH    32
#define DEFAULT_TILE_HEIGHT   28

// generic set of images
class TileSet {
public:
  TileSet( void ) : num_tiles(0) {}
  virtual ~TileSet( void ) {}

  virtual int Load( MemBuffer &file, const char *setname );

  unsigned short TileWidth( void ) const { return DEFAULT_TILE_WIDTH; }
  unsigned short TileHeight( void ) const { return DEFAULT_TILE_HEIGHT; }
  unsigned short TileShiftX( void ) const { return 9; }
  unsigned short TileShiftY( void ) const { return 14; }

  void DrawTile( unsigned short n, Surface *dest,
                 short px, short py, const Rect &clip ) const;
  const string &GetName( void ) const { return name; }
  unsigned short NumTiles( void ) const { return num_tiles; }

protected:
  unsigned short num_tiles;
  Surface tiles;
  string name;
};

class UnitSet : public TileSet {
public:
  UnitSet( void );
  ~UnitSet( void );

  int Load( MemBuffer &file, const char *setname );

  const UnitType *GetUnitInfo( unsigned short utid ) const;
  SoundEffect *GetSound( unsigned short sfxid ) const;
  Surface *GetPortrait( unsigned char ptid ) const { return &portraits[ptid]; }
  const char *UnitName( unsigned short utid ) const
             { return unit_names.GetMsg( utid ); }

  void SetLocale( const string &lang );

protected:
  int LoadUnitTypes( MemBuffer &file );
  int LoadSfx( MemBuffer &file );

  unsigned short num_sfx;

  UnitType *ut;
  SoundEffect **sfx;
  Surface *portraits;

  Locale unit_names;
};

class TerrainSet : public TileSet {
public:
  TerrainSet( void ) : tt(0) {}
  ~TerrainSet( void );

  int Load( MemBuffer &file, const char *setname );

  unsigned short GetTerrainClassID( unsigned char num ) const { return classid[num]; }
  const TerrainType *GetTerrainInfo( unsigned short ttid ) const;

  void DrawFog( Surface *dest, short px, short py, const Rect &clip ) const;
  const Surface &HexMask( void ) const { return fog; }

protected:
  int LoadTerrainTypes( MemBuffer &file );

  TerrainType *tt;
  Surface fog;
  unsigned short classid[10]; // IDs of the 10 most important terrain classes;
                              // determines which images are shown in unit info dialog
};

#endif	/* _INCLUDE_LSET_H */

