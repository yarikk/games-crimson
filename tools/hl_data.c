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
// hl_data.c:
// this file contains all the data necessary to convert HL maps.
//
// If no appropriate tile is available in CF a similar one is used.
// Same goes for units, though some are completely left out for now.
//-----------------------------------------------------------------------------

#include "bi2cf.h"

const unsigned short hl_rawtiles[0xAF] = {
   30, // 0x00 - OK plains
    0, // 0x01 - OK headquarters (north entrance) blue
    1, // 0x02 - OK headquarters (north entrance) yellow
   25, // 0x03 - headquarters (center)
   24, // 0x04 - headquarters (south)
   22, // 0x05 - headquarters (northwest)
   22, // 0x06 - headquarters (southwest)
   23, // 0x07 - headquarters (northeast)
   23, // 0x08 - headquarters (southeast)
   21, // 0x09 - factory (north)
   24, // 0x0A - factory (south)
   22, // 0x0B - factory (west)
   17, // 0x0C - OK factory (east entrance) neutral
   15, // 0x0D - OK factory (east entrance) blue
   16, // 0x0E - OK factory (east entrance) yellow
   11, // 0x0F - OK depot (north entrance) neutral
    9, // 0x10 - OK depot (north entrance) blue
   10, // 0x11 - OK depot (north entrance) yellow
   24, // 0x12 - OK depot (south)
   22, // 0x13 - OK depot (west)
   23, // 0x14 - OK depot (east)
   26, // 0x15 - OK unfinished depot (north)
   29, // 0x16 - OK unfinished depot (south)
   27, // 0x17 - OK unfinished depot (west)
   28, // 0x18 - OK unfinished depot (east)
  360, // 0x19 - OK shallow water
  355, // 0x1A - OK shallow water (coast southeast+south)
  350, // 0x1B - OK shallow water (coast northwest+north)
  354, // 0x1C - OK shallow water (coast southwest+south)
  351, // 0x1D - OK shallow water (coast north+northeast)
  359, // 0x1E - OK shallow water (coast southwest+south+southeast)
  356, // 0x1F - OK shallow water (coast northwest+north+northeast)
  357, // 0x20 - OK shallow water (coast north+northwest+southwest+south)
  358, // 0x21 - OK shallow water (coast north+northeast+southeast+south)
  308, // 0x22 - OK lake
  373, // 0x23 - OK harbor (south faces land)
  310, // 0x24 - OK river (northwest-southeast)
  311, // 0x25 - OK river (southwest-northeast)
  309, // 0x26 - OK river (north-south)
  312, // 0x27 - OK river (north-southwest)
  314, // 0x28 - OK river (northwest-south)
  315, // 0x29 - OK river (south-northeast)
  313, // 0x2A - OK river (north-southeast)
  317, // 0x2B - OK river (southwest-southeast)
  316, // 0x2C - OK river (northwest-northeast)
  319, // 0x2D - OK river (northwest-northeast-south)
  318, // 0x2E - OK river (north-southwest-southeast)
  361, // 0x2F - OK water
  362, // 0x30 - OK deep water
   53, // 0x31 - OK house (with tree)
   54, // 0x32 - OK house (with well)
   57, // 0x33 - OK hangars
   51, // 0x34 - harbor building (west)
   50, // 0x35 - harbor building (north)
   77, // 0x36 - light forest (3 trees)
   78, // 0x37 - OK deep forest (5 trees)
   80, // 0x38 - OK deep forest (9 trees)
   79, // 0x39 - OK deep forest (7 trees)
  343, // 0x3A - OK river (southwest-northeast) with tiny bridge (northwest-southeast)
  341, // 0x3B - OK river (north-south) with tiny bridge (east-west)
  365, // 0x3C - OK small cliffs (shallow water)
  364, // 0x3D - OK cliffs (shallow water)
  363, // 0x3E - OK cliff (shallow water)
  132, // 0x3F - OK rugged terrain with tank stoppers
  117, // 0x40 - OK small mountain
   68, // 0x41 - OK hill (northwest-northeast)
   69, // 0x42 - OK hill (southwest-northeast)
   71, // 0x43 - OK big hill (north)
   73, // 0x44 - OK big hill (south)
   70, // 0x45 - OK big hill (west)
   72, // 0x46 - OK big hill (east)
  118, // 0x47 - OK big mountain (north)
  119, // 0x48 - OK big mountain (south)
  120, // 0x49 - OK big mountain (west)
  121, // 0x4A - OK big mountain (east)
   76, // 0x4B - OK rocks
   75, // 0x4C - OK rocks
   74, // 0x4D - OK rock
   34, // 0x4E - OK plains (tiny rocks)
   35, // 0x4F - OK plains (small rocks)
   38, // 0x50 - OK tiny hills
   31, // 0x51 - OK slightly rugged terrain
   33, // 0x52 - OK rugged terrain
   33, // 0x53 - rugged terrain with craters
   33, // 0x54 - rugged terrain with crater
  248, // 0x55 - OK rail (north-south)
  249, // 0x56 - OK rail (northwest-southeast)
  250, // 0x57 - OK rail (southwest-northeast)
  251, // 0x58 - OK rail (southwest-north)
  253, // 0x59 - OK rail (northwest-south)
  254, // 0x5A - OK rail (south-northeast)
  252, // 0x5B - OK rail (north-southeast)
  255, // 0x5C - OK rail (northwest-northeast)
  256, // 0x5D - OK rail (southwest-southeast)
  259, // 0x5E - OK rail junction (north-south-southwest)
  260, // 0x5F - OK rail junction (south-north-southeast)
  261, // 0x60 - OK rail junction (north-south-northwest)
  289, // 0x61 - OK rail (north-south) crossing river (northwest-southeast)
  290, // 0x62 - OK rail (north-south) crossing river (southwest-northeast)
  287, // 0x63 - OK rail (southwest-northeast) crossing path (north-south)
  284, // 0x64 - OK rail (north-south) crossing path (southwest-northeast)
  277, // 0x65 - OK rail (north-south) crossing road (northwest-southeast)
  279, // 0x66 - OK rail (northwest-southeast) crossing road (north-south)
  281, // 0x67 - OK rail (southwest-northeast) crossing road (north-south)
  278, // 0x68 - OK rail (north-south) crossing road (southwest-northeast)
   58, // 0x69 - OK airfield (north-south) north
   59, // 0x6A - OK airfield (north-south) south
  218, // 0x6B - OK coast (south-southwest-northwest-north) with road/bridge (northwest-southeast)
  209, // 0x6C - OK road bridge (northwest-southeast)
  219, // 0x6D - OK coast (north-northeast-southeast-south) with road/bridge (northwest-southeast)
  210, // 0x6E - OK road bridge (southwest-northeast)
  221, // 0x6F - OK coast (north-northeast-southeast-south) with road/bridge (southwest-northeast)
  220, // 0x70 - OK coast (south-southwest-northwest-north) with road/bridge (southwest-northeast)
  214, // 0x71 - OK coast (northwest-north-northeast) with road/bridge (north-south)
  208, // 0x72 - OK road bridge (north south)
  215, // 0x73 - OK coast (southwest-south-southeast) with road/bridge (north-south)
  295, // 0x74 - OK river (northwest-southeast) with road bridge (north-south)
  296, // 0x75 - OK river (southwest-northeast) with road bridge (north-south)
  297, // 0x76 - OK river (north-south) with road bridge (northwest-southeast)
  367, // 0x77 - OK docks (west faces water)
   52, // 0x78 - OK harbor building (south with road)
  302, // 0x79 - OK river (southwest-northeast) with path bridge (north-south)
  301, // 0x7A - OK river (northwest-southeast) with path bridge (north-south)
  368, // 0x7B - OK docks with crane (east faces water)
  369, // 0x7C - OK docks with crane (west faces water)
  366, // 0x7D - OK docks (east faces water)
  370, // 0x7E - OK jetty
  176, // 0x7F - OK road (north-south)
  178, // 0x80 - OK road (southwest-northeast)
  177, // 0x81 - OK road (northwest-southeast)
  182, // 0x82 - OK road (south-northeast)
  180, // 0x83 - OK road (north-southeast)
  181, // 0x84 - OK road (northwest-south)
  179, // 0x85 - OK road (southwest-north)
  183, // 0x86 - OK road (northwest-northeast)
  184, // 0x87 - OK road (southwest-southeast)
  196, // 0x88 - OK road junction (southwest-northeast-northwest)
  195, // 0x89 - OK road junction (northwest-southeast-northeast)
  197, // 0x8A - OK road junction (southwest-northeast-southeast)
  198, // 0x8B - OK road junction (northwest-southeast-southwest)
  187, // 0x8C - OK road junction (north-south-southwest)
  189, // 0x8D - OK road junction (north-south-northwest)
  188, // 0x8E - OK road junction (north-south-southeast)
  190, // 0x8F - OK road junction (north-south-northeast)
  201, // 0x90 - OK road crossing (northwest-southeast-southwest-northeast)
  186, // 0x91 - OK road crossing (northwest-northeast-south)
  222, // 0x92 - OK path (north-south)
  224, // 0x93 - OK path (southwest-northeast)
  223, // 0x94 - OK path (northwest-southeast)
  227, // 0x95 - OK path (south-northeast)
  226, // 0x96 - OK path (north-southeast)
  228, // 0x97 - OK path (northwest-south)
  225, // 0x98 - OK path (southwest-north)
  229, // 0x99 - OK path (northwest-northeast)
  230, // 0x9A - OK path (southwest-southeast)
  233, // 0x9B - OK path junction (north-south-southwest)
  235, // 0x9C - OK path junction (north-south-northwest)
  234, // 0x9D - OK path junction (north-south-southeast)
  236, // 0x9E - OK path junction (north-south-northeast)
  242, // 0x9F - OK path junction (southwest-northeast-northwest)
  241, // 0xA0 - OK path junction (northwest-southeast-northeast)
  243, // 0xA1 - OK path junction (southwest-northeast-southeast)
  244, // 0xA2 - OK path junction (northwest-southeast-southwest)
  247, // 0xA3 - OK path crossing (northwest-southeast-southwest-northeast)
  232, // 0xA4 - OK path junction (northwest-northeast-south)
  175, // 0xA5 - OK trench with barbed wire (southwest-northeast)
  174, // 0xA6 - OK trench with barbed wire (northwest-southeast)
  172, // 0xA7 - OK trench (big hole)
  157, // 0xA8 - OK trench junction (northwest-southeast-northeast)
  159, // 0xA9 - OK trench junction (southwest-northeast-southeast)
  150, // 0xAA - OK trench (southwest-northeast)
  149, // 0xAB - OK trench (northwest-southeast)
  152, // 0xAC - OK trench (southwest-southeast)
  151, // 0xAD - OK trench (northwest-northeast)
  999};// 0xAE - non-visible end tile


// strings to use for unit conversions
const unsigned char hl_units[51][20] = {
  "Scouts",              // 0x00 = Spähpanzer
  "Heavy Tanks",         // 0x01 = A7V
  "Bomber Wing",         // 0x02 = Gotha
  "Bomber Wing",         // 0x03 = Zeppelin Staaken
  "Bomber Wing",         // 0x04 = Junkers J4-10
  "Interceptors",        // 0x05 = Fokker E I.
  "Interceptors",        // 0x06 = fokker E III.
  "Fighter Squadron",    // 0x07 = Albatros
  "Fighter Squadron",    // 0x08 = Fokker DR I
  "Fighter Squadron",    // 0x09 = Fokker D VII
  "Artillery",           // 0x0A = Leichte Ari
  "Artillery",           // 0x0B = Mittlere Ari
  "Artillery",           // 0x0C = Schwere Ari
  "Bunkers",             // 0x0D = Bunker
  "Anti-Aircraft Tanks", // 0x0E = Mobile Flak
  "Anti-Aircraft Guns",  // 0x0F = Stationäre Flak
  "Personnel Carriers",  // 0x10 = Depoteinheit
  "Infantry",            // 0x11 = Elite Infanterie
  "Infantry",            // 0x12 = Infanterie
  "Infantry",            // 0x13 = Panzerabwehr
  "Infantry",            // 0x14 = Pioniere
  "Infantry",            // 0x15 = Kavallerie
  "Personnel Carriers",  // 0x16 = Transportwagen
  "Mines",               // 0x17 = Ballon
  "Rail Guns",           // 0x18 = Schienengeschütz
  "Armoured Train",      // 0x19 = Panzerzug
  "Troop Train",         // 0x1A = Truppentransport
  "Patrol Boats",        // 0x1B = Patrouillenboot
  "Torpedo Boats",       // 0x1C = Torpedoboot
  "Submarines",          // 0x1D = U-Boot
  "",                    // 0x1E = U-Boot
  "Troopships",          // 0x1F = Transportschiff
  "Patrol Boats",        // 0x20 = Zerstörer
  "",                    // 0x21 = Zerstörer
  "Patrol Boats",        // 0x22 = Schlachtschiff
  "",                    // 0x23 = Schlachtschiff
  "Interceptors",        // 0x24 = Voisin
  "Bomber Wing",         // 0x25 = Handley
  "Bomber Wing",         // 0x26 = DH4
  "Interceptors",        // 0x27 = Morane
  "Interceptors",        // 0x28 = DH2
  "Interceptors",        // 0x29 = Nieuport
  "Interceptors",        // 0x2A = Spad VII
  "Fighter Squadron",    // 0x2B = SE5A
  "Fighter Squadron",    // 0x2C = Spad XIII
  "Fighter Squadron",    // 0x2D = Sopwith Camel
  "Scouts",              // 0x2E = Charron
  "Medium Tanks",        // 0x2F = Renault
  "Heavy Tanks",         // 0x30 = Mark I.
  "Heavy Tanks",         // 0x31 = St. Chammond
  "Heavy Tanks"};        // 0x32 = Mark IV.

// this is for identifying which units are transporters
const unsigned char hl_transport[51] = {
0, // 0x00 = Spähpanzer
0, // 0x01 = A7V
0, // 0x02 = Gotha
0, // 0x03 = Zeppelin Staaken
0, // 0x04 = Junkers J4-10
0, // 0x05 = Fokker E I.
0, // 0x06 = fokker E III.
0, // 0x07 = Albatros
0, // 0x08 = Fokker DR I
0, // 0x09 = Fokker D VII
0, // 0x0A = Leichte Ari
0, // 0x0B = Mittlere Ari
0, // 0x0C = Schwere Ari
0, // 0x0D = Bunker
0, // 0x0E = Mobile Flak
0, // 0x0F = Stationäre Flak
0, // 0x10 = Depoteinheit
0, // 0x11 = Elite Infanterie
0, // 0x12 = Infanterie
0, // 0x13 = Panzerabwehr
0, // 0x14 = Pioniere
0, // 0x15 = Kavallerie
1, // 0x16 = Transportwagen
0, // 0x17 = Ballon
0, // 0x18 = Schienengeschütz
0, // 0x19 = Panzerzug
1, // 0x1A = Truppentransport
0, // 0x1B = Patrouillenboot
0, // 0x1C = Torpedoboot
0, // 0x1D = U-Boot
0, // 0x1E = U-Boot
1, // 0x1F = Transportschiff
0, // 0x20 = Zerstörer
0, // 0x21 = Zerstörer
0, // 0x22 = Schlachtschiff
0, // 0x23 = Schlachtschiff
0, // 0x24 = Voisin
0, // 0x25 = Handley
0, // 0x26 = DH4
0, // 0x27 = Morane
0, // 0x28 = DH2
0, // 0x29 = Nieuport
0, // 0x2A = Spad VII
0, // 0x2B = SE5A
0, // 0x2C = Spad XIII
0, // 0x2D = Sopwith Camel
0, // 0x2E = Charron
0, // 0x2F = Renault
0, // 0x30 = Mark I.
0, // 0x31 = St. Chammond
0};// 0x32 = Mark IV.

// this is for finding out which buildings are which.
const unsigned char hl_buildings[8] = {
0x01,  // Headquarters Player 1
0x02,  // Headquarters Player 2
0x0C,  // Factory Neutral
0x0D,  // Factory Player 1
0x0E,  // Factory Player 2
0x0F,  // Depot Neutral
0x10,  // Depot Player 1
0x11}; // Depot player 2

// all the above in a struct, along with map type and number of units
const tconvdata hl_convdata =
{ MAP_HL, hl_units, hl_rawtiles, hl_buildings, hl_transport, 51 };

