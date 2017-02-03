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
// network.h - networking
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_NETWORK_H
#define _INCLUDE_NETWORK_H

#ifndef DISABLE_NETWORK

#include "SDL_net.h"
#include "fileio.h"
#include "widget.h"  // for UserActionHook

class DynBuffer : public MemBuffer {
public:
  DynBuffer( void );
  DynBuffer( unsigned long capacity );
  ~DynBuffer( void );

  unsigned long Size( void ) const { return size; }
  int SetSize( unsigned long size );

  int Read( void *buffer, int size );
  unsigned char Read8( void ) { return mem[pos++]; }
  unsigned short Read16( void )
    { unsigned short v = SDL_SwapLE16( *((Uint16 *)&mem[pos]) ); pos += 2; return v; }
  unsigned long Read32( void )
    { unsigned long v = SDL_SwapLE32( *((Uint32 *)&mem[pos]) ); pos += 4; return v; }

  int Write( const void *values, int size );
  int Write8( unsigned char value ) { return Write( &value, 1 ); }
  int Write16( unsigned short value )
    { value = SDL_SwapLE16( value ); return Write( &value, 2 ); }
  int Write32( unsigned long value )
    { value = SDL_SwapLE32( value ); return Write( &value, 4 ); }

  char *GetData( void ) const { return mem; }

private:
  int Init( unsigned long len );
  int CheckResize( unsigned long len );

  char *mem;
  unsigned long size;
  unsigned long capacity;
  unsigned long pos;
};

class TCPConnection {
public:
  TCPConnection( void ) : socket(0), client(0), set(0) {}
  ~TCPConnection( void ) { Close(); }

  int Open( const char *ipaddr, Uint16 port, UserActionHook *hook );
  void Close( void );

  bool Send( DynBuffer &buf );
  DynBuffer *Receive( UserActionHook *hook );

private:
  bool Read( TCPsocket socket, char *buf, Uint32 size,
             UserActionHook *hook );

  TCPsocket socket;
  TCPsocket client;
  SDLNet_SocketSet set;
  bool server;
};

#endif /* DISABLE_NETWORK */

class Network {
public:
  static int Init( void );
  static void Shutdown( void );
};

#endif  /* _INCLUDE_NETWORK_H */
