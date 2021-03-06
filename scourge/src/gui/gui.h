/***************************************************************************
                        gui.h  -  ScourgeGui class.  
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

#ifndef GUI_GUI_H
#define GUI_GUI_H
#pragma once

#include "../configlang.h"

class Widget;
class Window;
class Label;
class Button;
class Texture;
class EventHandler;

/**
 * This is the only class thru which the gui interacts with the rest of scourge.
 * It is important to keep this in mind so that:
 * -the gui classes can be used by other apps
 * -compilation is sped up (by not including things from outside of this dir.
 */
class ScourgeGui {
public:
	ScourgeGui();

	virtual ~ScourgeGui();

	virtual void processEventsAndRepaint() = 0;
	virtual void playSound( const std::string& file, int panning ) = 0;
	virtual void texPrint( GLfloat x, GLfloat y, const char *fmt, ... ) = 0;
	virtual int textWidth( const char* fmt, ... ) = 0;
	virtual int getScreenWidth() = 0;
	virtual int getScreenHeight() = 0;
	virtual void setCursorMode( int n, bool useTimer = false ) = 0;
	virtual int getCursorMode() = 0;
	virtual Texture const& getHighlightTexture() = 0;
	virtual Texture const& getGuiTexture() = 0;
	virtual Texture const& getGuiTexture2() = 0;
	virtual Uint16 getMouseX() = 0;
	virtual Uint16 getMouseY() = 0;
	virtual void drawTooltip( float xpos2, float ypos2, float zpos2,
	                          float zrot, float yrot,
	                          char* message,
	                          float r = 0, float g = 0.15f, float b = 0.05f, float zoom = 1.0f ) = 0;
	virtual void setFontType( int fontType ) = 0;
	virtual Texture const& loadSystemTexture( char *line ) = 0;
	// XXX: yo-yo. its because moving static stuff irrelevant for Window here 
	virtual void unlockMouse() {
		mouseLockWindow = NULL;
		mouseLockWidget = NULL;
	}
	virtual void lockMouse( Window* window, Widget* widget ) {
		mouseLockWindow = window;
		mouseLockWidget = widget;
	}
	virtual void allWindowsClosed() = 0;
	virtual void blockEvent() = 0;
	virtual void registerEventHandler( Widget* w, EventHandler* eh ) = 0; 
	virtual void unregisterEventHandler( Widget* w ) = 0;
	virtual EventHandler *getEventHandler( Widget* w ) = 0;

	static const int MAX_WINDOW = 1000;

	// window management
	Window* window[MAX_WINDOW];
	int windowCount;

	Window* message_dialog;
	Label* message_label;
	Window* currentWin;

	Window* mouseLockWindow;
	Widget* mouseLockWidget;
	bool windowWasClosed;

	void drawVisibleWindows();
	void addWindow( Window *win );
	void removeWindow( Window *win );
	// OLD STYLE
	Widget* delegateEvent( SDL_Event* event, int x, int y, Window** selectedWindow );
	// NEW STYLE
	bool sendMousePosition( int x, int y );

	Widget* selectCurrentEscapeHandler();
	void toTop( Window* win );
	void toBottom( Window* win );
	void nextWindowToTop( Window* win, bool includeLocked = true );
	void prevWindowToTop( Window* win, bool includeLocked = true );
	bool anyFloatingWindowsOpen();
	Window* getTopWindow() {return currentWin;}


	// static message dialog
	Button* message_button; // so you can check for it in other classes
	void showMessageDialog( int x, int y, int w, int h,
	                        char const* title, Texture texture,
	                        char const* message,
	                        char const* buttonLabel = _( Constants::messages[Constants::OK_LABEL][0] ) );
private:
	Window* findTopmostWindowAt( int x, int y );

};

#endif
