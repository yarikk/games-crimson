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
// misc.cpp - miscellaneous classless functions
////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>

#include "misc.h"

////////////////////////////////////////////////////////////////////////
// NAME       : random
// DESCRIPTION: Create a pseudo-random number between min and max.
// PARAMETERS : min - lower end of random numbers
//              max - high end of random numbers
// RETURNS    : min <= r <= max
////////////////////////////////////////////////////////////////////////

int random( int min, int max ) {
  return( rand_range( max - min + 1 ) + min );
}

////////////////////////////////////////////////////////////////////////
// NAME       : rand_range
// DESCRIPTION: Create a pseudo-random number between 0 and range.
// PARAMETERS : range - range of pseudo-random numbers;
//                      1 < range <= RAND_MAX+1
// RETURNS    : 0 <= r < range
////////////////////////////////////////////////////////////////////////

unsigned int rand_range( unsigned int range ) {
  unsigned int rmax, r, d;
  d = (RAND_MAX+1U-range) / range + 1; 
  rmax = d * range - 1;                // -1 to avoid "overflow to zero"
  do
    r = rand();
  while (r > rmax);
  return r/d;
}

////////////////////////////////////////////////////////////////////////
// NAME       : itoa
// DESCRIPTION: Convert a decimal number to an ASCII string.
// PARAMETERS : n   - number to convert
//              buf - buffer to hold the resulting string; make sure it
//                    is large enough for the string before calling!
// RETURNS    : pointer to buffer
////////////////////////////////////////////////////////////////////////

char *itoa( int n, char *buf ) {
  bool neg = false;
  short i = 0, j;
  char c;

  if (n < 0) {
    neg = true;
    n = -n;
  }

  // output number to buffer in reverse order
  do {
    buf[i++] = n % 10 + '0';
  } while ( (n /= 10) > 0 );

  if (neg) buf[i++] = '-';
  buf[i--] = '\0';

  // turn the string around
  for (j = 0; j < i; --i, ++j) {
    c = buf[i];
    buf[i] = buf[j];
    buf[j] = c;
  }
  return buf;
}

