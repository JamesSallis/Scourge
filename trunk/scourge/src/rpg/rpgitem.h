/***************************************************************************
                          rpgitem.cpp  -  description
                             -------------------
    begin                : Sun Sep 28 2003
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
#ifndef RPG_ITEM_H
#define RPG_ITEM_H

#include <map>
#include <string>
#include <vector>
#include "../constants.h"
#include "character.h"

using namespace std;

class RpgItem {
 private:

  int index;
  char *name, *desc, *shortDesc;
  int level;
  int rareness;
  int type;
  float weight; 
  int price, quality;
  int action; // damage, defence, potion str.
  int speed; // 0-100, 100-slowest, 0-fastest
  int shape_index;
  int twohanded;
  int distance; // how far can it reach?
  int equip; // where can it be worn?
  int skill; // which skill to check when using the item
  int maxCharges;
  int potionSkill; // which skill does this potion effect?
  int potionTime;
  GLuint acl; // 1 bit per character class

  static map<int, map<int, vector<const RpgItem*>*>*> typesMap;
  static map<string, const RpgItem *> itemsByName;

 public:

  enum itemTypes {
	SWORD=0,
	AXE,
	BOW,
	CONTAINER,
	ARMOR,
	FOOD,
	DRINK,
	POTION,
	OTHER,
	MISSION,
  SCROLL,
	
	// must be last
	ITEM_TYPE_COUNT
  };
  static char itemTypeStr[ITEM_TYPE_COUNT][40];

  enum twoHandedType {
	NOT_TWO_HANDED=0,
	ONLY_TWO_HANDED,
	OPTIONAL_TWO_HANDED
  };

  static RpgItem *items[1000];
  static int itemCount;
  
  RpgItem(int index, char *name, int level, int rareness, int type, float weight, int price, int quality, 
		  int action, int speed, char *desc, char *shortDesc, int equip, int shape_index, 
		  int twohanded=NOT_TWO_HANDED, int distance=1, int skill=-1, int maxCharges=0,
		  int potionSkill=-1, int potionTime=0);
  ~RpgItem();

  inline int getIndex() { return index; }
  inline char *getName() { return name; }
  inline int getAction() { return action; }
  inline int getLevel()  { return level; }
  inline int getRareness()  { return rareness; }
  inline int getSpeed() { return speed; }
  inline int getQuality() { return quality; }
  inline float getWeight() { return weight; }
  inline int getShapeIndex() { return shape_index; }
  inline char *getShortDesc() { return shortDesc; }  
  inline int getEquip() { return equip; }
  inline int getDistance() { return distance; }
  inline int getSkill() { return skill; } 
  inline int getType() { return type; }
  inline int getMaxCharges() { return maxCharges; }
  inline int getPotionSkill() { return potionSkill; }
  inline int getPotionTime() { return potionTime; }
  inline bool getAcl(int index) { return (acl & (1 << index) ? true : false); }
  inline void setAcl(int index, bool value) { if(value) acl |= (1 << index); else acl &= ((GLuint)0xffff - (GLuint)(1 << index)); }
  inline void setAllAcl(bool value) { if(value) acl = (GLuint)0xffff; else acl = (GLuint)0; }
  inline GLuint getAllAcl() { return acl; }

  // FIXME: make this more specific to item
  // e.g. multi-attack items, like sword of fireballs
  inline bool isRangedWeapon() { return type == BOW; }
  
  static RpgItem *getRandomItem(int level);
  static RpgItem *getRandomContainer();
  static RpgItem *getRandomContainerNS();
  inline static RpgItem *getItem(int index) { return items[index]; }

  static int getTypeByName(char *name);
  static void addItem(RpgItem *item);
  static RpgItem *getItemByName(char *name);
};

#endif
