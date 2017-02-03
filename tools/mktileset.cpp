/* mktileset -- create a terrain set for Crimson Fields
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


/* create a data set for CF, containing graphics and definitions
   for terrain

   level designers can specify custom data files to adjust the
   graphics and/or terrain characteristics to their likes
*/

#include "SDL.h"

#include "fileio.h"
#include "parser.h"
#include "lset.h"
#include "mksurface.h"

#ifdef _MSC_VER
// SDL_Main linkage destroys the command line in VS8
#undef main
#endif

/* .classids
   terrain classes currently used by CF (7) are:
   plains, forest, mountains, trenches, rails, shallow water, deep water

   reserved for future expansion (2): water, roads

   the final ID: trenches
   This denotes the first trenches hex in the tileset. Trenches need to
   appear in exactly the same order as in the default set if the set
   should be usable with pioneers! */

struct TilesetHeader {
  string images;
  unsigned short classids[10];
};

class TilesetHandler : public KeyValueSectionHandler {
public:
  TilesetHandler( TilesetHeader &head ) : info(head) {}

  int ParseSection( ifstream &in, const string *opt, unsigned long &line );

private:
  TilesetHeader &info;
};

class TileHandler : public KeyValueSectionHandler {
public:
  TileHandler( vector<TerrainType> &tiles ) : tiles(tiles) {}

  int ParseSection( ifstream &in, const string *opt, unsigned long &line );

private:
  int ParseColor( const string &s, Color &col ) const;

  vector<TerrainType> &tiles;
};

class TilesetCallback : public SectionParsedCallback {
public:
  void SectionParsed( const string &section,
       SectionHandler &handler, CFParser &parser );
};

int main( int argc, char *argv[] ) {
  int status, i;

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

  TilesetHeader info;
  vector<TerrainType> tiles;
  CFParser parser;
  SectionHandler *h;
  TilesetCallback cb;

  parser.AddHandler( "tileset", new TilesetHandler( info ) );

  h = new TileHandler( tiles );
  h->SetEnabled( false );
  parser.AddHandler( "tile", h );

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
  
    for ( i = 0; i < 10; ++i ) {
      if ( info.classids[i] > tiles.size() ) {
        cerr << "Error: Tile class " << i << " (" << info.classids[i]
             << ") is invalid" << endl;
        status = -1;
      }
    }

    if ( tiles.size() == 0 ) {
      cerr << "Error: No tiles defined" << endl;
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
      out.Write32( FID_TERRAINSET );
      out.Write16( tiles.size() );

      for ( i = 0; i < 10; ++i ) out.Write16( info.classids[i] );

      // write terrain data
      for ( vector<TerrainType>::iterator it = tiles.begin();
            it != tiles.end(); ++it ) {
        out.Write16( it->tt_type );
        out.Write16( it->tt_image );
        out.Write8( it->tt_att_mod );
        out.Write8( it->tt_def_mod );
        out.Write8( it->tt_move );
        out.Write32( it->tt_color );
      }

      // load graphics
      MkSurface img;
      status = img.SaveImageData( info.images, out, true );

      out.Close();
    }
  }

  return status;
}

// tileset handler
int TilesetHandler::ParseSection( ifstream &in, const string *opt, unsigned long &line ) {
  int rc = KeyValueSectionHandler::ParseSection( in, opt, line );

  if ( rc == 0 ) {
    info.images = "";
    for ( int i = 0; i < 10; ++i ) info.classids[i] = 0;

    for ( vector<pair<string, string> >::iterator it = pairs.begin();
          it != pairs.end(); ++it ) {
      string key = (*it).first;
      string val = (*it).second;

      if ( key == "icons" ) info.images = val;
      else if ( key == "name" ) /* ignore */;
      else if ( (key.size() == 6) && (key.substr( 0, 5 ) == "class") ) {
        int classid = StrToNum( key.c_str() + 5 );
        if ( info.classids[classid] != 0 ) {
          cerr << "Warning near line " << line << ": Tile class " << classid << " already set" << endl;
        }

        info.classids[classid] = StrToNum( val );
      } else {
        rc = 1;
        cerr << "Error near line " << line << ": Invalid keyword '" << key << "' in this context" << endl;
      }
    }
  }
  return rc;
}

// tile handler
int TileHandler::ParseSection( ifstream &in, const string *opt, unsigned long &line ) {
  int rc = KeyValueSectionHandler::ParseSection( in, opt, line );

  if ( rc == 0 ) {
    TerrainType t;
    t.tt_type = 0;
    t.tt_image = 0;
    t.tt_att_mod = 0;
    t.tt_def_mod = 0;
    t.tt_move = -1;
    t.tt_color = 0;

    for ( vector<pair<string, string> >::iterator it = pairs.begin();
          it != pairs.end(); ++it ) {
      string key = (*it).first;
      string val = (*it).second;

      if ( key == "terrain" ) {
        if ( val == "road" ) t.tt_type |= TT_ROAD;
        else if ( val == "plains" ) t.tt_type |= TT_PLAINS;
        else if ( val == "forest" ) t.tt_type |= TT_FOREST;
        else if ( val == "swamp" ) t.tt_type |= TT_SWAMP;
        else if ( val == "mountains" ) t.tt_type |= TT_MOUNTAINS;
        else if ( val == "shallowwater" ) t.tt_type |= TT_WATER_SHALLOW;
        else if ( val == "deepwater" ) t.tt_type |= TT_WATER_DEEP;
        else if ( val == "water" ) t.tt_type |= TT_WATER;
        else if ( val == "rough" ) t.tt_type |= TT_BARRICADES;
        else if ( val == "rails" ) t.tt_type |= TT_RAILS;
        else if ( val == "restricted" ) t.tt_type |= TT_RESTRICTED;
        else if ( val == "trenches" ) t.tt_type |= TT_TRENCHES;
        else if ( val == "shop" ) t.tt_type |= TT_ENTRANCE;
        else {
          rc = 1;
          cerr << "Error near line " << line << ": Invalid terrain '" << val << "'" << endl;
          break;
        }
      }
      else if ( key == "icon" ) t.tt_image = StrToNum( val );
      else if ( key == "attack" ) t.tt_att_mod = StrToNum( val );
      else if ( key == "defend" ) t.tt_def_mod = StrToNum( val );
      else if ( key == "move" ) t.tt_move = StrToNum( val );
      else if ( key == "color" ) {
        Color col;
        if ( !ParseColor( val, col ) )
          t.tt_color = (col.r << 16) | (col.g << 8) | col.b;
        else {
          rc = 1;
          cerr << "Error near line " << line << ": Could not parse color '" << val << "'" << endl;
        }
      } else {
        rc = 1;
        cerr << "Error near line " << line << ": Invalid keyword '" << key << "' in this context" << endl;
      }
    }
    if ( t.tt_move < -1 ) t.tt_move = -1;

    tiles.push_back( t );
  }
  return rc;
}

// parse a color defined as "rr,gg,bb"
int TileHandler::ParseColor( const string &s, Color &col ) const {
  size_t pos1, pos2;
  short comp;
  string val;

  pos1 = s.find( ',' );
  pos2 = s.rfind( ',' );

  if ( pos1 == pos2 ) return -1;

  val = s.substr( 0, pos1 );
  RemWhitespace( val );
  if ( val.size() == 0 ) return -1;
  comp = StrToNum( val );
  if ( (comp < 0) || (comp > 255) ) return -1;
  col.r = comp;

  val = s.substr( pos1 + 1, pos2 - pos1 - 1 );
  RemWhitespace( val );
  if ( val.size() == 0 ) return -1;
  comp = StrToNum( val );
  if ( (comp < 0) || (comp > 255) ) return -1;
  col.g = comp;

  val = s.substr( pos2 + 1 );
  RemWhitespace( val );
  if ( val.size() == 0 ) return -1;
  comp = StrToNum( val );
  if ( (comp < 0) || (comp > 255) ) return -1;
  col.b = comp;

  return 0;
}

void TilesetCallback::SectionParsed( const string &section,
       SectionHandler &handler, CFParser &parser ) {
  if ( section == "tileset" ) {
    handler.SetEnabled( false );
    parser.EnableHandler( "tile", true );
  }
}

