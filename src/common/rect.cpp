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
// rect.cpp
////////////////////////////////////////////////////////////////////////

#include "rect.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Rect::Align
// DESCRIPTION: This function ensures that the given Rect is placed
//              inside the boundaries of its parent Rect. If the child
//              is larger than the parent, this will not work correctly,
//              of course.
// PARAMETERS : parent - the parent Rect
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Rect::Align( const Rect &parent ) {
  if ( x < parent.x ) x = parent.x;
  else if ( x + w >= parent.x + parent.w ) x = parent.x + parent.w - w;

  if ( y < parent.y ) y = parent.y;
  else if ( y + h >= parent.y + parent.h ) y = parent.y + parent.h - h;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Rect::Center
// DESCRIPTION: Center the Rect in another rectangle.
// PARAMETERS : anchor - the rectangle to center the Rect in
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Rect::Center( const Rect &anchor ) {
  x = anchor.LeftEdge() + (anchor.Width() - w) / 2;
  y = anchor.TopEdge() + (anchor.Height() - h) / 2;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Rect::Clip
// DESCRIPTION: Cut away the parts of the Rect which are outside the
//              boundaries of the cliping rectangle.
// PARAMETERS : clip - clipping rectangle
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Rect::Clip( const Rect &clip ) {
  int delta;

  // horizontal clip
  if ( x < clip.x) {
    delta = clip.x - x;

    x += delta;
    if ( delta >= w ) w = 0;
    else w -= delta;
  }

  if ( (x + w) > (clip.x + clip.w) ) {
    delta = (x + w) - (clip.x + clip.w);

    if ( delta >= w ) w = 0;
    else w -= delta;
  }


  // vertical clip
  if ( y < clip.y) {
    delta = clip.y - y;

    y += delta;
    if ( delta >= h) h = 0;
    else h -= delta;
  }

  if ( (y + h) > (clip.y + clip.h) ) {
    delta = (y + h) - (clip.y + clip.h);

    if ( delta >= h ) h = 0;
    else h -= delta;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Rect::ClipBlit
// DESCRIPTION: Do clipping to the Rect and at the same time adjust the
//              boundaries of a source Rect. This function is used for
//              clipping before blitting.
// PARAMETERS : src  - source rectangle
//              clip - clipping rectangle
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Rect::ClipBlit( Rect &src, const Rect &clip ) {
  int delta;

  if ( x < clip.x) {
    delta = clip.x - x;
    x += delta;
    src.x += delta;

    if ( delta >= w ) {
      w = 0;
      src.w = 0;
    } else {
      w -= delta;
      src.w -= delta;
    }
  }

  if ( (x + w) > (clip.x + clip.w) ) {
    delta = (x + w) - (clip.x + clip.w);

    if ( delta >= w ) {
      w = 0;
      src.w = 0;
    } else {
      w -= delta;
      src.w -= delta;
    }
  }

  if ( y < clip.y ) {
    delta = clip.y - y;
    y += delta;
    src.y += delta;

    if ( delta >= h ) {
      h = 0;
      src.h = 0;
    } else {
      h -= delta;
      src.h -= delta;
    }
  }

  if ( (y + h) > (clip.y + clip.h) ) {
    delta = (y + h) - (clip.y + clip.h);

    if ( delta >= h ) {
      h = 0;
      src.h = 0;
    } else {
      h -= delta;
      src.h -= delta;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Rect::Contains
// DESCRIPTION: Check whether some coordinates lie inside the boundaries
//              of the Rect.
// PARAMETERS : x - horizontal coordinate
//              y - vertical coordinate
// RETURNS    : true if coordinates are contained within the Rect,
//              false otherwise
////////////////////////////////////////////////////////////////////////

bool Rect::Contains( short x, short y ) const {
  return( (x >= this->x) && (x < (this->x + w))
       && (y >= this->y) && (y < (this->y + h)) );
}

