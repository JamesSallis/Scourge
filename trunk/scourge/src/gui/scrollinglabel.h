/***************************************************************************
                          scrollinglabel.h  -  description
                             -------------------
    begin                : Thu Aug 28 2003
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

#ifndef SCROLLING_LABEL_H
#define SCROLLING_LABEL_H

#include "../constants.h"
#include "widget.h"
#include "button.h"
#include "window.h"
#include "draganddrop.h"
#include <vector>

using namespace std;

/**
  *@author Gabor Torok
  */

class ScrollingLabel : public Widget {
 protected:
  char text[3000];
  int lineWidth;
  vector<string> lines;

  //  int count;
  //  const char **list;
  //  const Color *colors;
  //  const GLuint *icons;
  int value;
  int scrollerWidth, scrollerHeight;
  int listHeight;
  float alpha, alphaInc;
  GLint lastTick;
  bool inside;
  int scrollerY;
  bool dragging;
  int dragX, dragY;
  int selectedLine;
  //  DragAndDropHandler *dragAndDropHandler;
  bool innerDrag;
  int innerDragX, innerDragY;
  //  bool highlightBorders;
  // GLuint highlight;
  bool canGetFocusVar;

  map<char, Color> coloring;

 public: 

   bool debug;

   ScrollingLabel(int x, int y, int w, int h, char *text );
   virtual ~ScrollingLabel();

   inline void addColoring( char c, Color color ) { coloring[c]=color; }

   inline char *getText() { return text; }
   void setText(char *s);

   //  inline int getLineCount() { return count; }
   //  void setLines(int count, const char *s[], const Color *colors=NULL, const GLuint *icon=NULL);
   //  inline const char *getLine(int index) { return list[index]; }

   //  inline int getSelectedLine() { return selectedLine; }
   //  void setSelectedLine(int n);

  void drawWidget(Widget *parent);

  /**
	 Return true, if the event activated this widget. (For example, button push, etc.)
	 Another way to think about it is that if true, the widget fires an "activated" event
	 to the outside world.
   */
  bool handleEvent(Widget *parent, SDL_Event *event, int x, int y);

  void removeEffects(Widget *parent);

  // don't play sound when the value changes
  virtual inline bool hasSound() { return false; }

  inline bool canGetFocus() { return canGetFocusVar; }
  inline void setCanGetFocus(bool b) { this->canGetFocusVar = b; }

 private:
   void printLine( Widget *parent, int x, int y, char *s );
  //  void selectLine(int x, int y);
  //  void drawIcon( int x, int y, GLuint icon );
};

#endif

