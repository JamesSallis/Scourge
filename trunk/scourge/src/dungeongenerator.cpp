/***************************************************************************
                          dungeongenerator.cpp  -  description
                             -------------------
    begin                : Thu May 15 2003
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

#include "dungeongenerator.h"

/*
width - max 31
height - max 31

curvyness - in %, the lower the more twisty maze
sparseness - (1-5) the higher the more sparse (more empty space)
loopyness - in %, the higher the more loops in the maze

roomcount
rooom max width
room max height

object count
*/
const int DungeonGenerator::levels[][9] = {
  { 10, 20,  90, 5,  1,    2,  4,  4,      5 },  
  { 15, 15,  70, 5, 10,    3,  6,  4,     10 },
  { 15, 15,  50, 4, 20,    4,  5,  5,     15 },
  { 20, 20,  50, 6, 20,    5,  6,  5,     20 },
  { 25, 25,  40, 6, 20,    5,  6,  5,     25 },
  { 30, 25,   5, 6, 25,    6,  6,  6,     30 },        
  { 31, 31,   3, 5, 25,    6,  7,  7,     35 }
};

DungeonGenerator::DungeonGenerator(int level){

  this->level = level;

  initByLevel();  
  
  this->nodes = (Uint16**)malloc(sizeof(void*) * width);
  for(int i = 0; i < width; i++) {
    nodes[i] = (Uint16*)malloc(sizeof(void*) * height);
  }

  for(int x = 0; x < width; x++) {
    for(int y = 0; y < height; y++) {
      nodes[x][y] = UNVISITED;
    }
  }
  
  notVisitedCount = width * height;
  notVisited = (int*)new int[notVisitedCount];  
  for(int i = 0; i < notVisitedCount; i++) {
    notVisited[i] = i;
  }
  visitedCount = 0;
  visited = (int*)new int[notVisitedCount];
  
  generateMaze();
//  printMaze();  

  makeSparse();
//  printMaze();

  makeLoops();
//  printMaze();

  makeRooms();
  printMaze();
}

DungeonGenerator::~DungeonGenerator(){
  for(int i = 0; i < width; i++) {
    free(nodes[i]);
  }
  free(nodes);
  delete[] notVisited;
  delete[] visited;
}

void DungeonGenerator::initByLevel() {
  if(level < 1) level = 1;
  
  this->width = levels[level - 1][dgWIDTH];
  this->height = levels[level - 1][dgHEIGHT];
  this->curvyness = levels[level - 1][dgCURVYNESS];
  this->sparseness = levels[level - 1][dgSPARSENESS];
  this->loopyness = levels[level - 1][dgLOOPYNESS];
  this->roomCount = levels[level - 1][dgROOMCOUNT];
  this->roomMaxWidth = levels[level - 1][dgROOMMAXWIDTH];
  this->roomMaxHeight = levels[level - 1][dgROOMMAXHEIGHT];
  this->objectCount = levels[level - 1][dgOBJECTCOUNT];
}

void DungeonGenerator::makeRooms() {
  int rw, rh, px, py;
  int best, score;

  for(int i = 0; i < roomCount; i++) {
    // create a room
    rw = (int) ((double)(roomMaxWidth / 2) * rand()/RAND_MAX) + (roomMaxWidth / 2);
    rh = (int) ((double)(roomMaxHeight / 2) * rand()/RAND_MAX) + (roomMaxHeight / 2);
    best = -1;
    px = py = -1;
    // find best place for this room
    if(i % 2) {
      for(int x = 0; x < width - rw; x++) {
        for(int y = 0; y < height - rh; y++) {
          score = getScore(x, y, rw, rh);
          if(score > 0 && (best == -1 || score < best)) {
            best = score;
            px = x;
            py = y;
          }
        }
      }
    } else {
      for(int x = width - rw - 1; x >= 0; x--) {
        for(int y = height - rh - 1; y >= 0; y--) {
          score = getScore(x, y, rw, rh);
          if(score > 0 && (best == -1 || score < best)) {
            best = score;
            px = x;
            py = y;
          }
        }        
      }
    }
    // set the room
    if(px > -1 && py > -1) {

	  // save the room info
	  room[i].x = px;
	  room[i].y = py;
	  room[i].w = rw;
	  room[i].h = rh;

      for(int x = px; x < px + rw; x++) {
        for(int y = py; y < py + rh; y++) {
          nodes[x][y] = ROOM + N_PASS + S_PASS + E_PASS + W_PASS;

          // 1. connect the room to the passage
          // 2. put in some doors: after each door, the chance of there being
          //    another door decreases.
          // 3. put in the walls
          if(x == px) {
            if(x > 0 && nodes[x - 1][y] != UNVISITED) {
              nodes[x - 1][y] |= E_PASS;
            } else {
              nodes[x][y] -= W_PASS;
            }
          }
          if(x == px + rw - 1) {
            if(x < width - 1 && nodes[x + 1][y] != UNVISITED) {
              nodes[x + 1][y] |= W_PASS;
            } else {
              nodes[x][y] -= E_PASS;
            }
          }
          if(y == py) {
            if(y > 0 && nodes[x][y - 1] != UNVISITED) {
              nodes[x][y - 1] |= S_PASS;
            } else {
              nodes[x][y] -= N_PASS;
            }
          }
          if(y == py + rh - 1) {
            if(y < height - 1 && nodes[x][y + 1] != UNVISITED) {
              nodes[x][y + 1] |= N_PASS;
            } else {
              nodes[x][y] -= S_PASS;
            }
          }
        }
      }

      // add doors
      for(int x = px; x < px + rw; x++) {
        for(int y = py; y < py + rh; y++) {
          if(x == px) {
            if(x > 0 && nodes[x - 1][y] != UNVISITED && nodes[x - 1][y] < ROOM) {
							nodes[x][y] |= W_DOOR;
            }
          }
          if(x == px + rw - 1) {
            if(x < width - 1 && nodes[x + 1][y] != UNVISITED && nodes[x + 1][y] < ROOM) {
							nodes[x][y] |= E_DOOR;
            }
          }
          if(y == py) {
            if(y > 0 && nodes[x][y - 1] != UNVISITED && nodes[x][y - 1] < ROOM) {
							nodes[x][y] |= N_DOOR;
            }
          }
          if(y == py + rh - 1) {
            if(y < height - 1 && nodes[x][y + 1] != UNVISITED && nodes[x][y + 1] < ROOM) {
							nodes[x][y] |= S_DOOR;
            }
          }
          
        }
      }
    }
    
  }
}

int DungeonGenerator::getScore(int px, int py, int rw, int rh) {
  int score = 0;
  for(int x = px; x < px + rw; x++) {
    for(int y = py; y < py + rh; y++) {
      if(nodes[x][y] == UNVISITED) {
        if(x < width - 1 && nodes[x + 1][y] != UNVISITED) score++;
        if(x > 0 && nodes[x - 1][y] != UNVISITED) score++;
        if(y < height - 1 && nodes[x][y + 1] != UNVISITED) score++;
        if(y > 0 && nodes[x][y - 1] != UNVISITED) score++;

        if(x < width - 1 && nodes[x + 1][y] >= ROOM) score+=100;
        if(x > 0 && nodes[x - 1][y] >= ROOM) score+=100;
        if(y < height - 1 && nodes[x][y + 1] >= ROOM) score+=100;
        if(y > 0 && nodes[x][y - 1] >= ROOM) score+=100;        
      } else if(nodes[x][y] >= ROOM) {
          score += 100;
      } else {
          score += 3;
      }
    }
  }
  return score;
}

void DungeonGenerator::makeLoops() {
  for(int i = 0; i < sparseness; i++) {
    for(int x = 0; x < width; x++) {
      for(int y = 0; y < height; y++) {        
        switch(nodes[x][y]) {
          case N_PASS:
          case S_PASS:
          case E_PASS:
          case W_PASS:
            if((int) (100.0 * rand()/RAND_MAX) <= loopyness)
            generatePassage(x, y, false);
            break;
          default:
            break;
        }
      }
    }
  } 
}

void DungeonGenerator::makeSparse() {
  for(int i = 0; i < sparseness; i++) {
    for(int x = 0; x < width; x++) {
      for(int y = 0; y < height; y++) {
        switch(nodes[x][y]) {
          case N_PASS:
            nodes[x][y] = UNVISITED;
            if(y > 0) nodes[x][y - 1] -= S_PASS;
            break;
          case S_PASS:
            nodes[x][y] = UNVISITED;
            if(y < height - 1) nodes[x][y + 1] -= N_PASS;
            break;
          case E_PASS:
            nodes[x][y] = UNVISITED;
            if(x < width - 1) nodes[x + 1][y] -= W_PASS;
            break;
          case W_PASS:
            nodes[x][y] = UNVISITED;
            if(x > 0) nodes[x - 1][y] -= E_PASS;
            break;
          default:
            break;
        }
      }
    }
  }
}

void DungeonGenerator::generateMaze() {
  int x, y;

  //fprintf(stderr, "Starting maze w=%d h=%d\n", width, height);
  nextNotVisited(&x, &y);
  while(notVisitedCount > 0) {

    // draw the passage
    generatePassage(x, y, true);

    // select a starting point
    nextVisited(&x, &y);
    if(x == -1) break;
  }
}

void DungeonGenerator::generatePassage(const int sx, const int sy, const bool stopAtVisited) {
  //char line[80];

  int x = sx;
  int y = sy;
  
  //fprintf(stderr, "\tnotVisitedCount=%d x=%d y=%d\n", notVisitedCount, x, y);
  int nx = x;
  int ny = y;

  int dir = initDirections();
  // keep going while you can
  int stepCount = 0;
  bool reachedVisited = false;
  bool inMap = false;
  while(true) {
    // take a step in the selected direction
    switch(dir) {
      case DIR_N: ny--; break;
      case DIR_S: ny++; break;
      case DIR_W: nx--; break;
      case DIR_E: nx++; break;
      default: fprintf(stderr, "ERROR: unknown direction selected: %d\n", dir);
    }

    // off the map or location already visited
    inMap = (nx >= 0 && nx < width && ny >= 0 && ny < height);

    if(!inMap ||
       nodes[nx][ny] != UNVISITED ||
       stepCount++ >= width / 2 ||
       (curvyness > 1 && curvyness < 100 &&
       (int) ((double)curvyness * rand()/RAND_MAX) == 0)) {

      if(!stopAtVisited && inMap && nodes[nx][ny] != UNVISITED) {
        reachedVisited = true;
      } else {
        // step back
        nx = x;
        ny = y;
        // pick another direction
        dir = nextDirection();
        if(dir == -1) {
          break;
        }
        stepCount = 0;        
        continue;
      }
    }

    // connect to the previous cell
    switch(dir) {
      case DIR_N:
        nodes[x][y] |= N_PASS;
        if(inMap) nodes[nx][ny] |= S_PASS;
        break;
      case DIR_S:
        nodes[x][y] |= S_PASS;
        if(inMap) nodes[nx][ny] |= N_PASS;
        break;
      case DIR_W:
        nodes[x][y] |= W_PASS;
        if(inMap) nodes[nx][ny] |= E_PASS;
        break;
      case DIR_E:
        nodes[x][y] |= E_PASS;
        if(inMap) nodes[nx][ny] |= W_PASS;
        break;
    }

    if(reachedVisited) {
      reachedVisited = false;
      // step back
      nx = x;
      ny = y;
      // pick another direction
      dir = nextDirection();
      if(dir == -1) {
        break;
      }
      stepCount = 0;      
      continue;
    }
    

    // mark the cell visited
    markVisited(nx, ny);

    // save last position
    x = nx;
    y = ny;

    // debug
    //printMaze();
    //gets(line);
  }
}

void DungeonGenerator::markVisited(int x, int y) {
  // add to visited
  int n = (y * width) + x;
  // has it already been visited?
  for(int i = 0; i < visitedCount; i++) {
    if(visited[i] == n) return;
  }
  visited[visitedCount++] = n;
  // remove from not visited
  for(int i = 0; i < notVisitedCount; i++) {
    if(notVisited[i] == n) {
      notVisitedCount--;
      for(int t = i; t < notVisitedCount; t++) {
        notVisited[t] = notVisited[t + 1];
      }
      return;
    }
  }
}

bool DungeonGenerator::isVisited(int x, int y) {
  int n = (y * width) + x;
  for(int i = 0; i < visitedCount; i++) {
    if(visited[i] == n) return true;
  }
  return false;
}

void DungeonGenerator::nextNotVisited(int *x, int *y) {
  // are there no more unvisited locations?
  if(notVisitedCount <= 0) {
    *x = *y = -1;
    return;
  }
  // get a random location
  int index = (int) ((double)notVisitedCount * rand()/RAND_MAX);
  int n = notVisited[index];
  // remove from visited areas
  notVisitedCount--;
  for(int i = index; i < notVisitedCount; i++) {
    notVisited[i] = notVisited[i + 1];
  }
  // break up into x,y coordinates
  *y = n / width;
  *x = n % width;
  // add it to visited
  visited[visitedCount++] = n;
}

void DungeonGenerator::nextVisited(int *x, int *y) {
  // are there no visited locations?
  if(visitedCount <= 0) {
    *x = *y = -1;
    return;
  }
  // get a random location
  int index = (int) ((double)visitedCount * rand()/RAND_MAX);
  int n = visited[index];
  // break up into x,y coordinates
  *y = n / width;
  *x = n % width;
}

int DungeonGenerator::initDirections() {
  // init all available directions
  dirCount = DIR_COUNT;
  for(int i = 0; i < dirCount; i++) {
    dirs[i] = i;
  }
  return dirs[(int) ((double)dirCount * rand()/RAND_MAX)];
}

int DungeonGenerator::nextDirection() {
  if(dirCount <= 0) return -1;
  int index = (int) ((double)dirCount * rand()/RAND_MAX);
  int dir = dirs[index];
  dirCount--;
  for(int i = index; i < dirCount; i++) {
    dirs[i] = dirs[i + 1];
  }
  return dir;
}

void DungeonGenerator::printMaze() {
  printf("---------------------------------------\n");
  int c = 0;
  for(int y = 0; y < height; y++) {    
    for(int i = 0; i < 3; i++) {
      for(int x = 0; x < width; x++) {

          switch(i) {
            case 0: // top row
              if((nodes[x][y] & N_PASS)) {
                printf(" | ");
              } else {
                printf("   ");
              }
              break;                   
            case 1:
              if((nodes[x][y] & W_PASS)) {
                printf("-");              
              } else {
                printf(" ");
              }
              if(nodes[x][y] == UNVISITED)
                printf(" ");
              else if(nodes[x][y] & ROOM)
                printf("*");
              else
                printf("O");          
              if((nodes[x][y] & E_PASS)) {
                printf("-");
              } else {
                printf(" ");
              }
              break;
            case 2: // bottom row
              if((nodes[x][y] & S_PASS)) {
                printf(" | ");
              } else {
                printf("   ");              
              }
              break;
          }        
        c++;
      }
      printf("\n");
    }
    c++;    
  }
  printf("---------------------------------------\n");
}

void DungeonGenerator::toMap(Map *map, Sint16 *startx, Sint16 *starty, ShapePalette *shapePal) {	 
  // add shapes to map
  Sint16 mapx, mapy;
  for(Sint16 x = 0; x < width; x++) {    
	for(Sint16 y = 0; y < height; y++) {
				
	  mapx = x * unitSide + offset;
	  mapy = y * unitSide + offset;
	  if(nodes[x][y] != UNVISITED) {
					 
		if(nodes[x][y] >= ROOM) {
		  map->setFloorPosition(mapx, mapy + unitSide, 
								shapePal->getShape(ShapePalette::ROOM_FLOOR_TILE_INDEX));
		} else {
		  map->setFloorPosition(mapx, mapy + unitSide, 
								shapePal->getShape(ShapePalette::FLOOR_TILE_INDEX));
		}
					 
		// init the free space
		if(nodes[x][y] & E_DOOR) {
		  drawDoor(map, shapePal, mapx, mapy, E_DOOR);
		} else if(nodes[x][y] & W_DOOR) {
		  drawDoor(map, shapePal, mapx, mapy, W_DOOR);
		} else if(nodes[x][y] & N_DOOR) {
		  drawDoor(map, shapePal, mapx, mapy, N_DOOR);
		} else if(nodes[x][y] & S_DOOR) {
		  drawDoor(map, shapePal, mapx, mapy, S_DOOR);
		}

		// random doors
		if((nodes[x][y] & W_PASS) &&
		   !(nodes[x][y] & N_PASS) &&
		   !(nodes[x][y] & S_PASS)) {
		  if((int)(100.0 * rand()/RAND_MAX) <= randomDoors)
			drawDoor(map, shapePal, mapx, mapy, W_DOOR);
		}
		if((nodes[x][y] & E_PASS) &&
		   !(nodes[x][y] & N_PASS) &&
		   !(nodes[x][y] & S_PASS)) {
		  if((int)(100.0 * rand()/RAND_MAX) <= randomDoors)
			drawDoor(map, shapePal, mapx, mapy, E_DOOR);
		}
		if((nodes[x][y] & S_PASS) &&
		   !(nodes[x][y] & W_PASS) &&
		   !(nodes[x][y] & E_PASS)) {
		  if((int)(100.0 * rand()/RAND_MAX) <= randomDoors)
			drawDoor(map, shapePal, mapx, mapy, S_DOOR);
		}
		if((nodes[x][y] & N_PASS) &&
		   !(nodes[x][y] & W_PASS) &&
		   !(nodes[x][y] & E_PASS)) {
		  if((int)(100.0 * rand()/RAND_MAX) <= randomDoors)
			drawDoor(map, shapePal, mapx, mapy, N_DOOR);
		}
					 
		if(!(nodes[x][y] & W_PASS)) {
		  if(nodes[x][y] & N_PASS && nodes[x][y] & S_PASS) {
			map->setPosition(mapx, mapy + unitSide, 
							 0, shapePal->getShape(ShapePalette::EW_WALL_TWO_EXTRAS_INDEX));								
		  } else if(nodes[x][y] & N_PASS) {
			map->setPosition(mapx, mapy + unitSide - unitOffset, 
							 0, shapePal->getShape(ShapePalette::EW_WALL_EXTRA_INDEX));
		  } else if(nodes[x][y] & S_PASS) {
			map->setPosition(mapx, mapy + unitSide, 
							 0, shapePal->getShape(ShapePalette::EW_WALL_EXTRA_INDEX));
		  } else {
			map->setPosition(mapx, mapy + unitSide - unitOffset, 
							 0, shapePal->getShape(ShapePalette::EW_WALL_INDEX));								
		  }						  
		  if((int) (100.0 * rand()/RAND_MAX) <= torches) {
			map->setPosition(mapx + unitOffset, mapy + unitSide - 4, 
							 6, shapePal->getShape(ShapePalette::LAMP_INDEX));
			map->setPosition(mapx + unitOffset, mapy + unitSide - 4, 
							 4, shapePal->getShape(ShapePalette::LAMP_BASE_INDEX));
		  }          
		}
		if(!(nodes[x][y] & E_PASS)) {
		  if(nodes[x][y] & N_PASS && nodes[x][y] & S_PASS) {
			map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide, 
							 0, shapePal->getShape(ShapePalette::EW_WALL_TWO_EXTRAS_INDEX));								
		  } else if(nodes[x][y] & N_PASS) {
			map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide - unitOffset, 
							 0, shapePal->getShape(ShapePalette::EW_WALL_EXTRA_INDEX));
		  } else if(nodes[x][y] & S_PASS) {
			map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide, 
							 0, shapePal->getShape(ShapePalette::EW_WALL_EXTRA_INDEX));
		  } else {
			map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide - unitOffset, 
							 0, shapePal->getShape(ShapePalette::EW_WALL_INDEX));
		  }
		  if((int) (100.0 * rand()/RAND_MAX) <= torches) {
			map->setPosition(mapx + unitSide - (unitOffset + 1), mapy + unitSide - 4, 
							 6, shapePal->getShape(ShapePalette::LAMP_INDEX));
			map->setPosition(mapx + unitSide - (unitOffset + 1), mapy + unitSide - 4, 
							 4, shapePal->getShape(ShapePalette::LAMP_BASE_INDEX));
		  }          
		}
		if(!(nodes[x][y] & N_PASS)) {
		  if(nodes[x][y] & W_PASS && nodes[x][y] & E_PASS) {
			map->setPosition(mapx, mapy + unitOffset, 0, 
							 shapePal->getShape(ShapePalette::NS_WALL_TWO_EXTRAS_INDEX));
		  } else if(nodes[x][y] & W_PASS) {
			map->setPosition(mapx, mapy + unitOffset, 0, 
							 shapePal->getShape(ShapePalette::NS_WALL_EXTRA_INDEX));
		  } else if(nodes[x][y] & E_PASS) {
			map->setPosition(mapx + unitOffset, mapy + unitOffset, 0, 
							 shapePal->getShape(ShapePalette::NS_WALL_EXTRA_INDEX));
		  } else {
			map->setPosition(mapx + unitOffset, mapy + unitOffset, 0, 
							 shapePal->getShape(ShapePalette::NS_WALL_INDEX));
		  }
		  if((int) (100.0 * rand()/RAND_MAX) <= torches) {
			map->setPosition(mapx + 4, mapy + unitOffset + 1, 6, 
							 shapePal->getShape(ShapePalette::LAMP_INDEX));
			map->setPosition(mapx + 4, mapy + unitOffset + 1, 4, 
							 shapePal->getShape(ShapePalette::LAMP_BASE_INDEX));
		  }            
		}
		if(!(nodes[x][y] & S_PASS)) {
		  if(nodes[x][y] & W_PASS && nodes[x][y] & E_PASS) {
			map->setPosition(mapx, mapy + unitSide, 0, 
							 shapePal->getShape(ShapePalette::NS_WALL_TWO_EXTRAS_INDEX));
		  } else if(nodes[x][y] & W_PASS) {
			map->setPosition(mapx, mapy + unitSide, 0, 
							 shapePal->getShape(ShapePalette::NS_WALL_EXTRA_INDEX));
		  } else if(nodes[x][y] & E_PASS) {
			map->setPosition(mapx + unitOffset, mapy + unitSide, 0, 
							 shapePal->getShape(ShapePalette::NS_WALL_EXTRA_INDEX));
		  } else {
			map->setPosition(mapx + unitOffset, mapy + unitSide, 0, 
							 shapePal->getShape(ShapePalette::NS_WALL_INDEX));
		  }
		}
					 
		if(nodes[x][y] & N_PASS && nodes[x][y] & W_PASS) {
		  map->setPosition(mapx, mapy + unitOffset, 0, 
						   shapePal->getShape(ShapePalette::CORNER_INDEX));
		}
		if(nodes[x][y] & N_PASS && nodes[x][y] & E_PASS) {
		  map->setPosition(mapx + unitSide - unitOffset, mapy + unitOffset, 0, 
						   shapePal->getShape(ShapePalette::CORNER_INDEX));
		}
		if(nodes[x][y] & S_PASS && nodes[x][y] & W_PASS) {
		  map->setPosition(mapx, mapy + unitSide, 0, 
						   shapePal->getShape(ShapePalette::CORNER_INDEX));
		}
		if(nodes[x][y] & S_PASS && nodes[x][y] & E_PASS) {
		  map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide, 0, 
						   shapePal->getShape(ShapePalette::CORNER_INDEX));
		}
		if(!(nodes[x][y] & N_PASS) && !(nodes[x][y] & W_PASS)) {
		  map->setPosition(mapx, mapy + unitOffset, 0, 
						   shapePal->getShape(ShapePalette::CORNER_INDEX));
		}
		if(!(nodes[x][y] & N_PASS) && !(nodes[x][y] & E_PASS)) {
		  map->setPosition(mapx + unitSide - unitOffset, mapy + unitOffset, 0, 
						   shapePal->getShape(ShapePalette::CORNER_INDEX)); 
		}
		if(!(nodes[x][y] & S_PASS) && !(nodes[x][y] & W_PASS)) {
		  map->setPosition(mapx, mapy + unitSide, 0, 
						   shapePal->getShape(ShapePalette::CORNER_INDEX)); 
		}
		if(!(nodes[x][y] & S_PASS) && !(nodes[x][y] & E_PASS)) {
		  map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide, 0, 
						   shapePal->getShape(ShapePalette::CORNER_INDEX)); 
		}					 
	  }
	}
  }
	 
  // Remove 'columns' from rooms
  for(int x = 0; x < MAP_WIDTH - unitSide; x++) {
	for(int y = 0; y < MAP_DEPTH - (unitSide * 2); y++) {
				
	  if(map->getFloorPosition(x, y + unitSide) ==
		 shapePal->getShape(ShapePalette::ROOM_FLOOR_TILE_INDEX) && 
		 map->getFloorPosition(x + unitSide, y + unitSide) ==
		 shapePal->getShape(ShapePalette::ROOM_FLOOR_TILE_INDEX) &&
		 map->getFloorPosition(x, y + unitSide + unitSide) ==
		 shapePal->getShape(ShapePalette::ROOM_FLOOR_TILE_INDEX) &&         
		 map->getFloorPosition(x + unitSide, y + unitSide + unitSide) ==
		 shapePal->getShape(ShapePalette::ROOM_FLOOR_TILE_INDEX)) {
					 
		//        map->setFloorPosition(x, y + unitSide, shapePal->getShape(ShapePalette::FLOOR_TILE_INDEX));
		map->removePosition(x + unitSide - unitOffset, y + unitSide, 0);
		map->removePosition(x + unitSide - unitOffset, y + unitSide + unitOffset, 0);
		map->removePosition(x + unitSide, y + unitSide, 0);
		map->removePosition(x + unitSide, y + unitSide + unitOffset, 0);                
	  }
	}    
  }

  // add the containers
  int x, y;
  Item *item;
  for(int i = 0; i < roomCount; i++) {
  	for(int pos = unitOffset; pos < room[i].h * unitSide; pos++) {
	  item = Item::getRandomContainer();
	  if(item) {
		// WEST side
		x = (room[i].x * unitSide) + unitOffset + offset;
		y = (room[i].y * unitSide) + pos + offset;
		if(map->shapeFits(item->getShape(), x, y, 0) && 
		   !coversDoor(map, shapePal, item->getShape(), x, y)) {
		  addItem(map, item, item->getShape(), x, y);
		}
	  }
	  item = Item::getRandomContainer();
	  if(item) {
		// EAST side
		x = ((room[i].x + room[i].w - 1) * unitSide) + unitSide - (unitOffset * 2) + offset;
		if(map->shapeFits(item->getShape(), x, y, 0) && 
		   !coversDoor(map, shapePal, item->getShape(), x, y)) {
		  addItem(map, item, item->getShape(), x, y);
		}
	  }
	}
  	for(int pos = unitOffset; pos < room[i].w * unitSide; pos++) {
	  item = Item::getRandomContainerNS();
	  if(item) {
		// NORTH side
		x = (room[i].x * unitSide) + pos + offset;
		y = (room[i].y * unitSide) + (unitOffset * 2) + offset;
		if(map->shapeFits(item->getShape(), x, y, 0) && 
		   !coversDoor(map, shapePal, item->getShape(), x, y)) {
		  addItem(map, item, item->getShape(), x, y);
		}
	  }
	  item = Item::getRandomContainerNS();
	  if(item) {
		// SOUTH side
		y = ((room[i].y + room[i].h - 1) * unitSide) + unitSide - unitOffset + offset;
		if(map->shapeFits(item->getShape(), x, y, 0) && 
		   !coversDoor(map, shapePal, item->getShape(), x, y)) {
		  addItem(map, item, item->getShape(), x, y);
		}
	  }
	}
  }
  /*
  // DEBUG stuff: draws room perimeter; to debug chunking
  Shape *shape = shapePal->getShape(ShapePalette::LAMP_BASE_INDEX);
  for(int i = 0; i < roomCount; i++) {
  	for(int pos = 1; pos < room[i].h * unitSide; pos++) {
	  // WEST side
	  x = (room[i].x * unitSide) + offset;
	  y = (room[i].y * unitSide) + pos + offset;
	  addItem(map, NULL, shape, x, y);
	  // EAST side
	  x = ((room[i].x + room[i].w - 1) * unitSide) + unitSide - 1 + offset;
	  addItem(map, NULL, shape, x, y);
	}
  	for(int pos = 0; pos < room[i].w * unitSide; pos++) {
	  // NORTH side
	  x = (room[i].x * unitSide) + pos + offset;
	  y = (room[i].y * unitSide) + 1 + offset;
	  addItem(map, NULL, shape, x, y);
	  // SOUTH side
	  y = ((room[i].y + room[i].h - 1) * unitSide) + unitSide + offset;
	  addItem(map, NULL, shape, x, y);
	}
  }
  */
  
  // Collapse the free space and put objects in the available spots
  ff = (Sint16*)malloc( 2 * sizeof(Sint16) * MAP_WIDTH * MAP_DEPTH );
  if(!ff) {
	fprintf(stderr, "out of mem\n");
	exit(0);    
  }
  ffCount = 0;
  for(int fx = offset; fx < MAP_WIDTH; fx+=unitSide) {
	for(int fy = offset; fy < MAP_DEPTH; fy+=unitSide) {
	  if(map->getFloorPosition(fx, fy + unitSide)) {
		for(int ffx = 0; ffx < unitSide; ffx++) {
		  for(int ffy = unitSide; ffy > 0; ffy--) {
			if(!map->getLocation(fx + ffx, fy + ffy, 0)) {
			  *(ff + ffCount * 2) = fx + ffx;
			  *(ff + ffCount * 2 + 1) = fy + ffy;
			  ffCount++;
			}
		  }
		}
	  }
	}
  } 
  for(int i = 0; i < objectCount; i++) {
	// Get an item
	Item *item = Item::getRandomItem(0); // FIXME: pass in level 
	getRandomLocation(map, item->getShape(), &x, &y);
	addItem(map, item, item->getShape(), x, y);
  }
  free(ff);  

  // find a starting position
  *startx = room[0].x * unitSide + (room[0].w * unitSide / 2) + offset;
  *starty = room[0].y * unitSide + (room[0].h * unitSide / 2) + offset;
}

void DungeonGenerator::drawDoor(Map *map, ShapePalette *shapePal, 
								Sint16 mapx, Sint16 mapy, int doorType) {
  switch(doorType) {
  case E_DOOR:
	if(!coversDoor(map, shapePal, 
				   shapePal->getShape(ShapePalette::EW_DOOR_INDEX), 
				   mapx + unitSide - unitOffset + 1, mapy + unitSide - unitOffset - 2)) {
	  map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide - unitOffset, 
					   wallHeight - 2, shapePal->getShape(ShapePalette::EW_DOOR_TOP_INDEX));
	  map->setPosition(mapx + unitSide - unitOffset, mapy + unitOffset +  2, 
					   0, shapePal->getShape(ShapePalette::DOOR_SIDE_INDEX));
	  map->setPosition(mapx + unitSide - unitOffset + 1, mapy + unitSide - unitOffset - 2, 
					   0, shapePal->getShape(ShapePalette::EW_DOOR_INDEX));
	  map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide - unitOffset, 
					   0, shapePal->getShape(ShapePalette::DOOR_SIDE_INDEX));
	}
	break;
  case W_DOOR:
	if(!coversDoor(map, shapePal, 
				   shapePal->getShape(ShapePalette::EW_DOOR_INDEX), 
				   mapx + 1, mapy + unitSide - unitOffset - 2)) {
	  map->setPosition(mapx, mapy + unitSide - unitOffset, 
					   wallHeight - 2, shapePal->getShape(ShapePalette::EW_DOOR_TOP_INDEX));
	  map->setPosition(mapx, mapy + unitOffset +  2, 
					   0, shapePal->getShape(ShapePalette::DOOR_SIDE_INDEX));
	  map->setPosition(mapx + 1, mapy + unitSide - unitOffset - 2, 
					   0, shapePal->getShape(ShapePalette::EW_DOOR_INDEX));
	  map->setPosition(mapx, mapy + unitSide - unitOffset, 
					   0, shapePal->getShape(ShapePalette::DOOR_SIDE_INDEX));
	}
	break;
  case N_DOOR:
	if(!coversDoor(map, shapePal, 
				   shapePal->getShape(ShapePalette::NS_DOOR_INDEX), 
				   mapx + unitOffset * 2, mapy + unitOffset - 1)) {
	  map->setPosition(mapx + unitOffset, mapy + unitOffset, 
					   wallHeight - 2, shapePal->getShape(ShapePalette::NS_DOOR_TOP_INDEX));
	  map->setPosition(mapx + unitOffset, mapy + unitOffset, 
					   0, shapePal->getShape(ShapePalette::DOOR_SIDE_INDEX));
	  map->setPosition(mapx + unitOffset * 2, mapy + unitOffset - 1, 
					   0, shapePal->getShape(ShapePalette::NS_DOOR_INDEX));
	  map->setPosition(mapx + unitSide - unitOffset * 2, mapy + unitOffset, 
					   0, shapePal->getShape(ShapePalette::DOOR_SIDE_INDEX));
	}
	break;
  case S_DOOR:
	if(!coversDoor(map, shapePal, 
				   shapePal->getShape(ShapePalette::NS_DOOR_INDEX), 
				   mapx + unitOffset * 2, mapy + unitSide - 1)) {
	  map->setPosition(mapx + unitOffset, mapy + unitSide, 
					   wallHeight - 2, shapePal->getShape(ShapePalette::NS_DOOR_TOP_INDEX));
	  map->setPosition(mapx + unitOffset, mapy + unitSide, 
					   0, shapePal->getShape(ShapePalette::DOOR_SIDE_INDEX));
	  map->setPosition(mapx + unitOffset * 2, mapy + unitSide - 1, 
					   0, shapePal->getShape(ShapePalette::NS_DOOR_INDEX));
	  map->setPosition(mapx + unitSide - unitOffset * 2, mapy + unitSide, 
					   0, shapePal->getShape(ShapePalette::DOOR_SIDE_INDEX));
	}
	break;
  }
}

void DungeonGenerator::getRandomLocation(Map *map, Shape *shape, int *xpos, int *ypos) {
  int x, y;
  while(1) {
	// get a random location
	int n = (int)((float)ffCount * rand()/RAND_MAX);
	x = ff[n * 2];
	y = ff[n * 2 + 1];
	
	// can it fit?
	bool fits = map->shapeFits(shape, x, y, 0);
	// doesn't fit? try again (could be inf. loop)
	if(fits) {
	  // remove from ff list
	  for(int i = n + 1; i < ffCount - 1; i++) {
		ff[i * 2] = ff[i * 2 + 2];
		ff[i * 2 + 1] = ff[ i * 2 + 3];
	  }
	  ffCount--;
	  // return result
	  *xpos = x;
	  *ypos = y;
	  return;
	}
  }	
}

bool DungeonGenerator::coversDoor(Map *map, ShapePalette *shapePal, 
								  Shape *shape, int x, int y) {
  for(int ty = y - shape->getDepth() - 3; ty < y + 3; ty++) {
	for(int tx = x - 3; tx < x + shape->getWidth() + 3; tx++) {
	  if(isDoor(map, shapePal, tx, ty)) return true;
	}
  }
  return false;
}

bool DungeonGenerator::isDoor(Map *map, ShapePalette *shapePal, int tx, int ty) {
  if(tx >= 0 && tx < MAP_WIDTH && 
	 ty >= 0 && ty < MAP_DEPTH) {
	Location *loc = map->getLocation(tx, ty, 0);
	if(loc && 
	   (loc->shape == shapePal->getShape(ShapePalette::EW_DOOR_INDEX) ||
		loc->shape == shapePal->getShape(ShapePalette::NS_DOOR_INDEX))) {
	  return true;
	}
  }
  return false;
}

void DungeonGenerator::addItem(Map *map, Item *item, Shape *shape, int x, int y) {
  if(item) map->setItem(x, y, 0, item);
  else map->setPosition(x, y, 0, shape);
}
