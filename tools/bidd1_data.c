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
// bidd1_data.c:
// this file contains all the data necessary to convert BI DATA1 maps.
//
// If no appropriate tile is available in CF a similar one is used.
// Same goes for units, though some are completely left out for now.
//-----------------------------------------------------------------------------

#include "bi2cf.h"

const unsigned short bidd1_rawtiles[0x68] = {
  362, // 0x00 - OK deep water
  361, // 0x01 - OK water
  360, // 0x02 - OK shallow water
   30, // 0x03 - OK plains
   38, // 0x04 - OK plains with tiny dunes
   34, // 0x05 - OK rocky plains
   35, // 0x06 - OK rocky plains
   65, // 0x07 - OK plains with small dunes
   66, // 0x08 - OK plains with small dunes
   68, // 0x09 - OK long dunes (southwest)
   69, // 0x0A - OK long dunes (northeast)
   71, // 0x0B - OK big dunes (north)
   73, // 0x0C - OK big dunes (south)
   70, // 0x0D - OK big dunes (west)
   72, // 0x0E - OK big dunes (east)
   77, // 0x0F - OK very few plants
   78, // 0x10 - OK few plants
   79, // 0x11 - OK many plants
   80, // 0x12 - OK many many plants (forest)
  363, // 0x13 - OK cliff
  364, // 0x14 - OK cliffs
  308, // 0x15 - OK lake (north exit)
  117, // 0x16 - OK mountain
  117, // 0x17 - mountains (northeast-southwest end)
  117, // 0x18 - mountains (southwest-northeast end)
  296, // 0x19 - OK road (north-south) over river (southwest-northeast)
  297, // 0x1A - OK road (northwest-southeast) over river (north-south)
  117, // 0x1B - mountains (southwest-northeast)
  208, // 0x1C - OK road bridge (north-south)
  209, // 0x1D - OK road bridge (northwest-southeast)
  210, // 0x1E - OK road bridge (southwest-northeast)
  368, // 0x1F - OK docks with crane (water east)
  367, // 0x20 - OK docks (water west)
  370, // 0x21 - OK jetty
  176, // 0x22 - OK road (north-south)
  177, // 0x23 - OK road (northwest-southeast)
  178, // 0x24 - OK road (southwest-northeast)
  180, // 0x25 - OK road (north-southeast)
  179, // 0x26 - OK road (southwest-north)
  181, // 0x27 - OK road (northwest-south)
  182, // 0x28 - OK road (south-northeast)
  184, // 0x29 - OK road (southwest-southeast)
  183, // 0x2A - OK road (northwest-northeast)
  198, // 0x2B - OK road junction (northwest-southeast-southwest)
  196, // 0x2C - OK road junction (northeast-southwest-northwest)
  195, // 0x2D - OK road junction (northwest-southeast-northeast)
  189, // 0x2E - OK road junction (south-north-northwest)
  190, // 0x2F - OK road junction (south-north-northeast)
  187, // 0x30 - OK road junction (north-south-southwest)
  201, // 0x31 - OK road crossing (southwest-northeast-northwest-southeast)
  309, // 0x32 - OK river (north-south)
  311, // 0x33 - OK river (southwest-northeast)
  310, // 0x34 - OK river (northwest-southeast)
  312, // 0x35 - OK river (southwest-north)
  313, // 0x36 - OK river (north-southeast)
  314, // 0x37 - OK river (south-northwest)
  315, // 0x38 - OK river (northeast-south)
  316, // 0x39 - OK river (northwest-northeast)
  317, // 0x3A - OK river (southwest-southeast)
  319, // 0x3B - OK river junction (south-northwest-northeast)
  318, // 0x3C - OK river junction (north-southwest-southeast)
  364, // 0x3D - reef (northwest-southeast)
  365, // 0x3E - reef (southwest-northeast)
    0, // 0x3F - OK headquarters (north entrance) red player 1
    1, // 0x40 - OK headquarters (north entrance) yellow player 2
   25, // 0x41 - headquarters (center)
   24, // 0x42 - headquarters (south)
   22, // 0x43 - headquarters (northwest)
   22, // 0x44 - headquarters (southwest)
   23, // 0x45 - headquarters (northeast)
   23, // 0x46 - headquarters (southeast)
   11, // 0x47 - OK depot (north entrance) neutral
    9, // 0x48 - OK depot (north entrance) red player 1
   10, // 0x49 - OK depot (north entrance) yellow player 2
   24, // 0x4A - OK depot (south)
   22, // 0x4B - OK depot (west)
   23, // 0x4C - OK depot (east)
   26, // 0x4D - OK unfinished depot (north)
   29, // 0x4E - OK unfinished depot (south)
   27, // 0x4F - OK unfinished depot (west)
   28, // 0x50 - OK unfinished depot (east)
   17, // 0x51 - OK factory (east entrance) neutral
   15, // 0x52 - OK factory (east entrance) red player 1
   16, // 0x53 - OK factory (east antrance) red player 2
   21, // 0x54 - factory (north)
   24, // 0x55 - factory (south)
   22, // 0x56 - factory (west)
  340, // 0x57 - OK river (northwest-southeast end)
  337, // 0x58 - OK river (southeast-northwest end)
   30, // 0x59 - medium floes
   30, // 0x5A - big floes
   30, // 0x5B - many small floes
   30, // 0x5C - ice
   38, // 0x5D - ice with small snow drifts
   38, // 0x5E - ice medium snow drifts
   65, // 0x5F - ice with snow hill
   30, // 0x60 - snow with dirt
  999, // 0x61 - "forbidden" tile
  366, // 0x62 - OK docks (water east)
  341, // 0x63 - OK river (north-south) with tiny bridge (east-west)
  342, // 0x64 - OK river (northwest-southeast) with tiny bridge (southwest-northeast)
  361, // 0x65 - shore (land west)
  361, // 0x66 - shore (land east)
  186};// 0x67 - OK road junction (south-northeast-northwest)

// strings to use for unit conversions
const unsigned char bidd1_units[27][20] = {
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
  "Aircraft Carriers",   // 0x13 = PEGASUS
  "",                    // 0x14 = PEGASUS
  "Patrol Boats",        // 0x15 = FORTRESS
  "",                    // 0x16 = FORTRESS
  "Patrol Boats",        // 0x17 = BUCCANEER
  "Medium Tanks",        // 0x18 = SCORPION
  "",                    // 0x19 = ALDINIUM
  "Mines"};              // 0x1A = BRICK

// this is for identifying which units are transporters
const unsigned char bidd1_transport[27] = {
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
1, // 0x26+0x27 = PEGASUS
0, // 0x28+0x29 = PEGASUS
0, // 0x2A+0x2B = FORTRESS
0, // 0x2C+0x2D = FORTRESS
1, // 0x2E+0x2F = BUCCANEER
0, // 0x30+0x31 = SCORPION
0, // 0x32+0x33 = ALDINIUM
0};// 0x34+0x35 = BRICK


// this is for finding out which buildings are which.
const unsigned char bidd1_buildings[8] = {
0x3F,  // Headquarters Player 1
0x40,  // Headquarters Player 2
0x51,  // Factory Neutral
0x52,  // Factory Player 1
0x53,  // Factory Player 2
0x47,  // Depot Neutral
0x48,  // Depot Player 1
0x49}; // Depot player 2

// all the above in a struct, along with map type and number of units
const tconvdata bidd1_convdata =
{ MAP_BIDD1, bidd1_units, bidd1_rawtiles, bidd1_buildings, bidd1_transport, 27 };

