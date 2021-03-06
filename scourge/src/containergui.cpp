/***************************************************************************
              containergui.cpp -  The container contents window
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

#include "common/constants.h"
#include "containerview.h"
#include "containergui.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "sound.h"
#include "item.h"
#include "creature.h"
#include "shapepalette.h"
#include "pcui.h"

using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

#define OFFSET_X 1
#define OFFSET_Y 0

ContainerGui::ContainerGui( Scourge *scourge, int x, int y ) {
	this->scourge = scourge;
	
	win = new Window( scourge->getSDLHandler(), x, y, 105, 40, "", scourge->getShapePalette()->getGuiTexture(), true, Window::BASIC_WINDOW, scourge->getShapePalette()->getGuiTexture2() );
	openButton = new Button( 10, 5, 90, 25, scourge->getShapePalette()->getHighlightTexture(), Constants::getMessage( Constants::OPEN_CONTAINER_LABEL ) );
	win->addWidget( ( Widget* )openButton );
	infoButton = new Button( 10, 30, 90, 50, scourge->getShapePalette()->getHighlightTexture(), _( "Info" ) );
	infoButton->setEnabled( false );
	win->addWidget( ( Widget* )infoButton );
	getAllButton = new Button( 10, 55, 90, 75, scourge->getShapePalette()->getHighlightTexture(), _( "Get All" ) );
	win->addWidget( ( Widget* )getAllButton );
	closeButton = new Button( 10, win->getHeight() - 55, 90, win->getHeight() - 35, scourge->getShapePalette()->getHighlightTexture(), _( "Close" ) );
	win->addWidget( ( Widget* )closeButton );

	view = new ContainerView( scourge, NULL, win, 95, 5 );
	win->addWidget( (Canvas*)view );

	win->registerEventHandler( this );
	win->setRawEventHandler( this );
	win->setVisible( true );	
}

ContainerGui::~ContainerGui() {
	delete win;
}

void ContainerGui::setContainer( Item *container ) {
  this->container = container;
  win->setTitle( container->getItemName() );
  win->resize( 105 + container->getRpgItem()->getContainerWidth() * GRID_SIZE, 
                40 + container->getRpgItem()->getContainerHeight() * GRID_SIZE );
  closeButton->move( 10, win->getHeight() - 55 );
  view->setItem( container );
}

bool ContainerGui::handleEvent( SDL_Event *event ) {
	view->handleEvent( event );
	return false;	
}

bool ContainerGui::handleEvent( Widget *widget, SDL_Event *event ) {
	if ( widget == win->closeButton || widget == closeButton ) {
		win->setVisible( false );
//		scourge->closeContainerGui( this );
	} else if ( widget == infoButton ) {
		if( view->getSelectedItem() ) {
			view->showInfo( view->getSelectedItem() );
		}
	} else if ( widget == openButton ) {
		if ( view->getSelectedItem() && view->getSelectedItem()->getRpgItem()->getType() == RpgItem::CONTAINER ) {
			scourge->openContainerGui( view->getSelectedItem() );
		}
	} else if ( widget == getAllButton ) {
		while ( view->getContainer()->getContainedItemCount() > 0 ) {
			Item *item = view->getContainer()->getContainedItem( 0 );
			// try to add it
			//if( scourge->getParty()->getPlayer()->addToBackpack( item ) ) {
			scourge->getPcUi()->setCreature( scourge->getParty()->getPlayer() );
			if ( scourge->getPcUi()->addToBackpack( item ) ) {
				if( item == view->getSelectedItem() ) {
					view->setSelectedItem( NULL );
				}
				view->getContainer()->removeContainedItem( item );
			} else {
				scourge->showMessageDialog( _( "There is not enough room in your backpack for everything." ) );
				break;
			}
		}
		scourge->getPcUi()->refresh();
		view->refresh();
	} else {
		bool b = view->handleEvent( widget, event );
		infoButton->setEnabled( view->getSelectedItem() != NULL );
		return b;
	}
	return false;
}

void ContainerGui::refresh() {
	view->refresh();
}

Item *ContainerGui::getSelectedItem() {
	return view->getSelectedItem();
}
