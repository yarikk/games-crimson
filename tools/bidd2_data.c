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
// bidd2_data.c:
// this file contains all the data necessary to convert BI DATA2 maps.
//
// If no appropriate tile is available in CF a similar one is used.
// Same goes for units, though some are completely left out for now.
// The "TITAN" unit still causes problems (2 maps won't compile in the editor)
// because there is no halfway similar unit in CF for TITAN, which is some kind
// of Super-Carrier-Transport-Glider (for now converted to Transport Planes).
//-----------------------------------------------------------------------------

#include "bi2cf.h"

const unsigned short bidd2_rawtiles[0x96] = {
    0, // 0x00 - OK headquarters (north entrance) green player 1
    1, // 0x01 - OK headquarters (north entrance) blue player 2
   25, // 0x02 - headquarters (center)
   24, // 0x03 - headquarters (south)
   22, // 0x04 - headquarters (northwest)
   22, // 0x05 - headquarters (southwest)
   23, // 0x06 - headquarters (northeast)
   23, // 0x07 - headquarters (southeast)
   17, // 0x08 - OK factory (east entrance) neutral
   15, // 0x09 - OK factory (east entrance) green player 1
   16, // 0x0A - OK factory (east entrance) blue player 2
   22, // 0x0B - factory (west)
   21, // 0x0C - factory (north)
   24, // 0x0D - factory (south)
   11, // 0x0E - OK depot (north entrance) neutral
    9, // 0x0F - OK depot (north entrance) green player 1
   10, // 0x10 - OK depot (north entrance) blue player 2
   24, // 0x11 - OK depot (south)
   22, // 0x12 - OK depot (west)
   23, // 0x13 - OK depot (east)
   26, // 0x14 - OK unfinished building (north)
   29, // 0x15 - OK unfinished building (south)
   27, // 0x16 - OK unfinished building (west)
   28, // 0x17 - OK unfinished building (east)
  360, // 0x18 - OK lava
  355, // 0x19 - OK lava coast (south-southeast)
  351, // 0x1A - OK lava coast (north-northeast)
  350, // 0x1B - OK lava coast (north-northwest)
  354, // 0x1C - OK lava coast (south-southwest)
  357, // 0x1D - OK lava coast (south-southwest-northwest-north)
  358, // 0x1E - OK lava coast (south-southeast-northeast-north)
  359, // 0x1F - OK lava coast (southwest-south-southeast)
  356, // 0x20 - OK lava coast (northwest-north-northeast)
  308, // 0x21 - OK lava pit
  176, // 0x22 - OK road (north-south)
  178, // 0x23 - OK road (southwest-northeast)
  177, // 0x24 - OK road (northwest-southeast)
  180, // 0x25 - OK road (north-southeast)
  179, // 0x26 - OK road (southwest-north)
  182, // 0x27 - OK road (south-northeast)
  181, // 0x28 - OK road (northwest-south)
  184, // 0x29 - OK road (southwest-southeast)
  183, // 0x2A - OK road (northwest-northeast)
  187, // 0x2B - OK road junction (north-south-southwest)
  188, // 0x2C - OK road junction (north-south-southeast)
  190, // 0x2D - OK road junction (south-north-northeast)
  189, // 0x2E - OK road junction (south-north-northwest)
  197, // 0x2F - OK road junction (southwest-northeast-southeast)
  198, // 0x30 - OK road junction (northwest-southeast-southwest)
  196, // 0x31 - OK road junction (southwest-northeast-northwest)
  195, // 0x32 - OK road junction (northwest-southeast-northeast)
  186, // 0x33 - OK road junction (south-northwest-northeast)
  208, // 0x34 - OK bridge (north-south) over lava
  296, // 0x35 - OK road (north-south) crossing ditch (southwest-northeast)
  295, // 0x36 - OK road (north-south) crossing ditch (northwest-southeast)
   88, // 0x37 - OK slope (north-south) east up
   87, // 0x38 - OK slope (north-south) west up
   91, // 0x39 - OK slope (southwest-northeast) northwest up
   92, // 0x3A - OK slope (southwest-northeast) southeast up
   89, // 0x3B - OK slope (southeast-northwest) northeast up
   90, // 0x3C - OK slope (northwest-southeast) southwest up
  103, // 0x3D - OK slope (southwest-southeast) north up
  102, // 0x3E - OK slope (northwest-northeast) south up
   97, // 0x3F - OK slope (south-northeast) northwest up
  100, // 0x40 - OK slope (south-northwest) northeast up
   94, // 0x41 - OK slope (north-southeast) southwest up
   96, // 0x42 - OK slope (southwest-north) southeast up
   95, // 0x43 - OK slope (southwest-north) northwest up
   93, // 0x44 - OK slope (north-southeast) northeast up
   99, // 0x45 - OK slope (south-northwest) southwest up
   98, // 0x46 - OK slope (south-northeast) southeast up
   34, // 0x47 - OK plains (slightly rubbled)
   30, // 0x48 - plains (tiny craters)
   30, // 0x49 - plains (many dents)
   30, // 0x4A - plains (few dents)
   38, // 0x4B - plains (big crater)
   38, // 0x4C - plains (medium crater)
   38, // 0x4D - plains (small craters)
   65, // 0x4E - OK small hill
   38, // 0x4F - OK tiny hills
   31, // 0x50 - OK plains (few red sand)
   32, // 0x51 - OK plains (more red sand)
   33, // 0x52 - OK plains (red sand)
   36, // 0x53 - OK plains (red sand) with few rocks
   37, // 0x54 - OK plains (red sand) with few rocks
   34, // 0x55 - OK few rocks
   35, // 0x56 - OK medium rocks
  309, // 0x57 - OK ditch (north-south)
  311, // 0x58 - OK ditch (southwest-northeast)
  310, // 0x59 - OK ditch (northwest-southeast)
  317, // 0x5A - OK ditch (southwest-southeast)
  316, // 0x5B - OK ditch (northwest-northeast)
  320, // 0x5C - OK ditch junction (southwest-northeast-north)
  315, // 0x5D - OK ditch (south-northeast)
  314, // 0x5E - OK ditch (south-northwest)
  313, // 0x5F - OK ditch (north-southeast)
  312, // 0x60 - OK ditch (north-southwest)
  338, // 0x61 - OK ditch (northeast-southwest end)
  339, // 0x62 - OK ditch (southwest-northeast end)
   30, // 0x63 - tile with skull (?)
  101, // 0x64 - OK slope (northwest-northeast) north up
  104, // 0x65 - OK slope (southwest-southeast) south up
   75, // 0x66 - medium rocks
   67, // 0x67 - red sand hills
   36, // 0x68 - rubble (destroyed something)
   36, // 0x69 - rubbled (destroyed square something)
   30, // 0x6A - structure (?)
   31, // 0x6B - rubble (destroyed mines?)
   30, // 0x6C - structure (?)
   30, // 0x6D - structure (?)
   30, // 0x6E - structure (?)
   30, // 0x6F - structure (?)
  117, // 0x70 - big mountain (north)
  117, // 0x71 - big mountain (northwest)
  117, // 0x72 - big mountain (west)
  117, // 0x73 - big mountain (southwest)
  117, // 0x74 - big mountain (south)
  117, // 0x75 - big mountain (southeast)
  117, // 0x76 - big mountain (northeast)
  117, // 0x77 - big mountain (east)
  117, // 0x78 - space ship structure (north)
  117, // 0x79 - space ship structure (south)
  117, // 0x7A - space ship structure (west)
  117, // 0x7B - space ship structure (east)
  349, // 0x7C - OK lava coast (south)
  344, // 0x7D - OK lava coast (north)
  335, // 0x7E - OK lava stream (north-south end)
  216, // 0x7F - OK bridge (northwest-southeast) over lava coast (northwest-north)
  217, // 0x80 - OK bridge (northwest-southeast) over lava coast (south-southeast)
  297, // 0x81 - OK road (northwest-southeast) over ditch (north-south)
  299, // 0x82 - OK road (southwest-northeast) over ditch (north-south)
   61, // 0x83 - OK big landing platform (north)
   64, // 0x84 - OK big landing platform (south)
   62, // 0x85 - OK big landing platform (west)
   63, // 0x86 - OK big landing platform (east)
   60, // 0x87 - OK landing platform
  112, // 0x88 - OK slope (northeast-southwest end) southeast up
  114, // 0x89 - OK slope (southwest-northeast end) southeast up
  111, // 0x8A - OK slope (northeast-southwest end) northwest up
  113, // 0x8B - OK slope (southwest-northeast end) northwest up
  115, // 0x8C - OK slope (southeast-northwest end) northeast up
  109, // 0x8D - OK slope (northwest-southeast end) northeast up
  110, // 0x8E - OK slope (northwest-southeast end) southwest up
  116, // 0x8F - OK slope (southeast-northwest end) southwest up
   77, // 0x90 - few plants
   78, // 0x91 - medium plants
   79, // 0x92 - OK medium plants
   80, // 0x93 - OK many plants
   81, // 0x94 - OK many many plants
   82};// 0x96 - bush


// strings to use for unit conversions
const unsigned char bidd2_units[27][20] = {
  "Infantry",            // 0x00 = R-1B DEMON
  "Infantry",            // 0x01 = R-4B DEMON
  "Infantry",            // 0x02 = M-21 VIRUS
  "Medium Tanks",        // 0x03 = T-8B SCORPION
  "Heavy Tanks",         // 0x04 = T-9B BLADE
  "Heavy Tanks",         // 0x05 = T-100 ZEUS
  "",                    // 0x06 = T-100 ZEUS
  "Personnel Carriers",  // 0x07 = SC-X MAMMUT
  "",                    // 0x08 = SC-X MAMMUT
  "Personnel Carriers",  // 0x09 = P-T2 PYTHON
  "Hovercraft",          // 0x0A = L32B TROLL
  "Hovercraft",          // 0x0B = TLAVB WHALE
  "Anti-Aircraft Tanks", // 0x0C = AD-6B BLITZ
  "Anti-Aircraft Tanks", // 0x0D = AD7 MAGIC
  "Anti-Aircraft Guns",  // 0x0E = AD-10B SPHINX
  "Scouts",              // 0x0F = FAV 2B BUSTER
  "Personnel Carriers",  // 0x10 = MM-2B GNOM
  "Artillery",           // 0x11 = G-12B LIGHT
  "Artillery",           // 0x12 = HG-13B ANGEL
  "Artillery",           // 0x13 = SR-100 FLAME
  "Interceptors",        // 0x14 = BF-2 COBRA
  "Fighter Squadron",    // 0x15 = BR-3 PIRATE
  "Bomber Wing",         // 0x16 = BR-1 CONDOR
  "Transport Planes",    // 0x17 = BT-1 TITAN
  "Medium Tanks",        // 0x18 = SC-PB WIZARD
  "",                    // 0x19 = ALDINIUM
  "Mines"};              // 0x1A = BB ROCKY

// this is for identifying which units are transporters
const unsigned char bidd2_transport[27] = {
0, // 0x00 = R-1B DEMON    - 00
0, // 0x01 = R-4B DEMON    - 02
0, // 0x02 = M-21 VIRUS    - 04
0, // 0x03 = T-8B SCORPION - 06
0, // 0x04 = T-9B BLADE    - 08
0, // 0x05 = T-100 ZEUS    - 0a
0, // 0x06 = T-100 ZEUS    - 0c
1, // 0x07 = SC-X MAMMUT   - 0e
0, // 0x08 = SC-X MAMMUT   - 10
1, // 0x09 = P-T2 PYTHON   - 12
0, // 0x0A = L32B TROLL    - 14
1, // 0x0B = TLAVB WHALE   - 16
0, // 0x0C = AD-6B BLITZ   - 18
0, // 0x0D = AD7 MAGIC     - 1a
0, // 0x0E = AD-10B SPHINX - 1c
0, // 0x0F = FAV 2B BUSTER - 1e
1, // 0x10 = MM-2B GNOM    - 20
0, // 0x11 = G-12B LIGHT   - 22
0, // 0x12 = HG-13B ANGEL  - 24
0, // 0x13 = SR-100 FLAME  - 26
0, // 0x14 = BF-2 COBRA    - 28
0, // 0x15 = BR-3 PIRATE   - 2a
0, // 0x16 = BR-1 CONDOR   - 2c
1, // 0x17 = BT-1 TITAN    - 2e
0, // 0x18 = SC-PB WIZARD  - 30
0, // 0x19 = ALDINIUM      - 32
0};// 0x1A = BB ROCKY      - 34

// this is for finding out which buildings are which.
const unsigned char bidd2_buildings[8] = {
0x00,  // Headquarters Player 1
0x01,  // Headquarters Player 2
0x08,  // Factory Neutral
0x09,  // Factory Player 1
0x0A,  // Factory Player 2
0x0E,  // Depot Neutral
0x0F,  // Depot Player 1
0x10}; // Depot player 2

// all the above in a struct, along with map type and number of units
const tconvdata bidd2_convdata =
{ MAP_BIDD2, bidd2_units, bidd2_rawtiles, bidd2_buildings, bidd2_transport, 27 };

