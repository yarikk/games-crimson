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
// lang.h - simple l10n or i18n support
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_LANG_H
#define _INCLUDE_LANG_H

#include <map>
#include <string>
#include <vector>

#include "fileio.h"
#include "misc.h"
#include "textbox.h"

#define CF_CATALOG_VERSION	1
#define FID_CATALOG		MakeID('L','C','A','T')

#define CF_LANG_DEFAULT		"en"

class Language {
public:
  int ReadCatalog( const char *catalog );
  int ReadCatalog( MemBuffer &file );
  int WriteCatalog( const char *catalog );
  int WriteCatalog( MemBuffer &file );

  const char *ID( void ) const { return id.c_str(); }
  const char *Name( void ) const { return name.c_str(); }

  void SetID( const char *lang ) { id.assign( lang ); }
  void SetName( const char *lang ) { name.assign( lang ); }

  void AddMsg( const string &msg ) { cat.push_back( msg ); }
  void SetMsg( const string &msg, size_t pos ) { cat[pos] = msg; }
  const char *GetMsg( short id ) const;
  short Find( const string &msg ) const;
  unsigned short Size( void ) const { return cat.size(); }

private:
  string id;
  string name;
  vector<string> cat;
};


class Locale {
public:
  Locale( void ) : lang(0) {}

  int Load( MemBuffer &file );
  int Save( MemBuffer &file );

  void AddLanguage( Language &lang );
  void RemoveLanguage( Language &lang );
  bool SetDefaultLanguage( const string &id );
  const Language *GetLanguage( const string &id ) const;

  const char *GetMsg( short msg ) const;

  const map<const string, Language> &GetLibrary( void ) const { return lib; }

private:
  const Language *lang;
  map<const string, Language> lib;
};

#endif   /* _INCLUDE_LANG_H */

