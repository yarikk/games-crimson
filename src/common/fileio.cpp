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
// fileio.cpp
//
//  * additional code for Win32 by Adam Gates <radad@xoasis.com>
//  * additional code for WinCE/Unicode by Maik Stohn <maik@stohn.de>
////////////////////////////////////////////////////////////////////////

#ifndef _WIN32_WCE
# include <sys/stat.h>
#endif

#ifdef WIN32
# include <windows.h>
# include <shlobj.h>
# ifndef _WIN32_WCE
#  include <shellapi.h>
# endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL_zlib.h"
#include "fileio.h"
#include "globals.h"      // for CF_SHORTNAME

////////////////////////////////////////////////////////////////////////
// NAME       : Directory::Directory
// DESCRIPTION: Open a directory and initialize the first directory
//              entry for reading. Whether the directory was
//              successfully opened can be checked using
//              Directory::IsValid().
// PARAMETERS : dir - name of the directory to open
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Directory::Directory( const char *dir ) {
#ifdef WIN32
  string search( dir );
  append_path_delim( search );
  search += "*.*";

# ifdef UNICODE                     // unicode handling for WindowsCE
  WCHAR *wsearch = new WCHAR[search.length()+1];

  size_t len = mbstowcs( wsearch, search.c_str(), search.length() );
  if ( len < 0 ) len = 0;
  wsearch[len] = 0;

  m_Dir = FindFirstFile( wsearch, &m_Entry );

  delete [] wsearch;
# else
  m_Dir = FindFirstFile( search.c_str(), &m_Entry );
# endif
#else
  m_Entry = NULL;
  m_Dir = opendir( dir );

  if ( m_Dir ) m_Entry = readdir( m_Dir );
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Directory::~Directory
// DESCRIPTION: Close a directory.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Directory::~Directory( void ) {
#ifdef WIN32
  if ( m_Dir ) FindClose( m_Dir );
#else
  if ( m_Dir ) closedir( m_Dir );
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Directory::GetFileName
// DESCRIPTION: Get the name of the currently selected file in the
//              directory.
// PARAMETERS : -
// RETURNS    : pointer to file name or NULL if end of directory reached
//              (or not opened).
////////////////////////////////////////////////////////////////////////

const char *Directory::GetFileName( void ) const {
#ifdef WIN32
# ifdef UNICODE                            // unicode handling for WindowsCE
  size_t len = wcstombs( (char *)m_AsciiDir, m_Entry.cFileName, wcslen(m_Entry.cFileName) );
  if ( len < 0 ) len = 0;
  ((char *)m_AsciiDir)[len] = 0;
  return m_AsciiDir;
# else
  return m_Entry.cFileName;
# endif
#else
  if ( m_Entry ) return m_Entry->d_name;
  else return NULL;
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Directory::GetFileNameLen
// DESCRIPTION: Get the length of the currently selected file name.
// PARAMETERS : -
// RETURNS    : length of file name in characters
////////////////////////////////////////////////////////////////////////

size_t Directory::GetFileNameLen( void ) const {
#ifdef WIN32
# ifdef UNICODE
  return wcslen( m_Entry.cFileName );
# else
  return strlen( m_Entry.cFileName );
# endif
#else
  if ( m_Entry ) {
# ifdef HAVE_DIRENT_H
    return strlen( m_Entry->d_name );
# else
    return m_Entry->d_namlen;
# endif
  }
  return 0;
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Directory::IsFileHidden
// DESCRIPTION: Determine whether the currently selected file is a
//              hidden file.
// PARAMETERS : -
// RETURNS    : TRUE if file is a hidden file, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool Directory::IsFileHidden( void ) const {
#ifdef WIN32
  return (m_Entry.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
#else
  if ( m_Entry ) return m_Entry->d_name[0] == '.';
  else return false;
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : Directory::NextFile
// DESCRIPTION: Initialize the next file in the directory for
//              examination.
// PARAMETERS : -
// RETURNS    : TRUE if another file was found, FALSE if the end of the
//              directory was reached.
////////////////////////////////////////////////////////////////////////

bool Directory::NextFile( void ) {
#ifdef WIN32
  return FindNextFile( m_Dir, &m_Entry ) != FALSE;
#else
  if ( m_Dir ) m_Entry = readdir( m_Dir );
  return m_Entry != NULL;
#endif
}


////////////////////////////////////////////////////////////////////////
// NAME       : File::Close
// DESCRIPTION: Close the file handle.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void File::Close( void ) {
  if ( fh != 0 ) {
    SDL_RWclose( fh );
    fh = 0;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : File::Open
// DESCRIPTION: Open the file.
// PARAMETERS : mode       - file access descriptors (r, w, b)
//              compressed - whether to open for compression. This
//                           parameter should only be used when opening
//                           a file for writing. Default is true.
// RETURNS    : TRUE on success, FALSE on error
////////////////////////////////////////////////////////////////////////

bool File::Open( const char *mode, bool compressed /* = true */ ) {
  bool rc = false;

  if ( !fh ) {
    if ( compressed ) fh = SDL_RWFromGzip( name.c_str(), mode );
    else fh = SDL_RWFromFile( name.c_str(), mode );
    rc = (fh != 0);
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : File::OpenData
// DESCRIPTION: Open the file. The "path" parameter passed to the
//              constructor is treated as a path relative to the data
//              (sub)directory used by the game. This method first tries
//              to open the file from the user's home directory (if
//              available), and only if this attempt fails the file is
//              loaded from the system data directory. After a
//              successful call, File::Name() returns the full path to
//              the file.
// PARAMETERS : subdir     - data subdirectory to look in
//              mode       - file access descriptors (r, w, b)
//              compressed - whether to open for compression. This
//                           parameter should only be used when opening
//                           a file for writing. Default is true.
// RETURNS    : TRUE on success, FALSE on error
////////////////////////////////////////////////////////////////////////

bool File::OpenData( const char *mode, const string &subdir /* = "" */,
                     bool compressed /* = true */ ) {
  bool rc = false;

  if ( !fh ) {
    string local( name );
    name = get_home_dir();
    if ( !name.empty() ) {
      append_path( name, subdir );
      append_path( name, local );
      rc = Open( mode, compressed );
    }

    if ( !rc ) {
      name = get_data_subdir( subdir );
      append_path( name, local );
      rc = Open( mode, compressed );
    }
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : File::Exists
// DESCRIPTION: Check whether the filename is already in use.
// PARAMETERS : name - file name to check for
// RETURNS    : TRUE if file exists, FALSE otherwise
////////////////////////////////////////////////////////////////////////

bool File::Exists( const string &name ) {
  // try to open the file
  FILE *fd = fopen( name.c_str(), "r" );
  if ( fd ) fclose( fd );
  return fd != NULL;
}

////////////////////////////////////////////////////////////////////////
// NAME       : File::Read8
// DESCRIPTION: Read a byte from the buffer.
// PARAMETERS : -
// RETURNS    : byte read
////////////////////////////////////////////////////////////////////////

unsigned char File::Read8( void ) {
  unsigned char val;
  Read( &val, 1 );
  return val;
}

////////////////////////////////////////////////////////////////////////
// NAME       : MemBuffer::ReadS
// DESCRIPTION: Read a string from the buffer.
// PARAMETERS : size - number of bytes to read
// RETURNS    : string read
////////////////////////////////////////////////////////////////////////

string MemBuffer::ReadS( int size ) {
  string str;

  for ( int i = 0; i < size; ++i )
    str += Read8();

  return str;
}

////////////////////////////////////////////////////////////////////////
// NAME       : MemBuffer::WriteS
// DESCRIPTION: Write a string to the buffer.
// PARAMETERS : value - string to write
//              len   - minimum number of bytes to write. If value is
//                      shorter than len, a number of NUL-bytes are
//                      written to the buffer.
// RETURNS    : -1 on error
////////////////////////////////////////////////////////////////////////

int MemBuffer::WriteS( string value, int len /* = 0 */ ) {
  int size = value.size();
  int rc = Write( value.c_str(), size );

  while ( (size < len) && (rc == 0) ) {
    rc = Write8( 0 );
    ++size;
  }
  return rc;
}


////////////////////////////////////////////////////////////////////////
// NAME       : append_path_delim
// DESCRIPTION: Append a path delimiter to the end of the string if
//              it isn't already there.
// PARAMETERS : path - string containing a path
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void append_path_delim( string &path ) {
  if ( path.at( path.length() - 1 ) != PATHDELIM ) path += PATHDELIM;
}

////////////////////////////////////////////////////////////////////////
// NAME       : make_dir
// DESCRIPTION: Create a directory.
// PARAMETERS : dir - name of the directory to create
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void make_dir( const char *dir ) {
#ifdef WIN32
# ifdef UNICODE
  WCHAR *wdir = new WCHAR[strlen(dir)+1];
  size_t len = mbstowcs( wdir, dir, strlen(dir) );
  if ( len < 0 ) len = 0;
  wdir[len] = 0;
  CreateDirectory( wdir, NULL );
  delete [] wdir;
# else
  CreateDirectory( dir, NULL );
# endif
#else
  mkdir( dir, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH );
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : create_config_dir
// DESCRIPTION: Create a configuration and save directory in the user's
//              home dir (UNIX & Co.) or his My Documents folder
//              (Win32).
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void create_config_dir( void ) {
  string confd = get_config_dir();

  // does the directory exist?
  Directory cfdir( confd.c_str() );
  if ( !cfdir.IsValid() ) make_dir( confd.c_str() );

  // now check for the saved games directory
  append_path( confd, "games" );

  Directory svdir( confd.c_str() );
  if ( !svdir.IsValid() ) make_dir( confd.c_str() );
}

////////////////////////////////////////////////////////////////////////
// NAME       : get_home_dir
// DESCRIPTION: Get the name of the user's home directory.
// PARAMETERS : -
// RETURNS    : the user's home directory if any
////////////////////////////////////////////////////////////////////////

string get_home_dir( void ) {
#ifdef WIN32
  TCHAR dir[MAX_PATH];
  SHGetSpecialFolderPath( NULL, dir, CSIDL_PERSONAL, TRUE );
# ifdef UNICODE
  char mbdir[MAX_PATH];
  wcstombs( mbdir, dir, wcslen(dir)+1 );
  return mbdir;
# else
  return dir;
# endif
#elif defined __BEOS__
  return "./";
#else
  return getenv( "HOME" );
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : get_config_dir
// DESCRIPTION: Get the name of the configuration directory.
// PARAMETERS : -
// RETURNS    : config directory
////////////////////////////////////////////////////////////////////////

string get_config_dir( void ) {
  string confd;
  string homed( get_home_dir() );

#ifdef WIN32
  if ( homed.empty() ) confd = get_data_dir();
#else
  if ( homed.empty() ) confd.append( CURRENTDIR );
#endif
  else {
    confd.append( homed );
    append_path_delim( confd );
#ifndef WIN32
    confd += '.';
#endif
    confd.append( CF_SHORTNAME );
  }
  append_path_delim( confd );

  return confd;
}

////////////////////////////////////////////////////////////////////////
// NAME       : get_data_dir
// DESCRIPTION: Get the name of the directory containing the data files.
// PARAMETERS : -
// RETURNS    : data directory name
////////////////////////////////////////////////////////////////////////

string get_data_dir( void ) {

#ifdef WIN32
  char dir[MAX_PATH];

# ifdef UNICODE
  WCHAR wdir[MAX_PATH];
  GetModuleFileName( NULL, wdir, MAX_PATH );
  size_t len = wcstombs( dir, wdir, wcslen(wdir) );
  if( len < 0 ) len = 0;
  dir[len] = 0;
# else
  GetModuleFileName( NULL, dir, MAX_PATH );
# endif
  {
    // Remove the file name
    char *l = dir;
    char *c = dir;
    while( *c != '\0' ) {
      if ( *c == PATHDELIM ) l = c;
      ++c;
    }
    ++l;
    *l = '\0';
  }
  return dir;
#elif defined CF_DATADIR
  return CF_DATADIR;
#else
  return CURRENTDIR;
#endif
}

////////////////////////////////////////////////////////////////////////
// NAME       : get_save_dir
// DESCRIPTION: Get the name of the directory to save games in.
// PARAMETERS : -
// RETURNS    : saved games directory
////////////////////////////////////////////////////////////////////////

string get_save_dir( void ) {
  string saved( get_config_dir() );

  saved.append( "games" );
  append_path_delim( saved );

  return saved;
}

////////////////////////////////////////////////////////////////////////
// NAME       : get_data_subdir
// DESCRIPTION: Get the full path of a data subdirectory.
// PARAMETERS : sub - name of the subdirectory
// RETURNS    : full path to directory (including trailing slash)
////////////////////////////////////////////////////////////////////////

string get_data_subdir( const string &sub ) {
  string d( get_data_dir() );
  append_path( d, sub );
  append_path_delim( d );
  return d;
}

////////////////////////////////////////////////////////////////////////
// NAME       : get_sfx_dir
// DESCRIPTION: Get the name of the directory containing the sound
//              effects.
// PARAMETERS : -
// RETURNS    : sound effects directory
////////////////////////////////////////////////////////////////////////

string get_sfx_dir( void ) {
  return get_data_subdir( "sound" );
}

////////////////////////////////////////////////////////////////////////
// NAME       : get_music_dir
// DESCRIPTION: Get the name of the directory containing the music
//              tracks.
// PARAMETERS : -
// RETURNS    : soundtracks directory
////////////////////////////////////////////////////////////////////////

string get_music_dir( void ) {
  return get_data_subdir( "music" );
}

////////////////////////////////////////////////////////////////////////
// NAME       : get_levels_dir
// DESCRIPTION: Get the name of the directory containing the map files.
// PARAMETERS : -
// RETURNS    : levels directory name
////////////////////////////////////////////////////////////////////////

string get_levels_dir( void ) {
  return get_data_subdir( "levels" );
}

////////////////////////////////////////////////////////////////////////
// NAME       : get_locale_dir
// DESCRIPTION: Get the name of the directory containing the language
//              data files.
// PARAMETERS : -
// RETURNS    : locale directory name
////////////////////////////////////////////////////////////////////////

string get_locale_dir( void ) {
  return get_data_subdir( "locale" );
}

////////////////////////////////////////////////////////////////////////
// NAME       : get_home_levels_dir
// DESCRIPTION: Get the name of an optional directory in the user's home
//              directory containing additional map files.
// PARAMETERS : -
// RETURNS    : levels directory name or an empty string if it does not
//              exist
////////////////////////////////////////////////////////////////////////

string get_home_levels_dir( void ) {
  string hld("");

#ifndef WIN32
  string homed( get_home_dir() );
  if ( !homed.empty() ) {
    hld = get_config_dir();
    hld.append( "levels" );
    append_path_delim( hld );
  }
#endif

  return hld;
}

////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Extract the name of a file from a given path.
// PARAMETERS : path - filename (including full or partial path)
// RETURNS    : file part of the path
////////////////////////////////////////////////////////////////////////

string file_part( const string &path ) {
  int pathend = path.rfind( PATHDELIM );
  return path.substr( pathend + 1 );
}

////////////////////////////////////////////////////////////////////////
// NAME       : append_path
// DESCRIPTION: Add another subpath to an existing path. Also add path
//              delimiter if required.
// PARAMETERS : path - existing path to be extended
//              sub  - subpath to be added
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void append_path( string &path, const string &sub ) {
  append_path_delim( path );
  path.append( sub );
}

