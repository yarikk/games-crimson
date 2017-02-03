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
#ifndef _ED_INCLUDE_MKSURFACE_H
#define _ED_INCLUDE_MKSURFACE_H

#include "surface.h"

// surface class that can save image data into a data file
class MkSurface : public Surface {
public:
  int SaveImageData( const string &from, MemBuffer &to, bool tp );
  int GuessColorKey( void ) const;
};

#endif	/* _ED_INCLUDE_MKSURFACE_H */

