/***************************************************************************
                          shapepalette.h  -  description
                             -------------------
    begin                : Sat Jun 14 2003
    copyright            : (C) 2003 by Gabor Torok
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

#ifndef SHAPEPALETTE_H
#define SHAPEPALETTE_H

#include <string.h>
#include "constants.h"
#include "shape.h"
#include "glshape.h"
#include "gltorch.h"
#include "gllocator.h"
#include "debugshape.h"
#include "md2shape.h"

/**
  *@author Gabor Torok
  */

class GLShape;
class GLTorch;
  
class ShapePalette {
private:
  GLShape *shapes[256];
  GLShape *creature_shapes[256];
  GLShape *item_shapes[256];  
  GLuint display_list, creature_display_list_start, item_display_list_start, max_display_list;
  GLuint gui_texture;
  GLuint textures[100]; // store textures
  GLShape *shapeNameArray[256];

  GLuint ns_tex[3];
  GLuint ew_tex[3];
  GLuint wood_tex[3];
  GLuint floor_tex[3], floor2_tex[3];;
  GLuint notex[3];
  GLuint lamptex[3];
  GLuint doorNStex[3];
  GLuint doorEWtex[3];      
  GLuint shelftex[3];

  // how big to make the walls
  const static Sint16 unitSide = 12;
  const static Sint16 unitOffset = 2;
  const static Sint16 wallHeight = 10;

  SDL_Surface *portraits[20];
  int portraitCount;

  void initShapes();
  void loadTextures();
  void loadPortraits();

  // shape descriptions
  static char *wallDescription[], *doorDescription[], *doorFrameDescription[], *torchDescription[], *bookshelfDescription[], *chestDescription[];
  static int wallDescriptionCount, doorDescriptionCount, doorFrameDescriptionCount, torchDescriptionCount, bookshelfDescriptionCount, chestDescriptionCount;

  static ShapePalette *instance;

public: 
	ShapePalette();
	~ShapePalette();

  // singleton
  inline static ShapePalette *getInstance() { return instance; }

  // cursor
  SDL_Surface *cursor;
  GLubyte *cursorImage;

  SDL_Surface *logo;
  GLubyte *logoImage;   

  SDL_Surface *scourge;
  GLubyte *scourgeImage;

  GLuint cloud, candle;
 
  enum { 
	EW_WALL_INDEX=1,
	EW_WALL_EXTRA_INDEX,
	EW_WALL_TWO_EXTRAS_INDEX,  		
	NS_WALL_INDEX,
	NS_WALL_EXTRA_INDEX,
	NS_WALL_TWO_EXTRAS_INDEX,  				
	CORNER_INDEX,
	DOOR_SIDE_INDEX,		
	EW_DOOR_INDEX,
	EW_DOOR_TOP_INDEX,
	NS_DOOR_INDEX,
	NS_DOOR_TOP_INDEX,
	FLOOR_TILE_INDEX,
	ROOM_FLOOR_TILE_INDEX,
	LAMP_INDEX,
	LAMP_BASE_INDEX,  
	DEBUG_INDEX, 
	LOCATOR_INDEX, 
	BOOKSHELF_INDEX,
	CHEST_INDEX,
	
	// must be the last one
	SHAPE_INDEX_COUNT
  };
  
  const static Uint8 FIGHTER_INDEX = 0;
  const static Uint8 ROGUE_INDEX = 1;
  const static Uint8 CLERIC_INDEX = 2;
  const static Uint8 WIZARD_INDEX = 3;

  // increment me if you add a new creature shape
  const static Uint8 CREATURE_INDEX_COUNT = 3;
  

  const static Uint8 SWORD_INDEX = 0;
  const static Uint8 AXE_INDEX = 1;
  const static Uint8 SHIELD_INDEX = 2;      

  // increment me if you add a new creature shape
  const static Uint8 ITEM_INDEX_COUNT = 3;
    
  
  inline GLShape *getShape(int index) { return shapes[index]; }
  inline GLShape *getCreatureShape(int index) { return creature_shapes[index]; }
  inline GLShape *getItemShape(int index) { return item_shapes[index]; }

  inline bool isItem(GLShape *shape) { return shape->getDisplayList() >= item_display_list_start; }
  
  inline Sint16 getUnitSide() { return unitSide; }
  inline Sint16 getUnitOffset() { return unitOffset; }
  inline Sint16 getWallHeight() { return wallHeight; }

  inline GLuint getGuiTexture() { return gui_texture; }

  // for these, call glRasterPos, first.
  void drawPortrait(int index);

protected:
  GLuint loadGLTextures(char *fileName);
  void setupAlphaBlendedBMP(char *filename, SDL_Surface **surface, GLubyte **image);
  void swap(unsigned char & a, unsigned char & b);
};

#endif
