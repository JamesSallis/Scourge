/***************************************************************************
              sdleventhandler.h  -  Maps SDL events to game events
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

#ifndef SDLEVENTHANDLER_H
#define SDLEVENTHANDLER_H
#pragma once

#include "gui/widget.h"

/**
  *@author Gabor Torok
  */

class Widget;

/// Handles events triggered by SDL or a GUI widget.
class SDLEventHandler {
public:
	SDLEventHandler();
	virtual ~SDLEventHandler();

	/**
	  Handle the SDL_Event. Return true to quit, false otherwise
	*/
	virtual bool handleEvent( SDL_Event *event ) = 0;

	/**
	Handle an event, fired by a widget
	 */
	virtual bool handleEvent( Widget *widget, SDL_Event *event ) = 0;

};

#endif
