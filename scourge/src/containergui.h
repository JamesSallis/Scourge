/***************************************************************************
                          containergui.h  -  description
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

#ifndef CONTAINER_GUI_H
#define CONTAINER_GUI_H

#include <iostream>
#include <string>
#include "constants.h"
#include "scourge.h"
#include "item.h"
#include "gui/window.h"
#include "gui/widget.h"
#include "gui/button.h"

class ContainerGui {

 private:
  Scourge *scourge;
  Item *container;
  Window *win;
  Button *closeButton, *dropButton, *openButton, *player1Button, *player2Button, *player3Button, *player4Button;
  ScrollingList *list;

 public:
  ContainerGui(Scourge *scourge, Item *container, int x, int y);
  ~ContainerGui();

  bool handleEvent(SDL_Event *event);
  bool handleEvent(Widget *widget, SDL_Event *event);

  inline Item *getContainer() { return container; }
  inline Window *getWindow() { return win; }
  
};

#endif
