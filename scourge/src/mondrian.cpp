/***************************************************************************
                     mondrian.cpp  -  Dungeon generator
                             -------------------
    begin                : Thu May 15 2003
    copyright            : (C) 2006 by Raphael Bosshard
    email                : raphael.bosshard@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "common/constants.h"
#include "mondrian.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "scourge.h"
#include "shapepalette.h"
#include "board.h"
#include "gui/progress.h"
#include "item.h"
#include "creature.h"
#include "debug.h"

using namespace std;

#define FORCE_WATER 0

// 1 out of SECRET_DOOR_CHANCE doors are secret doors
#ifdef DEBUG_SECRET_DOORS
#define SECRET_DOOR_CHANCE 2
#else
#define SECRET_DOOR_CHANCE 10
#endif

int totalWidth, totalHeight;

/*
width - max 31
height - max 31

curvyness - in %, the lower the more twisty maze
sparseness - (1-5) the higher the more sparse (more empty space)
loopyness - in %, the higher the more loops in the maze

roomcount
room max width
room max height

object count
*/
#define MAX_DUNGEON_LEVEL 7

const int MondrianGenerator::levels[][9] = {
	//height width roomMaxW    roomMaxH  roomMinW roomMinH  objects
	{ 10,  10,   4,    4,   2,   2,   5 },
	{ 15,  15,   6,    4,   2,   2,   10 },
	{ 15,  15,   5,   5,   3,   3,  15 },
	{ 20,  20,  6,   5,   3,   2,  20 },
	{ 25,  25,  6,   5,   3,   4,  25 },
	{ 30,  25,  6,   6,   4,   4,  30 },
	{ 31,  31,  7,   7,   5,   5,  35 }
};

MondrianGenerator::MondrianGenerator( Scourge *scourge, int level, int depth, int maxDepth,
    bool stairsDown, bool stairsUp, Mission *mission ) :
		TerrainGenerator( scourge, level, depth, maxDepth, stairsDown, stairsUp, mission, 18 ) {


	initByLevel();

	nodes = new Uint16*[ width ];
	for ( int i = 0; i < width; i++ ) {
		nodes[i] = new Uint16[ height ];
	}

	for ( int x = 0; x < width; x++ ) {
		for ( int y = 0; y < height; y++ ) {
			nodes[x][y] = UNVISITED;
		}
	}


	doorCount = 0;
}

MondrianGenerator::~MondrianGenerator() {
	for ( int i = 0; i < width; i++ ) {
		delete [] nodes[i];
	}
	delete [] nodes;

}

void MondrianGenerator::initByLevel() {
	int dungeonLevel = level / 8;
	if ( dungeonLevel < 0 ) dungeonLevel = 0;

	if ( dungeonLevel >= MAX_DUNGEON_LEVEL ) {
		cerr << "*** Warning: attempted to create dungeon level " << dungeonLevel <<
		". Max is " << MAX_DUNGEON_LEVEL << endl;
		dungeonLevel = MAX_DUNGEON_LEVEL - 1;
	}

//  cerr << "*** Creating dungeon level: " << dungeonLevel << " depth=" << depth << endl;

	this->width = levels[dungeonLevel][dgWIDTH];
	this->height = levels[dungeonLevel][dgHEIGHT];
	this->roomMaxWidth = levels[dungeonLevel][dgROOMMAXWIDTH];
	this->roomMaxHeight = levels[dungeonLevel][dgROOMMAXHEIGHT];
	this->roomMinWidth = levels[dungeonLevel][dgROOMMINWIDTH];
	this->roomMinHeight = levels[dungeonLevel][dgROOMMINHEIGHT];
	this->objectCount = levels[dungeonLevel][dgOBJECTCOUNT];

	this->monsters = true;
	this->roomCount = 0;
}

/* Nice unicode maze printer. Doesn't work with all fonts, through */

#ifdef _MSC_VER  // -=K=-: and i dont want the warnings
# pragma warning(push)
# pragma warning(disable : 4566) // "character cannot be represented in current code page"
#endif

void MondrianGenerator::printMazeUC() {
	printf( "---------------------------------------\n" );
	for ( int y = 0; y < height; y++ ) {
		for ( int x = 0; x < width; x++ ) {
			Uint16 node = nodes[x][y];

			if ( node & ROOM && node & N_PASS && node & W_PASS && node & S_PASS && node & E_PASS ) {
				printf( "\u253c" );
			} else if ( node & ROOM && node & N_PASS && node & W_PASS && node & S_PASS ) {
				printf( "\u2524" );
			} else if ( node & ROOM && node & N_PASS && node & S_PASS && node & E_PASS ) {
				printf( "\u251c" );
			} else if ( node & ROOM && node & S_PASS && node & E_PASS && node & W_PASS ) {
				printf( "\u252c" );
			} else if ( node & ROOM && node & N_PASS && node & E_PASS && node & W_PASS ) {
				printf( "\u2534" );
			} else if ( node & ROOM && node & S_PASS && node & N_PASS ) {
				printf( "\u2503" );
			} else if ( node & ROOM && node & S_PASS && node & E_PASS ) {
				printf( "\u250c" );
			} else if ( node & ROOM && node & S_PASS && node & W_PASS ) {
				printf( "\u2510" );
			} else if ( node & ROOM && node & N_PASS && node & E_PASS ) {
				printf( "\u2514" );
			} else if ( node & ROOM && node & N_PASS && node & W_PASS ) {
				printf( "\u2518" );
			} else if ( node & ROOM && node & E_PASS && node & W_PASS ) {
				printf( "\u2501" );
			} else if ( node & ROOM && node & E_PASS ) {
				printf( "\u257a" );
			} else if ( node & ROOM && node & W_PASS ) {
				printf( "\u2578" );
			} else if ( node & ROOM && node & S_PASS ) {
				printf( "\u257b" );
			} else if ( node & ROOM && node & N_PASS ) {
				printf( "\u2579" );
			} else if ( node & ROOM ) {
				printf( "*" );
			} else if ( node & ROOM2 ) {
				printf( "#" );
			} else {
				printf( " " );
			}
		}
		printf( "\n" );
	}
	printf( "---------------------------------------\n" );
}

#ifdef _MSC_VER
# pragma warning(pop) // back
#endif


void MondrianGenerator::printMaze() {
	printf( "---------------------------------------\n" );
	int c = 0;
	for ( int y = 0; y < height; y++ ) {
		for ( int i = 0; i < 3; i++ ) {
			for ( int x = 0; x < width; x++ ) {

				switch ( i ) {
				case 0: // top row
					if ( ( nodes[x][y] & N_PASS ) ) {
						printf( " | " );
					} else {
						printf( "   " );
					}
					break;
				case 1:
					if ( ( nodes[x][y] & W_PASS ) ) {
						printf( "-" );
					} else {
						printf( " " );
					}
					if ( nodes[x][y] == UNVISITED )
						printf( " " );
					else if ( nodes[x][y] & ROOM )
						printf( "*" );
					else
						printf( "O" );
					if ( ( nodes[x][y] & E_PASS ) ) {
						printf( "-" );
					} else {
						printf( " " );
					}
					break;
				case 2: // bottom row
					if ( ( nodes[x][y] & S_PASS ) ) {
						printf( " | " );
					} else {
						printf( "   " );
					}
					break;
				}
				c++;
			}
			printf( "\n" );
		}
		c++;
	}
	printf( "---------------------------------------\n" );
}



int MondrianGenerator::subdivideMaze( Sint16 x_start, Sint16 y_start, Sint16 width, Sint16 height, bool init ) {

	//If this is the first iteration, randomize init.
	//   Later it's used to decide wether to divide the room  horzontal or vertical
	if ( init )
//  init = (rand()%2 == 0);
		init = ( Util::dice( 2 ) ) != 0;

// fprintf( stderr, "Iteration: %d %d : %d %d\n", x_start, y_start, width, height);

	int horizontal = 0;
	int div = 0;

	bool passageHasDoor;

	Room roomA, roomB;
	int passage_x;
	int passage_y;
	Uint16 passage = ROOM;



	/* Try some sane defaults */
	int roomMinWidth = 2;// this->roomMinWidth;
	int roomMinHeight = 2; //this->roomMinHeight;
	float roomMulFac = 2.15f;

	// NOTE:
	//   Horizontal subdivision: divide width
	//   Vertical   subdivision: divide height
	//
	//   This is noted here because the original got confused himself. This is no
	//   uncommon state for him, though.
	//


	bool hSubdivisionOK = ( static_cast<float>( width ) > static_cast<float>( roomMinWidth * roomMulFac ) );
	bool vSubdivisionOK = ( static_cast<float>( height ) > static_cast<float>( roomMinHeight * roomMulFac ) );

	// What would fit now? Two horizontal or two vertial rooms?
	if ( hSubdivisionOK && vSubdivisionOK ) {
		// Everything fits
		horizontal = init;

	} else if ( hSubdivisionOK ) {
		//only horizonal rooms fit
		horizontal = 1;

	} else if ( vSubdivisionOK ) {
		//only vertical rooms fit
		horizontal = 0;
	} else {

		// there is no space for more rooms. Boil out and let the caller
		// place one big room instead of two little ones.
		return 0;
	}


	// Is it a door or a passage?


	// FIXME!
	// The "make-sure-that-there-is-no-clutter-in-front-of-the-door"-algorithm notices _DOORs only, thus
	// simple pathways are always cluttered. Improve Map::isDoor and enable the rand() again.

	//if(0 == rand()%2)
	passageHasDoor = true;

	// Now that we know that there's space left for another two rooms, lets make them.

	// make two rooms alongside
	if ( horizontal ) {
		while ( ( div < roomMinWidth ) || ( width - div < roomMinWidth ) ) {
			div = Util::dice( width );
		}

//  printf("horizontal, div %d, w2 %d, h %d\n", div, width - div, height  );
		roomA.x = offset + x_start * unitSide;
		roomA.y = offset + y_start * unitSide;
		roomA.w = div * unitSide;
		roomA.h = height * unitSide;
		roomA.valueBonus = depth / 2;

		roomB.x = offset + ( x_start + div ) * unitSide;
		roomB.y = offset + y_start * unitSide;
		roomB.w = ( width - div ) * unitSide;
		roomB.h = height * unitSide;
		roomB.valueBonus = depth / 2;

		passage_x = x_start + div - 1;
		passage_y = y_start + ( height / 2 );

		passage = E_PASS;
		if ( passageHasDoor )
			passage |= E_DOOR;

	} else {
		// make two rooms, on on top of the other
		while ( ( div < roomMinHeight ) || ( height - div < roomMinHeight ) ) {
			div = Util::dice( height );
		}

//  printf("vertical, div %d, w2 %d, h %d\n", div, width - div, height  );
		roomA.x = offset + x_start * unitSide;
		roomA.y = offset + y_start * unitSide;
		roomA.w = width * unitSide;
		roomA.h = div * unitSide;
		roomA.valueBonus = depth / 2;

		roomB.x = offset + x_start * unitSide;
		roomB.y = offset + ( y_start + div ) * unitSide;
		roomB.w = width * unitSide;
		roomB.h = ( height - div ) * unitSide;
		roomB.valueBonus = depth / 2;

		passage_x = x_start + ( width / 2 );
		passage_y = y_start + ( div - 1 );

		passage = S_PASS;
		if ( passageHasDoor )
			passage |= S_DOOR;
	}

	int r = roomCount; //this->roomCount;
	if ( !subdivideMaze( ( roomA.x - offset ) / unitSide, ( roomA.y - offset ) / unitSide, roomA.w / unitSide, roomA.h / unitSide, false ) ) {
		// if we cannot divide the space once more, make a room
		room[r].x = roomA.x;
		room[r].y = roomA.y;
		room[r].w = roomA.w;
		room[r].h = roomA.h;
		room[r].valueBonus = roomA.valueBonus;

		// alternate floor tile style
		if ( roomCount % 2 == 0 ) {
			for ( int x = 0; x < room[r].w; x++ ) {
				for ( int y = 0; y < room[r].h; y++ ) {
					//nodes[x + ( room[r].x - offset ) / unitSide][y + ( room[r].y - offset ) / unitSide] -= ROOM;
				}
			}
		}

//		printf("N: %d x/y %d/%d w/h %d/%d\n", r, roomA.x, roomA.y, roomA.w, roomA.h);

		roomCount++;//this->roomCount++;
	}
	assert( roomCount < 200 );


	
	r = roomCount;//this->roomCount;
	if ( !subdivideMaze( ( roomB.x - offset ) / unitSide, ( roomB.y - offset ) / unitSide, roomB.w / unitSide, roomB.h / unitSide, false ) ) {
		//if we cannot divice the space once more, make a room
		room[r].x = roomB.x;
		room[r].y = roomB.y;
		room[r].w = roomB.w;
		room[r].h = roomB.h;
		room[r].valueBonus = roomB.valueBonus;

		// alternate floor tile style
		if ( roomCount % 2 == 0 ) {
			for ( int x = 0; x < room[r].w; x++ ) {
				for ( int y = 0; y < room[r].h; y++ ) {
					//nodes[x + ( room[r].x - offset ) / unitSide][y + ( room[r].y - offset ) / unitSide] -= ROOM;
				}
			}
		}

//		printf("N: %d x/y %d/%d w/h %d/%d\n", r, roomB.x, roomB.y, roomB.w, roomB.h);

		roomCount ++;//this->roomCount++;
	}
	assert( roomCount < 200 );


	if ( horizontal ) {
		for ( int y = 0; y < height; y++ ) {
			assert( ( x_start + ( div - 1 ) ) >= 0 &&
			        ( x_start + ( div - 1 ) ) < totalWidth &&
			        ( y_start + y ) >= 0 &&
			        ( y_start + y ) < totalHeight );
			nodes[( x_start + ( div - 1 ) )][ ( y_start + y ) ] -= E_PASS;
		}
	} else {
		for ( int x = 0; x < width; x++ ) {
			assert( x_start + x >= 0 &&
			        x_start + x < totalWidth &&
			        y_start + ( div - 1 ) >= 0 &&
			        y_start + ( div - 1 ) < totalHeight );
			nodes[ ( x_start + x )][ ( y_start + ( div - 1 ) ) ] -= S_PASS;
		}
	}

	//connect the rooms
	assert( passage_x >= 0 && passage_x < totalWidth && passage_y >= 0 && passage_y < totalHeight );
	nodes[ passage_x ][passage_y ] |= passage;

	// the space has been subdivided successfully
	return 1;
}

void MondrianGenerator::initRoom( Room room ) {
	for ( int x = 0; x < room.w; x++ ) {
		for ( int y = 0; y < room.h; y++ ) {
			int pass = 0;
			if ( x > 0 )
				pass |= W_PASS;
			if ( x < room.w - 1 )
				pass |= E_PASS;
			if ( y > 0 )
				pass |= N_PASS;
			if ( y < room.h - 1 )
				pass |= S_PASS;

			assert( x + room.x >= 0 &&
			        x + room.x < totalWidth &&
			        y + room.y >= 0 &&
			        y + room.y < totalHeight );
			nodes[x + room.x][y + room.y] = ROOM | pass;
		}
	}

}


void MondrianGenerator::generate( Map *map, ShapePalette *shapePal ) {
// cerr << "MONDRIAN" << endl;

	updateStatus( _( "Assembling Dungeon Level" ) );

	//Sint16 mapx, mapy;
	for ( Sint16 x = 0; x < width; x++ ) {
		for ( Sint16 y = 0; y < height; y++ ) {
			int pass = 0;
			if ( x > 0 )
				pass |= W_PASS;
			if ( x < width - 1 )
				pass |= E_PASS;
			if ( y > 0 )
				pass |= N_PASS;
			if ( y < height - 1 )
				pass |= S_PASS;

			nodes[x][y] = ROOM | pass;
		}
	}


	this->roomCount = 0;

	totalWidth = width;
	totalHeight = height;
	subdivideMaze( 0, 0, width, height, true );

	/*
	int i = 0;
	room[i].x = offset + 0 * unitSide;
	room[i].y = offset + 0 * unitSide;
	room[i].w = width * unitSide;
	room[i].h = height * unitSide;
	room[i].valueBonus = depth / 2;
	roomCount = 1;
	*/

	//printMaze();
	//for( int i = 0; i < roomCount; i++ ) {
	//cerr << "Room: " << i << " " << room[i].x << "," << room[i].y << " dim=" << room[i].w << "," << room[i].h << endl;
	//}
}


bool MondrianGenerator::drawNodes( Map *map, ShapePalette *shapePal ) {
	// flooded map?
	map->setHasWater( FORCE_WATER ||
	                  0 == Util::dice( 5 ) );

	updateStatus( _( "Loading theme" ) );
	if ( map->getPreferences()->isDebugTheme() ) shapePal->loadDebugTheme();
	else shapePal->loadRandomTheme();

	updateStatus( _( "Drawing walls" ) );
	drawBasics( map, shapePal );

	updateStatus( _( "Fixing rooms" ) );
	removeColumns( map, shapePal );
	addRugs( map, shapePal );

	return true;
}



void MondrianGenerator::drawBasics( Map *map, ShapePalette *shapePal ) {
	// add shapes to map
	Sint16 mapx, mapy;
	for ( Sint16 x = 0; x < width; x++ ) {
		for ( Sint16 y = 0; y < height; y++ ) {

			mapx = x * unitSide + offset;
			mapy = y * unitSide + offset;
			if ( nodes[x][y] != UNVISITED ) {

				if ( nodes[x][y] >= ROOM ) {
					map->setFloorPosition( mapx, mapy + unitSide,
					                       shapePal->findShapeByName( "ROOM_FLOOR_TILE" ) );
				} else {
					map->setFloorPosition( mapx, mapy + unitSide,
					                       shapePal->findShapeByName( "FLOOR_TILE" ) );
				}

				// init the free space
				int secretDoor = 0;
				if ( nodes[x][y] & E_DOOR ) {
					if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
						nodes[x][y] -= E_DOOR;
						secretDoor = E_DOOR;
						nodes[x][y] -= E_PASS;
					} else {
						drawDoor( map, shapePal, mapx, mapy, E_DOOR );
					}
				} else if ( nodes[x][y] & W_DOOR ) {
					if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
						nodes[x][y] -= W_DOOR;
						secretDoor = W_DOOR;
						nodes[x][y] -= W_PASS;
					} else {
						drawDoor( map, shapePal, mapx, mapy, W_DOOR );
					}
				} else if ( nodes[x][y] & N_DOOR ) {
					if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
						nodes[x][y] -= N_DOOR;
						secretDoor = N_DOOR;
						nodes[x][y] -= N_PASS;
					} else {
						drawDoor( map, shapePal, mapx, mapy, N_DOOR );
					}
				} else if ( nodes[x][y] & S_DOOR ) {
					if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
						nodes[x][y] -= S_DOOR;
						secretDoor = S_DOOR;
						nodes[x][y] -= S_PASS;
					} else {
						drawDoor( map, shapePal, mapx, mapy, S_DOOR );
					}
				}

				// random doors
				if ( !secretDoor ) {
					if ( ( nodes[x][y] & W_PASS ) &&
					        !( nodes[x][y] & N_PASS ) &&
					        !( nodes[x][y] & S_PASS ) ) {
						if ( Util::dice( 100 ) <= randomDoors ) {
							if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
								nodes[x][y] -= W_DOOR;
								secretDoor = W_DOOR;
								nodes[x][y] -= W_PASS;
							} else {
								drawDoor( map, shapePal, mapx, mapy, W_DOOR );
							}
						}
					}
					if ( ( nodes[x][y] & E_PASS ) &&
					        !( nodes[x][y] & N_PASS ) &&
					        !( nodes[x][y] & S_PASS ) ) {
						if ( Util::dice( 100 ) <= randomDoors ) {
							if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
								nodes[x][y] -= E_DOOR;
								secretDoor = E_DOOR;
								nodes[x][y] -= E_PASS;
							} else {
								drawDoor( map, shapePal, mapx, mapy, E_DOOR );
							}
						}
					}
					if ( ( nodes[x][y] & S_PASS ) &&
					        !( nodes[x][y] & W_PASS ) &&
					        !( nodes[x][y] & E_PASS ) ) {
						if ( Util::dice( 100 ) <= randomDoors ) {
							if ( 0 == Util::dice( static_cast<int>( SECRET_DOOR_CHANCE ) ) ) {
								nodes[x][y] -= S_DOOR;
								secretDoor = S_DOOR;
								nodes[x][y] -= S_PASS;
							} else {
								drawDoor( map, shapePal, mapx, mapy, S_DOOR );
							}
						}
					}
					if ( ( nodes[x][y] & N_PASS ) &&
					        !( nodes[x][y] & W_PASS ) &&
					        !( nodes[x][y] & E_PASS ) ) {
						if ( Util::dice( 100 ) <= randomDoors ) {
							if ( 0 == Util::dice( SECRET_DOOR_CHANCE ) ) {
								nodes[x][y] -= N_DOOR;
								secretDoor = N_DOOR;
								nodes[x][y] -= N_PASS;
							} else {
								drawDoor( map, shapePal, mapx, mapy, N_DOOR );
							}
						}
					}
				}

				int wallX, wallY;
				Shape *wall = NULL;
				if ( !( nodes[x][y] & W_PASS ) ) {
					if ( nodes[x][y] & N_PASS && nodes[x][y] & S_PASS ) {
						wallX = mapx;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "EW_WALL_TWO_EXTRAS" );
					} else if ( nodes[x][y] & N_PASS ) {
						wallX = mapx;
						wallY = mapy + unitSide - unitOffset;
						wall = shapePal->findShapeByName( "EW_WALL_EXTRA" );
					} else if ( nodes[x][y] & S_PASS ) {
						wallX = mapx;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "EW_WALL_EXTRA" );
					} else {
						wallX = mapx;
						wallY = mapy + unitSide - unitOffset;
						wall = shapePal->findShapeByName( "EW_WALL" );
					}
					if ( wall ) {
						map->setPosition( wallX, wallY, 0, wall );
						if ( secretDoor == W_DOOR ) {
							map->addSecretDoor( wallX, wallY );
						} else {
							if ( Util::dice( 100 ) <= torches ) {
								map->setPosition( mapx + unitOffset, mapy + unitSide - 4,
								                  6, shapePal->findShapeByName( "LAMP_WEST" ) );
								map->setPosition( mapx + unitOffset, mapy + unitSide - 4,
								                  4, shapePal->findShapeByName( "LAMP_BASE" ) );
							}
						}
					}
				}
				if ( !( nodes[x][y] & E_PASS ) ) {
					if ( nodes[x][y] & N_PASS && nodes[x][y] & S_PASS ) {
						wallX = mapx + unitSide - unitOffset;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "EW_WALL_TWO_EXTRAS" );
					} else if ( nodes[x][y] & N_PASS ) {
						wallX = mapx + unitSide - unitOffset;
						wallY = mapy + unitSide - unitOffset;
						wall = shapePal->findShapeByName( "EW_WALL_EXTRA" );
					} else if ( nodes[x][y] & S_PASS ) {
						wallX = mapx + unitSide - unitOffset;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "EW_WALL_EXTRA" );
					} else {
						wallX = mapx + unitSide - unitOffset;
						wallY = mapy + unitSide - unitOffset;
						wall = shapePal->findShapeByName( "EW_WALL" );
					}
					if ( wall ) {
						map->setPosition( wallX, wallY, 0, wall );
						if ( secretDoor == E_DOOR ) {
							map->addSecretDoor( wallX, wallY );
						} else {
							if ( Util::dice( 100 ) <= torches ) {
								map->setPosition( mapx + unitSide - ( unitOffset + 1 ), mapy + unitSide - 4,
								                  6, shapePal->findShapeByName( "LAMP_EAST" ) );
								map->setPosition( mapx + unitSide - ( unitOffset + 1 ), mapy + unitSide - 4,
								                  4, shapePal->findShapeByName( "LAMP_BASE" ) );
							}
						}
					}
				}
				if ( !( nodes[x][y] & N_PASS ) ) {
					if ( nodes[x][y] & W_PASS && nodes[x][y] & E_PASS ) {
						wallX = mapx;
						wallY = mapy + unitOffset;
						wall = shapePal->findShapeByName( "NS_WALL_TWO_EXTRAS" );
					} else if ( nodes[x][y] & W_PASS ) {
						wallX = mapx;
						wallY = mapy + unitOffset;
						wall = shapePal->findShapeByName( "NS_WALL_EXTRA" );
					} else if ( nodes[x][y] & E_PASS ) {
						wallX = mapx + unitOffset;
						wallY = mapy + unitOffset;
						wall = shapePal->findShapeByName( "NS_WALL_EXTRA" );
					} else {
						wallX = mapx + unitOffset;
						wallY = mapy + unitOffset;
						wall = shapePal->findShapeByName( "NS_WALL" );
					}
					if ( wall ) {
						map->setPosition( wallX, wallY, 0, wall );
						if ( secretDoor == N_DOOR ) {
							map->addSecretDoor( wallX, wallY );
						} else {
							if ( Util::dice( 100 ) <= torches ) {
								map->setPosition( mapx + 4, mapy + unitOffset + 1, 6,
								                  shapePal->findShapeByName( "LAMP_NORTH" ) );
								map->setPosition( mapx + 4, mapy + unitOffset + 1, 4,
								                  shapePal->findShapeByName( "LAMP_BASE" ) );
							}
						}
					}
				}
				if ( !( nodes[x][y] & S_PASS ) ) {
					if ( nodes[x][y] & W_PASS && nodes[x][y] & E_PASS ) {
						wallX = mapx;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "NS_WALL_TWO_EXTRAS" );
					} else if ( nodes[x][y] & W_PASS ) {
						wallX = mapx;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "NS_WALL_EXTRA" );
					} else if ( nodes[x][y] & E_PASS ) {
						wallX = mapx + unitOffset;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "NS_WALL_EXTRA" );
					} else {
						wallX = mapx + unitOffset;
						wallY = mapy + unitSide;
						wall = shapePal->findShapeByName( "NS_WALL" );
					}
					if ( wall ) {
						map->setPosition( wallX, wallY, 0, wall );
						if ( secretDoor == S_DOOR ) {
							map->addSecretDoor( wallX, wallY );
						}
					}
				}


				if ( nodes[x][y] & N_PASS && nodes[x][y] & W_PASS ) {
					map->setPosition( mapx, mapy + unitOffset, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( nodes[x][y] & N_PASS && nodes[x][y] & E_PASS ) {
					map->setPosition( mapx + unitSide - unitOffset, mapy + unitOffset, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( nodes[x][y] & S_PASS && nodes[x][y] & W_PASS ) {
					map->setPosition( mapx, mapy + unitSide, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( nodes[x][y] & S_PASS && nodes[x][y] & E_PASS ) {
					map->setPosition( mapx + unitSide - unitOffset, mapy + unitSide, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( !( nodes[x][y] & N_PASS ) && !( nodes[x][y] & W_PASS ) ) {
					map->setPosition( mapx, mapy + unitOffset, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( !( nodes[x][y] & N_PASS ) && !( nodes[x][y] & E_PASS ) ) {
					map->setPosition( mapx + unitSide - unitOffset, mapy + unitOffset, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( !( nodes[x][y] & S_PASS ) && !( nodes[x][y] & W_PASS ) ) {
					map->setPosition( mapx, mapy + unitSide, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
				if ( !( nodes[x][y] & S_PASS ) && !( nodes[x][y] & E_PASS ) ) {
					map->setPosition( mapx + unitSide - unitOffset, mapy + unitSide, 0,
					                  shapePal->findShapeByName( "CORNER" ) );
				}
			}
		}
	}
}


void MondrianGenerator::removeColumns( Map *map, ShapePalette *shapePal ) {
	// Remove 'columns' from rooms
	for ( int roomIndex = 0; roomIndex < roomCount; roomIndex++ ) {
		int startx = ( room[roomIndex].x ) + unitOffset;
		int endx = ( room[roomIndex].x + room[roomIndex].w ) - ( unitOffset * 2 );
		int starty = ( room[roomIndex].y ) + ( unitOffset * 2 );
		int endy = ( room[roomIndex].y + room[roomIndex].h ) - unitOffset;
		for ( int x = startx; x < endx; x++ ) {
			for ( int y = starty; y < endy; y++ ) {
				map->removePosition( x, y, 0 );
			}
		}
	}
}


void MondrianGenerator::drawEastDoor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool secret ) {
	if ( !map->coversDoor( shapePal->findShapeByName( "EW_DOOR" ),
	                       mapx + unitSide - unitOffset + 1, mapy + unitSide - unitOffset - 2 ) ) {
		if ( secret ) {
		} else {
			map->setPosition( mapx + unitSide - unitOffset, mapy + unitSide - unitOffset,
			                  wallHeight - 2, shapePal->findShapeByName( "EW_DOOR_TOP" ) );
			map->setPosition( mapx + unitSide - unitOffset, mapy + unitOffset +  2,
			                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
			map->setPosition( mapx + unitSide - unitOffset, mapy + unitOffset * 2 +  2,
			                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
			map->setPosition( mapx + unitSide - unitOffset + 1, mapy + unitSide - unitOffset - 2,
			                  0, shapePal->findShapeByName( "EW_DOOR" ) );
			if ( doorCount < MAX_DOOR_COUNT ) {
				door[doorCount][0] = mapx + unitSide - unitOffset + 1;
				door[doorCount][1] = mapy + unitSide - unitOffset - 2;
				doorCount++;
			}
			map->setPosition( mapx + unitSide - unitOffset, mapy + unitSide - unitOffset,
			                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		}
	}
}

void MondrianGenerator::drawWestDoor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool secret ) {
	if ( !map->coversDoor( shapePal->findShapeByName( "EW_DOOR" ),
	                       mapx + 1, mapy + unitSide - unitOffset - 2 ) ) {
		map->setPosition( mapx, mapy + unitSide - unitOffset,
		                  wallHeight - 2, shapePal->findShapeByName( "EW_DOOR_TOP" ) );
		map->setPosition( mapx, mapy + unitOffset +  2,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		map->setPosition( mapx, mapy + unitOffset * 2 +  2,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		map->setPosition( mapx + 1, mapy + unitSide - unitOffset - 2,
		                  0, shapePal->findShapeByName( "EW_DOOR" ) );
		if ( doorCount < MAX_DOOR_COUNT ) {
			door[doorCount][0] = mapx + 1;
			door[doorCount][1] = mapy + unitSide - unitOffset - 2;
			doorCount++;
		}
		map->setPosition( mapx, mapy + unitSide - unitOffset,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
	}
}

void MondrianGenerator::drawSouthDoor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool secret ) {
	if ( !map->coversDoor( shapePal->findShapeByName( "NS_DOOR" ),
	                       mapx + unitOffset * 2, mapy + unitSide - 1 ) ) {
		map->setPosition( mapx + unitOffset, mapy + unitSide,
		                  wallHeight - 2, shapePal->findShapeByName( "NS_DOOR_TOP" ) );
		map->setPosition( mapx + unitOffset, mapy + unitSide,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		map->setPosition( mapx + unitOffset * 2, mapy + unitSide - 1,
		                  0, shapePal->findShapeByName( "NS_DOOR" ) );
		if ( doorCount < MAX_DOOR_COUNT ) {
			door[doorCount][0] = mapx + unitOffset * 2;
			door[doorCount][1] = mapy + unitSide - 1;
			doorCount++;
		}
		map->setPosition( mapx + unitSide - unitOffset * 2, mapy + unitSide,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		map->setPosition( mapx + unitSide - unitOffset * 3, mapy + unitSide,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
	}
}

void MondrianGenerator::drawNorthDoor( Map *map, ShapePalette *shapePal, Sint16 mapx, Sint16 mapy, bool secret ) {
	if ( !map->coversDoor( shapePal->findShapeByName( "NS_DOOR" ),
	                       mapx + unitOffset * 2, mapy + unitOffset - 1 ) ) {
		map->setPosition( mapx + unitOffset, mapy + unitOffset,
		                  wallHeight - 2, shapePal->findShapeByName( "NS_DOOR_TOP" ) );
		map->setPosition( mapx + unitOffset, mapy + unitOffset,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		map->setPosition( mapx + unitOffset * 2, mapy + unitOffset - 1,
		                  0, shapePal->findShapeByName( "NS_DOOR" ) );
		if ( doorCount < MAX_DOOR_COUNT ) {
			door[doorCount][0] = mapx + unitOffset * 2;
			door[doorCount][1] = mapy + unitOffset - 1;
			doorCount++;
		}
		map->setPosition( mapx + unitSide - unitOffset * 2, mapy + unitOffset,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		map->setPosition( mapx + unitSide - unitOffset * 3, mapy + unitOffset,
		                  0, shapePal->findShapeByName( "DOOR_SIDE" ) );
	}
}

void MondrianGenerator::drawDoor( Map *map, ShapePalette *shapePal,
    Sint16 mapx, Sint16 mapy, int doorType, bool secret ) {
	switch ( doorType ) {
	case E_DOOR: drawEastDoor( map, shapePal, mapx, mapy, secret ); break;
	case W_DOOR: drawWestDoor( map, shapePal, mapx, mapy, secret ); break;
	case N_DOOR: drawNorthDoor( map, shapePal, mapx, mapy, secret ); break;
	case S_DOOR: drawSouthDoor( map, shapePal, mapx, mapy, secret ); break;
	default: cerr << "*** Error: Unknown door type: " << doorType << endl;
	}
}

void MondrianGenerator::addFurniture( Map *map, ShapePalette *shapePal ) {

	// add tables, chairs, etc.
	addItemsInEveryRoom( RpgItem::getItemByName( "Table" ), 1 );
	addItemsInEveryRoom( RpgItem::getItemByName( "Chair" ), 2 );

	// add some magic pools
	DisplayInfo di;
	for ( int i = 0; i < roomCount; i++ ) {
		MagicSchool *ms = MagicSchool::getRandomSchool();
		di.red = ms->getDeityRed();
		di.green = ms->getDeityGreen();
		di.blue = ms->getDeityBlue();
		Location *pos = addShapeInRoom( scourge->getShapePalette()->findShapeByName( "POOL" ), i, &di );
		if ( pos ) {
			// store pos->deity in scourge
			scourge->addDeityLocation( pos, ms );
		}
		//}
	}
}

void MondrianGenerator::addContainers( Map *map, ShapePalette *shapePal ) {
	addContainersInRooms( map, shapePal );
}

MapRenderHelper *MondrianGenerator::getMapRenderHelper() {
	return MapRenderHelper::helpers[ MapRenderHelper::ROOM_HELPER ];
}

