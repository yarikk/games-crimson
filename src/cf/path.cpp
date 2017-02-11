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
// path.cpp -- pathfinding functions
////////////////////////////////////////////////////////////////////////

#include <vector>
#include <algorithm>
using namespace std;

#include "path.h"

////////////////////////////////////////////////////////////////////////
// NAME       : BasicPath::BasicPath
// DESCRIPTION: Create a new path object.
// PARAMETERS : map    - map to use for pathfinding
//              buffer - buffer to store path in; if non-NULL must be
//                       large enough to hold at least (map height x
//                       map width) bytes. If NULL, a new buffer will
//                       be allocated and freed when the path object is
//                       destroyed.
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

BasicPath::BasicPath( Map *map, signed char *buffer ) {
  this->map = map;
  buf_private = false;

  SetBuffer( buffer );
}

////////////////////////////////////////////////////////////////////////
// NAME       : BasicPath::~BasicPath
// DESCRIPTION: Destroy the Path object.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

BasicPath::~BasicPath( void ) {
  if ( buf_private ) delete [] path;
}

////////////////////////////////////////////////////////////////////////
// NAME       : BasicPath::Clear
// DESCRIPTION: Clear the path buffer.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void BasicPath::Clear( void ) {
  unsigned short size = map->Width() * map->Height();
  for ( int i = 0; i < size; ++i ) path[i] = -1;
}

////////////////////////////////////////////////////////////////////////
// NAME       : BasicPath::SetBuffer
// DESCRIPTION: Set a new path buffer.
// PARAMETERS : buffer - new buffer. If NULL a new buffer is allocated.
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void BasicPath::SetBuffer( signed char *buffer ) {
  if ( buf_private ) delete [] path;

  buf_private = !buffer;
  if ( buf_private ) {
    path = new signed char [map->Width() * map->Height()];
  } else {
    path = buffer;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : BasicPath::Find
// DESCRIPTION: Search a path on the map.
// PARAMETERS : u     - unit to search a path for; this may be NULL
//                      only in some special cases (see subclasses)
//              start - hex to start from (can be different from the
//                      unit's current position)
//              end   - destination hex
// RETURNS    : approximate number of turns to reach the destination
//              hex, or -1 if no valid path was found
////////////////////////////////////////////////////////////////////////

short BasicPath::Find( const Unit *u, const Point &start, const Point &end ) {
  vector<PathNode> openlist;
  greater<PathNode> comp;
  this->start = start;
  this->end = end;

  // array to store node cost in so that we don't have to look it
  // up in the open list
  unsigned short mapsize = map->Width() * map->Height();
  short *nodeval = new short [mapsize];
  for ( int i = 0; i < mapsize; ++i ) {
    nodeval[i] = -1;
    path[i] = -1;
  }

  // to begin, insert the starting node in openlist
  PathNode pnode;

  // starting from the destination would make it a bit easier to record the actual path
  // but this way we can support going towards the target until we have reached a certain
  // distance which is rather useful for AI routines...
  pnode.pos = start;
  pnode.eta = ETA( start );
  pnode.cost = 0;
  pnode.switched = false;
  openlist.push_back( pnode );

  while( !openlist.empty() ) {
    pnode = openlist.front();
    pop_heap( openlist.begin(), openlist.end(), comp );
    openlist.pop_back();

    // check for destination
    if ( StopSearch( pnode ) ) break;

    // not there yet, so let's look at the hexes around it
    for ( short dir = NORTH; dir <= NORTHWEST; ++dir ) {
      Point p;

      // get the hex in that direction
      if ( !map->Dir2Hex( pnode.pos, (Direction)dir, p ) ) {
        PathNode pnode2;
        pnode2.pos = p;
        if ( AddNode( u, pnode, pnode2 ) ) {
          unsigned short index = map->Hex2Index( p );
          if ( path[index] == -1 ) {
            // node has not yet been visited; queue it
            path[index] = ReverseDir((Direction)dir);
            nodeval[index] = pnode2.cost;
            openlist.push_back( pnode2 );
            push_heap( openlist.begin(), openlist.end(), comp );
          } else if ( nodeval[index] > pnode2.cost ) {
            // node has been visited before; look for it in the
            // list of open nodes and replace it if the cost of
            // the new one is smaller
            for ( vector<PathNode>::iterator it = openlist.begin();
                  it != openlist.end(); ++it ) {
              if ( (*it).pos == pnode2.pos ) {
                path[index] = ReverseDir((Direction)dir);
                nodeval[index] = pnode2.cost;
                (*it).cost = pnode2.cost;
                (*it).switched = pnode2.switched;
                push_heap( openlist.begin(), it+1, comp );
                break;
              }
            }
          }
        }
      }
    }
  }

  delete [] nodeval;

  // check if we reached our destination
  return FinalizePath( u, pnode );
}

// PATH - a Path is the object used for normal unit pathfinding,
//        trying to move a unit from A to B
//
////////////////////////////////////////////////////////////////////////
// NAME       : Path::Find
// DESCRIPTION: Search a path on the map.
// PARAMETERS : u     - unit to search a path for; this may be NULL
//                      only in some special cases (see subclasses)
//              start - hex to start from (can be different from the
//                      unit's current position)
//              end   - destination hex
//              qual  - path quality/speed trade-off (1 is best, 10 is
//                      worst but fastest). Default PATH_BEST.
//              off   - sometimes you don't want to reach the target
//                      hex proper but only get close to it. This is
//                      the maximum distance to the destination for a
//                      path to be considered valid. Default 0.
// RETURNS    : approximate number of turns to reach the destination
//              hex, or -1 if no valid path was found
////////////////////////////////////////////////////////////////////////

short Path::Find( const Unit *u, const Point &start, const Point &end,
                  unsigned char qual, unsigned char off ) {
  quality = qual;
  deviation = off;
  return BasicPath::Find( u, start, end );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Path::ETA
// DESCRIPTION: Estimate the cost to the destination hex. This is used
//              as an heuristic for the path finding algorithm.
// PARAMETERS : current - current location
// RETURNS    : estimated cost to destination
////////////////////////////////////////////////////////////////////////

unsigned short Path::ETA( const Point &p ) const {
  return Distance( p, end ) * quality;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Path::StepsToDest
// DESCRIPTION: Get the number of steps on the path from a given hex to
//              the destination.
// PARAMETERS : pos - hex position to start from
// RETURNS    : number of steps until destination is reached, or 0 if
//              no path available or already there
////////////////////////////////////////////////////////////////////////

unsigned short Path::StepsToDest( const Point &pos ) const {
  Point p( pos );
  short index = map->Hex2Index( p );
  Direction dir = (Direction)path[index];
  unsigned short steps = 0;

  while ( dir != (Direction)-1 ) {
    map->Dir2Hex( p, dir, p );
    index = map->Hex2Index( p );
    dir = (Direction)path[index];
    ++steps;
  }
  return steps;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Path::StopSearch
// DESCRIPTION: For the normal pathfinder, the search ends when the
//              destination is reached (or we're sufficiently close).
// PARAMETERS : next - path node to be checked next if search is not
//                     cancelled
// RETURNS    : TRUE if search aborted, FALSE otherwise
////////////////////////////////////////////////////////////////////////

inline bool Path::StopSearch( const PathNode &next ) const {
  return Distance( next.pos, end ) <= deviation;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Path::AddNode
// DESCRIPTION: Calculate the cost for the unit to move from one hex to
//              another and complete the respective PathNode.
// PARAMETERS : u    - unit
//              from - path node for source hex
//              to   - destination path node (adjacent to from). Only
//                     the pos member is initialized and this method
//                     must take care of the rest
// RETURNS    : TRUE if step is allowed and the node should be added to
//              the open list, FALSE if step is not allowed
////////////////////////////////////////////////////////////////////////

bool Path::AddNode( const Unit *u, const PathNode &from, PathNode &to ) const {
  unsigned short obst;
  short cost = map->MoveCost( u, from.pos, to.pos, obst );
  bool rc = false;

  if ( obst != 0 ) {
    if ( !map->GetMapObject( to.pos ) || (to.pos == end) )
      cost = MAX( u->Moves() - from.cost, cost );
    else cost = -1;
  }

  if ( cost >= 0 ) {
    to.cost = from.cost + cost;
    to.eta = ETA( to.pos );
    rc = true;
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Path::FinalizePath
// DESCRIPTION: Check whether a path has been found, prepare the buffer
//              and set the return code of the pathfinder.
// PARAMETERS : u    - unit searching the path
//              last - final path node
// RETURNS    : cost of the path in turns for the unit (approximately),
//              or -1 if no valid path was found
////////////////////////////////////////////////////////////////////////

short Path::FinalizePath( const Unit *u, const PathNode &last ) const {
  short turns = -1;
  Point p = last.pos;
  if ( Distance( p, end ) <= deviation ) {
    // correct the path
    // move back to beginning
    short index = map->Hex2Index( p );
    Direction dir = (Direction)path[index], dir2;
    path[index] = -1;

    while ( p != start ) {
      map->Dir2Hex( p, dir, p );
      index = map->Hex2Index( p );
      dir2 = (Direction)path[index];
      path[index] = ReverseDir( dir );
      dir = dir2;
    }

    if ( u->Type()->Speed() == 0 ) turns = 100;
    else turns = (last.cost + u->Type()->Speed() - 1) / u->Type()->Speed();
  }
  return turns;
}


// MOVE SHADER - this is used to create the buffer for shading of
//               illegal moves when selecting a unit
//
//
////////////////////////////////////////////////////////////////////////
// NAME       : MoveShader::ETA
// DESCRIPTION: Estimate the cost to the destination hex. This is used
//              as an heuristic for the path finding algorithm.
// PARAMETERS : current - current location
// RETURNS    : estimated cost to destination
////////////////////////////////////////////////////////////////////////

// inline unsigned short MoveShader::ETA( const Point &p ) const;

////////////////////////////////////////////////////////////////////////
// NAME       : MoveShader::StopSearch
// DESCRIPTION: The shader never stops the search while there are nodes
//              left in the open list.
// PARAMETERS : next - path node to be checked next if search is not
//                     cancelled
// RETURNS    : TRUE if search aborted, FALSE otherwise
////////////////////////////////////////////////////////////////////////

// inline bool MoveShader::StopSearch( const PathNode &next ) const;

////////////////////////////////////////////////////////////////////////
// NAME       : MoveShader::AddNode
// DESCRIPTION: Calculate the cost for the unit to move from one hex to
//              another.
// PARAMETERS : u    - unit
//              from - path node for source hex
//              to   - destination path node
// RETURNS    : TRUE if step is allowed and the node should be added to
//              the open list, FALSE if step is not allowed
////////////////////////////////////////////////////////////////////////

bool MoveShader::AddNode( const Unit *u, const PathNode &from, PathNode &to ) const {
  unsigned short obst;
  short cost = map->MoveCost( u, from.pos, to.pos, obst );
  bool rc = false;

  if ( cost > u->Moves() - from.cost ) cost = -1;
  else if ( obst != 0 ) cost = u->Moves() - from.cost;

  if ( cost >= 0 ) {
    to.cost = from.cost + cost;
    to.eta = ETA( to.pos );
    rc = true;
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : MoveShader::FinalizePath
// DESCRIPTION: The shader doesn't have a path to prepare, but we must
//              check for valid targets which must not be shaded.
// PARAMETERS : u    - unit searching the path
//              last - final path node
// RETURNS    : always 1
////////////////////////////////////////////////////////////////////////

short MoveShader::FinalizePath( const Unit *u, const PathNode &last ) const {
  for ( Unit *target = static_cast<Unit *>(units.Head());
        target; target = static_cast<Unit *>(target->Next()) ) {
    if ( u->CanHit( target ) )
      path[map->Hex2Index( target->Position() )] = 1;
  }
  path[map->Hex2Index( u->Position() )] = 1;

  return 1;
}


////////////////////////////////////////////////////////////////////////
// NAME       : MinesweeperShader::AddNode
// DESCRIPTION: See whether there's a mine to be swept on a hex next to
//              the mine-sweeper unit. Enemy mines can only be cleared
//              if there is no other enemy unit (excluding mines) on a
//              field adjacent to the mine.
// PARAMETERS : u    - mine-sweeper unit
//              from - path node for source hex
//              to   - destination path node (adjacent to from)
// RETURNS    : TRUE if step is allowed and the node should be added to
//              the open list, FALSE if step is not allowed
////////////////////////////////////////////////////////////////////////

bool MinesweeperShader::AddNode( const Unit *u, const PathNode &from,
                                 PathNode &to ) const {
  bool rc = false;

  if ( NextTo( u->Position(), to.pos ) &&
     (u->Terrain() & map->TerrainTypes(to.pos)) ) {
    Unit *m = map->GetUnit( to.pos );

    if ( m && m->IsMine() ) {
      bool enemy = false;

      if ( m->Owner() != u->Owner() ) {
        Point adj[6];
        map->GetNeighbors( to.pos, adj );
        for ( int i = NORTH; (i <= NORTHWEST) && !enemy; ++i ) {
          if ( adj[i].x != -1 ) {
            Unit *e = map->GetUnit( adj[i] );
            if ( e && (e->Owner() == m->Owner()) && !e->IsMine() ) enemy = true;
          }
        }
      }

      if ( !enemy ) {
        to.cost = 0;
        to.eta = ETA( to.pos );
        rc = true;
      }
    }
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : MinesweeperShader::FinalizePath
// DESCRIPTION: The shader doesn't have a path to prepare.
// PARAMETERS : u    - unit searching the path
//              last - final path node
// RETURNS    : always 1
////////////////////////////////////////////////////////////////////////

short MinesweeperShader::FinalizePath( const Unit *u, const PathNode &last ) const {
  path[map->Hex2Index( u->Position() )] = 1;
  return 1;
}


////////////////////////////////////////////////////////////////////////
// NAME       : TransPath::TransPath
// DESCRIPTION: Create a new path object for finding a path from a
//              source to a destination via a transport. A single
//              TransPath does only half of the work, either the unit
//              getting to the transport or the transport containing the
//              unit getting to the destination. Therefore you always
//              need two TransPaths to really service a transportation
//              request. Always call TransPath::Find( <unit>, <transport
//              position>, <unit position or destination>, ... )!
// PARAMETERS : map    - map to use for pathfinding
//              trans  - transport which will carry the unit
//              buffer - buffer to store path in; if non-NULL must be
//                       large enough to hold at least (map height x
//                       map width) bytes. If NULL, a new buffer will
//                       be allocated and freed when the path object is
//                       destroyed.
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

TransPath::TransPath( Map *map, const Transport *trans, signed char *buffer ) :
           Path( map, buffer ) {
  t = trans;
}

////////////////////////////////////////////////////////////////////////
// NAME       : TransPath::AddNode
// DESCRIPTION: Calculate the cost for the unit to move from one hex to
//              another.
// PARAMETERS : u    - unit
//              from - path node for source hex
//              to   - destination path node (adjacent to from)
// RETURNS    : TRUE if step is allowed and the node should be added to
//              the open list, FALSE if step is not allowed
////////////////////////////////////////////////////////////////////////

bool TransPath::AddNode( const Unit *u, const PathNode &from, PathNode &to ) const {
  const Unit *cur = (from.switched ? u : t);
  bool rc = false;

  // important for all checks: on a TransPath the path is checked in the
  // direction Transport->Destination where destination may be the unit
  // or the final destination. This means that the unit must have made at
  // least one step before reaching the destination (switched == true)
  // for any path to be legal.
  short cost = -1;
  const TerrainType *type = map->HexType( to.pos );
  bool hexok = (type->tt_type & cur->Terrain()) != 0;

  Unit *block = map->GetUnit( to.pos );
  if ( (to.pos != end) || from.switched ) {
    if ( block ) {
      if ( block->IsTransport() && (to.pos == end) && from.switched &&
           !cur->IsSheltered() && static_cast<Transport *>(block)->Allow(cur) )
        cost = MAX( cur->Moves() - from.cost, MCOST_MIN );
      else if ( hexok ) cost = MCOST_UNIT;
    } else {
      unsigned short state;
      cost = map->MoveCost( cur, from.pos, to.pos, state );
      if ( state != 0 ) {
        if ( from.cost >= cur->Moves() ) cost = cur->Moves();
        else cost = MAX( cur->Moves() - from.cost, cost );
      }
    }
  }

  if ( cost != -1 ) {
    to.cost = from.cost + cost;
    to.eta = ETA( to.pos );
    to.switched = from.switched;
    rc = true;
  } else if ( !from.switched ) {
    // let's see if we can continue here when switching to the other unit
    PathNode from2 = from;
    from2.switched = to.switched = true;
    rc = AddNode( u, from2, to );
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : TransPath::Reverse
// DESCRIPTION: Rerecord a path in the opposite direction.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TransPath::Reverse( void ) {
  Point p( start );
  short index = map->Hex2Index( p );
  Direction dir = (Direction)path[index], dir2;
  path[index] = -1;

  while ( p != end ) {
    map->Dir2Hex( p, dir, p );
    index = map->Hex2Index( p );
    dir2 = (Direction)path[index];
    path[index] = ReverseDir( dir );
    dir = dir2;
  }

  end = start;
  start = p;
}

