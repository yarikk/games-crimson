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
// strutil.h
///////////////////////////////////////////////////////////////

#ifndef _INCLUDE_STRUTIL_H
#define _INCLUDE_STRUTIL_H

#include <string>
using namespace std;

class StringUtil {
public:
  static string crypt( const string &str );

  static string strprintf( const string &format, int arg );
  static string strprintf( const string &format, char arg );
  static string strprintf( const string &format, const string &arg );

  static string tostring( int number );
  static short utf8chartoascii( const char *str );
};

#endif  /* _INCLUDE_STRUTIL_H */

