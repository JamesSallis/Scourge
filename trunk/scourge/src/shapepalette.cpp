/***************************************************************************
                          shapepalette.cpp  -  description
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

#include "shapepalette.h"

ShapePalette *ShapePalette::instance = NULL;

char *ShapePalette::wallDescription[] = {
  "Pale weeds grow over these ancient walls",
  "The walls crumble with age, yet appear sturdy",
  "Dark limestone walls mark these corridors"
};
int ShapePalette::wallDescriptionCount = 3;

char *ShapePalette::doorDescription[] = {
  "It appears to be a thick wooden door",
  "A dungeon door made of wood",
  "The aged oak door is scarred by knife points and torch flame"
};
int ShapePalette::doorDescriptionCount = 3;

char *ShapePalette::doorFrameDescription[] = {
  "The door frames seem to be made of some hard wood",
  "The wood around the doors is reinforced by metal nails"
};
int ShapePalette::doorFrameDescriptionCount = 2;

char *ShapePalette::torchDescription[] = {
  "A torch blazes nearby",
  "The light is firmly attached to the wall, you cannot get it",
  "It is a burning torch"
};
int ShapePalette::torchDescriptionCount = 3;



ShapePalette::ShapePalette(){

  // set up the cursor
  setupAlphaBlendedBMP("data/cursor.bmp", &cursor, &cursorImage);

  // set up the logo
  setupAlphaBlendedBMP("data/logo.bmp", &logo, &logoImage);

  // set up the scourge
  setupAlphaBlendedBMP("data/scourge.bmp", &scourge, &scourgeImage);
  
  // load portraits
  loadPortraits();
  
  // load textures
  loadTextures();

  // Create the display lists
  display_list = glGenLists((SHAPE_INDEX_COUNT + CREATURE_INDEX_COUNT) * 3);
 
 	// init the shapes
  initShapes();

  if(!instance) instance = this;
}

ShapePalette::~ShapePalette(){
}

void ShapePalette::initShapes() {
	Uint32 color = 0xac8060ff;
  bool debug = false;
  int count = 0;
  shapes[EW_WALL_INDEX] =
    new GLShape(ew_tex,
                unitOffset, unitSide - (unitOffset * 2), wallHeight,
                "EAST WALL", wallDescription, wallDescriptionCount,
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), EW_WALL_INDEX);
  shapes[EW_WALL_EXTRA_INDEX] =
		new GLShape(ew_tex,
				  unitOffset, unitSide - unitOffset, wallHeight,
				  "EAST WALL EXTRA", wallDescription, wallDescriptionCount,
				  (debug ? 0xff0000ff : color),
				  display_list + (count++ * 3), EW_WALL_EXTRA_INDEX);
  shapes[EW_WALL_TWO_EXTRAS_INDEX] =
		new GLShape(ew_tex,
				  unitOffset, unitSide, wallHeight,
				  "EAST WALL TWO EXTRAS", wallDescription, wallDescriptionCount,
				  (debug ? 0xff0000ff : color),
				  display_list + (count++ * 3), EW_WALL_TWO_EXTRAS_INDEX);
  
  shapes[NS_WALL_INDEX] =
		new GLShape(ns_tex,
				  unitSide - (unitOffset * 2), unitOffset, wallHeight,
				  "SOUTH WALL", wallDescription, wallDescriptionCount,
				  (debug ? 0xff0000ff : color),
				  display_list + (count++ * 3), NS_WALL_INDEX);  
  shapes[NS_WALL_EXTRA_INDEX] =
    new GLShape(ns_tex,
                unitSide - unitOffset, unitOffset, wallHeight,
                "SOUTH WALL EXTRA", wallDescription, wallDescriptionCount,
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), NS_WALL_EXTRA_INDEX);
  shapes[NS_WALL_TWO_EXTRAS_INDEX] =
    new GLShape(ns_tex,
                unitSide, unitOffset, wallHeight,
                "SOUTH WALL TWO EXTRAS", wallDescription, wallDescriptionCount,
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), NS_WALL_TWO_EXTRAS_INDEX);

  //FRONT_SIDE = 0;
  //static const int TOP_SIDE = 1;
  //static const int LEFT_RIGHT_SIDE

  // make the walls seethru
  ((GLShape*)shapes[EW_WALL_INDEX])->setSkipSide( 1 << GLShape::FRONT_SIDE );
  ((GLShape*)shapes[EW_WALL_EXTRA_INDEX])->setSkipSide( 1 << GLShape::FRONT_SIDE );  
  ((GLShape*)shapes[EW_WALL_TWO_EXTRAS_INDEX])->setSkipSide( 1 << GLShape::FRONT_SIDE );  
  ((GLShape*)shapes[NS_WALL_INDEX])->setSkipSide( 1 << GLShape::LEFT_RIGHT_SIDE );
  ((GLShape*)shapes[NS_WALL_EXTRA_INDEX])->setSkipSide( 1 << GLShape::LEFT_RIGHT_SIDE );
  ((GLShape*)shapes[NS_WALL_TWO_EXTRAS_INDEX])->setSkipSide( 1 << GLShape::LEFT_RIGHT_SIDE );

  shapes[EW_WALL_INDEX]->setStencil(true);
  shapes[EW_WALL_EXTRA_INDEX]->setStencil(true);
  shapes[EW_WALL_TWO_EXTRAS_INDEX]->setStencil(true);
  shapes[NS_WALL_INDEX]->setStencil(true);
  shapes[NS_WALL_EXTRA_INDEX]->setStencil(true);
  shapes[NS_WALL_TWO_EXTRAS_INDEX]->setStencil(true);


  // corners
  shapes[CORNER_INDEX] =
    new GLShape(wood_tex,
                unitOffset, unitOffset, wallHeight,
                "CORNER",
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), CORNER_INDEX);

	// ew door
  Uint32 doorColor = 0xaa6633ff;
	int doorSize = unitSide - unitOffset * 2 - 4;
  shapes[EW_DOOR_INDEX] =
    new GLShape(doorEWtex,
                1, doorSize, wallHeight - 2,
                "EW DOOR", doorDescription, doorDescriptionCount,
                (debug ? 0xff0000ff : doorColor),
                display_list + (count++ * 3), EW_DOOR_INDEX);
	shapes[DOOR_SIDE_INDEX] =
    new GLShape(wood_tex,
                unitOffset, 2, wallHeight - 2,
                "CORNER SIDE", doorFrameDescription, doorFrameDescriptionCount,
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), DOOR_SIDE_INDEX);
	shapes[EW_DOOR_TOP_INDEX] =
    new GLShape(wood_tex,
                2, unitSide - unitOffset * 2, 2,
                "EW DOOR TOP", doorFrameDescription, doorFrameDescriptionCount,
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), EW_DOOR_TOP_INDEX);

	// ns door
  shapes[NS_DOOR_INDEX] =
    new GLShape(doorNStex,
                doorSize, 1, wallHeight - 2,
                "NS DOOR", doorDescription, doorDescriptionCount,
                (debug ? 0xff0000ff : doorColor),
                display_list + (count++ * 3), NS_DOOR_INDEX);
	shapes[NS_DOOR_TOP_INDEX] =
    new GLShape(wood_tex,
                unitSide - unitOffset * 2, 2, 2,
                "NS DOOR TOP", doorFrameDescription, doorFrameDescriptionCount,
                (debug ? 0xff0000ff : color),
                display_list + (count++ * 3), NS_DOOR_TOP_INDEX);

  // floor tile 1
	shapes[FLOOR_TILE_INDEX] =
    new GLShape(floor2_tex,
                unitSide, unitSide, 0,
                "FLOOR TILE",
                (debug ? 0xff0000ff : 0x806040ff),
                display_list + (count++ * 3), FLOOR_TILE_INDEX);
	shapes[ROOM_FLOOR_TILE_INDEX] =
    new GLShape(floor_tex,
                unitSide, unitSide, 0,
                "ROOM FLOOR TILE",
                (debug ? 0xff0000ff : 0xa08040ff),
                display_list + (count++ * 3), ROOM_FLOOR_TILE_INDEX);

	shapes[LAMP_INDEX] =
    new GLTorch(notex, textures[9],
                1, 1, 2,
                "LAMP", torchDescription, torchDescriptionCount,
                (debug ? 0xff0000ff : 0xf0f0ffff),
                display_list + (count++ * 3),
                LAMP_INDEX);
	shapes[LAMP_BASE_INDEX] =
    new GLShape(wood_tex, 
                1, 1, 2,
                "LAMP", torchDescription, torchDescriptionCount,
                (debug ? 0xff0000ff : 0xf0f0ffff),
                display_list + (count++ * 3),
                LAMP_BASE_INDEX);                

	shapes[DEBUG_INDEX] =
    new DebugShape(wood_tex, 
                2, 1, 2,
                "DEBUG", torchDescription, torchDescriptionCount,
                (debug ? 0xff0000ff : 0xf0f0ffff),
                display_list + (count++ * 3),
                DEBUG_INDEX);                

	shapes[LOCATOR_INDEX] =
        new GLLocator(notex, 
                3, 3, 1,
                "LOCATOR", torchDescription, torchDescriptionCount,
                (debug ? 0xff0000ff : 0xf0f0ffff),
                display_list + (count++ * 3),
                LOCATOR_INDEX);


  // creatures
  creature_display_list_start = display_list + (count * 3);                

  // creatures
  creature_shapes[FIGHTER_INDEX] =
    new MD2Shape("data/models/m2.md2", "data/models/m2.bmp", 2.0f,
                 notex,
                 3, 3, 6,
                 "FIGHTER",
                 (debug ? 0xff0000ff : 0xf0f0ffff),
                 display_list + (count++ * 3),
                 FIGHTER_INDEX);
  
  creature_shapes[ROGUE_INDEX] =
    new MD2Shape("data/models/m1.md2", "data/models/m1.bmp", 2.0f,
                 notex,
                 3, 3, 6,
                 "ROGUE",
                 (debug ? 0xff0000ff : 0xf0f0ffff),
                 display_list + (count++ * 3),
                 ROGUE_INDEX);
  creature_shapes[CLERIC_INDEX] =
    new MD2Shape("data/models/m3.md2", "data/models/m3.bmp", 2.5f,
                 notex,
                 3, 3, 6,
                 "CLERIC",
                 (debug ? 0xff0000ff : 0xf0f0ffff),
                 display_list + (count++ * 3),
                 CLERIC_INDEX);  
  creature_shapes[WIZARD_INDEX] =
    new MD2Shape("data/models/m4.md2", "data/models/m4.bmp", 2.5f,
                 notex,
                 3, 3, 6,
                 "WIZARD",
                 (debug ? 0xff0000ff : 0xf0f0ffff),
                 display_list + (count++ * 3),
                 WIZARD_INDEX);  
  
  // items
  item_display_list_start = display_list + (count * 3);                                

	item_shapes[SWORD_INDEX] =
    new GLShape(wood_tex,
                2, 2, 1,
                "SWORD",
                (debug ? 0xff0000ff : 0xf0f0ffff),
                display_list + (count++ * 3),
                SWORD_INDEX);
	item_shapes[BOOKSHELF_INDEX] =
    new GLShape(shelftex,
                2, 5, 7,
                "BOOKSHELF", 
                0x0000ffff,
                display_list + (count++ * 3), BOOKSHELF_INDEX);
	item_shapes[CHEST_INDEX] =
    new GLShape(chesttex,
                2, 3, 2,
                "CHEST", 
                0xffaa80ff,
                display_list + (count++ * 3), CHEST_INDEX);
	item_shapes[BOOKSHELF2_INDEX] =
    new GLShape(shelftex2,
                5, 2, 7,
                "BOOKSHELF", 
                0x0000ffff,
                display_list + (count++ * 3), BOOKSHELF2_INDEX);
	item_shapes[CHEST2_INDEX] =
    new GLShape(chesttex2,
                3, 2, 2,
                "CHEST", 
                0xffaa80ff,
                display_list + (count++ * 3), CHEST2_INDEX);

  
  max_display_list = display_list + (count * 3);
}

void ShapePalette::loadTextures() {
  gui_texture = loadGLTextures("data/gui.bmp");

  textures[0] = loadGLTextures("data/front.bmp");
  textures[1] = loadGLTextures("data/top.bmp");
  textures[2] = loadGLTextures("data/side.bmp");
  textures[3] = loadGLTextures("data/wood.bmp");
  textures[4] = loadGLTextures("data/floor.bmp");
  textures[5] = loadGLTextures("data/floor2.bmp");
  textures[6] = loadGLTextures("data/woodtop.bmp");
  textures[7] = loadGLTextures("data/fronttop.bmp");
  textures[8] = loadGLTextures("data/light.bmp");
  textures[9] = loadGLTextures("data/flame.bmp");
  textures[10] = loadGLTextures("data/doorNS.bmp");
  textures[11] = loadGLTextures("data/doorEW.bmp");
  textures[12] = loadGLTextures("data/bookshelf.bmp");
  textures[13] = loadGLTextures("data/chestfront.bmp");
  textures[14] = loadGLTextures("data/chestside.bmp");
  textures[15] = loadGLTextures("data/chesttop.bmp");
  

  // set up the scourge
  cloud = loadGLTextures("data/cloud.bmp");
  candle = loadGLTextures("data/candle.bmp");


  ns_tex[GLShape::FRONT_SIDE] = textures[0];
  ns_tex[GLShape::TOP_SIDE] = textures[7];
  ns_tex[GLShape::LEFT_RIGHT_SIDE] = textures[2];

  ew_tex[GLShape::FRONT_SIDE] = textures[2];
  ew_tex[GLShape::TOP_SIDE] = textures[7];
  ew_tex[GLShape::LEFT_RIGHT_SIDE] = textures[0];

  wood_tex[GLShape::FRONT_SIDE] = textures[3];
  wood_tex[GLShape::TOP_SIDE] = textures[6];
  wood_tex[GLShape::LEFT_RIGHT_SIDE] = textures[3];

  floor_tex[GLShape::FRONT_SIDE] = 0; //textures[4];
  floor_tex[GLShape::TOP_SIDE] = textures[4];
  floor_tex[GLShape::LEFT_RIGHT_SIDE] = 0; //textures[4];

  floor2_tex[GLShape::FRONT_SIDE] = 0; //textures[4];
  floor2_tex[GLShape::TOP_SIDE] = textures[5];
  floor2_tex[GLShape::LEFT_RIGHT_SIDE] = 0; //textures[4];

  lamptex[GLShape::FRONT_SIDE] = textures[8];
  lamptex[GLShape::TOP_SIDE] = 0;
  lamptex[GLShape::LEFT_RIGHT_SIDE] = 0;

  doorNStex[GLShape::FRONT_SIDE] = textures[10];
  doorNStex[GLShape::TOP_SIDE] = textures[6];
  doorNStex[GLShape::LEFT_RIGHT_SIDE] = textures[6];

  doorEWtex[GLShape::FRONT_SIDE] = textures[6];
  doorEWtex[GLShape::TOP_SIDE] = textures[6];
  doorEWtex[GLShape::LEFT_RIGHT_SIDE] = textures[11];

  shelftex[GLShape::FRONT_SIDE] = textures[6];
  shelftex[GLShape::TOP_SIDE] = textures[6];
  shelftex[GLShape::LEFT_RIGHT_SIDE] = textures[12];

  chesttex[GLShape::FRONT_SIDE] = textures[14];
  chesttex[GLShape::TOP_SIDE] = textures[15];
  chesttex[GLShape::LEFT_RIGHT_SIDE] = textures[13];

  shelftex2[GLShape::FRONT_SIDE] = textures[12];
  shelftex2[GLShape::TOP_SIDE] = textures[6];
  shelftex2[GLShape::LEFT_RIGHT_SIDE] = textures[6];

  chesttex2[GLShape::FRONT_SIDE] = textures[13];
  chesttex2[GLShape::TOP_SIDE] = textures[15];
  chesttex2[GLShape::LEFT_RIGHT_SIDE] = textures[14];

  
  notex[0] = 0;
  notex[1] = 0;
  notex[2] = 0;  
}

/* function to load in bitmap as a GL texture */
GLuint ShapePalette::loadGLTextures(char *filename) {
    GLuint texture[1];

    /* Create storage space for the texture */
    SDL_Surface *TextureImage[1];

    /* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
    fprintf(stderr, "Loading texture: %s\n", filename);
    if ( ( TextureImage[0] = SDL_LoadBMP( filename ) ) ) {
      fprintf(stderr, "\tFound it.\n");

	    /* Create The Texture */
	    glGenTextures( 1, &texture[0] );

	    /* Typical Texture Generation Using Data From The Bitmap */
	    glBindTexture( GL_TEXTURE_2D, texture[0] );

	    /* Generate The Texture */
//	    glTexImage2D( GL_TEXTURE_2D, 0, 3,
//                    TextureImage[0]->w, TextureImage[0]->h, 0, GL_BGR,
//            			  GL_UNSIGNED_BYTE, TextureImage[0]->pixels );

	    /* Linear Filtering */
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
                        TextureImage[0]->w, TextureImage[0]->h,
                        GL_BGR, GL_UNSIGNED_BYTE, TextureImage[0]->pixels);
    } else {
      texture[0] = 0;
    }
    fprintf(stderr, "\tStored texture at: %u\n", texture[0]);

    /* Free up any memory we may have used */
    if ( TextureImage[0] )
	    SDL_FreeSurface( TextureImage[0] );

    return texture[0];
}

/* function to load in bitmap as a GL texture */
void ShapePalette::loadPortraits() {
    portraitCount = 0;
  
    /* Create storage space for the texture */
    SDL_Surface *image;

    char filename[100];
    
    /* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
    for(int i = 0; i < 20; i++) {
      sprintf(filename, "data/portraits/p%d.bmp", (i + 1));
      fprintf(stderr, "Loading texture: %s\n", filename);
      if ( ( image = SDL_LoadBMP( filename ) ) ) {
        fprintf(stderr, "\tFound it.\n");
        portraits[portraitCount++] = image;
      } else {
        break;
      }
    }
    fprintf(stderr, "Done. Loaded %d portraits.\n", portraitCount);
}

/**
 * Assume that glRasterPos has been called.
 */
void ShapePalette::drawPortrait(int index) {
  if(index >= 0 && index < portraitCount) {
    glPixelZoom( 1.0, -1.0 );    
    glDrawPixels(portraits[index]->w,
                 portraits[index]->h,
                 GL_BGR,
                 GL_UNSIGNED_BYTE,
                 portraits[index]->pixels);
  }
}


void ShapePalette::swap(unsigned char & a, unsigned char & b) {
    unsigned char temp;

    temp = a;
    a    = b;
    b    = temp;

    return;
}

void ShapePalette::setupAlphaBlendedBMP(char *filename, SDL_Surface **surface, GLubyte **image) {
  *image = NULL;
  if(((*surface) = SDL_LoadBMP( filename ))) {

    // Rearrange the pixelData
    int width  = (*surface) -> w;
    int height = (*surface) -> h;

	fprintf(stderr, "*** file=%s w=%d h=%d bpp=%d byte/pix=%d scanline=%d\n", 
			filename, width, height, (*surface)->format->BitsPerPixel,
			(*surface)->format->BytesPerPixel, (*surface)->pitch);

    unsigned char * data = (unsigned char *) ((*surface) -> pixels);         // the pixel data

    int BytesPerPixel = (*surface) -> format -> BytesPerPixel;
    //(*image) = new unsigned char[width * height * 4];
	(*image) = (unsigned char*)malloc(width * height * 4);
    int count = 0;
	int c = 0;
	unsigned char r,g,b;
    // the following lines extract R,G and B values from any bitmap
    for(int i = 0; i < width * height; ++i) {
	  if(i > 0 && i % width == 0) 
		c += (	(*surface)->pitch - (width * (*surface)->format->BytesPerPixel) );
	  r = data[c++];
	  g = data[c++];
	  b = data[c++];
	  
	  (*image)[count++] = r;
	  (*image)[count++] = g;
	  (*image)[count++] = b;
	  //(*image)[count++] = (GLubyte)( (float)(b + g + r) / 3.0f );
	  (*image)[count++] = (GLubyte)( (b + g + r == 0 ? 0x00 : 0xff) );
    }
  }
}

