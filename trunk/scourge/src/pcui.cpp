/***************************************************************************
                          pcui.cpp  -  description
                             -------------------
    begin                : Sat May 3 2003
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

#include "pcui.h"
#include "rpg/rpglib.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"
#include "item.h"
#include "creature.h"
#include "equip.h"
#include "inven.h"

using namespace std;

/**
  *@author Gabor Torok
  */

#define EQUIP_WIDTH 230
#define EQUIP_HEIGHT 300
#define INVEN_WIDTH 530
#define INVEN_HEIGHT 160
#define WIN_WIDTH EQUIP_WIDTH + 320
#define WIN_HEIGHT EQUIP_HEIGHT + 220

PcUi::PcUi( Scourge *scourge ) {
	this->scourge = scourge;
	this->creature = NULL;

	mainWin = scourge->createWindow( ( scourge->getSDLHandler()->getScreen()->w - WIN_WIDTH ) / 2, 
																	 ( scourge->getSDLHandler()->getScreen()->h - WIN_HEIGHT ) / 2,
																	 WIN_WIDTH, WIN_HEIGHT,
																	 _( "Character Information" ) );
 equip = new Equip( scourge, mainWin, 10, 5, EQUIP_WIDTH, EQUIP_HEIGHT );
 mainWin->addWidget( equip->getWidget() );
 inven = new Inven( scourge, mainWin, 10, EQUIP_HEIGHT + 15, INVEN_WIDTH, INVEN_HEIGHT );
 mainWin->addWidget( inven->getWidget() );
 mainWin->addWidget( new Label( 8, WIN_HEIGHT - 30, 
																_( "Right click for info, double-click to open." ) ) );
}

PcUi::~PcUi() {
}

bool PcUi::handleEvent(Widget *widget, SDL_Event *event) {

	equip->handleEvent( widget, event );

	inven->handleEvent( widget, event );

  if( widget == mainWin->closeButton ) {
    scourge->toggleInventoryWindow();
	}
  return false;
}

bool PcUi::handleEvent(SDL_Event *event) {

	equip->handleEvent( event );

	inven->handleEvent( event );

  switch(event->type) {
  case SDL_KEYUP:
    switch(event->key.keysym.sym) {
    case SDLK_ESCAPE: 
    scourge->toggleInventoryWindow();
    return true;
    default: break;
    }
  default: break;
  }
  return false;
}

void PcUi::setCreature( Creature *creature ) {
	this->creature = creature;
	equip->setCreature( creature );
	inven->setCreature( creature );
}

