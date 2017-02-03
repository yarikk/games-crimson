/*
bi2cf - A map converter for Crimson Fields
copyright (c) 2002 by Florian Dietrich

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

//-----------------------------------------------------------------------------
// bi_data.c:
// this file contains all the data necessary to convert BI maps.
//
// if no appropriate tile is available in CF a similar one is used.
// Same goes for units, though some are completely left out for now.
//-----------------------------------------------------------------------------

#include "bi2cf.h"

const unsigned short bi_rawtiles[0x65] = {
  360, // 0x00 - OK shallow water
  361, // 0x01 - OK water
  362, // 0x02 - OK deep water
   30, // 0x03 - OK plains
   79, // 0x04 - OK light forest
   80, // 0x05 - OK medium forest
   81, // 0x06 - OK deep forest 1
   82, // 0x07 - OK very deep forest 1
   82, // 0x08 - OK very deep forest 2
   81, // 0x09 - OK deep forest 2
   84, // 0x0A - OK big forest (west)
   86, // 0x0B - OK big forest (south)
   83, // 0x0C - OK big forest (north)
   85, // 0x0D - OK big forest (east)
  119, // 0x0E - OK big mountain (south)
  118, // 0x0F - OK big mountain (north)
  120, // 0x10 - OK big mountain (west)
  121, // 0x11 - OK big mountain (east)
  117, // 0x12 - OK mountain
  123, // 0x13 - OK big mountain (northwest)
  124, // 0x14 - OK big mountain (northeast)
  122, // 0x15 - OK big mountain (top)
  125, // 0x16 - OK big mountain (southwest)
  126, // 0x17 - OK big mountain (southeast)
  117, // 0x18 - mountain?
  117, // 0x19 - mountain?
  117, // 0x1A - mountain?
  117, // 0x1B - mountain?
  208, // 0x1C - OK road bridge (north-south)
  209, // 0x1D - OK road bridge (southeast-northwest)
  210, // 0x1E - OK road bridge (southwest-northeast)
   22, // 0x1F - OK depot (west)
    9, // 0x20 - OK depot (north entrance) red player 1
   23, // 0x21 - OK depot (east)
   24, // 0x22 - OK depot (south)
   26, // 0x23 - OK unfinished building (north)
   27, // 0x24 - OK unfinished building (west)
   28, // 0x25 - OK unfinished building (east)
   29, // 0x26 - OK unfinished building (south)
  362, // 0x27 - OK deep water (dark northeast)
  372, // 0x28 - OK jetty
  362, // 0x29 - OK deep water (dark northwest)
   22, // 0x2A - factory (west)
   24, // 0x2B - factory (south)
   15, // 0x2C - OK factory (east entrance) red player 1
   21, // 0x2D - factory (north)
   17, // 0x2E - OK factory (east entrance) neutral
    0, // 0x2F - OK headquarters (north entrance) red player 1
   22, // 0x30 - headquarters (southwest)
   22, // 0x31 - headquarters (northwest)
   24, // 0x32 - headquarters (south)
   23, // 0x33 - headquarters (southeast)
   23, // 0x34 - headquarters (northeast)
   25, // 0x35 - headquarters (center)
  363, // 0x36 - OK cliff
  364, // 0x37 - OK cliffs
  176, // 0x38 - OK road (south-north)
  177, // 0x39 - OK road (southeast-northwest)
  178, // 0x3A - OK road (southwest-northeast)
  188, // 0x3B - OK road junction (north-south-southeast)
  187, // 0x3C - OK road junction (north-south-southwest)
  201, // 0x3D - OK road crossing (southwest-northwest-northeast-southeast)
  191, // 0x3E - OK road junction (south-southeast-northwest)
  192, // 0x3F - OK road junction (southwest-south-northeast)
  181, // 0x40 - OK road (northwest-south)
  182, // 0x41 - OK road (northeast-south)
  179, // 0x42 - OK road (north-southwest)
  180, // 0x43 - OK road (north-southeast)
  183, // 0x44 - OK road (northwest-northeast)
  184, // 0x45 - OK road (southwest-southeast)
  194, // 0x46 - OK road junction (southeast-north-northwest)
  193, // 0x47 - OK road junction (southwest-north-northeast)
  189, // 0x48 - OK road junction (north-south-northwest)
  190, // 0x49 - OK road junction (north-south-northeast)
  134, // 0x4A - OK fence (northwest-southeast)
  135, // 0x4B - OK fence (southwest-northeast)
  133, // 0x4C - OK fence (north-south)
  138, // 0x4D - OK fence (northwest-south)
  136, // 0x4E - OK fence (southwest-north)
  137, // 0x4F - OK fence (southeast-north)
  139, // 0x50 - OK fence (northeast-south)
  140, // 0x51 - OK fence (northwest-northeast)
  141, // 0x52 - OK fence (southwest-southeast)
  147, // 0x53 - OK fence end (northwest-southeast)
  146, // 0x54 - OK fence end (northeast-southwest)
  143, // 0x55 - OK fence end (north-south)
  144, // 0x56 - OK fence end (southeast-northwest)
  145, // 0x57 - OK fence end (southwest-northeast)
  142, // 0x58 - OK fence end (south-north)
   74, // 0x59 - rubble
   75, // 0x5A - rubble
   76, // 0x5B - rubble
   76, // 0x5C - rubble
   10, // 0x5D - OK depot (north entrance) yellow player 2
   16, // 0x5E - OK factory (east entrance) yellow player 2
    1, // 0x5F - OK headquarters (north entrance) yellow player 2
   11, // 0x60 - OK depot (north entrance) neutral
   76, // 0x61 - crater
  211, // 0x62 - destroyed bridge (south-north)
  212, // 0x63 - destroyed bridge (southeast-northwest)
  213};// 0x64 - destroyed bridge (southwest-northeast)

// strings to use for unit conversions
const unsigned char bi_units[27][20] = {
  "Anti-Aircraft Guns",  // 0x00 = SPHINX
  "Infantry",            // 0x01 = DEMON
  "Medium Tanks",        // 0x02 = GLADIATOR
  "Heavy Tanks",         // 0x03 = CRUSADER
  "Anti-Aircraft Tanks", // 0x04 = BLITZ
  "Scouts",              // 0x05 = BUSTER
  "Personnel Carriers",  // 0x06 = PROVIDER
  "Artillery",           // 0x07 = ANGEL
  "Personnel Carriers",  // 0x08 = MERLIN
  "Interceptors",        // 0x09 = MOSQUITO
  "Bomber Wing",         // 0x0A = RAVEN
  "Gunships",            // 0x0B = FIREBIRD
  "Transport Planes",    // 0x0C = GIANT
  "Torpedo Boats",       // 0x0D = MARAUDER
  "Hovercraft",          // 0x0E = INVADER
  "Troopships",          // 0x0F = AMAZON
  "",                    // 0x10 = AMAZON
  "Submarines",          // 0x11 = BARRACUDA
  "",                    // 0x12 = BARRACUDA
  "",                    // 0x13 = PEGASUS
  "Aircraft Carriers",   // 0x14 = PEGASUS
  "Patrol Boats",        // 0x15 = FORTRESS
  "",                    // 0x16 = FORTRESS
  "Patrol Boats",        // 0x17 = BUCCANEER
  "Medium Tanks",        // 0x18 = SCORPION
  "",                    // 0x19 = ALDINIUM
  "Mines"};              // 0x1A = BRICK

// this is for identifying which units are transporters
const unsigned char bi_transport[27] = {
0, // 0x00+0x01 = SPHINX
0, // 0x02+0x03 = DEMON
0, // 0x04+0x05 = GLADIATOR
0, // 0x06+0x07 = CRUSADER
0, // 0x08+0x09 = BLITZ
0, // 0x0A+0x0B = BUSTER
1, // 0x0C+0x0D = PROVIDER
0, // 0x0E+0x0F = ANGEL
0, // 0x10+0x11 = MERLIN
0, // 0x12+0x13 = MOSQUITO
0, // 0x14+0x15 = RAVEN
0, // 0x16+0x17 = FIREBIRD
1, // 0x18+0x19 = GIANT
0, // 0x1A+0x1B = MARAUDER
1, // 0x1C+0x1D = INVADER
1, // 0x1E+0x1F = AMAZON
0, // 0x20+0x21 = AMAZON
0, // 0x22+0x23 = BARRACUDA
0, // 0x24+0x25 = BARRACUDA
0, // 0x26+0x27 = PEGASUS
1, // 0x28+0x29 = PEGASUS
0, // 0x2A+0x2B = FORTRESS
0, // 0x2C+0x2D = FORTRESS
1, // 0x2E+0x2F = BUCCANEER
0, // 0x30+0x31 = SCORPION
0, // 0x32+0x33 = ALDINIUM
0};// 0x34+0x35 = BRICK

// this is for finding out which buildings are which.
const unsigned char bi_buildings[8] = {
0x2F,  // Headquarters Player 1
0x5F,  // Headquarters Player 2
0x2E,  // Factory Neutral
0x2C,  // Factory Player 1
0x5E,  // Factory Player 2
0x60,  // Depot Neutral
0x20,  // Depot Player 1
0x5D}; // Depot player 2

// all the above in a struct, along with map type and number of units
const tconvdata bi_convdata =
{ MAP_BI, bi_units, bi_rawtiles, bi_buildings, bi_transport, 27 };

