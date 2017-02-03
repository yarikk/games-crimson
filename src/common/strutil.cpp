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
// strutil.cpp - string utilities
////////////////////////////////////////////////////////////////////////

#include <sstream>
#include "strutil.h"

////////////////////////////////////////////////////////////////////////
// NAME       : crypt
// DESCRIPTION: Encrypt/decrypt ASCII strings. The "algorithm" is really
//              simple and designed to prevent users from "accidentally"
//              browsing through data files with a hex editor.
// PARAMETERS : str - string to encrypt or decrypt
// RETURNS    : encrypted or decrypted string, depending on input
////////////////////////////////////////////////////////////////////////

string StringUtil::crypt( const string &str ) {
  string crypted;

  for ( string::const_iterator iter = str.begin();
        (iter != str.end()) && (*iter != '\0'); ++iter )
    crypted += ~(*iter);

  return crypted;
}

////////////////////////////////////////////////////////////////////////
// NAME       : strprintf
// DESCRIPTION: Replace the first occurence of %d in a printf-style
//              format string with the given input.
// PARAMETERS : format - printf-style formatting template
//              arg    - value to insert
// RETURNS    : formatted string
////////////////////////////////////////////////////////////////////////

string StringUtil::strprintf( const string &format, int arg ) {
  string result( format );
  size_t pos = result.find( "%d", 0 );

  if ( pos != string::npos )
    result.replace( pos, 2, tostring(arg) );

  return result;
}

////////////////////////////////////////////////////////////////////////
// NAME       : strprintf
// DESCRIPTION: Replace the first occurence of %c in a printf-style
//              format string with the given input.
// PARAMETERS : format - printf-style formatting template
//              arg    - value to insert
// RETURNS    : formatted string
////////////////////////////////////////////////////////////////////////

string StringUtil::strprintf( const string &format, char arg ) {
  string result( format );
  size_t pos = result.find( "%c", 0 );

  if ( pos != string::npos ) {
    char buf[2] = { arg, '\0' };
    result.replace( pos, 2, buf );
  }

  return result;
}

////////////////////////////////////////////////////////////////////////
// NAME       : strprintf
// DESCRIPTION: Replace the first occurence of %s in a printf-style
//              format string with the given input.
// PARAMETERS : format - printf-style formatting template
//              arg    - value to insert
// RETURNS    : formatted string
////////////////////////////////////////////////////////////////////////

string StringUtil::strprintf( const string &format, const string &arg ) {
  string result( format );
  size_t pos = result.find( "%s", 0 );

  if ( pos != string::npos )
    result.replace( pos, 2, arg );

  return result;
}

////////////////////////////////////////////////////////////////////////
// NAME       : tostring
// DESCRIPTION: Convert an integer to a string.
// PARAMETERS : number - integer to convert
// RETURNS    : number as string
////////////////////////////////////////////////////////////////////////

string StringUtil::tostring( int number ) {
  stringstream numstr;

  numstr << number;
  return numstr.str();
}

////////////////////////////////////////////////////////////////////////
// NAME       : utf8chartoascii
// DESCRIPTION: Returns lowercase ASCII equivalent of the first
//              character of the UTF-8 argument string. For example, the
//              ASCII equivalent of 'Ð', 'Δ' or 'Д' is 'd'. Returns
//              literal 0 if it can't find the equivalent.
//              Currently only supports Cyrillic.
// PARAMETERS : str - pointer to a UTF-8 string
// RETURNS    : see description
////////////////////////////////////////////////////////////////////////

short StringUtil::utf8chartoascii( const char *str ) {
  // if the character's last bit is unset it's plain ASCII,
  // so just return lowercase version of it
  if ( ((unsigned char)str[0] & 0x80) == 0 )
    return (short)tolower( str[0] );

  // ASCII equivalents for Unicode range 0400-043F
  const static char *cir1 =
    "eedgeziijlntkiudabvgdezzijklmnoprstufhccss\0\0\0ejjabvgdezzijklmnop";
  // ASCII equivalents for Unicode range 0440-045F
  const static char *cir2 =
    "rstufhccss\0\0\0ejjeedgeziijlntkiud";

  // Unicode range 0400-043F translates to UTF-8 D080-D0BF. If the character's
  // first byte is D0, and the second byte is >= 80 and < C0, return ASCII
  // equivalent from cir1, with the second byte reduced by 80
  if ( (unsigned char)str[0] == 0xD0 ) {
    if( (str[1] & 0x80) && !(str[1] & 0x40) )
      return cir1[str[1] & 0x3F];
  } else if( (unsigned char)str[0] == 0xD1 ) {
    // Similarly, but for range 0440-045F which translates to UTF-8 D180-D19F
    if ( (str[1] & 0x80) && !(str[1] & 0x20) )
      return cir2[str[1] & 0x1F];
  }

  // otherwise, it is an unsupported character, so just return 0
  return 0;
}
