levelsdir = $(pkgdatadir)/levels
levels_DATA = \
Anthill.lev \
ArmsRace.lev \
Baptism.lev \
BeachRaid.lev \
ClippedWings.lev \
Foxhole.lev \
General.lev \
GreatBattle.lev \
HeavyMetal.lev \
IslandHoppers.lev \
LakeYukarwa.lev \
LankhValley.lev \
LostFactories.lev \
MountainDefense.lev \
OmyarGorge.lev \
Plowshares.lev \
RadioSilence.lev \
Revelation.lev \
Tutorial1.lev \
Tutorial2.lev \
Tutorial3.lev \
Uprising.lev

EXTRA_DIST = $(levels_DATA:.lev=.src)
CLEANFILES = $(levels_DATA)

def_tiles = $(top_builddir)/tools/default.tiles
def_units = $(top_builddir)/tools/default.units

%.lev:	%.src $(top_srcdir)/tools/cfed.cpp $(top_srcdir)/src/comet/mission.cpp
	$(top_builddir)/tools/cfed "$<" --tiles $(def_tiles) --units $(def_units) -o "$@"

