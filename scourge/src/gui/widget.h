/***************************************************************************
                       widget.h  -  Widget base class
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

#ifndef WIDGET_H
#define WIDGET_H
#pragma once

#include "gui.h"
#include "guievent.h"

/**
  *@author Gabor Torok
  */

class SDLHandler;

/// Base class for GUI widgets.
class Widget {
protected:
	int x, y, w, h;
	bool visible;
	Color color;
	Color background;
	Color selColor;
	Color borderColor;
	Color borderColor2;
	Color focusColor;
	bool focus;
	float alpha, alphaInc;
	GLint lastTick;
	char tooltip[3000];
	GLuint tooltipTicks;
	bool tooltipShowing;
	GLuint displayList;
	bool invalid;
	bool enabled;

public:
	Widget( int x, int y, int w, int h );
	virtual ~Widget();

	/// Example GUI Event  
	static GuiEvent Draw;

	typedef std::map<GuiEvent, Notifier*> ObserverMap;

	/// registrate callback of any real listener of GUI events
	template < typename T > 
	void attach( GuiEvent e, bool(T::*update)(Widget*), T* observer ) {
		// not implemented: multiple simultaneous observers of same event of same widget.
		ObserverMap::iterator oldObserver = observers.find( e );
		if ( oldObserver != observers.end() ) {
			delete oldObserver->second;  
		}

		ObserverInfo<T>* info = new ObserverInfo<T>( update, observer );
		observers[e] = info;
	}

	/// XXX: it is confusing that Widget has both draw(parent) and drawWidget(parent) member functions 
	/// removed virtuality from draw()
	void draw( Window* parent );

	inline void invalidate() {
		this->invalid = true;
	}

	inline bool isInvalid() {
		return this->invalid;
	}

	virtual inline int getX() {
		return x;
	}
	virtual inline int getY() {
		return y;
	}
	virtual inline int getWidth() {
		return w;
	}
	virtual inline int getHeight() {
		return h;
	}

	virtual inline void move( int x, int y ) {
		this->x = x; this->y = y;
	}
	virtual inline void resize( int w, int h ) {
		this->w = w; this->h = h;
	}

	virtual inline void setVisible( bool b ) {
		visible = b;
	}
	virtual inline bool isVisible() {
		return visible;
	}

	virtual inline void setEnabled( bool b ) {
		enabled = b;
	}
	virtual inline bool isEnabled() {
		return enabled;
	}

	virtual void drawWidget( Window* parent ) = 0;

	virtual inline bool hasFocus() {
		return focus;
	}
	virtual inline void setFocus( bool b ) {
		focus = b;
	}
	virtual inline bool canGetFocus() {
		return isVisible();
	}


	// color
	inline void setColor( float r, float g, float b, float a = 1.0f ) {
		this->color.r = r; this->color.g = g; this->color.b = b; this->color.a = a;
	}
	inline void setBorderColor( float r, float g, float b, float a = 1.0f ) {
		this->borderColor.r = r; this->borderColor.g = g; this->borderColor.b = b; this->borderColor.a = a;
	}
	inline void setHighlightedBorderColor( float r, float g, float b, float a = 1.0f ) {
		this->borderColor2.r = r; this->borderColor2.g = g; this->borderColor2.b = b; this->borderColor2.a = a;
	}
	inline void setBackground( float r, float g, float b, float a = 1.0f ) {
		background.r = r; background.g = g; background.b = b; background.a = a;
	}
	inline void setSelectionColor( float r, float g, float b, float a = 1.0f ) {
		selColor.r = r; selColor.g = g; selColor.b = b; selColor.a = a;
	}
	inline void setColor( Color *c ) {
		this->color.r = c->r; this->color.g = c->g; this->color.b = c->b; this->color.a = c->a;
	}
	inline void setBorderColor( Color *c ) {
		this->borderColor.r = c->r; this->borderColor.g = c->g; this->borderColor.b = c->b; this->borderColor.a = c->a;
	}
	inline void setBackground( Color *c ) {
		background.r = c->r; background.g = c->g; background.b = c->b; background.a = c->a;
	}
	inline void setSelectionColor( Color *c ) {
		selColor.r = c->r; selColor.g = c->g; selColor.b = c->b; selColor.a = c->a;
	}
	inline void setFocusColor( float r, float g, float b, float a = 1.0f ) {
		this->focusColor.r = r; this->focusColor.g = g; this->focusColor.b = b; this->focusColor.a = a;
	}
	inline void setFocusColor( Color *c ) {
		focusColor.r = c->r; focusColor.g = c->g; focusColor.b = c->b; focusColor.a = c->a;
	}

	inline void applyColor() {
		glColor4f( color.r, color.g, color.b, color.a );
	}
	inline void applyBorderColor() {
		glColor4f( borderColor.r, borderColor.g, borderColor.b, borderColor.a );
	}
	inline void applyHighlightedBorderColor() {
		glColor4f( borderColor2.r, borderColor2.g, borderColor2.b, borderColor2.a );
	}
	inline void applyBackgroundColor( bool opaque = false ) {
		glColor4f( ( enabled ? background.r : background.r / 2.0f ),
		           ( enabled ? background.g : background.g / 2.0f ),
		           ( enabled ? background.b : background.b / 2.0f ),
		           ( opaque ? 1.0f : 0.85f ) );
	}
	inline void applySelectionColor() {
		glColor4f( selColor.r, selColor.g, selColor.b, selColor.a );
	}
	inline void applyFocusColor() {
		glColor4f( focusColor.r, focusColor.g, focusColor.b, focusColor.a );
	}

	inline Color *getColor() {
		return &color;
	}
	inline Color *getBorderColor() {
		return &borderColor;
	}
	inline Color *getHighlightedBorderColor() {
		return &borderColor2;
	}
	inline Color *getBackgroundColor() {
		return &background;
	}
	inline Color * getSelectionColor() {
		return &selColor;
	}

	inline bool isTranslucent() {
		return ( background.a < 1.0f );
	}

	/**
	Return true, if the event activated this widget. (For example, button push, etc.)
	Another way to think about it is that if true, the widget fires an "activated" event
	to the outside world.
	 */
	virtual bool handleEvent( Window* parent, SDL_Event* event, int x, int y );
	
	/**
	 * Called when no events are sent to this widget. (ie. the mouse is elsewhere.)
	 */
	virtual void removeEffects();
	virtual bool isInside( int x, int y );

	inline void setDebug( bool d ) {
		debug = d;
	}

	/**
	 * Ability to distinguish between different types of events.
	 */
	virtual inline int getEventType() {
		return 0;
	}

	virtual inline bool hasSound() {
		return true;
	}

	inline void setTooltip( const char *s ) {
		strncpy( tooltip, ( s ? s : "" ), 2999 ); tooltip[2999] = '\0';
	}
	inline char *getTooltip() {
		return tooltip;
	}

	void drawTooltip( Window* parent );

	static void drawBorderedTexture( Texture texture, int x, int y, int w, int h,
	                                 int left, int right, int textureWidth,
	                                 bool inverse = false );

protected:
	bool debug;
	inline float getAlpha() {
		return alpha;
	}
	std::map< std::string, int > textWidthCache;
	virtual void drawButton( Window* parent, int x, int y, int x2, int y2,
	                         bool toggle, bool selected, bool inverse,
	                         bool glowing, bool inside );
	void breakText( char *text, int lineWidth, std::vector<std::string> *lines );
	int getTextWidth( Window* parent, const char* s );

	/// call attached observer to notify about event
	bool notify( GuiEvent e ) {
		if ( observers.find( e ) == observers.end() ) return false; // not all events must be observed 
		return observers[e]->notify( this );
	}

private:
	/// map of GUI Event observers
	ObserverMap observers;
};

#endif

