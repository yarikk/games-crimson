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
// list.h - List and Node classes
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_LIST_H
#define _INCLUDE_LIST_H

// class definitions

class Node {
public:
  friend class List;

  Node( void ) {};
  virtual ~Node( void ) {};
  void Remove( void );

  Node *Next( void ) const;
  Node *Prev( void ) const;

private:
  Node *ln_Next;
  Node *ln_Prev;
};


class List {
public:
  List( void );
  ~List( void ) { Clear(); }

  void Clear( void );
  bool IsEmpty( void ) const
       { return( lh_Tail.ln_Prev == &lh_Head ); }

  void AddHead( Node *n );
  void AddTail( Node *n );
  void InsertNode( Node *n, Node *prev );

  Node *RemHead( void );
  Node *RemTail( void );

  Node *Head( void ) const;
  Node *Tail( void ) const;
  Node *GetNode( unsigned short n ) const;
  Node *NextNode( const Node *n ) const;

  unsigned short CountNodes( void ) const;

private:
  Node lh_Head;
  Node lh_Tail;
};

#endif	/* _INCLUDE_LIST_H */

