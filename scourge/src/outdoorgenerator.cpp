/***************************************************************************
                          outdoorgenerator.cpp  -  description
                             -------------------
    begin                : Sat May 12 2007
    copyright            : (C) 2007 by Gabor Torok
    email                : cctorok@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "outdoorgenerator.h"
#include "render/map.h"
#include "shapepalette.h"
#include "scourge.h"
#include "board.h"
#include "render/maprenderhelper.h"
#include "render/glshape.h"
#include <vector>

using namespace std;

OutdoorGenerator::OutdoorGenerator( Scourge *scourge, int level, int depth, int maxDepth,
																		bool stairsDown, bool stairsUp, 
																		Mission *mission) : 
TerrainGenerator( scourge, level, depth, maxDepth, stairsDown, stairsUp, mission, 12 ) {
	for( int x = 0; x < MAP_WIDTH; x++ ) {
		for( int y = 0; y < MAP_DEPTH; y++ ) {
			ground[x][y] = 0;
		}
	}

  // reasonable defaults
  TerrainGenerator::doorCount = 0;
  TerrainGenerator::roomCount = 1;
  TerrainGenerator::room[0].x = room[0].y = 0;
  TerrainGenerator::room[0].w = ( MAP_WIDTH - 2 * MAP_OFFSET ) / MAP_UNIT;
  TerrainGenerator::room[0].h = ( MAP_DEPTH - 2 * MAP_OFFSET ) / MAP_UNIT;
  TerrainGenerator::room[0].valueBonus = 0;
  TerrainGenerator::roomMaxWidth = 0;
  TerrainGenerator::roomMaxHeight = 0;
  TerrainGenerator::objectCount = 7 + ( level / 8 ) * 5;
  TerrainGenerator::monsters = true;

}

OutdoorGenerator::~OutdoorGenerator() {
}

void OutdoorGenerator::generate( Map *map, ShapePalette *shapePal ) {
	// create the undulating ground
	for( int x = 0; x < MAP_WIDTH; x++ ) {
		for( int y = 0; y < MAP_DEPTH; y++ ) {
			// fixme: use a more sinoid function here
			// ground[x][y] = ( 1.0f * rand() / RAND_MAX );
			ground[x][y] = 3 + 
				( 3.0f * 
					sin( PI / ( 180.0f / (float)( x * 8.0f ) ) ) * 
					cos( PI / ( 180.0f / (float)( y  * 8.0f )) ) );
			if( ground[x][y] < 0 ) ground[x][y] = 0;
		}
	}
}

bool OutdoorGenerator::drawNodes( Map *map, ShapePalette *shapePal ) {

  updateStatus( _( "Loading theme" ) );
  if( map->getPreferences()->isDebugTheme() ) shapePal->loadDebugTheme();
	else shapePal->loadRandomTheme();

	// do this first, before adding shapes
	// FIXME: elevate shapes if needed, in Map::setGroundHeight, so this method can be called anytime
	for( int x = 0; x < MAP_WIDTH; x++ ) {
		for( int y = 0; y < MAP_DEPTH; y++ ) {
			map->setGroundHeight( x, y, ground[x][y] );
		}
	}

	map->setHeightMapEnabled( true );
	map->setFloor( CAVE_CHUNK_SIZE, CAVE_CHUNK_SIZE, shapePal->getNamedTexture( "grass" ) );

	// set the floor, so random positioning works in terrain generator
	for( int x = MAP_OFFSET; x < MAP_WIDTH - MAP_OFFSET; x += MAP_UNIT ) {
		for( int y = MAP_OFFSET; y < MAP_DEPTH - MAP_OFFSET; y += MAP_UNIT ) {
			map->setFloorPosition( (Sint16)x, (Sint16)y + MAP_UNIT, (Shape*)shapePal->findShapeByName( "FLOOR_TILE", true ) );
		}
	}

	// add some groups of trees
	for( int i = 0; i < 20; i++ ) {
		int w = (int)( 40.0f * rand() / RAND_MAX ) + 20;
		int h = (int)( 40.0f * rand() / RAND_MAX ) + 20;
		int x = MAP_OFFSET + (int)( (float)( MAP_WIDTH - w - MAP_OFFSET * 2 ) * rand() / RAND_MAX );
		int y = MAP_OFFSET + (int)( (float)( MAP_DEPTH - h - MAP_OFFSET * 2 ) * rand() / RAND_MAX );
		int count = 0;
		while( count < 10 ) {
			GLShape *shape = getRandomTreeShape( shapePal );
			int xx = x + (int)( (float)( w - shape->getWidth() ) * rand() / RAND_MAX );
			int yy = y + (int)( (float)( h - shape->getDepth() ) * rand() / RAND_MAX );
			if( !map->isBlocked( xx, yy, 0, 0, 0, 0, shape ) ) {
				map->setPosition( xx, yy, 0, shape );
			} else {
				count++;
			}
		}
	}

	return true;
}

vector<GLShape*> trees;
GLShape *OutdoorGenerator::getRandomTreeShape( ShapePalette *shapePal ) {
	if( trees.size() == 0 ) {
		// this should be in config file
		trees.push_back( shapePal->findShapeByName( "PINE_TREE" ) );
		trees.push_back( shapePal->findShapeByName( "WILLOW_TREE" ) );
		trees.push_back( shapePal->findShapeByName( "OAK_TREE" ) );
		trees.push_back( shapePal->findShapeByName( "OAK2_TREE" ) );
		trees.push_back( shapePal->findShapeByName( "BUSH" ) );
		trees.push_back( shapePal->findShapeByName( "BUSH2" ) );
		trees.push_back( shapePal->findShapeByName( "DEMO_TREE" ) );
	}
	return trees[ (int)( (float)( trees.size() ) * rand() / RAND_MAX ) ];
}

MapRenderHelper* OutdoorGenerator::getMapRenderHelper() {
	// we need fog
	//return MapRenderHelper::helpers[ MapRenderHelper::CAVE_HELPER ];
	return MapRenderHelper::helpers[ MapRenderHelper::ROOM_HELPER ];
}
