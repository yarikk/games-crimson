/* mkunitset -- create a unit set for Crimson Fields
   Copyright (C) 2000-2007 Jens Granseuer

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


/* create a data set for CF, containing graphics and definitions for units

   level designers can specify custom data files to adjust the
   graphics and/or units to their likes
*/

#include <algorithm>
using namespace std;
#include "SDL.h"

#include "fileio.h"
#include "parser.h"
#include "lset.h"
#include "mksurface.h"

#ifdef _MSC_VER
// SDL_Main linkage destroys the command line in VS8
#undef main
#endif

struct UnitsetHeader {
  string images;
  vector <string> sfx;
  vector <string> portraits;
};

class EdUnitType : public UnitType {
public:
  void SetTerrain( unsigned short t ) { ut_terrain = t; }
  void AddTerrain( unsigned short t ) { ut_terrain |= t; }
  void SetImage( unsigned short i ) { ut_image = i; }
  void SetFlags( unsigned short f ) { ut_flags = f; }
  void AddFlags( unsigned short f ) { ut_flags |= f; }
  void SetMoves( unsigned char m ) { ut_moves = m; }
  void SetWeight( unsigned char w ) { ut_weight = w; }
  void SetDefence( unsigned char d ) { ut_defence = d; }
  void SetPowerGround( unsigned char p ) { ut_pow_ground = p; }
  void SetPowerShip( unsigned char p ) { ut_pow_ship = p; }
  void SetPowerAir( unsigned char p ) { ut_pow_air = p; }
  void SetMinRangeGround( unsigned char r ) { ut_min_range_ground = r; }
  void SetMaxRangeGround( unsigned char r ) { ut_max_range_ground = r; }
  void SetMinRangeShip( unsigned char r ) { ut_min_range_ship = r; }
  void SetMaxRangeShip( unsigned char r ) { ut_max_range_ship = r; }
  void SetMinRangeAir( unsigned char r ) { ut_min_range_air = r; }
  void SetMaxRangeAir( unsigned char r ) { ut_max_range_air = r; }
  void SetCost( unsigned char c ) { ut_build = c; }
  void SetTransSlots( unsigned char s ) { ut_trans_slots = s; }
  void SetTransMinWeight( unsigned char w ) { ut_trans_min_weight = w; }
  void SetTransMaxWeight( unsigned char w ) { ut_trans_max_weight = w; }
  void SetPortrait( signed char p ) { ut_portrait = p; }

private:
  map<string, string> names;
};

class UnitsetHandler : public KeyValueSectionHandler {
public:
  UnitsetHandler( UnitsetHeader &head ) : info(head) {}

  int ParseSection( ifstream &in, const string *opt, unsigned long &line );

private:
  UnitsetHeader &info;
};

class UnitHandler : public KeyValueSectionHandler {
public:
  UnitHandler( UnitsetHeader &head, vector<EdUnitType> &units, map<string, Language> &names) :
          units(units), names(names), info(head) {}

  int ParseSection( ifstream &in, const string *opt, unsigned long &line );

private:
  int ParsePower( const string &s, const string &type, EdUnitType &t ) const;
  string ParseOpt( const string &s ) const;

  vector<EdUnitType> &units;
  map<string, Language> &names;
  UnitsetHeader &info;
};

class UnitsetCallback : public SectionParsedCallback {
public:
  void SectionParsed( const string &section,
       SectionHandler &handler, CFParser &parser );
};


int main( int argc, char *argv[] ) {
  int status;
  unsigned short i;

  if ( (argc < 3) || (argc > 4) ) {
    cerr << "Invalid number of arguments\n"
            "Usage: " << argv[0] << " <datafile> <outfile> [<gfxdir>]" << endl;
    exit(-1);
  }

  if ( SDL_Init(0) < 0 ) {
    cerr << "Couldn't init SDL: " << SDL_GetError() << endl;
    exit(-1);
  }
  atexit(SDL_Quit);

  status = 0;

  UnitsetHeader info;
  vector<EdUnitType> units;
  map<string, Language> names;
  CFParser parser;
  SectionHandler *h;
  UnitsetCallback cb;
  Locale locale;

  parser.AddHandler( "unitset", new UnitsetHandler( info ) );

  h = new UnitHandler( info, units, names );
  h->SetEnabled( false );
  parser.AddHandler( "unit", h );

  parser.SetCallback( &cb );
  status = parser.Parse( argv[1] );

  if ( status == 0 ) {
    // validate what we have parsed
    if ( info.images.size() == 0 ) {
      cerr << "Error: No images defined" << endl;
      status = -1;
    } else {
      if ( argc == 4 ) {
        string gfxdir( argv[3] );
        append_path_delim( gfxdir );
        info.images.insert( 0, gfxdir );
      }
      if ( !File::Exists( info.images ) ) {
        cerr << "Error: Could not find file " << info.images << endl;
        status = -1;
      }
    }

    if ( units.size() == 0 ) {
      cerr << "Error: No units defined" << endl;
      status = -1;
    }

    for ( map<string, Language>::iterator it = names.begin();
          it != names.end(); ++it ) {
      locale.AddLanguage( it->second );
    }
    if ( !locale.SetDefaultLanguage( CF_LANG_DEFAULT ) ) {
      cerr << "Error: Could not find info for default language (" << CF_LANG_DEFAULT << ")" << endl;
      status = -1;
    }
  }

  if ( status == 0 ) {
    File out( argv[2] );
    if ( !out.Open( "wb" ) ) {
      cerr << "Couldn't open output file " << argv[2] << endl;
      status = -1;
    } else {
      // write data file header
      out.Write32( FID_UNITSET );
      out.Write16( units.size() );

      // write sound file names
      out.Write8( info.sfx.size() );
      for ( i = 0; i < info.sfx.size(); ++i ) {
        out.Write8( info.sfx[i].size() );
        out.WriteS( info.sfx[i] );
      }

      // write unit data
      for ( i = 0; i < units.size(); ++i ) {
        out.Write16( units[i].Terrain() );
        out.Write16( units[i].Image() );
        out.Write16( units[i].Flags() );
        out.Write8( units[i].Speed() );
        out.Write8( units[i].Weight() );
        out.Write8( units[i].Armour() );
        out.Write8( units[i].Firepower( U_GROUND) );
        out.Write8( units[i].Firepower( U_SHIP ) );
        out.Write8( units[i].Firepower( U_AIR ) );
        out.Write8( units[i].MinFOF( U_GROUND ) );
        out.Write8( units[i].MaxFOF( U_GROUND ) );
        out.Write8( units[i].MinFOF( U_SHIP ) );
        out.Write8( units[i].MaxFOF( U_SHIP ) );
        out.Write8( units[i].MinFOF( U_AIR ) );
        out.Write8( units[i].MaxFOF( U_AIR) );
        out.Write8( units[i].Cost() );
        out.Write8( UT_NO_SOUND ); // move sound
        out.Write8( UT_NO_SOUND ); // fire sound
        out.Write8( units[i].Slots() );
        out.Write8( units[i].MinWeight() );
        out.Write8( units[i].MaxWeight() );
        out.Write8( i );
        out.Write8( units[i].Portrait() );
      }

      locale.Save( out );

      // load graphics
      MkSurface img;
      status = img.SaveImageData( info.images, out, true );

      // load optional unit portraits
      out.Write8( info.portraits.size() );
      for ( i = 0; (i <info.portraits.size()) && (status == 0); ++i ) {
        if ( argc == 4 ) {
          string gfxdir( argv[3] );
          append_path_delim( gfxdir );
          info.portraits[i].insert( 0, gfxdir );
        }
        status = img.SaveImageData( info.portraits[i], out, false );
      }

      out.Close();
    }
  }
  return status;
}

// unitset handler
int UnitsetHandler::ParseSection( ifstream &in, const string *opt, unsigned long &line ) {
  int rc = KeyValueSectionHandler::ParseSection( in, opt, line );

  if ( rc == 0 ) {
    info.images = "";

    for ( vector<pair<string, string> >::iterator it = pairs.begin();
          it != pairs.end(); ++it ) {
      string key = (*it).first;
      string val = (*it).second;

      if ( key == "icons" ) info.images = val;
      else if ( key == "name" ) /* ignore */;
      else {
        rc = 1;
        cerr << "Error near line " << line << ": Invalid keyword '" << key << "' in this context" << endl;
      }
    }
  }
  return rc;
}

// unit handler
int UnitHandler::ParseSection( ifstream &in, const string *opt, unsigned long &line ) {
  int rc = KeyValueSectionHandler::ParseSection( in, opt, line );

  if ( rc == 0 ) {
    bool named = false;

    EdUnitType t;
    t.SetTerrain( TT_ENTRANCE );
    t.SetImage( 0 );
    t.SetFlags( 0 );
    t.SetMoves( 0 );
    t.SetWeight( 0 );
    t.SetDefence( 0 );
    t.SetPowerGround( 0 );
    t.SetPowerShip( 0 );
    t.SetPowerAir( 0 );
    t.SetMinRangeGround( 1 );
    t.SetMaxRangeGround( 1 );
    t.SetMinRangeShip( 1 );
    t.SetMaxRangeShip( 1 );
    t.SetMinRangeAir( 1 );
    t.SetMaxRangeAir( 1 );
    t.SetCost( 0 );
    t.SetTransSlots( 0 );
    t.SetTransMinWeight( 0 );
    t.SetTransMaxWeight( 0 );
    t.SetPortrait( -1 );

    for ( vector<pair<string, string> >::iterator it = pairs.begin();
          it != pairs.end(); ++it ) {
      string key = (*it).first;
      string val = (*it).second;

      if ( key == "type" ) {
        if ( val == "ground" ) t.AddFlags( U_GROUND );
        else if ( val == "ship" ) t.AddFlags( U_SHIP );
        else if ( val == "aircraft" ) t.AddFlags( U_AIR );
        else if ( val == "trooper" ) t.AddFlags( U_CONQUER );
        else if ( val == "slow" ) t.AddFlags( U_SLOW );
        else if ( val == "mine" ) t.AddFlags( U_MINE );
        else if ( val == "medic" ) t.AddFlags( U_MEDIC );
        else if ( val == "sweeper" ) t.AddFlags( U_MINESWEEPER );
        else {
          rc = 1;
          cerr << "Error near line " << line << ": Invalid unit class '" << val << "'" << endl;
          break;
        }
      }
      else if ( key == "terrain" ) {
        if ( val == "road" ) t.AddTerrain( TT_ROAD );
        else if ( val == "plains" ) t.AddTerrain( TT_PLAINS );
        else if ( val == "forest" ) t.AddTerrain( TT_FOREST );
        else if ( val == "swamp" ) t.AddTerrain( TT_SWAMP );
        else if ( val == "mountains" ) t.AddTerrain( TT_MOUNTAINS );
        else if ( val == "shallowwater" ) t.AddTerrain( TT_WATER_SHALLOW );
        else if ( val == "deepwater" ) t.AddTerrain( TT_WATER_DEEP );
        else if ( val == "water" ) t.AddTerrain( TT_WATER );
        else if ( val == "rough" ) t.AddTerrain( TT_BARRICADES );
        else if ( val == "rails" ) t.AddTerrain( TT_RAILS );
        else if ( val == "restricted" ) t.AddTerrain( TT_RESTRICTED );
        else if ( val == "trenches" ) t.AddTerrain( TT_TRENCHES );
        else {
          rc = 1;
          cerr << "Error near line " << line << ": Invalid terrain '" << val << "'" << endl;
          break;
        }
      }
      else if ( key == "icon" ) t.SetImage( StrToNum( val ) );
      else if ( key == "armour" ) t.SetDefence( StrToNum( val ) );
      else if ( key == "speed" ) t.SetMoves( StrToNum( val ) );
      else if ( key == "price" ) t.SetCost( StrToNum( val ) );
      else if ( key == "weight" ) t.SetWeight( StrToNum( val ) );
      else if ( key == "transslots" ) t.SetTransSlots( StrToNum( val ) );
      else if ( key == "transminweight" ) t.SetTransMinWeight( StrToNum( val ) );
      else if ( key == "transmaxweight" ) t.SetTransMaxWeight( StrToNum( val ) );
      else if ( key == "portrait" ) {
        vector<string>::iterator it =
                find( info.portraits.begin(), info.portraits.end(), val );
        if ( it == info.portraits.end() ) {
          t.SetPortrait( info.portraits.size() );
          info.portraits.push_back( val );
        } else {
          t.SetPortrait( distance( info.portraits.begin(), it ) );
        }
      }
      else if ( key.substr( 0, 5 ) == "power" ) {
        string type = ParseOpt( key );

        if ( type == "" ) rc = 1;
        else rc = ParsePower( val, type, t );
      }
      else if ( key.substr( 0, 4 ) == "name" ) {
        string lang = ParseOpt( key );

        if ( lang == "" ) rc = 1;
        else if ( lang.size() != 2 ) {
          rc = 1;
          cerr << "Error near line " << line << ": '" << val << "' is not a valid language" << endl;
        } else {
          map <string, Language>::iterator it = names.find( lang );
          if ( it == names.end() ) {
            // only accept new languages for the first unit
            if ( units.size() == 0 ) {
              Language l;
              l.SetID( lang.c_str() );
              l.AddMsg( val );
              names[lang] = l;
              named = true;
            } else
              cerr << "Warning near line " << line << ": New language '" << lang << "' ignored" << endl;
          } else {
            it->second.AddMsg( val );
            named = true;
          }
        }
      } else {
        rc = 1;
        cerr << "Error near line " << line << ": Invalid keyword '" << key << "' in this context" << endl;
      }
    }

    if ( t.Firepower( U_GROUND ) == 0 ) {
      t.SetMinRangeGround( 0 );
      t.SetMaxRangeGround( 0 );
    }
    if ( t.Firepower( U_SHIP ) == 0 ) {
      t.SetMinRangeShip( 0 );
      t.SetMaxRangeShip( 0 );
    }
    if ( t.Firepower( U_AIR ) == 0 ) {
      t.SetMinRangeAir( 0 );
      t.SetMaxRangeAir( 0 );
    }
    if ( t.Slots() > 0 ) t.AddFlags( U_TRANSPORT );
    if ( t.Terrain() == 0 ) {
      rc = 1;
      cerr << "Error near line " << line << ": Unit does not specify terrain" << endl;
    }

    if ( !named ) {
      rc = 1;
      cerr << "Error near line " << line << ": Unit does not specify a name" << endl;
    } else {
      unsigned short msgs = names[CF_LANG_DEFAULT].Size();
      for ( map<string, Language>::iterator it = names.begin();
            it != names.end(); ++it ) {
        if ( it->second.Size() != msgs ) {
          const char *defname = names[CF_LANG_DEFAULT].GetMsg(msgs-1);
          if ( defname ) {
            cerr << "Warning near line " << line
                 << ": Unit does not specify name for language '" << it->first
                 << "', using '" << CF_LANG_DEFAULT << "' default ("
                 << defname << ')' << endl;
            it->second.AddMsg( defname );
          } else {
            rc = 1;
            cerr << "Error near line " << line
                 << ": Unit does not specify name for language '" << it->first << "'" << endl;
          }
        }
      }
    }

    units.push_back( t );
  }
  return rc;
}

// parse option ("key(option) = value")
string UnitHandler::ParseOpt( const string &s ) const {
  size_t pos1, pos2;

  pos1 = s.find( '(' );
  pos2 = s.rfind( ')' );

  if ( (pos1 == string::npos) ||
       (pos2 == string::npos) ||
       (pos2 < pos1) ) {
    cerr << "Warning: No option in key '" << s << "'" << endl;
    return "";
  } else return s.substr( pos1 + 1, pos2 - pos1 - 1 );
}

// parse firepower and range ("10 (1-3)")
int UnitHandler::ParsePower( const string &s, const string &type, EdUnitType &t ) const {
  size_t pos1, pos2;
  unsigned short min = 1, max = 1;
  string val;

  pos1 = s.find( '(' );
  pos2 = s.rfind( ')' );

  if ( pos2 < pos1 ) {
    cerr << "Error: Invalid firepower '" << s << "'" << endl;
    return -1;
  }

  if ( pos1 == string::npos ) {
    // default range (1-1)
    val = s;
  } else {
    val = s.substr( 0, pos1 - 1 );
    RemWhitespace( val );

    size_t pos3 = s.find( '-', pos1 + 1 );
    if ( (pos3 == string::npos) || (pos2 < pos3) ) {
      cerr << "Error: Invalid range '" << s << "'" << endl;
      return -1;
    }
    min = StrToNum( s.substr( pos1 + 1, pos3 - pos1 - 1 ) );
    max = StrToNum( s.substr( pos3 + 1, pos2 - pos3 - 1 ) );
  }

  if ( type == "ground" ) {
    t.SetMinRangeGround( min );
    t.SetMaxRangeGround( max );
    t.SetPowerGround( StrToNum( val ) );
  } else if ( type == "ship" ) {
    t.SetMinRangeShip( min );
    t.SetMaxRangeShip( max );
    t.SetPowerShip( StrToNum( val ) );
  } else if ( type == "air" ) {
    t.SetMinRangeAir( min );
    t.SetMaxRangeAir( max );
    t.SetPowerAir( StrToNum( val ) );
  }
  return 0;
}

void UnitsetCallback::SectionParsed( const string &section,
       SectionHandler &handler, CFParser &parser ) {
  if ( section == "unitset" ) {
    handler.SetEnabled( false );
    parser.EnableHandler( "unit", true );
  }
}

