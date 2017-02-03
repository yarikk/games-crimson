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
// lang.cpp
////////////////////////////////////////////////////////////////////////

#include <algorithm>

#include "lang.h"
#include "strutil.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Language::ReadCatalog
// DESCRIPTION: Load the messages for this language from a catalog file.
// PARAMETERS : catalog - language catalog file name
// RETURNS    : number of messages read (>= 0) on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Language::ReadCatalog( const char *catalog ) {
  File file( catalog );
  int rc = -1;

  if ( file.Open( "rb" ) &&
       (file.Read32() == FID_CATALOG) &&
       (file.Read8() == CF_CATALOG_VERSION) ) {
    unsigned char len = file.Read8();
    name = file.ReadS( len );

    rc = ReadCatalog( file );
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Language::ReadCatalog
// DESCRIPTION: Load the messages for this language from a catalog file.
// PARAMETERS : file - language catalog file descriptor
// RETURNS    : number of messages read (>= 0) on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Language::ReadCatalog( MemBuffer &file ) {
  int rc;

  // dump old messages
  cat.clear();

  id = file.ReadS( 2 );

  // load messages
  unsigned long msg_len = file.Read32();
  char *ptr, *msgs = new char[msg_len];

  file.Read( msgs, msg_len );

  ptr = msgs;
  while ( ptr < &msgs[msg_len] ) {
    string text = StringUtil::crypt( ptr );
    AddMsg( text.c_str() );

    while ( *ptr != '\0' ) ++ptr;
    ++ptr;
  }

  delete [] msgs;

  rc = cat.size();

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Language::WriteCatalog
// DESCRIPTION: Save the messages for this language to a catalog file.
// PARAMETERS : catalog - language catalog file name
// RETURNS    : number of messages written on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Language::WriteCatalog( const char *catalog ) {
  File file( catalog );
  int rc = -1;

  if ( file.Open( "wb" ) ) {
    file.Write32( FID_CATALOG );
    file.Write8( CF_CATALOG_VERSION );
    file.Write8( name.length() );
    file.WriteS( name );

    rc = WriteCatalog( file );
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Language::WriteCatalog
// DESCRIPTION: Save the messages for this language to catalog file.
// PARAMETERS : file - language catalog file descriptor
// RETURNS    : number of messages written on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Language::WriteCatalog( MemBuffer &file ) {
  file.WriteS( id );

  // calculate catalog size
  vector<string>::iterator iter;
  unsigned long msg_len = 0;
  for ( iter = cat.begin(); iter != cat.end(); ++iter )
    msg_len += (*iter).size() + 1;

  file.Write32( msg_len );

  for ( iter = cat.begin(); iter != cat.end(); ++iter ) {
    file.WriteS( StringUtil::crypt(*iter) );
    file.Write8( '\0' );
  }

  return cat.size();
}

////////////////////////////////////////////////////////////////////////
// NAME       : Language::GetMsg
// DESCRIPTION: Retrieve a specific message from the catalog.
// PARAMETERS : msg - message identifier
// RETURNS    : pointer to translated message if available, NULL
//              otherwise
////////////////////////////////////////////////////////////////////////

const char *Language::GetMsg( short id ) const {
  if ( (id >= 0) && ((short)cat.size() > id) ) return cat[id].c_str();
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Language::Find
// DESCRIPTION: Check whether a given message is already stored.
// PARAMETERS : msg - message to look for
// RETURNS    : message identifier or -1 if message wasn't found
////////////////////////////////////////////////////////////////////////

short Language::Find( const string &msg ) const {
  short id = -1;
  vector<string>::const_iterator it = find( cat.begin(), cat.end(), msg );
  if ( it != cat.end() ) id = distance( cat.begin(), it );
  return id;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Locale::Load
// DESCRIPTION: Load a number of language catalogs from a file.
// PARAMETERS : file - file descriptor
// RETURNS    : number of languages read (>= 0) on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Locale::Load( MemBuffer &file ) {

  unsigned short langs = file.Read16();
  for ( int i = 0; i < langs; ++i ) {
    Language lang;
    int rc = lang.ReadCatalog( file );

    if ( rc >= 0 ) AddLanguage( lang );
    else return -1;
  }

  if ( lib.size() == 0 ) return -1;

  if ( !SetDefaultLanguage( CF_LANG_DEFAULT ) ) {
    // no english translation found, use something we have
    SetDefaultLanguage( lib.begin()->first );
  }

  return lib.size();
}

////////////////////////////////////////////////////////////////////////
// NAME       : Locale::Save
// DESCRIPTION: Save a number of language catalogs to a file.
// PARAMETERS : file - file descriptor
// RETURNS    : number of languages written on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Locale::Save( MemBuffer &file ) {
  file.Write16( lib.size() );

  for ( map<const string, Language>::iterator iter = lib.begin();
        iter != lib.end(); ++iter ) {
    if ( iter->second.WriteCatalog( file ) == -1 ) return -1;
  }

  return lib.size();
}

////////////////////////////////////////////////////////////////////////
// NAME       : Locale::GetLanguage
// DESCRIPTION: Get a specific language.
// PARAMETERS : id - language identifier
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

const Language *Locale::GetLanguage( const string &id ) const {
  map<const string, Language>::const_iterator it;

  it = lib.find( id );

  return (it != lib.end()) ? &(it->second) : 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Locale::SetDefaultLanguage
// DESCRIPTION: Set the preferred language.
// PARAMETERS : id - language identifier
// RETURNS    : true if new language set, false on error
////////////////////////////////////////////////////////////////////////

bool Locale::SetDefaultLanguage( const string &id ) {
  const Language *l = GetLanguage( id );

  if ( l ) lang = l;
  return l != 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Locale::GetMsg
// DESCRIPTION: Retrieve a specific message from the preferred
//              translation. If the requested message is not available
//              in the current locale, we fall back to the default
//              (english, usually).
// PARAMETERS : msg - message identifier
// RETURNS    : pointer to translated message if available, NULL
//              otherwise
////////////////////////////////////////////////////////////////////////

const char *Locale::GetMsg( short msg ) const {
  const char *text = 0;

  if ( lang ) {
    text = lang->GetMsg( msg );

    if ( text == 0 ) {
      // try default
      const Language *def = GetLanguage( CF_LANG_DEFAULT );

      if ( def ) text = def->GetMsg( msg );
    }
  }
  return text;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Locale::AddLanguage
// DESCRIPTION: Add an additional language catalog to the Locale. If it
//              is the first language to be added, also make it the
//              default.
// PARAMETERS : lang - language to add
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Locale::AddLanguage( Language &lang ) {
  lib[lang.ID()] = lang;
  if ( this->lang == NULL ) this->lang = &lib[lang.ID()];
}

////////////////////////////////////////////////////////////////////////
// NAME       : Locale::RemoveLanguage
// DESCRIPTION: Remove a language catalog from the Locale. Also adjust
//              the default language if necessary.
// PARAMETERS : lang - language to remove
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Locale::RemoveLanguage( Language &lang ) {
  if ( this->lang && (this->lang->ID() == lang.ID()) ) this->lang = NULL;
  lib.erase( lang.ID() );
}

