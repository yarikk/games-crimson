/* Crimson Fields - a game of tactical warfare
   Copyright (C) 2000-2007 Jens Granseuer

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _ED_INCLUDE_PARSER_H
#define _ED_INCLUDE_PARSER_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <utility>
using namespace std;

// callback when a section has been successfully parsed
class SectionParsedCallback {
public:
  virtual ~SectionParsedCallback( void ) {}
  virtual void SectionParsed( const string &section,
                              class SectionHandler &handler,
                              class CFParser &parser ) = 0;
};


// class that handles parsing of a section type, e.g. [unit]
class SectionHandler {
public:
  SectionHandler( void ) : enabled(true) {}
  virtual ~SectionHandler( void ) {}

  virtual int ParseSection( ifstream &in, const string *opt, unsigned long &line ) = 0;

  void SetEnabled( bool flag ) { enabled = flag; }
  bool Enabled( void ) const { return enabled; }

protected:
  void RemWhitespace( string &s ) const;
  int StrToNum( const string &s ) const;

  bool enabled;
};

class CFParser {
public:
  CFParser( void ) : hook(0) {}
  ~CFParser( void );

  int Parse( const string &file );

  void AddHandler( const string &section, SectionHandler *handler );
  void EnableHandler( const string &section, bool enabled );
  void SetCallback( SectionParsedCallback *cb ) { hook = cb; }

  static void RemWhitespace( string &s );
  static int StrToNum( const string &s );

private:
  const pair<string, string> ParseSectionHeader( const string &s );

  map<const string, SectionHandler *> handlers;
  SectionParsedCallback *hook;
};

// section handler subclass for parsing key/value pairs
class KeyValueSectionHandler : public SectionHandler {
public:
  KeyValueSectionHandler( void ) {}
  virtual ~KeyValueSectionHandler( void ) {}

  virtual int ParseSection( ifstream &in, const string *opt, unsigned long &line );

protected:
  vector<pair<string, string> > pairs;
};

#endif	/* _ED_INCLUDE_PARSER_H */

