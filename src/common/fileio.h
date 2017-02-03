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
// fileio.h
////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDE_FILEIO_H
#define _INCLUDE_FILEIO_H

#if defined WIN32
# include <windows.h>
#elif HAVE_DIRENT_H
# include <dirent.h>
#elif HAVE_SYS_NDIR_H
# include <sys/ndir.h>
#elif HAVE_SYS_DIR_H
# include <sys/dir.h>
#elif HAVE_NDIR_H
# include <ndir.h>
#endif

#include <string>
using namespace std;

#include "SDL.h"
#include "SDL_endian.h"

#ifdef WIN32
#define PATHDELIM  '\\'
#define CRIMSONRC  "crimson.ini"
#else
#define PATHDELIM  '/'
#define CRIMSONRC  "crimsonrc"
#endif

#define CURRENTDIR  "."

class Directory {
public:
  Directory( const char *dir );
  ~Directory( void );

  bool IsValid( void ) const
#ifdef WIN32
    { return m_Dir != INVALID_HANDLE_VALUE; }
#else
    { return m_Dir != NULL; }
#endif

  const char *GetFileName( void ) const;
  size_t GetFileNameLen() const;
  bool IsFileHidden( void ) const;
  bool NextFile( void );

private:
#ifdef WIN32
  HANDLE           m_Dir;
  WIN32_FIND_DATA  m_Entry;
# ifdef UNICODE
  char             m_AsciiDir[MAX_PATH];
# endif
#elif HAVE_DIRENT_H
  DIR    *m_Dir;
  dirent *m_Entry;
#else
  DIR    *m_Dir;
  direct *m_Entry;
#endif
};

// generic buffer access
class MemBuffer {
public:
  MemBuffer( void ) {}
  virtual ~MemBuffer( void ) {}

  virtual int Read( void *buffer, int size ) = 0;
  virtual unsigned char Read8( void ) = 0;
  virtual unsigned short Read16( void ) = 0;
  virtual unsigned long Read32( void ) = 0;
  string ReadS( int size );

  virtual int Write( const void *values, int size ) = 0;
  virtual int Write8( unsigned char value ) = 0;
  virtual int Write16( unsigned short value ) = 0;
  virtual int Write32( unsigned long value ) = 0;
  int WriteS( string value, int len = 0 );
};

// file abstraction to encapsulate SDL file access layer
class File : public MemBuffer {
public:
  File( const string &path ) : fh(0), name(path) {}
  ~File( void ) { if (fh) Close(); }

  int Read( void *buffer, int size )
      { return SDL_RWread(fh, buffer, 1, size); }
  unsigned char Read8( void );
  unsigned short Read16( void ) { return SDL_ReadLE16(fh); }
  unsigned long Read32( void ) { return SDL_ReadLE32(fh); }

  int Write( const void *values, int size )
      { return (SDL_RWwrite(fh, values, size, 1) == 1) ? 0 : -1; }
  int Write8( unsigned char value )
      { return (SDL_RWwrite(fh, &value, 1, 1) == 1) ? 0 : -1; }
  int Write16( unsigned short value )
      { return (SDL_WriteLE16(fh, value) == 1) ? 0 : -1; }
  int Write32( unsigned long value )
      { return (SDL_WriteLE32(fh, value) == 1) ? 0 : -1; }

  bool Open( const char *mode, bool compressed = true );
  bool OpenData( const char *mode, const string &subdir = "", bool compressed = true );
  void Close( void );
  static bool Exists( const string &name );
  const string &Name( void ) const { return name; }

private:
  SDL_RWops *fh;
  string name;
};

string get_config_dir( void );
string get_home_dir( void );
string get_save_dir( void );
string get_data_dir( void );
string get_data_subdir( const string &sub );
string get_sfx_dir( void );
string get_music_dir( void );
string get_levels_dir( void );
string get_locale_dir( void );
string get_home_levels_dir( void );
void create_config_dir( void );

string file_part( const string &path );
void append_path( string &path, const string &sub );
void append_path_delim( string &path );
void make_dir( const char *dir );

#endif  /* _INCLUDE_FILEIO_H */

