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
// network.cpp - networking
////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "network.h"

#ifndef DISABLE_NETWORK

#include "misc.h"

#define DEFAULT_CAPACITY  512

////////////////////////////////////////////////////////////////////////
// NAME       : DynBuffer::DynBuffer
// DESCRIPTION: Create a new dynamic buffer with the default capacity in
//              memory. When using the Write*() methods defined for this
//              class, the buffer will grow dynamically as required.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

DynBuffer::DynBuffer( void ) : mem(0), size(0), capacity(0), pos(0) {
  Init( DEFAULT_CAPACITY);
}

////////////////////////////////////////////////////////////////////////
// NAME       : DynBuffer::DynBuffer
// DESCRIPTION: Create a new buffer in memory.
// PARAMETERS : capacity - desired initial buffer size
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

DynBuffer::DynBuffer( unsigned long capacity ) : mem(0), size(0), capacity(0), pos(0) {
  Init( capacity );
}

////////////////////////////////////////////////////////////////////////
// NAME       : DynBuffer::~DynBuffer
// DESCRIPTION: Free the buffer.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

DynBuffer::~DynBuffer( void ) {
  if (mem) free( mem );
}

////////////////////////////////////////////////////////////////////////
// NAME       : DynBuffer::Init
// DESCRIPTION: Initialize a new buffer in memory.
// PARAMETERS : len - desired initial buffer size
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int DynBuffer::Init( unsigned long len ) {
  mem = (char *)malloc( len );

  if (mem) {
    capacity = len;
    return 0;
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////
// NAME       : DynBuffer::CheckResize
// DESCRIPTION: Check whether a given amount of data fits into the
//              currently allocated buffer. If it doesn't, resize the
//              buffer.
// PARAMETERS : len - total amount of data that the buffer must be able
//                    to hold
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int DynBuffer::CheckResize( unsigned long len ) {
  int rc = 0;

  if (len > capacity)  {
    unsigned long new_capacity = MAX( len, capacity * 2 );

    mem = (char *)realloc( mem, new_capacity );

    if (mem)
      capacity = new_capacity;
    else
      rc = -1;
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : DynBuffer::SetSize
// DESCRIPTION: Set buffer to a given size. Current position inside the
//              buffer is not modified, unless the buffer size is
//              reduced and the position is outside the valid buffer
//              after the resize. In that case, position is moved to the
//              end of the buffer.
// PARAMETERS : size - size to set
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int DynBuffer::SetSize( unsigned long size ) {
  int rc;

  if (size > capacity) {
    rc = CheckResize( size );
  } else {
    // buffer reduction will always succeed
    rc = 0;
    pos = MIN( size, pos );
  }

  this->size = size;
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : DynBuffer::Read
// DESCRIPTION: Read a number of bytes from the buffer.
// PARAMETERS : buffer - buffer to read into
//              size   - amount of data to read
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int DynBuffer::Read( void *buffer, int size ) {
  if (pos + size > this->size)
    return -1;

  memcpy( buffer, &mem[pos], size );
  pos += size;

  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : DynBuffer::Write
// DESCRIPTION: Write a number of bytes to the buffer.
// PARAMETERS : buffer - buffer to read from
//              size   - amount of data to write
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int DynBuffer::Write( const void *buffer, int size ) {
  if (CheckResize( pos + size ) != 0) {
    return -1;
  }

  memcpy( &mem[pos], buffer, size );
  pos += size;
  this->size += size;

  return 0;
}

#define CF_PACKET_ID      0xcfcfcfcf
#define CF_PACKET_VERSION 0

enum {
  CF_PACKET_TYPE_DATA,
  CF_PACKET_TYPE_ACK    // might be used for unreliable protocols
};

extern "C" {
  struct TCPPacketHeader {
    Uint32 id;
    Uint8 version;
    Uint8 type;
    Uint16 reserved;
    Uint32 size;
  };
};

////////////////////////////////////////////////////////////////////////
// NAME       : TCPConnection::Open
// DESCRIPTION: Open the connection.
// PARAMETERS : ipaddr - IP address/server name for client connection/
//                       NULL for server connection
//              port   - port to connect to/to accept clients on
//              hook   - UserActionHook which can be used to abort this
//                       otherwise synchronous operation
// RETURNS    : 0 on success, -1 on error, 1 if user aborted
////////////////////////////////////////////////////////////////////////

int TCPConnection::Open( const char *ipaddr, Uint16 port, UserActionHook *hook ) {
  // create the socket set; since we'll only have one peer we don't
  // really need a set, but the SDLNet API doesn't support
  // asynchronous operation without it...
  if ( !set ) {
    set = SDLNet_AllocSocketSet( 1 );
    if ( !set ) {
      fprintf( stderr, "SDLNet_AllocSocketSet failed\n" );
      return -1;
    }
  }

  // create the socket
  if ( !socket ) {
    IPaddress ip;

    server = (ipaddr == NULL);

    if ( !SDLNet_ResolveHost( &ip, ipaddr, port ) ) {
      socket = SDLNet_TCP_Open( &ip );

      if ( !socket ) {
        fprintf( stderr, "SDLNet_Open: %s\n", SDLNet_GetError() );
        return -1;
      }

    } else {
      fprintf( stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError() );
      return -1;
    }
  }

  if ( server ) {
    // wait for a client
    while ( !client ) {
      client = SDLNet_TCP_Accept( socket );

      if ( !client ) {
        if ( hook && hook->Cancelled() )
          return 1;

        SDL_Delay( 200 );
      } else {
        SDLNet_TCP_AddSocket( set, client );
      }
    }
  } else {
    SDLNet_TCP_AddSocket( set, socket );
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : TCPConnection::Close
// DESCRIPTION: Close the connection.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void TCPConnection::Close( void ) {
  if (client) {
    SDLNet_TCP_Close( client );
    client = NULL;
  }
  if (socket) {
    SDLNet_TCP_Close( socket );
    socket = NULL;
  }
  if (set) {
    SDLNet_FreeSocketSet( set );
    set = NULL;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : TCPConnection::Send
// DESCRIPTION: Send a buffer over the network.
// PARAMETERS : buf - buffer to send
// RETURNS    : TRUE if buffer was sent successfully, FALSE on error
////////////////////////////////////////////////////////////////////////

bool TCPConnection::Send( DynBuffer &buf ) {
  bool rc = false;
  TCPsocket dest;

  dest = (server ? client : socket);

  if (dest) {
    int len;
    struct TCPPacketHeader hdr;

    hdr.id = SDL_SwapLE32(CF_PACKET_ID);
    hdr.version = CF_PACKET_VERSION;
    hdr.type = CF_PACKET_TYPE_DATA;
    hdr.reserved = 0;
    hdr.size = SDL_SwapLE32(buf.Size());

    len = SDLNet_TCP_Send( dest, &hdr, sizeof(struct TCPPacketHeader) );
    if ( len != sizeof(struct TCPPacketHeader) ) {
      fprintf( stderr, "SDLNet_Send (header): %s\n", SDLNet_GetError() );
    } else {
      len = SDLNet_TCP_Send( dest, buf.GetData(), buf.Size() );
      rc = ((unsigned long)len == buf.Size());

      if (!rc)
        fprintf( stderr, "SDLNet_Send (data): %s\n", SDLNet_GetError() );
    }
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : TCPConnection::Receive
// DESCRIPTION: Receive a buffer over the network. If a buffer is
//              returned, it must be freed by the caller after use.
// PARAMETERS : hook - UserActionHook which can be used to abort this
//                     otherwise synchronous operation
// RETURNS    : received buffer or NULL on error or user abort
////////////////////////////////////////////////////////////////////////

DynBuffer *TCPConnection::Receive( UserActionHook *hook ) {
  DynBuffer *buf = NULL;
  TCPsocket src;

  src = (server ? client : socket);

  if ( src ) {
    struct TCPPacketHeader hdr;

    if ( !Read( src, (char *)&hdr, sizeof(struct TCPPacketHeader), hook ) )
      return NULL;

    hdr.id = SDL_SwapLE32(hdr.id);
    hdr.size = SDL_SwapLE32(hdr.size);

    if (hdr.id != CF_PACKET_ID)  {
      fprintf( stderr, "TCP_Receive: Received unknown ID %d\n", hdr.id );
      return NULL;
    } else if (hdr.version != CF_PACKET_VERSION)  {
      fprintf( stderr, "TCP_Receive: Received unsupported version %d\n", hdr.version );
      return NULL;
    } else if (hdr.type != CF_PACKET_TYPE_DATA)  {
      fprintf( stderr, "TCP_Receive: Received packet of type %d\n", hdr.type );
      return NULL;
    }

    buf = new DynBuffer( hdr.size );
    buf->SetSize( hdr.size );

    if ( !Read( src, buf->GetData(), hdr.size, hook ) ) {
      delete buf;
      return NULL;
    }
  }

  return buf;
}

////////////////////////////////////////////////////////////////////////
// NAME       : TCPConnection::Read
// DESCRIPTION: Receive a buffer of <size> bytes over the network. This
//              method waits until the requested amount of data has
//              arrived.
// PARAMETERS : socket - socket to read from
//              buf    - buffer to read into. The buffer must be big
//                       enough to hold the requested data.
//              size   - number of bytes to read
//              hook   - UserActionHook which can be used to abort this
//                       otherwise synchronous operation
// RETURNS    : TRUE on success, FALSE on error
////////////////////////////////////////////////////////////////////////

bool TCPConnection::Read( TCPsocket socket, char *buf, Uint32 size,
                          UserActionHook *hook ) {
  int len;
  unsigned int read = 0;

  do  {
    // we don't want SDLNet_TCP_Recv to block, so we check in advance
    // whether there is any data waiting for us
    int ready = SDLNet_CheckSockets( set, 0 );
    if ( ready < 0 ) {
      return false;

    } else if ( ready == 0 ) {
      if ( hook && hook->Cancelled() )
        return false;
      SDL_Delay( 200 );

    } else {
      // data available
      len = SDLNet_TCP_Recv( socket, buf + read, size - read );

      if ( len <= 0 ) {
        fprintf( stderr, "SDLNet_TCP_Recv: %s\n", SDLNet_GetError() );
        return false;
      }

      read += len;
    }

  } while (read < size);

  return true;
}

#endif // DISABLE_NETWORK

////////////////////////////////////////////////////////////////////////
// NAME       : Network::Init
// DESCRIPTION: Initialize the networking subsystem.
// PARAMETERS : -
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Network::Init( void ) {
  int rc = 0;

#ifndef DISABLE_NETWORK
  if ( SDLNet_Init() != 0 ) {
    rc = -1;
    fprintf( stderr, "SDLNet_Init: %s\n", SDLNet_GetError() );
  }
#endif
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Network::Shutdown
// DESCRIPTION: Shutdown the networking subsystem.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Network::Shutdown( void ) {
#ifndef DISABLE_NETWORK
  SDLNet_Quit();
#endif
}
