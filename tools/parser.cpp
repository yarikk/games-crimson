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

/* parser.cpp */

#include <stdlib.h>
#include "parser.h"

////////////////////////////////////////////////////////////////////////
// NAME       : SectionHandler::StrToNum
// DESCRIPTION: Convert a string to an integer.
// PARAMETERS : s - string to convert
// RETURNS    : number
////////////////////////////////////////////////////////////////////////

int SectionHandler::StrToNum( const string &s ) const {
  return CFParser::StrToNum( s );
}

////////////////////////////////////////////////////////////////////////
// NAME       : SectionHandler::RemWhitespace
// DESCRIPTION: Remove leading and trailing whitespace characters from
//              a string.
// PARAMETERS : s - string to crop
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void SectionHandler::RemWhitespace( string &s ) const {
  CFParser::RemWhitespace( s );
}


////////////////////////////////////////////////////////////////////////
// NAME       : CFParser::~CFParser
// DESCRIPTION: Destroy the parser and all registered handlers.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

CFParser::~CFParser( void ) {
  for ( map<const string, SectionHandler *>::iterator it = handlers.begin();
        it != handlers.end(); ++it ) {
    delete it->second;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : CFParser::AddHandler
// DESCRIPTION: Register a SectionHandler for a named section type (aka
//              entity). All handlers attached to the parser will be
//              destroyed when the parser is destroyed.
// PARAMETERS : section - section name the handler is responsible for
//              handler - the handler itself
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void CFParser::AddHandler( const string &section, SectionHandler *handler ) {
  handlers[section] = handler;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CFParser::EnableHandler
// DESCRIPTION: Enable or disable a handler for a named section.
// PARAMETERS : section - section name the handler is responsible for
//              enabled - whether to enable or disable the handler
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void CFParser::EnableHandler( const string &section, bool enabled ) {
  map<const string, SectionHandler *>::iterator it = handlers.find( section );
  if ( it != handlers.end() ) {
    it->second->SetEnabled( enabled );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : CFParser::RemWhitespace
// DESCRIPTION: Remove leading and trailing whitespace characters from
//              a string.
// PARAMETERS : s - string to crop
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void CFParser::RemWhitespace( string &s ) {
  int len = s.size(), start = 0, end;

  if (len == 0)
    return;

  while ( (start < len) && (s[start] == ' ') ) ++start;
  for ( end = len - 1;
        ((s[end] == ' ') || (s[end] == '\n') ||
         (s[end] == '\r')) && (end >= start);
        --end );  /* empty loop */

  s.erase( end + 1 );
  s.erase( 0, start );
}

////////////////////////////////////////////////////////////////////////
// NAME       : CFParser::StrToNum
// DESCRIPTION: Convert a string to an integer.
// PARAMETERS : s - string to convert
// RETURNS    : number
////////////////////////////////////////////////////////////////////////

int CFParser::StrToNum( const string &s ) {
  if ( s.size() > 0 && s[0] != '-' && !isdigit(s[0]) ) // complain
    cerr << "Warning: trying to convert non-numeric chars to number: " << s << endl;
  return atoi(s.c_str());
}

////////////////////////////////////////////////////////////////////////
// NAME       : CFParser::Parse
// DESCRIPTION: Parse a file using the registered handlers.
// PARAMETERS : file - name of the file to parse
// RETURNS    : 0 on success, non-0 on error
////////////////////////////////////////////////////////////////////////

int CFParser::Parse( const string &file ) {
  int rc = 0;
  unsigned long line = 0;

  ifstream f( file.c_str() );
  if ( !f.is_open() ) {
    cerr << "Unable to open file " << file << endl;
    return 1;
  }

  // skip possible UTF-8 BOM
  unsigned char s[4];
  f.get( (char *)s, 4 );
  if ((s[0] != 0xef) || (s[1] != 0xbb) || (s[2] != 0xbf))
    f.seekg( 0 );

  string buf;
  while ( !f.eof() && (rc == 0) ) {
    getline( f, buf );
    ++line;
    RemWhitespace( buf );

    if ((buf.size() > 0) && (buf[0] != '#')) {   // comment
      rc = 1;

      // get the section name
      pair<string, string> sec = ParseSectionHeader( buf );
      if ( sec.first.size() == 0 ) {
        cerr << "Syntax error in line " << line << ": Invalid section header '" << buf << "'" << endl;
        break;
      }

      map<const string, SectionHandler *>::iterator it = handlers.find( sec.first );
      if ( it == handlers.end() ) {
        cerr << "Error in line " << line << ": No handler available for section '" << sec.first << "'" << endl;
        break;
      } else if ( !it->second->Enabled() ) {
        cerr << "Error in line " << line << ": Handler for section '" << sec.first << "' is disabled" << endl;
        break;
      }

      rc = it->second->ParseSection( f,
                                    (sec.second.size() > 0) ? &sec.second : NULL,
                                    line );
      if ( (rc == 0) && hook ) hook->SectionParsed( sec.first, *it->second, *this );
    }
  }

  f.close();
  return rc;
}

////////////////////////////////////////////////////////////////////////
// NAME       : CFParser::ParseSectionHeader
// DESCRIPTION: Parse a section header to retrieve the name and any
//              options. A section header can llok like this:
//              "[unit]"         - standard header
//              "[messages(en)]" - header with optional argument
// PARAMETERS : s - section header string
// RETURNS    : pair containing the section name and the optional
//              parameter if any
////////////////////////////////////////////////////////////////////////

const pair<string, string> CFParser::ParseSectionHeader( const string &s ) {
  size_t pos;
  pair<string, string> sec("", "");

  if ( s[0] == '[' ) {       // starts a new entity
    pos = s.find( ']' );
    if ( pos != string::npos ) {
      string head = s.substr( 1, pos - 1 );

      // look for optional parameter
      pos = head.find( '(' );
      if ( pos != string::npos ) {
        sec.second = head.substr( pos + 1, head.size() - pos - 2 );
        head.erase( pos );
      }
      sec.first = head;
    }
  }
  return sec;
}


////////////////////////////////////////////////////////////////////////
// NAME       : KeyValueSectionHandler::ParseSection
// DESCRIPTION: Parse a section or entity containing key/value pairs
//              ("key = value").
// PARAMETERS : in   - stream to read data from
//              opt  - if not NULL, optional parameter for this section
//              line - line counter to be incremented while reading
// RETURNS    : 0 on success, non-0 on error
////////////////////////////////////////////////////////////////////////

int KeyValueSectionHandler::ParseSection( ifstream &in,
    const string *opt, unsigned long &line ) {
  int rc = 0;
  size_t pos;
  string buf, val;

  pairs.clear();

  while ( !in.eof() ) {
    getline( in, buf );
    ++line;
    RemWhitespace( buf );

    if ( buf.size() == 0 ) break; // empty lines are treated as section separators
    else if ( buf[0] != '#' ) {   // comment
      pos = buf.find( '=' );
      if ( pos == string::npos ) {
        cerr << "Syntax error in line " << line << ": '=' missing" << endl;
        rc = 1;
        break;
      }
      val = buf.substr( pos+1 );  // separate key from data
      buf.erase( pos );
      RemWhitespace( buf );
      RemWhitespace( val );

      pairs.push_back( pair<string, string>( buf, val ) );
    }
  }

  return rc;
}


