/***************************************************************************
                landgenerator.cpp  -  Generates outdoor maps
                             -------------------
    begin                : Sat March 28, 2009
    copyright            : (C) 2009 by Gabor Torok
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
#include "common/constants.h"
#include "landgenerator.h"
#include "render/map.h"
#include "shapepalette.h"
#include "scourge.h"
#include "board.h"
#include "render/maprender.h"
#include "render/maprenderhelper.h"
#include "render/glshape.h"
#include "render/virtualshape.h"
#include "sqbinding/sqbinding.h"
#include "rpg/rpglib.h"
#include "creature.h"
#include "render/texture.h"
#include "board.h"
#include <vector>
#include <map>

using namespace std;


/**
 * This generator generates a 296x296 (quarter) map. Before calling generate(), be sure to call setRegion().
 * 
 * A region is a 32x32 pixel section of the map bitmap. The map bitmap is broken into 128x128 pixel section.
 * (See: scourge_data/mapgrid/world/) There are 4x4 regions per bitmap grid (128/32). So for example, calling
 * setRegion( 6, 39 ) would load map_100.png ( (int)( 39 / 4 ) * 11 + (int)( 6 / 4 ) ). Then inside map_100.png, 
 * the region shown is 2, 1.
 * 
 * A Map object can thus contain 4 296x296 map section (since it's 592x592 units large) which is the max number
 * of sections seen by the player at one time. (If the player reaches the corner of one 296x296 section.) 
 * 
 * When the player reaches the corner, existing sections will be passivated (saved) and new ones will be loaded or 
 * generated by this class. The section(s) that remain in memory (drawn on the Map class) will be shuffled around. 
 * For example, when traveling north, the southern section will be passivated, the northern section becomes the southern
 * one and a new section is loaded/generated into the north part of the in-memory Map class.
 * 
 * Also remember to call setMapPosition() before calling generate to denote where in the map the new quarter
 * should get rendered to. The arguments to setMapPosition() are in OUTDOORS_STEP units (so, 75, 75 is the fourth quarter.)
 */
LandGenerator::LandGenerator( Scourge *scourge, int level, int depth, int maxDepth,
                              bool stairsDown, bool stairsUp,
                              Mission *mission ) :
		TerrainGenerator( scourge, level, depth, maxDepth, stairsDown, stairsUp, mission, 13 ) {
	// init the ground
	for ( int x = 0; x < QUARTER_WIDTH_IN_NODES; x++ ) {
		for ( int y = 0; y < QUARTER_DEPTH_IN_NODES; y++ ) {
			ground[x][y] = 0;
		}
	}
	this->cellular = new CellularAutomaton( QUARTER_WIDTH_IN_NODES, QUARTER_DEPTH_IN_NODES );
	
	this->regionX = this->regionY = 0;
	this->mapPosX = this->mapPosY = 0;
	this->willAddParty = false;
	this->bitmapIndex = -1;
	this->bitmapSurface = NULL;
}

LandGenerator::~LandGenerator() {
	delete cellular;
}


bool LandGenerator::drawNodes( Map *map, ShapePalette *shapePal ) {
//	updateStatus( _( "Loading theme" ) );
	if ( map->getPreferences()->isDebugTheme() ) shapePal->loadDebugTheme();
	else shapePal->loadTheme( "Gurrea2" );

	map->setHeightMapEnabled( true );

	// add mountains
	for ( int x = 0; x < QUARTER_WIDTH_IN_NODES; x++ ) {
		for ( int y = 0; y < QUARTER_DEPTH_IN_NODES; y++ ) {
			if ( cellular->getNode( x, y )->wall ) {
				map->setGroundHeight( mapPosX + x, mapPosY + y, Util::roll( 14.0f, 20.0f ) );
			} else if ( cellular->getNode( x, y )->water ) {
				map->setGroundHeight( mapPosX + x, mapPosY + y, -Util::roll( 14.0f, 20.0f )	 );
			} else {
				map->setGroundHeight( mapPosX + x, mapPosY + y, ground[x][y] );
			}
		}
	}
	
	// set the climate and vegetation values
	for ( int x = 0; x < QUARTER_WIDTH_IN_NODES; x++ ) {
		for ( int y = 0; y < QUARTER_DEPTH_IN_NODES; y++ ) {
			if ( !cellular->getNode( x, y )->water ) {
				map->setClimate( mapPosX + x, mapPosY + y, cellular->getNode( x, y )->climate );
				map->setVegetation( mapPosX + x, mapPosY + y, cellular->getNode( x, y )->vegetation );
			}
		}
	}
	
	// add any dungeon/cave entrances
	vector<MapPlace*> *places = shapePal->getSession()->getBoard()->getPlacesForRegion( regionX, regionY );
	for( unsigned i = 0; places && i < places->size(); i++ ) {
		MapPlace *place = places->at(i);
		map->flattenChunk( mapPosX * OUTDOORS_STEP + place->x, 
			                  mapPosY * OUTDOORS_STEP + place->y );
		map->setPosition( mapPosX * OUTDOORS_STEP + place->x, 
		                  mapPosY * OUTDOORS_STEP + place->y, 
		                  0, 
		                  shapePal->findShapeByName( "GATE_DOWN_OUTDOORS" ) );
	}
	
	// add roads
	class MyRoadWalker : public RoadWalker {
public:		
		LandGenerator *generator;
		int last_abs_x, last_abs_y;
		
		MyRoadWalker( LandGenerator *generator ) {
			this->generator = generator;
			last_abs_x = last_abs_y = -1;
		}
		
		void drawRoad( int x, int y, const char *name, float angle ) {
			int mapx = generator->getMapPositionX() * OUTDOORS_STEP + x;
			int mapy = generator->getMapPositionY() * OUTDOORS_STEP + y;
			if( mapx >= 0 && mapy >= 0 && mapx < MAP_WIDTH && mapy < MAP_DEPTH ) {
				Session::instance->getMap()->flattenChunkWalkable( mapx + MAP_UNIT / 2, mapy - MAP_UNIT / 2, 0 );
				string name_str = name;
				Session::instance->getMap()->addOutdoorTexture( mapx, mapy, name_str, angle, false, false );
			}
		}
		
		const char *roadTurn() {
			return Util::pickOne( 0, 1 ) == 0 ? "road_turn" : "road_cutoff";
		}
		
		void walk( Road *road, int rx, int ry, int x, int y, float angle ) {
			int abs_x = rx * ( MAP_WIDTH / 2 ) + x;
			int abs_y = ry * ( MAP_DEPTH / 2 ) + y;
							
			if( rx == generator->getRegionX() && ry == generator->getRegionY() ) {
				cerr << "adding road=" << road->name << " region=" << rx << "," << ry << " pos=" << x << "," << y << " angle=" << angle << endl;
				
				if( abs_x == last_abs_x ) {
					cerr << "horiz, angle=" << angle << endl;
					drawRoad( x, y, "road", 0 );
				} else if( abs_y == last_abs_y ) {
					cerr << "vert, angle=" << angle << endl;
					drawRoad( x, y, "road", 90 );					
				} else if( abs_y < last_abs_y && abs_x < last_abs_x ) {
					cerr << "1, angle=" << angle << endl;
					drawRoad( x, y, roadTurn(), 0 );
					drawRoad( x, y + MAP_UNIT, roadTurn(), 180 );
				} else if( abs_y < last_abs_y && abs_x > last_abs_x ) {
					cerr << "2, angle=" << angle << endl;
					drawRoad( x, y, roadTurn(), 90 );
					drawRoad( x, y + MAP_UNIT, roadTurn(), 270 );
				} else if( abs_y > last_abs_y && abs_x < last_abs_x ) {
					cerr << "3, angle=" << angle << endl;
					drawRoad( x, y, roadTurn(), 180 );
					drawRoad( x + MAP_UNIT, y, roadTurn(), 0 );
				} else if( abs_y > last_abs_y && abs_x > last_abs_x ) {
					cerr << "4, angle=" << angle << endl;
					drawRoad( x, y, roadTurn(), 270 );
					drawRoad( x, y - MAP_UNIT, roadTurn(), 90 );
				} else {
					cerr << "*** unknown, angle=" << angle << endl;
				}
			}
			
			last_abs_x = abs_x;
			last_abs_y = abs_y;
		}
	};
	MyRoadWalker walker( this );
			
	set<Road*> *roads = shapePal->getSession()->getBoard()->getRoadsForRegion( regionX, regionY );
	if( roads ) {
		for( set<Road*>::iterator e = roads->begin(); e != roads->end(); ++e ) {
			Road *road = *e;
			road->walk( &walker );
		}
	}
	
	// add any cities
	// todo: cities may extend into neighboring regions
	int params[8];
	vector<MapCity*> *cities = shapePal->getSession()->getBoard()->getCitiesForRegion( regionX, regionY );
	for( unsigned i = 0; cities && i < cities->size(); i++ ) {
		MapCity *city = cities->at(i);
		params[0] = city->rx;
		params[1] = city->ry;
		params[2] = mapPosX * OUTDOORS_STEP + city->x;
		params[3] = mapPosY * OUTDOORS_STEP + city->y;
		params[4] = city->w;
		params[5] = city->h;
		shapePal->getSession()->getSquirrel()->callIntArgMethod( "generate_city", 6, params );
	}	
	
	// add any generators
	vector<CreatureGenerator*> *generators = shapePal->getSession()->getBoard()->getGeneratorsForRegion( regionX, regionY );
	for( unsigned i = 0; generators && i < generators->size(); i++ ) {
		CreatureGenerator *generator = generators->at( i );
		// adjust the generator's coordinates: this assumes that each region is only generated once
		generator->x += mapPosX * OUTDOORS_STEP;
		generator->y += mapPosY * OUTDOORS_STEP;
		shapePal->getSession()->addGenerator( generator );
	}	
	
	// event handler for custom map processing
	params[0] = regionX;
	params[1] = regionY;
	params[2] = mapPosX * OUTDOORS_STEP;
	params[3] = mapPosY * OUTDOORS_STEP;
	bool ret = shapePal->getSession()->getSquirrel()->callIntArgMethod( "generate_land", 4, params );
	
	if( !ret ) {
		// add trees
		for ( int x = 0; x < QUARTER_WIDTH_IN_NODES; x++ ) {
			for ( int y = 0; y < QUARTER_DEPTH_IN_NODES; y++ ) {
				if ( !cellular->getNode( x, y )->water && 
						!map->isRoad( ( mapPosX + x ) * OUTDOORS_STEP, 
						              ( mapPosY + y ) * OUTDOORS_STEP, 
						              true )	) {
					params[4] = x * OUTDOORS_STEP;
					params[5] = y * OUTDOORS_STEP;
					params[6] = cellular->getNode( x, y )->climate;
					params[7] = cellular->getNode( x, y )->vegetation;
					shapePal->getSession()->getSquirrel()->callIntArgMethod( "generate_tree", 8, params );
				}
			}
		}
	}

	// create a set of rooms for outdoor items
	doorCount = 0;
	roomCount = 0;
	room[ roomCount ].x = mapPosX * OUTDOORS_STEP;
	room[ roomCount ].y = mapPosY * OUTDOORS_STEP;
	room[ roomCount ].w = QUARTER_WIDTH_IN_NODES * OUTDOORS_STEP;
	room[ roomCount ].h = QUARTER_DEPTH_IN_NODES * OUTDOORS_STEP;
	room[ roomCount ].valueBonus = 0;
	roomCount++;
	roomMaxWidth = 0;
	roomMaxHeight = 0;
	objectCount = 7 + ( level / 8 ) * 5;
	monsters = true;

	return true;
}

void LandGenerator::drawRoad( Road *road, Map *map, ShapePalette *shapePal ) {
	
	
}

MapRenderHelper* LandGenerator::getMapRenderHelper() {
	// we need fog
	return MapRenderHelper::helpers[ MapRenderHelper::OUTDOOR_HELPER ];
	//return MapRenderHelper::helpers[ MapRenderHelper::DEBUG_OUTDOOR_HELPER ];
}

// =====================================================
// =====================================================
// generation
//

// a 16x16 block of data (in four 8x8 sections, one per cellular section)
int data[ REGION_SIZE * REGION_SIZE ];
int climate[ REGION_SIZE * REGION_SIZE ];
int vegetation[ REGION_SIZE * REGION_SIZE ];

void printData() {
	cerr << "-----------------------------------" << endl;
	for( int y = 0; y < REGION_SIZE; y++ ) {
		for( int x = 0; x < REGION_SIZE; x++ ) {
			fprintf( stderr, " %2d", data[ y * REGION_SIZE + x ] );
		}
		fprintf( stderr, "\n" );
	}
}

void printRGB( vector<GLubyte> *image ) {
	for( int y = 0; y < REGION_SIZE; y++ ) {
		for( int x = 0; x < REGION_SIZE; x++ ) {
			int offs = y * REGION_SIZE * BYTES_PER_PIXEL + x * BYTES_PER_PIXEL;
			fprintf( stderr, " %02x%02x%02x", image->at( offs + 0 ), image->at( offs + 1 ), image->at( offs + 2 ) );
		}
		cerr << "\n";
	}
}	

void LandGenerator::loadMapGridBitmap() {
	// load the correct bitmap (if not already in memory)
	int bitmapX = regionX / REGIONS_PER_BITMAP;
	int bitmapY = regionY / REGIONS_PER_BITMAP;
	int bitmapIndex = bitmapY * BITMAPS_PER_ROW + bitmapX;
	cerr << "Needs bitmap index=" << bitmapIndex << 
		" region: " << regionX << "," << regionY << 
		" bitmap: " << bitmapX << "," << bitmapY << endl;
	
	if( this->bitmapIndex != bitmapIndex ) {
		char bitmapName[255];
		sprintf( bitmapName, "/mapgrid/world/map_%02d.png", bitmapIndex );
		cerr << "Needs bitmap " << bitmapName << endl;
		
		if( bitmapSurface ) {
			SDL_FreeSurface( bitmapSurface );
		}

		string fn = rootDir + bitmapName;
		cerr << "Loading bitmap: " << fn << endl;
		if ( !( bitmapSurface = IMG_Load( fn.c_str() ) ) ) {
			cerr << "*** Error loading map chunk (" << fn << "): " << IMG_GetError() << endl;
		}
	}
	
	loadMapGridBitmapRegion();
}

void LandGenerator::loadMapGridBitmapRegion() {
	// select the correct section of the image
	// The raw data of the source image.
	unsigned char * data = ( unsigned char * ) ( bitmapSurface->pixels );
//		for( int i = 0; i < 128 * 128 * BYTES_PER_PIXEL; i+=BYTES_PER_PIXEL ) {
//			fprintf( stderr, "%02x%02x%02x%02x,", data[i], data[i+1], data[i+2], data[i+3]);
//			if( i > 0 && i % ( 128 * BYTES_PER_PIXEL ) == 0 ) cerr << endl;
//		}
//	cerr << "bytesPerPixel: " << bitmapSurface->format->BytesPerPixel <<
//		" bitsPerPixel: " << bitmapSurface->format->BitsPerPixel <<
//		" pitch=" << bitmapSurface->pitch << endl;

	// The destination image (a single tile)
	std::vector<GLubyte> image( REGION_SIZE * REGION_SIZE * BYTES_PER_PIXEL );

	int rx = regionX % REGIONS_PER_BITMAP;
	int ry = regionY % REGIONS_PER_BITMAP;
	int count = 0;
	// where the tile starts in a line
	int offs = rx * REGION_SIZE * BYTES_PER_PIXEL;
	// where the tile ends in a line
	int rest = ( rx + 1 ) * REGION_SIZE * BYTES_PER_PIXEL;
	// Current position in the source data
	int c = offs + ( ry * REGION_SIZE * bitmapSurface->pitch );
	// the following lines extract R,G and B values from any bitmap

//	cerr << " c:" << c << " ";
	for ( int i = 0; i < REGION_SIZE * REGION_SIZE; ++i ) {

		if ( i > 0 && i % REGION_SIZE == 0 ) {
			// skip the rest of the line
			c += ( bitmapSurface->pitch - rest );
			// skip the offset (go to where the tile starts)
			c += offs;
//			cerr << endl;
//			cerr << " c:" << c << " ";
		}

		for ( int p = 0; p < BYTES_PER_PIXEL; p++ ) {
//			fprintf( stderr, "%02x", data[c] );
			image[count++] = data[c++];
		}
//		cerr << ",";
	}
//	cerr << endl << "found " << image.size() << " pixels." << endl;
	//printRGB( &image );
	
	packMapData( image );
}

void LandGenerator::packMapData( std::vector<GLubyte> &image ) {
	unsigned int r, g, b, color;

	for( int i = 0; i < (int)image.size(); i += BYTES_PER_PIXEL ) {
		r = (unsigned int)image[ i ]; g = (unsigned int)image[ i + 1 ]; b = (unsigned int)image[ i + 2 ];
		color = ( r << 16 ) + ( g << 8 ) + b ;

		int d = 0;
		
		// Check red byte (land elevation)
		switch( r ) {
		case 200:
			d |= TERRAIN_MOUNTAINS;
			break;
		case 150:
			d |= TERRAIN_HIGHLANDS;
			break;
		case 100:
			d |= TERRAIN_LOWLANDS;
			break;
		case 50:
			d |= TERRAIN_PLAINS;
			break;
		default:
			d |= TERRAIN_PLAINS;
			break;
		}

		// Black samples are always water.
		if( !color ) d = TERRAIN_WATER;
	
		data[ i / BYTES_PER_PIXEL ] = d;

		vegetation[ i / BYTES_PER_PIXEL ] = g;
		climate[ i / BYTES_PER_PIXEL ] = b;
	}

	cellular->initialize( REGION_SIZE, REGION_SIZE, data, vegetation, climate );
}

void LandGenerator::generate( Map *map, ShapePalette *shapePal ) {
	loadMapGridBitmap();
	
	cellular->generate( true, true, 4, true, false );
	cellular->makeMinSpace( 4 );
	//cellular->print();
	
	createGround();
}

void LandGenerator::createGround() {
	// create the undulating ground
	float a,b,f;
	for ( int x = 0; x < QUARTER_WIDTH_IN_NODES; x++ ) {
		for ( int y = 0; y < QUARTER_DEPTH_IN_NODES; y++ ) {
			if ( cellular->getNode( x, y )->elevated || cellular->getNode( x, y )->high ) {
				// smooth hills
				b = 2.0f;
				a = Util::roll( 2.75f, 3.0f );
				f = 8.0f;
			} else {
				// flatland with variations
				b = 1.0f;
				a = Util::roll( 0.25f, 1.0f );
				f = 40.0f / 2 + Util::roll( 0.25f, 40.0f / 2 );
			}
			
			ground[x][y] = b +
			               ( a *
			                 sin( PI / ( 180.0f / static_cast<float>( x * OUTDOORS_STEP * f ) ) ) *
			                 cos( PI / ( 180.0f / static_cast<float>( y * OUTDOORS_STEP  * f ) ) ) );
			if ( ground[x][y] < 0 ) ground[x][y] = 0;
		}
	}
		
	// add some random mountains
	for ( int x = 1; x < QUARTER_WIDTH_IN_NODES; x++ ) {
		for ( int y = 1; y < QUARTER_DEPTH_IN_NODES; y++ ) {
			if ( ( cellular->getNode( x, y )->high && Util::dice( 150 ) < 2 ) || 
					( cellular->getNode( x, y )->elevated && Util::dice( 300 ) < 2 ) ) {
				ground[x][y] = Util::roll( 14.0f, 20.0f );
				ground[x - 1][y] = Util::roll( 14.0f, 20.0f );
				ground[x][y - 1] = Util::roll( 14.0f, 20.0f );
				ground[x - 1][y - 1] = Util::roll( 14.0f, 20.0f );
			}
		}
	}
}

void LandGenerator::printMaze() {
	cellular->print(); 
}

bool LandGenerator::addParty( Map *map, ShapePalette *shapePal, bool goingUp, bool goingDown ) {
	if( willAddParty ) {
		TerrainGenerator::addParty( map, shapePal, goingUp, goingDown );
	}
	return true;
}

/// Initializes the outdoor ground textures. Takes height into account.
// todo: it's incorrect to have this function here because it operates over the entire map (not just the quarter)
// so in this function you should not reference LandGenerator members (like mapPosX, etc.)
void LandGenerator::initOutdoorsGroundTexture( Map *map ) {
	// set ground texture

	std::map<int, int> texturesUsed;
	
	int params[2];
	char answer[255];
	string name;
	int ex = MAP_TILES_X;
	int ey = MAP_TILES_Y;
	// ideally the below would be refs[ex][ey] but that won't work in C++... :-(
	GroundTexture *refs[MAP_WIDTH][MAP_DEPTH];
	bool keep[MAP_WIDTH][MAP_DEPTH][4];
	for ( int x = 0; x < ex; x += OUTDOOR_FLOOR_TEX_SIZE ) {
		for ( int y = 0; y < ey; y += OUTDOOR_FLOOR_TEX_SIZE ) {
			bool high = isRockTexture( map, x, y );
			bool low = isLakebedTexture( map, x, y );
			// if it's both high and low, make rock texture. Otherwise mountain sides will be drawn with lakebed texture.
			if( high ) {
				name = "rock";
			} else if( low ) {
				name = "lakebed";
			} else {
				params[0] = map->getClimate( x, y );
				params[1] = map->getVegetation( x, y );
				bool ret = map->getShapes()->getSession()->getSquirrel()->callIntArgStringReturnMethod( "get_ground_texture", answer, 2, params );
				name = ret ? answer : "grass";
			}
			GroundTexture *gt = map->getShapes()->getGroundTexture( name );
			if( gt ) {
				keep[x][y][Constants::NORTH] = 
					keep[x][y][Constants::EAST] = 
					keep[x][y][Constants::WEST] = 
					keep[x][y][Constants::SOUTH] = false;
				Texture tex = gt->getRandomTexture();
				for ( int xx = 0; xx < OUTDOOR_FLOOR_TEX_SIZE; xx++ ) {
					for ( int yy = 0; yy < OUTDOOR_FLOOR_TEX_SIZE; yy++ ) {
						refs[x + xx][y + yy] = gt;
						map->setGroundTex( x + xx, y + yy, tex );
					}
				}
			}
		}
	}
	
	// do all the blending for a given type of ground cover in one step
	std::map<string, vector<int>*> px;
	std::map<string, vector<int>*> py;
	for ( int x = OUTDOOR_FLOOR_TEX_SIZE; x < ex - OUTDOOR_FLOOR_TEX_SIZE; x += OUTDOOR_FLOOR_TEX_SIZE ) {
		for ( int y = OUTDOOR_FLOOR_TEX_SIZE; y < ey - OUTDOOR_FLOOR_TEX_SIZE; y += OUTDOOR_FLOOR_TEX_SIZE ) {
			string s = refs[x][y]->getName();
			vector<int> *vx;
			if( px.find( s ) == px.end() ) {
				vx = new vector<int>();
				px[ s ] = vx;
			} else {
				vx = px[ s ];
			}
			vx->push_back( x );
			
			vector<int> *vy;
			if( py.find( s ) == py.end() ) {
				vy = new vector<int>();
				py[ s ] = vy;
			} else {
				vy = py[ s ];
			}
			vy->push_back( y );
		}
	}
	
	Texture edge = map->getShapes()->getGroundTexture( "grass_edge" )->getRandomTexture(); // 1 side
	Texture corner = map->getShapes()->getGroundTexture( "grass_corner" )->getRandomTexture(); // 2 sides
	Texture tip = map->getShapes()->getGroundTexture( "grass_tip" )->getRandomTexture(); // 3 sides
	Texture hole = map->getShapes()->getGroundTexture( "grass_hole" )->getRandomTexture(); // 4 sides
	Texture empty;
	
	for ( std::map<string, vector<int>*>::iterator i = px.begin(); i != px.end(); ++i ) {
		string s = i->first;
		vector<int> *vx = px[s];
		vector<int> *vy = py[s];
		while( vx->size() > 0 ) {
			// select a location randomly to avoid predictable edges
			int index = (int)( (float)( vx->size() ) * Util::mt_rand() );
			int x = vx->at( index );
			int y = vy->at( index );
			vx->erase( vx->begin() + index );
			vy->erase( vy->begin() + index );
	
			bool w = refs[x - OUTDOOR_FLOOR_TEX_SIZE][y] == refs[x][y] ? true : false;
			bool e = refs[x + OUTDOOR_FLOOR_TEX_SIZE][y] == refs[x][y] ? true : false;
			bool s = refs[x][y + OUTDOOR_FLOOR_TEX_SIZE] == refs[x][y] ? true : false;
			bool n = refs[x][y - OUTDOOR_FLOOR_TEX_SIZE] == refs[x][y] ? true : false;
			if ( !( w && e && s && n ) ) {
				// get or create a new texture that is the blend of the four edges
				Texture tex;
				tex.createEdgeBlend( refs[x][y]->getRandomTexture(),
				                     keep[x][y][Constants::WEST] ? empty : refs[x - OUTDOOR_FLOOR_TEX_SIZE][y]->getRandomTexture(),
	                           keep[x][y][Constants::EAST] ? empty : refs[x + OUTDOOR_FLOOR_TEX_SIZE][y]->getRandomTexture(),
	                           keep[x][y][Constants::SOUTH] ? empty : refs[x][y + OUTDOOR_FLOOR_TEX_SIZE]->getRandomTexture(),
	                           keep[x][y][Constants::NORTH] ? empty : refs[x][y - OUTDOOR_FLOOR_TEX_SIZE]->getRandomTexture(),
				                     edge, corner, tip, hole );
				for ( int xx = 0; xx < OUTDOOR_FLOOR_TEX_SIZE; xx++ ) {
					for ( int yy = 0; yy < OUTDOOR_FLOOR_TEX_SIZE; yy++ ) {
						map->setGroundTex( x + xx, y + yy, tex );
					}
				}
				
				keep[x - OUTDOOR_FLOOR_TEX_SIZE][y][Constants::EAST] = 
					keep[x + OUTDOOR_FLOOR_TEX_SIZE][y][Constants::WEST] = 
					keep[x][y + OUTDOOR_FLOOR_TEX_SIZE][Constants::NORTH] = 
					keep[x][y - OUTDOOR_FLOOR_TEX_SIZE][Constants::SOUTH] = true;				
			}
		}
		delete vx;
		delete vy;
	}

	addHighVariation( map, "snow", GROUND_LAYER );
	addHighVariation( map, "snow_big", GROUND_LAYER );

}

/// Adds semi-random height variation to an outdoor map.

/// Higher parts of the map are randomly selected, their height value
/// set to z and textured with the referenced theme specific texture.

void LandGenerator::addHighVariation( Map *map, std::string ref, int z ) {
	GroundTexture *gt = map->getShapes()->getGroundTexture( ref );
	int width = gt->getWidth();
	int height = gt->getHeight();
	int outdoor_w = width / OUTDOORS_STEP;
	int outdoor_h = height / OUTDOORS_STEP;
	int ex = MAP_TILES_X;
	int ey = MAP_TILES_Y;
	for ( int x = 0; x < ex; x += outdoor_w ) {
		for ( int y = 0; y < ey; y += outdoor_h ) {
			if ( isAllHigh( map, x, y, outdoor_w, outdoor_h ) && !Util::dice( 10 ) && !map->hasOutdoorTexture( x, y, width, height ) ) {
				map->setOutdoorTexture( x * OUTDOORS_STEP, ( y + outdoor_h + 1 ) * OUTDOORS_STEP,
				                        0, 0, ref, Util::dice( 4 ) * 90.0f, false, false, z );
			}
		}
	}
}

/// Should a rock texture be applied to this map position due to its height?

bool LandGenerator::isRockTexture( Map *map, int x, int y ) {
	bool high = false;
	for ( int xx = 0; xx < OUTDOOR_FLOOR_TEX_SIZE + 1; xx++ ) {
		for ( int yy = 0; yy < OUTDOOR_FLOOR_TEX_SIZE + 1; yy++ ) {
			if ( map->getGroundHeight( x + xx, y + yy ) > 10 ) {
				high = true;
				break;
			}
		}
	}
	return high;
}


bool LandGenerator::isLakebedTexture( Map *map, int x, int y ) {
	bool low = false;
	for ( int xx = 0; xx < OUTDOOR_FLOOR_TEX_SIZE + 1; xx++ ) {
		for ( int yy = 0; yy < OUTDOOR_FLOOR_TEX_SIZE + 1; yy++ ) {
			if ( map->getGroundHeight( x + xx, y + yy ) < -10 ) {
				low = true;
				break;
			}
		}
	}
	return low;
}

/// Are all map tiles in the specified area high above "sea level"?

bool LandGenerator::isAllHigh( Map *map, int x, int y, int w, int h ) {
	bool high = true;
	for ( int xx = 0; xx < w + 1; xx++ ) {
		for ( int yy = 0; yy < h + 1; yy++ ) {
			if ( map->getGroundHeight( x + xx, y + yy ) < 10 ) {
				high = false;
				break;
			}
		}
	}
	return high;
}
