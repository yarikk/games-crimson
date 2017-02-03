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
// list.cpp -- contains both List and Node class definitions
////////////////////////////////////////////////////////////////////////

#include "list.h"

// List class

////////////////////////////////////////////////////////////////////////
// NAME       : List::List
// DESCRIPTION: Initialize the List object.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

List::List( void ) {
  lh_Head.ln_Prev = 0;
  lh_Head.ln_Next = &lh_Tail;
  lh_Tail.ln_Prev = &lh_Head;
  lh_Tail.ln_Next = 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : List::Clear
// DESCRIPTION: Deallocate all list nodes and the list itself.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void List::Clear( void ) {
  while ( !IsEmpty() ) delete RemHead();
}

////////////////////////////////////////////////////////////////////////
// NAME       : List::AddHead
// DESCRIPTION: Insert a node at the head of the list.
// PARAMETERS : n - node to insert
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void List::AddHead( Node *n ) {
  InsertNode( n, &lh_Head );
}

////////////////////////////////////////////////////////////////////////
// NAME       : List::AddTail
// DESCRIPTION: Insert a node at the tail of the list.
// PARAMETERS : n - node to insert
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void List::AddTail( Node *n ) {
  InsertNode( n, lh_Tail.ln_Prev );
}

////////////////////////////////////////////////////////////////////////
// NAME       : List::RemHead
// DESCRIPTION: Remove the first node from the list. The function does
//              NOT check whether the list is already empty!
// PARAMETERS : -
// RETURNS    : former head node
////////////////////////////////////////////////////////////////////////

Node *List::RemHead( void ) {
  Node *n = lh_Head.ln_Next;
  n->Remove();
  return n;
}

////////////////////////////////////////////////////////////////////////
// NAME       : List::RemTail
// DESCRIPTION: Remove the last node from the list. The function does
//              NOT check whether the list is already empty!
// PARAMETERS : -
// RETURNS    : former tail node
////////////////////////////////////////////////////////////////////////

Node *List::RemTail( void ) {
  Node *n = lh_Tail.ln_Prev;
  n->Remove();
  return n;
}

////////////////////////////////////////////////////////////////////////
// NAME       : List::NextNode
// DESCRIPTION: Get the next node from the list. If the current node is
//              the list tail, this function will start from the head
//              again, so it can be used like a circular list.
// PARAMETERS : n - current node
// RETURNS    : next node in circular order or NULL if list is empty
////////////////////////////////////////////////////////////////////////

Node *List::NextNode( const Node *n ) const {
  if ( n == 0 ) return Head();
  else if ( IsEmpty() ) return 0;
  else {
    Node *next = n->Next();
    if ( next ) return next;
    else return Head();
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : List::InsertNode
// DESCRIPTION: Put a new node into the list.
// PARAMETERS : n    - node to insert
//              prev - node after which to insert n; if NULL make n the
//                     new list head.
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void List::InsertNode( Node *const n, Node *prev ) {
  if ( prev == 0 ) prev = &lh_Head;
  n->ln_Next = prev->ln_Next;
  n->ln_Next->ln_Prev = n;
  n->ln_Prev = prev;
  prev->ln_Next = n;
}

////////////////////////////////////////////////////////////////////////
// NAME       : List::Head
// DESCRIPTION: Get the first node from the list.
// PARAMETERS : -
// RETURNS    : first list node or NULL if list is empty
////////////////////////////////////////////////////////////////////////

Node *List::Head( void ) const {
  if ( IsEmpty() ) return 0;
  return lh_Head.ln_Next;
}

////////////////////////////////////////////////////////////////////////
// NAME       : List::Tail
// DESCRIPTION: Get the last node from the list.
// PARAMETERS : -
// RETURNS    : last list node or NULL if list is empty
////////////////////////////////////////////////////////////////////////

Node *List::Tail( void ) const {
  if ( IsEmpty() ) return 0;
  return lh_Tail.ln_Prev;
}

////////////////////////////////////////////////////////////////////////
// NAME       : List::GetNode
// DESCRIPTION: Get the n-th node from the list.
// PARAMETERS : n - position of the node in the list
// RETURNS    : n-th node or NULL if list contains fewer nodes
////////////////////////////////////////////////////////////////////////

Node *List::GetNode( unsigned short n ) const {
  Node *node = lh_Head.ln_Next;

  while ( node->ln_Next ) {
    if ( n == 0 ) return node;
    --n;
    node = node->ln_Next;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : List::CountNodes
// DESCRIPTION: Get the number of list items.
// PARAMETERS : -
// RETURNS    : number of nodes in the list
////////////////////////////////////////////////////////////////////////

unsigned short List::CountNodes( void ) const {
  Node *n = lh_Head.ln_Next;
  unsigned short cnt = 0;

  while ( n->ln_Next ) {
    ++cnt;
    n = n->ln_Next;
  }
  return cnt;
}


// Node class

////////////////////////////////////////////////////////////////////////
// NAME       : Node::Remove
// DESCRIPTION: Remove the node from its list. The function does NOT
//              check whether the node actually IS in a list.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Node::Remove( void ) {
  ln_Next->ln_Prev = ln_Prev;
  ln_Prev->ln_Next = ln_Next;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Node::Next
// DESCRIPTION: Get the node following this node.
// PARAMETERS : -
// RETURNS    : next node or NULL if node is last node
////////////////////////////////////////////////////////////////////////

Node *Node::Next( void ) const {
  Node *n = ln_Next;
  if ( n->ln_Next ) return n;
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Node::Prev
// DESCRIPTION: Get the node's predecessor in the list.
// PARAMETERS : -
// RETURNS    : previous node or NULL if node is list head
////////////////////////////////////////////////////////////////////////

Node *Node::Prev( void ) const {
  Node *n = ln_Prev;
  if ( n->ln_Prev ) return n;
  return 0;
}

