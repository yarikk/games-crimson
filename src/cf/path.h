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
// path.h - pathfinding
/////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDE_PATH_H
#define _INCLUDE_PATH_H

#include "map.h"
#include "misc.h"

#define PATH_BEST	1
#define PATH_GOOD	4
#define PATH_FAST	10

struct PathNode {
  Point pos;
  unsigned short eta;   // estimated time of arrival
  unsigned short cost;  // travelling cost so far
  bool switched;        // only used by TransPath

  bool operator>(const PathNode &p) const
       { return eta + cost > p.eta + p.cost; }
};


class BasicPath {
public:
  BasicPath( Map *map, signed char *buffer );
  virtual ~BasicPath( void );

  void Clear( void );
  const Point &Destination( void ) const { return end; }
  signed char GetStep( const Point &current ) const
    { return path[map->Hex2Index(current)]; }

protected:
  void SetBuffer( signed char *buffer );
  short Find( const Unit *u, const Point &start, const Point &end );

  virtual unsigned short ETA( const Point &p ) const = 0;
  virtual bool StopSearch( const PathNode &next ) const = 0;
  virtual bool AddNode( const Unit *u, const PathNode &from,
                        PathNode &to ) const = 0;
  virtual short FinalizePath( const Unit *u, const PathNode &last ) const = 0;

  Map *map;
  signed char *path;
  Point start;
  Point end;

private:
  bool buf_private;     // delete buffer when path is destroyed
};


class Path : public BasicPath {
public:
  Path( Map *map, signed char *buffer = NULL ) : BasicPath(map, buffer) {}

  short Find( const Unit *u,
              const Point &start, const Point &end,
              unsigned char qual = PATH_BEST,
              unsigned char off = 0 );
  unsigned short StepsToDest( const Point &pos ) const;

protected:
  unsigned short ETA( const Point &p ) const;
  bool StopSearch( const PathNode &next ) const;
  virtual bool AddNode( const Unit *u, const PathNode &from,
                        PathNode &to ) const;
  short FinalizePath( const Unit *u, const PathNode &last ) const;

  unsigned char quality;
  unsigned char deviation;
};

class MoveShader : public BasicPath {
public:
  MoveShader( Map *map, const List &units, signed char *buffer = NULL ) :
              BasicPath(map, buffer), units(units) {}
  void ShadeMap( const Unit *u )
            { Find( u, u->Position(), Point(0,0) ); }

protected:
  unsigned short ETA( const Point &p ) const { return 1; }
  bool StopSearch( const PathNode &next ) const { return false; }
  virtual bool AddNode( const Unit *u, const PathNode &from,
                        PathNode &to ) const;
  virtual short FinalizePath( const Unit *u, const PathNode &last ) const;

  const List &units;
};

class MinesweeperShader : public MoveShader {
public:
  MinesweeperShader( Map *map, const List &units, signed char *buffer = NULL ) :
                    MoveShader( map, units, buffer ) {}

protected:
  bool AddNode( const Unit *u, const PathNode &from, PathNode &to ) const;
  short FinalizePath( const Unit *u, const PathNode &last ) const;
};


class TransPath : public Path {
public:
  TransPath( Map *map, const Transport *trans, signed char *buffer = NULL );
  void Reverse( void );

protected:
  bool AddNode( const Unit *u, const PathNode &from, PathNode &to ) const;

  const Transport *t;
};

#endif	/* _INCLUDE_PATH_H */

