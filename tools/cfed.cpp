/* cfed -- a level editor for Crimson Fields
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

/* cfed.cpp */

#include <stdlib.h>
#include <map>

#include "SDL.h"

#include "parser.h"
#include "mission.h"
#include "gamedefs.h"
#include "strutil.h"
#include "globals.h"

#ifdef _MSC_VER
// SDL_Main linkage destroys the command line in VS8
#undef main
#endif

#define MAX_MAP_WIDTH	180
#define MAX_MAP_HEIGHT	180

class CfedParserUtils {
protected:
  CfedParserUtils( Mission &m ) : m(m) {}

  Point ParseCoords( const string &s ) const;
  const UnitType *ParseUnitName( const string &name ) const;
  int ParsePlayerId( const string &s ) const;

  Mission &m;
};

class MissionHandler : public KeyValueSectionHandler {
public:
  MissionHandler( Mission &m, string &tileset, string &unitset, Point &mapsize );

  int ParseSection( ifstream &in, const string *opt, unsigned long &line );

private:
  Mission &m;
  string &tiles;
  string &units;
  Point &mapsize;
};

class PlayerHandler : public KeyValueSectionHandler {
public:
  PlayerHandler( Mission &m ) : m(m) {}

  int ParseSection( ifstream &in, const string *opt, unsigned long &line );

private:
  int ParseColor( const string &s, Color &col ) const;

  Mission &m;
  static int parsed;
};

class UnitHandler : public KeyValueSectionHandler, public CfedParserUtils {
public:
  UnitHandler( Mission &m ) : CfedParserUtils(m) {}

  int ParseSection( ifstream &in, const string *opt, unsigned long &line );
};

class ShopHandler : public KeyValueSectionHandler, public CfedParserUtils {
public:
  ShopHandler( Mission &m ) : CfedParserUtils(m) {}

  int ParseSection( ifstream &in, const string *opt, unsigned long &line );
};

class EventHandler : public KeyValueSectionHandler, public CfedParserUtils {
public:
  EventHandler( Mission &m ) : CfedParserUtils(m) {}

  int ParseSection( ifstream &in, const string *opt, unsigned long &line );
};

class MessagesHandler : public SectionHandler {
public:
  MessagesHandler( Mission &m ) : m(m) {}

  int ParseSection( ifstream &in, const string *opt, unsigned long &line );

private:
  Mission &m;
};

class MapHandler : public SectionHandler {
public:
  MapHandler( Mission &m, const Point &size ) : m(m), size(size) {}

  int ParseSection( ifstream &in, const string *opt, unsigned long &line );

private:
  Mission &m;
  const Point &size;
};

class MapRawHandler : public SectionHandler {
public:
  MapRawHandler( Mission &m, const Point &size ) : m(m), size(size) {}

  int ParseSection( ifstream &in, const string *opt, unsigned long &line );

private:
  Mission &m;
  const Point &size;
};


// parser helpers
// parse positional information of the form "x/y"
Point CfedParserUtils::ParseCoords( const string &s ) const {
  string val;
  Point pos;

  size_t sep = s.find( '/' );
  if ( sep == string::npos ) return Point(-1,-1);

  val = s.substr( 0, sep );
  CFParser::RemWhitespace( val );
  if ( val.size() == 0 ) return Point(-1,-1);
  pos.x = CFParser::StrToNum( val );

  val = s.substr( sep + 1 );
  CFParser::RemWhitespace( val );
  if ( val.size() == 0 ) return Point(-1,-1);
  pos.y = CFParser::StrToNum( val );

  if ( (pos.x < 0) || (pos.y < 0) ) return Point(-1,-1);
  return pos;
}

// get the unit description from a unit name
const UnitType *CfedParserUtils::ParseUnitName( const string &name ) const {
  const UnitType *ut = NULL;
  const UnitSet &us = m.GetUnitSet();

  for ( int i = 0; i < us.NumTiles(); ++i ) {
    if ( name == us.UnitName(i) ) {
      ut = us.GetUnitInfo(i);
      break;
    }
  }
  return ut;
}

// turn the number given into a player id
int CfedParserUtils::ParsePlayerId( const string &s ) const {
  int rc = -1;

  // this is different from internal use in the game! here 0 means no owner!
  int id = CFParser::StrToNum(s);
  if ( id == 1 ) rc = PLAYER_ONE;
  else if ( id == 2 ) rc = PLAYER_TWO;
  else if ( id == 0 ) rc = PLAYER_NONE;

  return rc;
}

// mission parser
MissionHandler::MissionHandler( Mission &m, string &tileset, string &unitset, Point &mapsize ) :
  m(m), tiles(tileset), units(unitset), mapsize(mapsize) {
  tiles = "default";
  units = "default";
  mapsize = Point(-1, -1);
}

int MissionHandler::ParseSection( ifstream &in, const string *opt, unsigned long &line ) {
  int rc = KeyValueSectionHandler::ParseSection( in, opt, line );

  if ( rc == 0 ) {
    for ( vector<pair<string, string> >::iterator it = pairs.begin();
          it != pairs.end(); ++it ) {
      string key = (*it).first;
      string val = (*it).second;

      if ( key == "mapwidth" ) mapsize.x = StrToNum(val);                 // map width
      else if ( key == "mapheight" ) mapsize.y = StrToNum(val);           // map height
      else if ( key == "name" ) m.SetName(StrToNum(val));                 // map name
      else if ( key == "info" ) m.SetLevelInfoMsg(StrToNum(val));         // level info
      else if ( key == "campaignname" ) m.SetCampaignName(StrToNum(val)); // campaign name
      else if ( key == "campaigninfo" ) m.SetCampaignInfo(StrToNum(val)); // campaign info
      else if ( key == "players" ) m.SetNumPlayers(StrToNum(val));        // 1 or 2 player map
      else if ( key == "unitset" ) units = val;                           // unit set to use
      else if ( key == "tileset" ) tiles = val;                           // tile set to use
      else if ( key == "nextmap" ) m.SetSequel(val.c_str());              // next map
      else if ( key == "music" ) m.SetMusic(val.c_str());                 // soundtrack
      else if ( key == "campaign" ) m.SetCampaign(StrToNum(val) != 0);    // campaign map
      else if ( key == "skirmish" ) m.SetSkirmish(StrToNum(val) != 0);    // suitable for skirmishes
      else {
        rc = 1;
        cerr << "Error near line " << line << ": Invalid keyword '" << key << "'" << endl;
        break;
      }
    }
  }

  return rc;
}

// player parser
int PlayerHandler::parsed = 0;

int PlayerHandler::ParseSection( ifstream &in, const string *opt, unsigned long &line ) {
  if ( parsed > 1 ) {
    cerr << "Error in line " << line << ": more than two [player] sections defined" << endl;
    return 1;
  }

  int rc = KeyValueSectionHandler::ParseSection( in, opt, line );

  if ( rc == 0 ) {
    Player &p = m.GetPlayer( parsed );

    for ( vector<pair<string, string> >::iterator it = pairs.begin();
          it != pairs.end(); ++it ) {
      string key = (*it).first;
      string val = (*it).second;

      if ( key == "name" ) p.SetNameID(StrToNum(val));
      else if ( key == "briefing" ) p.SetBriefing(StrToNum(val));
      else if ( (key == "fcolor") || (key == "bcolor") ) {
        Color col;
        if ( !ParseColor(val, col) ) {
          if ( key[0] == 'f' ) p.SetLightColor(col);
          else p.SetDarkColor(col);
        } else {
          rc = 1;
          cerr << "Error near line " << line << ": Could not parse color '" << val << "'" << endl;
        }
      } else {
        rc = 1;
        cerr << "Error near line " << line << ": Invalid keyword '" << key << "' in this context" << endl;
      }
    }
  }

  ++parsed;
  return rc;
}

// parse a color defined as "rr,gg,bb"
int PlayerHandler::ParseColor( const string &s, Color &col ) const {
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

// unit parser
int UnitHandler::ParseSection( ifstream &in, const string *opt, unsigned long &line ) {
  int rc = KeyValueSectionHandler::ParseSection( in, opt, line );

  if ( rc == 0 ) {
    Unit *u = new Unit();
    u->SetPosition( Point(-1,-1) );
    u->SetID( 0 );
    u->UnsetFlags( ~0 );
    u->SetOwner( PLAYER_NONE );
    u->SetType( m.GetUnitSet().GetUnitInfo(0) );
    u->SetCrystals( 0 );
    u->SetDirection( 99 );
    u->SetGroupSize( MAX_GROUP_SIZE );
    u->SetXP( 0 );

    for ( vector<pair<string, string> >::iterator it = pairs.begin();
          (it != pairs.end()) && (rc == 0); ++it ) {
      string key = (*it).first;
      string val = (*it).second;

      if ( key == "pos" ) u->SetPosition(ParseCoords(val));           // unit position
      else if ( key == "id" ) u->SetID(StrToNum(val));                // unit id
      else if ( key == "crystals" ) u->SetCrystals(StrToNum(val));    // crystals
      else if ( key == "face" ) u->SetDirection(StrToNum(val));       // direction
      else if ( key == "size" ) u->SetGroupSize(StrToNum(val));       // group size
      else if ( key == "xp" ) u->SetXP(StrToNum(val) * XP_PER_LEVEL); // experience
      else if ( key == "type" ) {                                     // unit type
        const UnitType *type = ParseUnitName( val );
        if ( type ) {
          u->SetType( type );
          u->SetFlags( u->Flags() | type->Flags() );
        } else {
          rc = 1;
          cerr << "Error: Unknown unit type '" << val << "' near line " << line << endl;
        }
      } else if ( key == "player" ) {                             // owning player
        int id = ParsePlayerId( val );
        if ( id != -1 ) u->SetOwner( id );
        else {
          rc = 1;
          cerr << "Error near line " << line << ": Invalid owner of unit" << endl;
        }
      } else {
        rc = 1;
        cerr << "Error near line " << line << ": Invalid keyword '" << key << "' in this context" << endl;
      }
    }

    if ( rc == 0 ) m.GetUnits().AddTail( u );
    else delete u;
  }

  return rc;
}


// shop parser
int ShopHandler::ParseSection( ifstream &in, const string *opt, unsigned long &line ) {
  int rc = KeyValueSectionHandler::ParseSection( in, opt, line );

  if ( rc == 0 ) {
    Building *b = new Building();
    b->SetPosition( Point(0,0) );
    b->SetID( 0 );
    b->SetOwner( PLAYER_NONE );
    b->SetCrystalProduction( 0 );
    b->SetCrystals( 0 );
    b->SetMaxCrystals( 1000 );
    b->SetUnitProduction( 0 );
    b->UnsetFlags( ~0 );
    b->SetMinWeight( 0 );
    b->SetMaxWeight( 99 );
    b->SetNameID( -1 );

    for ( vector<pair<string, string> >::iterator it = pairs.begin();
          (it != pairs.end()) && (rc == 0); ++it ) {
      string key = (*it).first;
      string val = (*it).second;

      if ( key == "pos" ) b->SetPosition(ParseCoords(val));               // shop position
      else if ( key == "id" ) b->SetID(StrToNum(val));                    // ID
      else if ( key == "name" ) b->SetNameID(StrToNum(val));              // name
      else if ( key == "mining" ) b->SetCrystalProduction(StrToNum(val)); // mining
      else if ( key == "crystals" ) b->SetCrystals(StrToNum(val));        // resources
      else if ( key == "capacity" ) b->SetMaxCrystals(StrToNum(val));     // max. resources
      else if ( key == "minweight" ) b->SetMinWeight(StrToNum(val));      // smallest unit allowed
      else if ( key == "maxweight" ) b->SetMaxWeight(StrToNum(val));      // biggest unit allowed
      else if ( key == "player" ) {                                       // owning player
        int id = ParsePlayerId( val );
        if ( id != -1 ) b->SetOwner( id );
        else {
          rc = 1;
          cerr << "Error near line " << line << ": Invalid owner of building" << endl;
        }
      } else if ( key == "type" ) {                                       // type
        if ( val == "workshop" ) b->SetFlags(BLD_WORKSHOP);
        else if ( val == "factory" ) b->SetFlags(BLD_FACTORY);
        else {
          rc = 1;
          cerr << "Error in line " << line << ": Unknown type '" << val << "'" << endl;
        }
      } else if ( key == "factory" ) {                            // can produce which units
        const UnitType *type = ParseUnitName( val );
        if ( type ) {
          b->SetFlags(BLD_FACTORY);
          b->SetUnitProduction(b->UnitProduction()|(1<<type->ID()));
        } else {
          rc = 1;
          cerr << "Error: Unknown unit type '" << val << "' near line " << line << endl;
        }
      } else {
        rc = 1;
        cerr << "Error near line " << line << ": Invalid keyword '" << key << "' in this context" << endl;
      }
    }

    if ( rc == 0 ) m.GetBuildings().AddTail( b );
    else delete b;
  }

  return rc;
}


// event parser
int EventHandler::ParseSection( ifstream &in, const string *opt, unsigned long &line ) {
  int rc = KeyValueSectionHandler::ParseSection( in, opt, line );

  if ( rc == 0 ) {
    Event *e = new Event();
    e->SetID( 0 );
    e->SetType( 99 );
    e->SetPlayer( PLAYER_NONE );
    e->SetTrigger( 99 );
    e->SetDependency( -1 );
    e->SetDiscard( -1 );
    for ( int i = 0; i < 3; ++i ) {
      e->SetData( i, -9999 );
      e->SetTData( i, -9999 );
    }
    e->SetTitle( -1 );
    e->SetMessage( -1 );
    e->SetFlags( 0 );
    e->SetTmpBuf( "" );

    for ( vector<pair<string, string> >::iterator it = pairs.begin();
          (it != pairs.end()) && (rc == 0); ++it ) {
      string key = (*it).first;
      string val = (*it).second;
      Point loc;
      int id;

      if ( key == "type" ) {                                 // type
        if ( val == "message" ) e->SetType(EVENT_MESSAGE);
        else if ( val == "research" ) e->SetType(EVENT_RESEARCH);
        else if ( val == "mining" ) e->SetType(EVENT_MINING);
        else if ( val == "score" ) e->SetType(EVENT_SCORE);
        else if ( val == "configure" ) e->SetType(EVENT_CONFIGURE);
        else if ( val == "manipulateevent" ) e->SetType(EVENT_MANIPULATE_EVENT);
        else if ( val == "sethex" ) e->SetType(EVENT_SET_HEX);
        else if ( val == "settimer" ) e->SetType(EVENT_SET_TIMER);
        else if ( val == "destroyunit" ) e->SetType(EVENT_DESTROY_UNIT);
        else if ( val == "createunit" ) {
          e->SetType(EVENT_CREATE_UNIT);
          e->SetData(2, NORTH | (MAX_GROUP_SIZE << 3) | (0 << 6)); // dir, group size, xp defaults 
        } else {
          rc = 1;
          cerr << "Error near line " << line << ": Unknown event type '" << val << "'" << endl;
        }

      } else if ( key == "trigger" ) {                       // trigger
        if ( val == "timer" ) e->SetTrigger(ETRIGGER_TIMER);
        else if ( val == "unitdestroyed" ) e->SetTrigger(ETRIGGER_UNIT_DESTROYED);
        else if ( val == "unitposition" ) e->SetTrigger(ETRIGGER_UNIT_POSITION);
        else if ( val == "handicap" ) e->SetTrigger(ETRIGGER_HANDICAP);
        else if ( val == "havecrystals" ) e->SetTrigger(ETRIGGER_HAVE_CRYSTALS);
        else if ( val == "havebuilding" ) {
          e->SetTrigger(ETRIGGER_HAVE_BUILDING);
          e->SetTData(2, -1);
        } else if ( val == "haveunit" ) {
          e->SetTrigger(ETRIGGER_HAVE_UNIT);
          e->SetTData(2, -1);
        } else {
          rc = 1;
          cerr << "Error near line " << line << ": Unknown trigger type '" << val << "'" << endl;
        }
      }

      else if ( key == "player" ) {                             // player
        id = ParsePlayerId( val );
        if ( (id != -1) && (id != PLAYER_NONE) ) e->SetPlayer( id );
        else {
          rc = 1;
          cerr << "Error near line " << line << ": Invalid player" << endl;
        }
      }
      else if ( key == "title" ) e->SetTitle(StrToNum(val));
      else if ( key == "message" ) e->SetMessage(StrToNum(val));
      else if ( key == "id" ) e->SetID(StrToNum(val));
      else if ( key == "flags" ) e->SetFlags(StrToNum(val));
      else if ( key == "depend" ) e->SetDependency(StrToNum(val));
      else if ( key == "discard" ) e->SetDiscard(StrToNum(val));
      else {
        if ( (e->Trigger() == ETRIGGER_TIMER) &&
             (key == "ttime") ) e->SetTData(0, StrToNum(val));      // timer
        else if ( (e->Trigger() == ETRIGGER_HAVE_UNIT) &&
             (key == "tunit") ) e->SetTData(0, StrToNum(val));      // unit
        else if ( (e->Trigger() == ETRIGGER_HAVE_BUILDING) &&
             (key == "tbuilding") ) e->SetTData(0, StrToNum(val));  // building id
        else if ( (e->Trigger() == ETRIGGER_HAVE_CRYSTALS) &&
             (key == "tbuilding") ) e->SetTData(2, StrToNum(val));  // building id
        else if ( (e->Trigger() == ETRIGGER_HAVE_CRYSTALS) &&
             (key == "tcrystals") ) e->SetTData(0, StrToNum(val));  // number of crystals
        else if ( ((e->Trigger() == ETRIGGER_HAVE_BUILDING) ||
                   (e->Trigger() == ETRIGGER_HAVE_UNIT) ||
                   (e->Trigger() == ETRIGGER_UNIT_DESTROYED) ||
                   (e->Trigger() == ETRIGGER_HAVE_CRYSTALS)) &&
             (key == "towner") ) e->SetTData(1, StrToNum(val)-1);   // owner of shop/unit
        else if ( (e->Trigger() == ETRIGGER_UNIT_POSITION) &&
             (key == "towner") ) e->SetTData(2, StrToNum(val)-1);   // owner of unit
        else if ( ((e->Trigger() == ETRIGGER_HAVE_BUILDING) ||
                   (e->Trigger() == ETRIGGER_HAVE_UNIT)) &&
             (key == "ttime") ) e->SetTData(2, StrToNum(val));      // time after which to check
        else if ( (e->Trigger() == ETRIGGER_UNIT_POSITION) && (key == "tpos") ) {
          loc = ParseCoords(val);
          if ( (loc.x < 0) || (loc.y < 0) ) {
            rc = 1;
            cerr << "Error near line " << line << ": Invalid position " << val << endl;
          } else e->SetTData(1, m.GetMap().Hex2Index(loc));         // unit position
        }
        else if ( ((e->Trigger() == ETRIGGER_UNIT_POSITION) ||
                   (e->Trigger() == ETRIGGER_UNIT_DESTROYED)) && (key == "tunit") ) { // unit or unit type
          if ( val.size() > 0 && (val[0] == '-' || isdigit(val[0])) ) {
            // seems to be a number -> specific unit or -1
            e->SetTData(0, StrToNum(val));
          } else {
            const UnitType *type = ParseUnitName( val );
            if ( type ) e->SetTData(0, -type->ID() - 2);
            else {
              rc = 1;
              cerr << "Error near line " << line << ": Invalid unit type " << val << endl;
            }
          }
        }
        else if ( (e->Trigger() == ETRIGGER_HANDICAP) &&
             (key == "thandicap") ) e->SetTData(0, StrToNum(val));   // handicap

        else switch ( e->Type() ) {
          case EVENT_CREATE_UNIT:
            // e_data[2] is split: 0000000 111 111 111
            //                             xp size dir
            if ( key == "face" )                                     // direction to face
              e->SetData(2, (e->GetData(2) & ~0x0007) | (StrToNum(val) & 0x0007));
            else if ( key == "size" )                                // group size
              e->SetData(2, (e->GetData(2) & ~0x0038) | ((StrToNum(val) << 3) & 0x0038));
            else if ( key == "xp" )                                  // experience
              e->SetData(2, (e->GetData(2) & ~0x01C0) | ((StrToNum(val) << 6) & 0x01C0));
            else if ( key == "pos" ) {                               // where to create
              loc = ParseCoords(val);
              if ( (loc.x < 0) || (loc.y < 0) ) {
                rc = 1;
                cerr << "Error near line " << line << ": Invalid position " << val << endl;
              } else e->SetData(1, m.GetMap().Hex2Index(loc));
            }
            else if ( key == "unit" ) {                              // unit type
              const UnitType *type = ParseUnitName( val );
              if ( type ) e->SetData(0, type->ID());
              else {
                rc = 1;
                cerr << "Error: Unknown unit type '" << val << "' near line " << line << endl;
              }
            } else {
              rc = 1;
              cerr << "Error near line " << line << ": Invalid keyword '" << key << "'" << endl;
            }
            break;

          case EVENT_DESTROY_UNIT:
            if ( key == "unit" ) e->SetData(0, StrToNum(val));         // id of unit to destroy
            else if ( key == "owner" ) e->SetData(1, StrToNum(val)-1); // unit owner
            else if ( key == "pos" ) {                                 // unit position
              loc = ParseCoords(val);
              if ( (loc.x < 0) || (loc.y < 0) ) {
                rc = 1;
                cerr << "Error near line " << line << ": Invalid position " << val << endl;
              } else e->SetData(2, m.GetMap().Hex2Index(loc));
            }
            break;

          case EVENT_MANIPULATE_EVENT:
            if ( key == "event" ) e->SetData(0, StrToNum(val));       // ID of target event
            else if ( key == "eflags" ) e->SetData(1, StrToNum(val)); // flags to modify
            else if ( key == "action" ) e->SetData(2, StrToNum(val)); // set/clear/toggle
            else {
              rc = 1;
              cerr << "Error near line " << line << ": Invalid keyword '" << key << "'" << endl;
            }
            break;

          case EVENT_MESSAGE:
            if ( key == "pos" ) {                                  // hex to focus on
              loc = ParseCoords(val);
              if ( (loc.x < 0) || (loc.y < 0) ) {
                rc = 1;
                cerr << "Error near line " << line << ": Invalid position " << val << endl;
              } else e->SetData(0, m.GetMap().Hex2Index(loc));
            } else {
              rc = 1;
              cerr << "Error near line " << line << ": Invalid keyword '" << key << "'" << endl;
            }
            break;

          case EVENT_MINING:
            if ( key == "building" ) e->SetData(0, StrToNum(val));      // id of building
            else if ( key == "crystals" ) e->SetData(1, StrToNum(val)); // crystal production
            else if ( key == "action" ) e->SetData(2, StrToNum(val));   // modification action
            else {
              rc = 1;
              cerr << "Error near line " << line << ": Invalid keyword '" << key << "'" << endl;
            }
            break;

         case EVENT_CONFIGURE:
            if ( key == "setting" ) {                          // setting to change
              if ( val == "briefing1" ) e->SetData(0, 0);
              else if ( val == "briefing2" ) e->SetData(0, 1);
              else if ( val == "nextmap" ) e->SetData(0, 2);
            } else if ( key == "value" ) {                     // value to set
              switch (e->GetData(0)) {
              case 0:
              case 1:
                e->SetData(1, StrToNum(val));
                break;
              case 2:
                e->SetTmpBuf(val);
                break;
              default:
                rc = 1;
                cerr << "Error near line " << line << ": Configure event does not support value before setting" << endl;
              }
            } else {
              rc = 1;
              cerr << "Error near line " << line << ": Invalid keyword '" << key << "'" << endl;
            }
            break;

          case EVENT_RESEARCH:
            if ( key == "building" ) e->SetData(0, StrToNum(val));    // id of building
            else if ( key == "action" ) e->SetData(2, StrToNum(val)); // whether to allow or disallow this unit
            else if ( key == "unit" ) {                               // unit type
              const UnitType *type = ParseUnitName( val );
              if ( type ) e->SetData(1, type->ID());
              else {
                rc = 1;
                cerr << "Error: Unknown unit type '" << val << "' near line " << line << endl;
              }
            } else {
              rc = 1;
              cerr << "Error near line " << line << ": Invalid keyword '" << key << "'" << endl;
            }
            break;

          case EVENT_SCORE:
            if ( key == "success" ) e->SetData(0, StrToNum(val));         // success rate increase
            else if ( key == "othermsg" ) e->SetData(1, StrToNum(val));   // message for other player
            else if ( key == "othertitle" ) e->SetData(2, StrToNum(val)); // message title for other player
            else {
              rc = 1;
              cerr << "Error near line " << line << ": Invalid keyword '" << key << "'" << endl;
            }
            break;

          case EVENT_SET_HEX:
            if ( key == "pos" ) {                                         // hex to change
              loc = ParseCoords(val);
              if ( (loc.x < 0) || (loc.y < 0) ) {
                rc = 1;
                cerr << "Error near line " << line << ": Invalid position " << val << endl;
              } else e->SetData(1, m.GetMap().Hex2Index(loc));
            }
            else if ( key == "tile" ) {                                   // new terrain type
              id = StrToNum(val);
              if ( (id < m.GetTerrainSet().NumTiles()) && (id >= 0) ) e->SetData(0, id);
              else {
                rc = 0;
                cerr << "Error: Invalid tile '" << val << "' near line " << line << endl;
              }
            } else {
              rc = 1;
              cerr << "Error near line " << line << ": Invalid keyword '" << key << "'" << endl;
            }
            break;

          case EVENT_SET_TIMER:
            if ( key == "event" ) e->SetData(0, StrToNum(val));       // ID of target event
            else if ( key == "time" ) e->SetData(1, StrToNum(val));   // time
            else if ( key == "offset" ) e->SetData(2, StrToNum(val)); // time offset
            else {
              rc = 1;
              cerr << "Error near line " << line << ": Invalid keyword '" << key << "'" << endl;
            }
            break;
        }
      }
    }

    if ( rc == 0 ) m.GetEvents().AddTail( e );
    else delete e;
  }

  return rc;
}

// map parser
int MapHandler::ParseSection( ifstream &in, const string *opt, unsigned long &line ) {

  if ( (size.x <= 0) || (size.x > MAX_MAP_WIDTH) || (size.y <= 0) || (size.y > MAX_MAP_HEIGHT) ) {
    cerr << "Error: Invalid map size" << endl;
    return -1;
  }

  m.GetMap().SetSize( size );

  string buf;
  for ( int i = 0; i < size.y; ++i ) {
    getline( in, buf );
    ++line;

    if ( in.eof() ) {
      cerr << "Error in line " << line << ": Unexpected end of map data" << endl;
      return -1;
    }

    RemWhitespace( buf );
    if ( buf.size() != (unsigned short)size.x ) {
      cerr << "Error in line " << line << ": map row " << i << " does not comply with width parameter" << endl;
      return -1;
    }

    for ( int j = 0; j < size.x; ++j ) {
      unsigned short terrain;
      switch ( buf[j] ) {
        case '.': terrain = 30; break;                                  /* plains */
        case '*': terrain = 81; break;                                  /* forest */
        case '%': terrain = 117; break;                                 /* mountains */
        case '=': terrain = 360; break;                                 /* shallow water */
        case '~': terrain = 361; break;                                 /* water */
        case '-': terrain = 362; break;                                 /* deep water */
        case '1': terrain = 6; break;                                   /* yellow hq entrance, east */
        case '2': terrain = 7; break;                                   /* blue hq entrance, east */
        case '0': terrain = 8; break;                                   /* neutral hq entrance, east */
        case '<': terrain = 22; break;                                  /* hq, left part */
        case '>': terrain = 23; break;                                  /* hq, right part */
        case '^': terrain = 21; break;                                  /* hq, upper part */
        case 'v': terrain = 24; break;                                  /* hq, lower part */
        case '#': terrain = 307; break;                                 /* swamp */
        case '\\': terrain = 177; break;                                /* road, se-nw */
        case '|': terrain = 176; break;                                 /* road, s-n */
        case '/': terrain = 178; break;                                 /* road, sw-ne */
        case 'y': terrain = 193; break;                                 /* road, sw-n-ne */
        case 'Y': terrain = 194; break;                                 /* road, se-n-nw */
        case '(': terrain = 180; break;                                 /* road, n-se */
        case ')': terrain = 179; break;                                 /* road, n-sw */
        case ']': terrain = 181; break;                                 /* road, nw-s */
        case '[': terrain = 182; break;                                 /* road, ne-s */
        case 'n': terrain = 184; break;                                 /* road, sw-se */
        case 'u': terrain = 183; break;                                 /* road, nw-ne */
        case 'o': terrain = 201; break;                                 /* road, sw-nw-ne-se */
        case 'k': terrain = 192; break;                                 /* road, sw-s-ne */
        case 'K': terrain = 191; break;                                 /* road, s-se-nw */
        case 'T': terrain = 188; break;                                 /* road, n-s-se */
        case 'U': terrain = 187; break;                                 /* road, n-s-sw */
        case 'V': terrain = 198; break;                                 /* road, n-s-ne */
        case 'W': terrain = 189; break;                                 /* road, n-s-nw */
        case 'X': terrain = 199; break;                                 /* road, s-se-nw-n */
        case 'x': terrain = 200; break;                                 /* road, s-sw-ne-n */
        case '!': terrain = 208; break;                                 /* bridge, n-s */
        case '`': terrain = 209; break;                                 /* bridge, se-nw */
        case '\'': terrain = 210; break;                                /* bridge, sw-ne */
        case '4': terrain = 9; break;                                   /* yellow depot entrance */
        case '5': terrain = 10; break;                                  /* blue depot entrance */
        case '3': terrain = 11; break;                                  /* neutral depot entrance */
        case '7': terrain = 12; break;                                  /* yellow factory entrance, north */
        case '8': terrain = 13; break;                                  /* blue factory entrance, north */
        case '6': terrain = 14; break;                                  /* neutral factory entrance, north */
        case 'a': terrain = 144; break;                                 /* wire fence, se-nw end */
        case 'b': terrain = 147; break;                                 /* wire fence, nw-se end */
        case 'c': terrain = 146; break;                                 /* wire fence, ne-sw end */
        case 'd': terrain = 145; break;                                 /* wire fence, sw-ne end */
        case 'e': terrain = 133; break;                                 /* wire fence, n-s */
        case 'f': terrain = 135; break;                                 /* wire fence, sw-ne */
        case 'g': terrain = 134; break;                                 /* wire fence, nw-se */
        case 'h': terrain = 138; break;                                 /* wire fence, nw-s */
        case 'i': terrain = 139; break;                                 /* wire fence, ne-s */
        case 'j': terrain = 136; break;                                 /* wire fence, sw-n */
        case 'l': terrain = 137; break;                                 /* wire fence, se-n */
        case 'm': terrain = 140; break;                                 /* wire fence, nw-ne */
        case 'p': terrain = 141; break;                                 /* wire fence, sw-se */
        case '"': terrain = 363; break;                                 /* cliff 1 */
        case 'A': terrain = 18; break;                                  /* yellow city */
        case 'B': terrain = 19; break;                                  /* blue city */
        case 'C': terrain = 20; break;                                  /* neutral city */
        case 'D': terrain = 3; break;                                   /* yellow hq entrance, west */
        case 'E': terrain = 4; break;                                   /* blue hq entrance, west */
        case 'F': terrain = 5; break;                                   /* neutral hq entrance, west */
        case 'G': terrain = 0; break;                                   /* yellow hq entrance, north */
        case 'H': terrain = 1; break;                                   /* blue hq entrance, north */
        case 'I': terrain = 2; break;                                   /* neutral hq entrance, north */
        case '9': terrain = 15; break;                                  /* yellow factory entrance, east */
        case 'J': terrain = 16; break;                                  /* blue factory entrance, east */
        case 'L': terrain = 17; break;                                  /* neutral factory entrance, east */
        default:
          cerr << "Error in line " << line << ": Invalid character '" << buf[j] << "' in map" << endl;
          return -1;
      }

      m.GetMap().SetHexType( j, i, terrain );
    }
  }

  return 0;
}

// read a raw map; a hex is represented by its ID and hexes are separated by comma;
// this way, much more hex types can be accessed than by using plain ASCII code
int MapRawHandler::ParseSection( ifstream &in, const string *opt, unsigned long &line ) {
  string buf;
  unsigned short terrain;
  size_t pos;
  const char *start;
  Map &map = m.GetMap();

  if ( (size.x <= 0) || (size.x > MAX_MAP_WIDTH) || (size.y <= 0) || (size.y > MAX_MAP_HEIGHT) ) {
    cerr << "Error: Invalid map size" << endl;
    return -1;
  }

  map.SetSize( size );

  for ( int i = 0; i < size.y; ++i ) {
    getline(in, buf);
    ++line;
    if ( in.eof() ) {
      cerr << "Error in line " << line << ": unexpected end of map data" << endl;
      return -1;
    }

    RemWhitespace( buf );
    pos = 0;
    for ( int j = 0; j < size.x; ++j ) {
      start = &buf.c_str()[pos];
      while ( (buf[pos] >= '0') && (buf[pos] <= '9') ) ++pos;

      if ( (buf[pos] == ',') || (buf[pos] == '\0') ) {
        buf[pos++] = '\0';

        terrain = atoi( start );
        if ( terrain >= map.GetTerrainSet()->NumTiles() ) {
          cerr << "Error in line " << line << ": Invalid hex id '" << terrain << "'" << endl;
          return -1;
        } else map.SetHexType( j, i, terrain );
      } else {
        cerr << "Error in line " << line << ": Invalid character '" << buf[pos] << "' in map" << endl;
        return -1;
      }
    }
  }

  return 0;
}

// messages parser
int MessagesHandler::ParseSection( ifstream &in, const string *opt, unsigned long &line ) {
  if ( !opt ) {
    cerr << "Error in line " << line << ": [Messages] section does not specify a language" << endl;
    return -1;
  } else if ( opt->size() != 2 ) {
    cerr << "Error in line " << line << ": " << *opt << " is not a valid language" << endl;
    return -1;
  }

  Language lang;
  lang.SetID( opt->c_str() );
  string buf, msg;
  bool done = false;

  do {
    getline(in, buf);
    ++line;

    RemWhitespace( buf );

    if ((buf.size() > 0) && 
       ((buf[0] == '%') || (buf.substr(0,11) == "[/messages]"))) {
      // save last message
      if (!msg.empty()) {
        lang.AddMsg(msg);
        msg.erase();
      }

      if (buf.substr(0,11) == "[/messages]") done = true;
    } else {
      if (!msg.empty()) msg += '\n';
      msg.append(buf);
    }
  } while ( !in.eof() && !done );

  int rc;

  if (in.eof()) {
    rc = -1;
    cerr << "Error in line " << line << ": Messages section unterminated" << endl;
  } else {
    rc = 0;
    m.GetMessages().AddLanguage( lang );
  }

  return rc;
}

#define SECTION_MAP	(1<<0)
#define SECTION_MISSION	(1<<1)
#define SECTION_PLAYER1	(1<<2)
#define SECTION_PLAYER2	(1<<3)

class MissionParser : public Mission, public SectionParsedCallback {
public:
  MissionParser( void );

  int parse( const char *file );

  int load_unit_set( const char *set );
  int load_tile_set( const char *set );

  void SectionParsed( const string &section,
       SectionHandler &handler, CFParser &parser ); 

private:
  int check_game( void ) const;
  int check_units( void ) const;
  int check_shops( void ) const;
  int check_events( void ) const;
  int check_player( const Player &p ) const;

  template <typename T>
  bool find_dups( const List &l, const T *o1 ) const {
    int cnt = 0;
    for ( T *o2 = static_cast<T *>(l.Head());
          o2; o2 = static_cast<T *>(o2->Next()) ) {
      if ( o1->ID() == o2->ID() ) ++cnt;
    }
    return cnt > 1;
  }

  unsigned short parsed_sections;
};

// start of program functions
int main( int argc, char **argv ) {
  short show_help = 0;
  int rc, i;
  char *uset = NULL, *tset = NULL;
  string outname;

  if ( SDL_Init( 0 ) < 0 ) {
    cerr << "Error: Could not init SDL. " << SDL_GetError() << endl;
    return 1;
  }

  if ( argc < 2 ) show_help = 1;
  else {
    for ( i = argc - 1; i > 1; --i ) {
      if (strcmp(argv[i], "--help") == 0) show_help = 1;
      else if (strcmp(argv[i], "--version") == 0) {
        cout << "cfed "VERSION << endl;
        return 0;
      }
      else if (strcmp(argv[i-1], "--units") == 0) uset = argv[i];
      else if (strcmp(argv[i-1], "--tiles") == 0) tset = argv[i];
      else if (strcmp(argv[i-1], "-o") == 0) outname = argv[i];
    }
  }

  if ( !uset || !tset ) show_help = 1;

  if ( show_help ) {
    cout << "Usage: " << argv[0] << " file --tiles <tileset> --units <unitset> [-o <outfile>]" << endl
              << "  --help     display this help and exit" << endl
              << "  --version  output version information and exit" << endl;
    return 0;
  }

  MissionParser parser;
  if ( parser.load_unit_set( uset ) != 0 ) {
    cerr << "Unable to open unit set " << uset << endl;
    return 1;
  }
  if ( parser.load_tile_set( tset ) != 0 ) {
    cerr << "Unable to open tile set " << tset << endl;
    return 1;
  }

  rc = parser.parse( argv[1] );

  if ( !rc ) {
    if ( outname.empty() ) {
      outname = argv[1];
      size_t pos = outname.rfind( '.' );
      if ( pos != string::npos ) outname.erase( pos );
      outname.append( ".lev" );
    }

    rc = parser.Save( outname.c_str() );

    if ( rc )
      cerr << "Error opening file '" << outname << "' for writing" << endl;
  }

  return rc;
}

/* parse the file and create a binary mission file from it */
int MissionParser::parse( const char *file ) {
  parsed_sections = 0;

  int err = 0;
  Point mapsize;
  string tileset, unitset;

  CFParser parser;
  SectionHandler *h;

  parser.AddHandler( "mission", new MissionHandler( *this, tileset, unitset, mapsize ) );
  parser.AddHandler( "player", new PlayerHandler( *this ) );
  parser.AddHandler( "messages", new MessagesHandler( *this ) );

  h = new UnitHandler( *this );
  h->SetEnabled( false );
  parser.AddHandler( "unit", h );

  h = new ShopHandler( *this );
  h->SetEnabled( false );
  parser.AddHandler( "building", h );

  h = new EventHandler( *this );
  h->SetEnabled( false );
  parser.AddHandler( "event", h );

  h = new MapHandler( *this, mapsize );
  h->SetEnabled( false );
  parser.AddHandler( "map", h );

  h = new MapRawHandler( *this, mapsize );
  h->SetEnabled( false );
  parser.AddHandler( "map-raw", h );

  parser.SetCallback( this );
  err = parser.Parse( file );

  if ( !err ) {
    if ( (parsed_sections & SECTION_MISSION) == 0 ) {
      cerr << "Error: Basic mission definitions missing" << endl;
      err = 1;
    }
    else if ( (parsed_sections & SECTION_MAP) == 0 ) {
      cerr << "Error: No map" << endl;
      err = 1;
    }
  }

  if ( !err && !messages.SetDefaultLanguage( CF_LANG_DEFAULT ) ) {
    cerr << "Error: No english language data found" << endl;
    err = 1;
  }
  if ( !err ) err = check_game();
  if ( !err ) err = check_shops();
  if ( !err ) err = check_units();
  if ( !err ) err = check_events();
  if ( !err ) err = check_player( p1 );
  if ( !err ) err = check_player( p2 );

  return err;
}

// mission parser callback called when a section has been
// successfully parsed
void MissionParser::SectionParsed( const string &section,
     SectionHandler &handler, CFParser &parser ) {

  if ( section == "mission" ) {
    parsed_sections |= SECTION_MISSION;
    handler.SetEnabled( false );
    parser.EnableHandler( "unit", true );
    parser.EnableHandler( "building", true );
    parser.EnableHandler( "event", true );
    parser.EnableHandler( "map", true );
    parser.EnableHandler( "map-raw", true );
  } else if ( section == "player" ) {
    if ( parsed_sections & SECTION_PLAYER1 ) {
      parsed_sections |= SECTION_PLAYER2;
      handler.SetEnabled( false );
    } else parsed_sections |= SECTION_PLAYER1;
  } else if ( (section == "map") || (section == "map-raw") ) {
    parsed_sections |= SECTION_MAP;
    parser.EnableHandler( "map", false );
    parser.EnableHandler( "map-raw", false );
  }
}

/* check game data for validity */
int MissionParser::check_game( void ) const {

  if ( (map.Width() == 0) || (map.Height() == 0) ) {
    cerr << "Error: map width or height is 0" << endl;
    return 1;
  }

  if ( GetName() == -1 ) {
    cerr << "Error: Level has not been given a name" << endl;
    return 1;
  } else if ( !GetMessage(GetName()) ) {
    cerr << "Error: Invalid mission name " << (short)GetName() << endl;
    return 1;
  } else {
    short num_msgs = -1, cnt;

    for ( std::map<const string, Language>::const_iterator it = messages.GetLibrary().begin();
          it != messages.GetLibrary().end(); ++it ) {
      const Language *lang = &it->second;

      // count messages
      for ( cnt = 0; lang->GetMsg(cnt); ++cnt ); // empty loop
      if ( num_msgs == -1 ) num_msgs = cnt;
      else if ( num_msgs != cnt ) {
        cerr << "Error: Language data for '" << lang->ID() << "' contains " << cnt << " messages." << endl
             << "       Previous languages had " << num_msgs << endl;
        return 1;
      }

      if ( strlen(lang->GetMsg(GetName())) > 30 ) {
        cerr << "Error (" << lang->ID() << "): Mission name must not exceed 30 characters" << endl;
        return 1;
      }
    }
  }

  if ( (GetLevelInfoMsg() != -1) && !GetMessage(GetLevelInfoMsg()) ) {
    cerr << "Error: Invalid level info message " << (short)GetLevelInfoMsg() << endl;
    return 1;
  }
  if ( (GetCampaignName() != -1) && !GetMessage(GetCampaignName()) ) {
    cerr << "Error: Invalid campaign name " << (short)GetCampaignName() << endl;
    return 1;
  }
  if ( (GetCampaignInfo() != -1) && !GetMessage(GetCampaignInfo()) ) {
    cerr << "Error: Invalid campaign info message " << (short)GetCampaignInfo() << endl;
    return 1;
  }
  if ( !IsCampaign() &&
     (GetSequel() || (GetCampaignName() != -1) || (GetCampaignInfo() != -1)) ) {
    cerr << "Error: Campaign settings found but map is not marked as a campaign map" << endl;
    return 1;
  }

  return 0;
}

/* check players */
int MissionParser::check_player( const Player &p ) const {

  short msg = p.NameID();
  if ( msg == -1 ) {
    cerr << "Error: No name set for player" << endl;
    return 1;
  } else {
    if ( !GetMessage(msg) ) {
      cerr << "Error: Invalid name " << msg << " for player" << endl;
      return 1;
    } else {
      for ( std::map<const string, Language>::const_iterator it = messages.GetLibrary().begin();
            it != messages.GetLibrary().end(); ++it ) {
        const Language *lang = &it->second;

        if ( strlen(lang->GetMsg(msg)) > 30 ) {
          cerr << "Error(" << lang->ID() << "): Player names must not be longer than 30 characters" << endl;
          return 1;
        }
      }
    }
  }

  msg = p.Briefing();
  if ( (msg != -1) && !GetMessage(msg) ) {
    cerr << "Error: Invalid briefing " << msg << " for player" << endl;
    return 1;
  }

  return 0;
}

/* check units */
int MissionParser::check_units( void ) const {
  Unit *u, *walk;

  /* if there is a transport at a unit's location put it in if possible */
  /* this only checks for transports appearing before the actual unit in the file */
  for ( u = static_cast<Unit *>(units.Head()); u; u = static_cast<Unit *>(u->Next()) ) {

    if ( (u->Position().x >= map.Width()) || (u->Position().y >= map.Height()) ||
         (u->Position().x < 0) || (u->Position().y < 0) ) {
      cerr << "Error: unit (ID: " << u->ID() << ") out of map" << endl;
      return 1;
    }

    if ( u->IsTransport() && !u->IsSheltered() ) {

      if ( StorageLeft( *u ) < 0 ) {
        cerr << "Error: Unit " << u->Name() << " at "
                  << u->Position().x << '/' << u->Position().y
                  << " carries too many units or crystals" << endl;
        return 1;
      }

      for ( walk = static_cast<Unit *>(u->Next()); walk; walk = static_cast<Unit *>(walk->Next()) ) {

        if ( walk->Position() == u->Position() ) {
          if ( walk->Weight() > u->MaxWeight() ) {
            cerr << "Error: Unit " << walk->Name() << " too heavy for transport at "
                      << u->Position().x << '/' << u->Position().y << endl;
            return 1;
          } else if ( walk->Weight() < u->MinWeight() ) {
            cerr << "Error: Unit " << walk->Name() << " too light for transport at "
                      << u->Position().x << '/' << u->Position().y << endl;
            return 1;
          } else walk->SetFlags( U_SHELTERED );
        }
      }
    }

    if ( u->Crystals() && !u->IsTransport() )
      cerr << "Error: non-transport unit (ID: " << u->ID() << ") cannot carry crystals" << endl;

    for ( walk = static_cast<Unit *>(u->Next()); walk; walk = static_cast<Unit *>(walk->Next()) ) {
      if ( u->ID() == walk->ID() ) {
        cerr << "Error: Two or more units sharing one ID (" << u->ID() << ')' << endl;
        return 1;
      }

      if ( (u->Position() == walk->Position()) &&
           (!(u->IsSheltered() || u->IsTransport()) || !walk->IsSheltered()) ) {
        cerr << "Error: Two or more units sharing one hex (" << u->Position().x << '/'
                  << u->Position().y << ')' << endl;

        if ( walk->IsTransport() )
          cerr << "       This might be caused by a transport being declared after a unit it carries." << endl;
        return 1;
      }
    }

    if ( (u->Owner() == PLAYER_ONE) || (u->Owner() == PLAYER_TWO) ) {
      if ( u->GetDirection() == 99 ) u->SetDirection( u->Owner() * 3 );    /* direction not set, use defaults */
      else if ( u->GetDirection() > NORTHWEST ) {
        cerr << "Error: Invalid direction " << (short)u->GetDirection() << " for unit (ID: "
                  << u->ID() << ')' << endl;
        return 1;
      }
    } else {
      u->SetDirection( 0 );
      if ( !(u->Owner() == PLAYER_NONE) || !u->IsSheltered() ) {
        cerr << "Error: unit with ID " << u->ID() << " has no legal owner" << endl;
        return 1;
      }
    }

    if ( (u->GroupSize() > MAX_GROUP_SIZE) || (u->GroupSize() == 0) ) {
      cerr << "Error: unit with ID " << u->ID() << " has size " << (short)u->GroupSize() << endl;
      return 1;
    }

    if ( u->XPLevel() > XP_MAX_LEVEL ) {
      cerr << "Error: unit with ID " << u->ID() << " has been promoted to XP level " << (short)u->XPLevel()
           << ", maximum is " << XP_MAX_LEVEL << endl;
      return 1;
    }
  }
  return 0;
}

/* check buildings */
int MissionParser::check_shops( void ) const {
  Building *b, *walk;
  Unit *u;

  for ( b = static_cast<Building *>(buildings.Head()); b; b = static_cast<Building *>(b->Next()) ) {

    if ( (b->Position().x >= map.Width()) || (b->Position().y >= map.Height()) ) {
      cerr << "Error: Shop (" << b->Position().x << '/' << b->Position().y
                << ") out of map" << endl;
      return 1;
    }

    if ( !(map.TerrainTypes( b->Position() ) & TT_ENTRANCE) ) {
      cerr << "Error: Map does not have a shop entrance at " << b->Position().x
                << '/' << b->Position().y << endl;
      return 1;
    }

    if ( b->NameID() == -1 ) {
      cerr << "Error: No name set for shop " << b->ID() <<  endl;
      return 1;
    } else if ( !GetMessage(b->NameID()) ) {
      cerr << "Error: Invalid name " << (short)b->NameID() << " for shop " << b->ID() <<  endl;
      return 1;
    } else {
      for ( std::map<const string, Language>::const_iterator it = messages.GetLibrary().begin();
            it != messages.GetLibrary().end(); ++it ) {
        const Language *lang = &it->second;

        if ( strlen(lang->GetMsg(b->NameID())) > 30 ) {
          cerr << "Error(" << lang->ID() << "): Shop names must not be longer than 30 characters" << endl;
          return 1;
        }
      }
    }

    if ( b->Crystals() > b->MaxCrystals() ) {
      cerr << "Error: Shop at " << b->Position().x << '/' << b->Position().y
                << " contains " << b->Crystals() << " crystals, but only "
                << b->MaxCrystals() << " fit in" << endl;
      return 1;
    }

    if ( b->MinWeight() > b->MaxWeight() ) {
      cerr << "Error: Shop at " << b->Position().x << '/' << b->Position().y
                << " has minweight (" << (short)b->MinWeight() << ") > maxweight ("
                << b->MaxWeight() << ')' << endl;
      return 1;
    }


    for ( walk = static_cast<Building *>(b->Next()); walk; walk = static_cast<Building *>(walk->Next()) ) {
      if ( b->ID() == walk->ID() ) {
        cerr << "Error: Two shops sharing ID " << b->ID() << endl;
        return 1;
      }

      if ( b->Position() == walk->Position() ) {
        cerr << "Error: Two shops sharing one hex ("
                  << b->Position().x << '/' << b->Position().y << ')' << endl;
        return 1;
      }
    }

    for ( u = static_cast<Unit *>(units.Head()); u; u = static_cast<Unit *>(u->Next()) ) {
      if ( b->Position() == u->Position() ) {
        if ( (u->Owner() != b->Owner()) && (b->Owner() != PLAYER_NONE) ) {
          cerr << "Error: Hostile unit (" << u->ID() << ") in building ("
                    << b->Position().x << '/' << b->Position().y << ')' << endl;
          return 1;
        } else u->SetOwner( b->Owner() );

        if ( u->Weight() < b->MinWeight() ) {
          cerr << "Error: Unit (" << u->ID() << ") too light for building at "
	            << b->Position().x << '/' << b->Position().y << endl;
          return 1;
       } else if ( u->Weight() > b->MaxWeight() ) {
          cerr << "Error: Unit (" << u->ID() << ") too heavy for building at "
	            << b->Position().x << '/' << b->Position().y << endl;
          return 1;
        }

        u->SetFlags( U_SHELTERED );
      }
    }
  }
  return 0;
}

/* check events for consistency */
int MissionParser::check_events( void ) const {
  Event *e;
  Building *b;
  Unit *u;
  short msg;

  for ( e = static_cast<Event *>(events.Head()); e; e = static_cast<Event *>(e->Next()) ){

    if ( (e->Player() != PLAYER_ONE) && (e->Player() != PLAYER_TWO) ) {
      cerr << "Error: Event " << (short)e->ID() << " has no player assigned" << endl;
      return 1;
    }

    msg = e->Message();
    if ( (msg != -1) && !messages.GetMsg(msg) ) {
      cerr << "Error: Event " << (short)e->ID() << " has invalid message '" << msg << "'" << endl;
      return 1;
    }
    msg = e->Title();
    if ( (msg != -1) && !messages.GetMsg(msg) ) {
      cerr << "Error: Event " << (short)e->ID() << " has invalid title '" << msg << "'" << endl;
      return 1;
    }

    /* check dependencies */
    if ( e->Dependency() != -1 ) {
      if ( !GetEventByID( e->Dependency() ) ) {
        cerr << "Error: Event " << (short)e->ID() << " depends on non-existing event "
                  << (short)e->Dependency() << endl;
        return 1;
      }
    }

    if ( e->Discard() != -1 ) {
      if ( !GetEventByID( e->Discard() ) ) {
        cerr << "Error: Event " << (short)e->ID() << " discards non-existing event "
                  << (short)e->Discard() << endl;
        return 1;
      }
    }

    /* check ID's */
    if ( find_dups( events, e ) ) {
      cerr << "Error: Two or more events with the same ID '" << (short)e->ID() << "'" << endl;
      return 1;
    }


    switch ( e->Type() ) {
      case EVENT_CREATE_UNIT:
        if ( e->GetData(0) < 0 ) {
          cerr << "Error: No unit type specified for Create Unit event " << (short)e->ID() << endl;
          return 1;
        }

        if ( map.Index2Hex(e->GetData(1)) == Point(-1, -1) ) {
          cerr << "Error: Invalid location for event " << (short)e->ID() << endl;
          return 1;
        }

        {
          unsigned short cu_dir = e->GetData(2) & 0x0007;
          unsigned short cu_size = (e->GetData(2) & 0x0038) >> 3;
          unsigned short cu_xp = (e->GetData(2) & 0x01C0) >> 6;
	  if ( cu_dir > NORTHWEST ) {
            cerr << "Error: Invalid direction " << cu_dir << " for event " << (short)e->ID() << endl;
            return 1;
          }

	  if ( (cu_size == 0) || (cu_size > MAX_GROUP_SIZE) ) {
            cerr << "Error: Invalid unit size " << cu_size << " for event " << (short)e->ID() << endl;
            return 1;
          }

	  if ( cu_xp > XP_MAX_LEVEL ) {
            cerr << "Error: Invalid unit experience level " << cu_xp << " for event " << (short)e->ID() << endl;
            return 1;
          }
        }
        break;
      case EVENT_DESTROY_UNIT:
        if ( e->GetData(0) < 0 ) {
          if (e->GetData(2) == -9999) {
            cerr << "Error: No target specified for Destroy Unit event " << (short)e->ID() << endl;
            return 1;
          }
          e->SetData(0, -1);
        } else {

          if ( !GetUnitByID( e->GetData(0) ) ) {
            cerr << "Error: Unit with ID " << e->GetData(0) << " specified in event "
                 << (short)e->ID() << " does not exist" << endl;
            return 1;
          }
          e->SetData(2, 0);
        }

        if ( e->GetData(1) < -1 ) e->SetData(1, -1);
        else if ( e->GetData(1) > PLAYER_TWO ) {
          cerr << "Error: Invalid unit owner for event " << (short)e->ID() << endl;
          return 1;
        }
        break;
      case EVENT_MANIPULATE_EVENT:
        if ( e->GetData(0) < 0 ) {
          cerr << "Error: Event manipulation event " << (short)e->ID() << " without valid target" << endl;
          return 1;
        }

        /* for now, this event can only enable/disable other events */
        e->SetData(1, EFLAG_DISABLED);
        if ( e->GetData(1) == 0 ) {
          cerr << "Error: Event manipulation (" << (short)e->ID() << ") with invalid flags 0" << endl;
          return 1;
        }

        if ( (e->GetData(2) < 0) || (e->GetData(2) > 2) ) {
          cerr << "Error: Event manipulation (" << (short)e->ID() << ") with invalid action "
                    << e->GetData(2) << endl;
          return 1;
        }

        if ( !GetEventByID( e->GetData(0) ) ) {
          cerr << "Error: Event with ID " << e->GetData(0) << " does not exist" << endl;
          return 1;
        }
        break;
      case EVENT_MESSAGE:
        if ( e->Message() == -1 ) {
          cerr << "Error: Message event " << (short)e->ID() << " has no message" << endl;
          return 1;
        }

        if ( map.Index2Hex( e->GetData(0) ) == Point(-1,-1) ) e->SetData(0, -1);
        else if ( !map.Contains( map.Index2Hex( e->GetData(0) ) ) ) {
          cerr << "Error: Invalid location set for message event (" << (short)e->ID() << ')' << endl;
          return 1;
        }

        e->SetData(1, 0);
        e->SetData(2, 0);
        break;
      case EVENT_MINING:
        if ( e->GetData(0) < 0 ) {
          cerr << "Error: Mining event " << (short)e->ID() << " with no shop" << endl;
          return 1;
        }

        if ( (e->GetData(2) < 0) || (e->GetData(2) > 3) ) {
          cerr << "Error: Mining event " << (short)e->ID() << " with invalid action "
                    << e->GetData(2) << endl;
          return 1;
        }

        if ( ((e->GetData(2) == 0) || (e->GetData(2) == 2)) && (e->GetData(1) < 0) ) {
          cerr << "Error: Trying to set negative absolute amount for mining event "
                    << (short)e->ID() << endl;
          return 1;
        }

        if ( !GetBuildingByID(e->GetData(0)) ) {
          cerr << "Error: Shop with ID " << e->GetData(0) << " does not exist (event "
                    << (short)e->ID() << ')' << endl;
          return 1;
        }
        break;
      case EVENT_CONFIGURE:
        if ( e->GetData(0) < -9999 ) {
          cerr << "Error: Configure event " << (short)e->ID() << " does not specify setting" << endl;
          return 1;
        }

        if ( (e->GetData(0) == 0) || (e->GetData(0) == 1) ) {
          msg = e->GetData(1);
          if ( (msg != -1) && !messages.GetMsg(msg) ) {
            cerr << "Error: Event " << (short)e->ID() << " references invalid message '" << msg << "'" << endl;
            return 1;
          }
        }
        e->SetData(2, 0);
        break;
      case EVENT_RESEARCH:
        if ( e->GetData(0) < 0 ) {
          cerr << "Error: Research event " << (short)e->ID() << " with no shop" << endl;
          return 1;
        }

        if ( e->GetData(2) == -9999 ) e->SetData(2, 0);
        else if ( (e->GetData(2) < 0) || (e->GetData(2) > 1) ) {
          cerr << "Error: Research event " << (short)e->ID() << " specifies invalid action " << e->GetData(2) << endl;
          return 1;
        }

        if ( e->GetData(1) < 0 ) {
          cerr << "Error: Research event " << (short)e->ID() << " with no unit type" << endl;
          return 1;
        }

        if ( !GetBuildingByID(e->GetData(0)) ) {
          cerr << "Error: Shop with ID " << e->GetData(0) << " does not exist (event "
                    << (short)e->ID() << ')' << endl;
          return 1;
        }
        break;
      case EVENT_SCORE:
        if ( e->GetData(0) < 0 ) {
          cerr << "Warning: Corrected success rate for score event < 0" << endl;
          e->SetData(0, 0);
        }

        if ( e->GetData(1) < -1 ) e->SetData(1, -1);
        msg = e->GetData(1);
        if ( (msg != -1) && !messages.GetMsg(msg) ) {
          cerr << "Error: Event " << (short)e->ID() << " references invalid message '" << msg << "'" << endl;
          return 1;
        }

        if ( e->GetData(2) < -1 ) e->SetData(2, -1);
        msg = e->GetData(2);
        if ( (e->GetData(1) != -1) && (msg != -1) && !messages.GetMsg(msg) ) {
          cerr << "Error: Event " << (short)e->ID() << " references invalid message '" << msg << "'" << endl;
          return 1;
        } else if ( (e->GetData(1) == -1) && (msg != -1) ) {
          cerr << "Warning: Event " << (short)e->ID() << " set a title but no message" << endl;
          e->SetData(2, -1);
        }
        break;
      case EVENT_SET_HEX:
        if ( e->GetData(0) < 0 ) {
          cerr << "Error: No tile specified for Set Hex event " << (short)e->ID() << endl;
          return 1;
        }

        if ( map.Index2Hex( e->GetTData(1) ) == Point(-1,-1) ) e->SetTData(1, -1);
        else if ( !map.Contains( map.Index2Hex( e->GetTData(1) ) ) ) {
          cerr << "Error: Invalid location set for event trigger (" << (short)e->ID() << ')' << endl;
          return 1;
        }
        break;
      case EVENT_SET_TIMER:
        if ( e->GetData(0) < 0 ) {
          cerr << "Error: Set Timer event " << (short)e->ID() << " without valid target" << endl;
          return 1;
        } else {
          Event *tev = GetEventByID( e->GetData(0) );
          if ( !tev ) {
            cerr << "Error: Event with ID " << e->GetData(0) << " does not exist (event "
                 << (short)e->ID() << ")" << endl;
            return 1;
          } else if ( tev->Trigger() != ETRIGGER_TIMER ) {
            cerr << "Error: Event with ID " << e->GetData(0) << " has no timer trigger (event "
                 << (short)e->ID() << ")" << endl;
            return 1;
          }
        }

        if ( e->GetData(1) < 0 ) {
          cerr << "Error: Set Timer (" << (short)e->ID() << ") with invalid time "
                    << e->GetData(1) << endl;
          return 1;
        }

        if ( (e->GetData(2) < 0) || (e->GetData(2) > 2) ) {
          cerr << "Error: Set Timer (" << (short)e->ID() << ") with invalid offset "
                    << e->GetData(2) << endl;
          return 1;
        }
        break;
      default:
          cerr << "Error: Event with ID " << (short)e->ID() << " has invalid type" << endl;
    }

    switch ( e->Trigger() ) {
      case ETRIGGER_TIMER:
        if ( e->GetTData(0) < 0 ) {
          cerr << "Error: Event trigger lacks time (" << (short)e->ID() << ')' << endl;
          return 1;
        }
        e->SetTData(1, 0);
        e->SetTData(2, 0);
        break;
      case ETRIGGER_UNIT_DESTROYED:
        if ( e->GetTData(0) >= 0 ) {
          u = GetUnitByID( e->GetTData(0) );

          /* the event must also be triggered when the unit is not
             destroyed but captured by the enemy. Therefore we need
             the original owner */
          if (u) e->SetTData(1, u->Owner());
          else {
            cerr << "Error: Event trigger targets non-existing unit with ID " << e->GetTData(0) << endl;
            return 1;
          }
        } else {
          if ( -e->GetTData(0) - 2 >= unit_set->NumTiles() ) {
            cerr << "Error: Event trigger targets non-existing unit type " << -e->GetTData(0) - 2
                 << " (" << (short)e->ID() << ')' << endl;
            return 1;
          }
          if ( e->GetTData(1) == -9999 ) e->SetTData(1, e->Player()^1);
        }
        e->SetTData(2, 0);
        break;
      case ETRIGGER_HAVE_UNIT:
        if ( (e->GetTData(1) != PLAYER_ONE) && (e->GetTData(1) != PLAYER_TWO) ) {
          cerr << "Error: Event trigger wants invalid player to own a unit ("
               << (short)e->ID() << endl;
          return 1;
        }
        if ( e->GetTData(2) < 0 ) e->SetTData(2, -1);

        u = GetUnitByID( e->GetTData(0) );

        if (u) {
          if ( (u->Owner() == e->GetTData(1)) && (e->GetTData(2) < 0) && (e->Dependency() == -1) ) {
            cerr << "Error: Event trigger: unit " << u->ID() << " is already owned by player "
                 << u->Owner()+1 << '(' << (short)e->ID() << ')' << endl;
              return 1;
            }
        } else {
          cerr << "Error: Event trigger targets non-existing unit " << e->GetTData(0)
               << " (" << (short)e->ID() << ')' << endl;
          return 1;
        }
        break;
      case ETRIGGER_HAVE_BUILDING:
        if ( (e->GetTData(1) != PLAYER_ONE) && (e->GetTData(1) != PLAYER_TWO) ) {
          cerr << "Error: Event trigger wants invalid player to own a shop ("
               << (short)e->ID() << endl;
          return 1;
        }
        if ( e->GetTData(2) < 0 ) e->SetTData(2, -1);

        b = GetBuildingByID( e->GetTData(0) );

        if (b) {
          if ( (b->Owner() == e->GetTData(1)) && (e->GetTData(2) < 0) && (e->Dependency() == -1) ) {
            cerr << "Error: Event trigger: shop " << b->ID() << " is already owned by player "
                 << b->Owner()+1 << '(' << (short)e->ID() << ')' << endl;
              return 1;
            }
        } else {
          cerr << "Error: Event trigger targets non-existing shop " << e->GetTData(0)
               << " (" << (short)e->ID() << ')' << endl;
          return 1;
        }
        break;
      case ETRIGGER_HAVE_CRYSTALS:
        if ( e->GetTData(0) == -9999 ) {
          cerr << "Error: Crystals trigger does not specify amount (" << (short)e->ID() << ")" << endl;
          return 1;
        } else if ( ABS(e->GetTData(0)) > 5000 ) {
          cerr << "Error: Invalid crystals amount " << e->GetTData(0) << " for event trigger ("
               << (short)e->ID() << ")" << endl;
          return 1;
        }

        if ( e->GetTData(1) == -9999 ) e->SetTData(1, e->Player());
        else if ( (e->GetTData(1) != PLAYER_ONE) && (e->GetTData(1) != PLAYER_TWO) ) {
          cerr << "Error: Event trigger wants invalid player to own a shop ("
               << (short)e->ID() << ")" << endl;
          return 1;
        }

        if ( e->GetTData(2) < -2 ) e->SetTData(2, -1);
        else if ( e->GetTData(2) >= 0 ) {
          if ( !GetBuildingByID( e->GetTData(2) ) ) {
            cerr << "Error: Event trigger targets non-existing shop " << e->GetTData(2)
                 << " (" << (short)e->ID() << ')' << endl;
            return 1;
          }
        }
        break;
      case ETRIGGER_UNIT_POSITION:
        if ( map.Index2Hex( e->GetTData(1) ) == Point(-1,-1) ) {
          cerr << "Error: Invalid location set for event trigger (" << (short)e->ID() << ')' << endl;
          return 1;
        }

        if ( e->GetTData(0) < -1 ) {
          if ( -e->GetTData(0) - 2 >= unit_set->NumTiles() ) {
            cerr << "Error: Event trigger targets non-existing unit type " << -e->GetTData(0) - 2
                 << " (" << (short)e->ID() << ')' << endl;
            return 1;
          }
        } else if ( e->GetTData(0) >= 0 ) {
          if (!GetUnitByID( e->GetTData(0) )) {
            cerr << "Error: Event trigger targets non-existing unit " << e->GetTData(0)
                 << " (" << (short)e->ID() << ')' << endl;
            return 1;
          }
        }

        if ( e->GetTData(2) == -9999 ) e->SetTData( 2, e->Player() );
        else if ( (e->GetTData(2) != PLAYER_ONE) && (e->GetTData(2) != PLAYER_TWO) ) {
          cerr << "Error: Event trigger wants invalid player to control a unit ("
               << (short)e->ID() << endl;
          return 1;
        }
        break;
      case ETRIGGER_HANDICAP:
        if ( e->GetTData(0) == -9999 ) {
          cerr << "Error: Handicap event trigger without a handicap ("
               << (short)e->ID() << ')' << endl;
          return 1;
        } else if ( e->GetTData(0) & 0xFFF8 ) {
          cerr << "Error: Invalid handicap " << e->GetTData(0) << " ("
               << (short)e->ID() << ')' << endl;
          return 1;
        }
        e->SetTData(1, 0);
        e->SetTData(2, 0);
        break;
      default:
        cerr << "Error: Invalid event trigger type " << (short)e->Trigger() << " ("
             << (short)e->ID() << ')' << endl;
        return 1;
    }
  }
  return 0;
}

/* load a unit set */
int MissionParser::load_unit_set( const char *set ) {
  string setshort( set );

  // keep only the file part; check for both Unix and Windows path
  // separator characters or this breaks when building with MinGW
  setshort = setshort.substr( setshort.find_last_of( "/\\" ) + 1 );

  size_t pos = setshort.find( ".units" );
  if ( pos != string::npos ) setshort.erase( pos );
  File file( set );
  if ( !file.Open("rb") ) return -1;

  unit_set = new UnitSet();
  if ( unit_set->Load( file, setshort.c_str() ) == -1 ) {
    delete unit_set;
    unit_set = 0;
    return -1;
  }

  map.SetUnitSet( unit_set );
  return 0;
}

/* load a terrain set */
int MissionParser::load_tile_set( const char *set ) {
  string setshort( set );

  // keep only the file part; check for both Unix and Windows path
  // separator characters or this breaks when building with MinGW
  setshort = setshort.substr( setshort.find_last_of( "/\\" ) + 1 );

  size_t pos = setshort.find( ".tiles" );
  if ( pos != string::npos ) setshort.erase( pos );
  File file( set );
  if ( !file.Open("rb") ) return -1;

  terrain_set = new TerrainSet();
  if ( terrain_set->Load( file, setshort.c_str() ) == -1 ) {
    delete terrain_set;
    return -1;
  }

  map.SetTerrainSet( terrain_set );
  return 0;
}

MissionParser::MissionParser( void ) {
  SetLevelInfoMsg( -1 );
  SetCampaignName( -1 );
  SetCampaignInfo( -1 );
  SetTitle( "Unknown" );
}

