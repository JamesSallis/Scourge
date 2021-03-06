/***************************************************************************
                      widget.cpp  -  Widget base class
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

#include "../common/constants.h"
#include "widget.h"
#include "window.h"
#include "guitheme.h"
#include "../render/texture.h"

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

using namespace std;

#define TOOLTIP_DELAY 500

/**
  *@author Gabor Torok
  */

GuiEvent Widget::Draw = "Widget::Draw Event"; // actually only fired by Canvas


Widget::Widget( int x, int y, int w, int h ) {
	move( x, y );
	this->w = w;
	this->h = h;
	setColor( 0.05f, 0.05f, 0.05f, 1 );
	setBackground( 1, 0.75f, 0.45f );
	//  setSelectionColor( 1, 0.5f, 0.45f );
	setSelectionColor( 0.75f, 0.75f, 0.8f );
	//setBorderColor( 0.8f, 0.5f, 0.2f );
	setBorderColor( 0.6f, 0.3f, 0.1f );
	setHighlightedBorderColor( 1.0f, 0.8f, 0.4f );
	setFocusColor( 0.8f, 0.5f, 0.2f, 1 );
	visible = true;
	enabled = true;
	debug = false;
	focus = false;
	alpha = 0.5f;
	alphaInc = 0.05f;
	lastTick = 0;
	strcpy( tooltip, "" );
	tooltipTicks = 0;
	tooltipShowing = false;
	invalid = true;
	displayList = glGenLists( 1 );
	if ( !displayList ) {
		cerr << "*** Error: couldn't generate display list for gui." << endl;
		exit( 1 );
	}
}

Widget::~Widget() {
	glDeleteLists( displayList, 1 );

	for ( ObserverMap::iterator i = observers.begin(); i != observers.end(); ++i ) {
		delete i->second;
	}
	observers.clear();
}

void Widget::draw( Window* parent ) {
	glTranslatef( x, y, 0.0f );
	if ( hasFocus() ) {
		GuiTheme *theme = parent->getTheme();
		if ( theme->getSelectedBorder() ) {
			glColor4f( theme->getSelectedBorder()->color.r,
			           theme->getSelectedBorder()->color.g,
			           theme->getSelectedBorder()->color.b,
			           theme->getSelectedBorder()->color.a );
		} else {
			applyFocusColor();
		}
		glLineWidth( 2.0f );
		glBegin( GL_LINES );
		glVertex2i( -1, -1 );
		glVertex2i( -1, getHeight() + 1 );

		glVertex2i( -1, -1 );
		glVertex2i( getWidth() + 2, -1 );

		glVertex2i( -1, getHeight() + 1 );
		glVertex2i( getWidth() + 2, getHeight() + 1 );

		glVertex2i( getWidth() + 2, -1 );
		glVertex2i( getWidth() + 2, getHeight() + 1 );
		glEnd();
		glLineWidth( 1.0f );
	}
	//if( invalid ) {
	//glNewList( displayList, GL_COMPILE );
	drawWidget( parent );
	//glEndList();
	//invalid = false;
	//}
	//glCallList( displayList );
}

bool Widget::handleEvent( Window* parent, SDL_Event* event, int x, int y ) {
	// do nothing by default
	return false;
}

void Widget::removeEffects() {
}

bool Widget::isInside( int x, int y ) {
	return( x >= getX() && x < getX() + w &&
	        y >= getY() && y < getY() + h );
}

void Widget::drawButton( Window* parent, int x, int y, int x2, int y2, bool toggle, bool selected, bool inverse, bool glowing, bool inside ) {
	GuiTheme *theme = parent->getTheme();

	glPushMatrix();

	glTranslatef( x, y, 0.0f );

	if ( isTranslucent() ) {
		glsEnable( GLS_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	}

	int n = 0;
	Texture tex;

	if ( toggle && selected ) {

		if ( theme->getButtonSelectionBackground() ) {
			glsEnable( GLS_TEXTURE_2D );

			tex = theme->getButtonSelectionBackground()->texture;
			tex.glBind();

			if ( isEnabled() ) {
				glColor4f( theme->getButtonSelectionBackground()->color.r, theme->getButtonSelectionBackground()->color.g, theme->getButtonSelectionBackground()->color.b, theme->getButtonSelectionBackground()->color.a );
			} else {
				glColor4f( theme->getButtonSelectionBackground()->color.r / 2.0f, theme->getButtonSelectionBackground()->color.g / 2.0f, theme->getButtonSelectionBackground()->color.b / 2.0f, theme->getButtonSelectionBackground()->color.a / 2.0f );
			}

			n = theme->getButtonSelectionBackground()->width;
		} else {
			applySelectionColor();
		}

	} else if ( theme->getButtonBackground() ) {

		glsEnable( GLS_TEXTURE_2D );

		tex = theme->getButtonBackground()->texture;
		tex.glBind();

		if ( isEnabled() ) {
			glColor4f( theme->getButtonBackground()->color.r, theme->getButtonBackground()->color.g, theme->getButtonBackground()->color.b, theme->getButtonBackground()->color.a );
		} else {
			glColor4f( theme->getButtonBackground()->color.r / 2.0f, theme->getButtonBackground()->color.g / 2.0f, theme->getButtonBackground()->color.b / 2.0f, theme->getButtonBackground()->color.a / 2.0f );
		}

		n = theme->getButtonBackground()->width;

	} else {

		applyBackgroundColor( true );

	}

	if ( inverse ) {
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 1, 1 );
		glVertex2i( n, n );
		glTexCoord2i( 0, 1 );
		glVertex2i( x2 - x - n, n );
		glTexCoord2i( 1, 0 );
		glVertex2i( n, y2 - y - n );
		glTexCoord2i( 0, 0 );
		glVertex2i( x2 - x - n, y2 - y - n );
		glEnd();
	} else {
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( n, n );
		glTexCoord2i( 1, 0 );
		glVertex2i( x2 - x - n, n );
		glTexCoord2i( 0, 1 );
		glVertex2i( n, y2 - y - n );
		glTexCoord2i( 1, 1 );
		glVertex2i( x2 - x - n, y2 - y - n );
		glEnd();
	}

	if ( n ) {
		glPushMatrix();

		if ( toggle && selected ) {
			if ( inverse ) theme->getButtonSelectionBackground()->tex_south.glBind();
			else theme->getButtonSelectionBackground()->tex_north.glBind();
		} else {
			if ( inverse ) theme->getButtonBackground()->tex_south.glBind();
			else theme->getButtonBackground()->tex_north.glBind();
		}

		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2i( 1, 0 );
		glVertex2i( x2 - x, 0 );
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, n );
		glTexCoord2i( 1, 1 );
		glVertex2i( x2 - x, n );
		glEnd();
		glPopMatrix();

		glPushMatrix();

		glTranslatef( 0.0f, y2 - y - n, 0.0f );

		if ( toggle && selected ) {
			if ( inverse ) theme->getButtonSelectionBackground()->tex_north.glBind();
			else theme->getButtonSelectionBackground()->tex_south.glBind();
		} else {
			if ( inverse ) theme->getButtonBackground()->tex_north.glBind();
			else theme->getButtonBackground()->tex_south.glBind();
		}

		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2i( 1, 0 );
		glVertex2i( x2 - x, 0 );
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, n );
		glTexCoord2i( 1, 1 );
		glVertex2i( x2 - x, n );
		glEnd();

		glPopMatrix();

		glPushMatrix();

		glTranslatef( 0.0f, n, 0.0f );

		if ( toggle && selected ) {
			if ( inverse ) theme->getButtonSelectionBackground()->tex_east.glBind();
			else theme->getButtonSelectionBackground()->tex_west.glBind();
		} else {
			if ( inverse ) theme->getButtonBackground()->tex_east.glBind();
			else theme->getButtonBackground()->tex_west.glBind();
		}

		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2i( 1, 0 );
		glVertex2i( n, 0 );
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, y2 - y - ( 2 * n ) );
		glTexCoord2i( 1, 1 );
		glVertex2i( n, y2 - y - ( 2 * n ) );
		glEnd();

		glPopMatrix();

		glPushMatrix();

		glTranslatef( x2 - x - n, n, 0.0f );

		if ( toggle && selected ) {
			if ( inverse ) theme->getButtonSelectionBackground()->tex_west.glBind();
			else theme->getButtonSelectionBackground()->tex_east.glBind();
		} else {
			if ( inverse ) theme->getButtonBackground()->tex_west.glBind();
			else theme->getButtonBackground()->tex_east.glBind();
		}

		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2i( 1, 0 );
		glVertex2i( n, 0 );
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, y2 - y - ( 2 * n ) );
		glTexCoord2i( 1, 1 );
		glVertex2i( n, y2 - y - ( 2 * n ) );
		glEnd();

		glPopMatrix();
	}

	glsDisable( GLS_TEXTURE_2D );
	if ( isTranslucent() ) {
		glsDisable( GLS_BLEND );
	}

	GLint t = SDL_GetTicks();
	if ( lastTick == 0 || t - lastTick > 50 ) {
		lastTick = t;
		alpha += alphaInc;
		if ( alpha >= 0.7f || alpha < 0.4f ) alphaInc *= -1.0f;
	}


	// glowing red
	if ( glowing ) {
		if ( theme->getButtonHighlight() ) {
			glsEnable( GLS_TEXTURE_2D );
			theme->getButtonHighlight()->texture.glBind();
		}
		// FIXME: use theme
		glsEnable( GLS_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		glColor4f( 1.0f, 0.15f, 0.15f, alpha );

		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( n, n );
		glTexCoord2i( 1, 0 );
		glVertex2i( x2 - x - n, n );
		glTexCoord2i( 0, 1 );
		glVertex2i( n, y2 - y - n );
		glTexCoord2i( 1, 1 );
		glVertex2i( x2 - x - n, y2 - y - n );
		glEnd();

		glsDisable( GLS_TEXTURE_2D | GLS_BLEND );
	}


	if ( inside && isEnabled() ) {
		if ( theme->getButtonHighlight() ) {
			glsEnable( GLS_TEXTURE_2D );

			glColor4f( theme->getButtonHighlight()->color.r, theme->getButtonHighlight()->color.g, theme->getButtonHighlight()->color.b, alpha );
			theme->getButtonHighlight()->texture.glBind();
		}

		glsEnable( GLS_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( n, n );
		glTexCoord2i( 1, 0 );
		glVertex2i( x2 - x - n, n );
		glTexCoord2i( 0, 1 );
		glVertex2i( n, y2 - y - n );
		glTexCoord2i( 1, 1 );
		glVertex2i( x2 - x - n, y2 - y - n );
		glEnd();

		glsDisable( GLS_TEXTURE_2D | GLS_BLEND );
	}

	if ( theme->getButtonBorder() ) {
		glColor4f( theme->getButtonBorder()->color.r, theme->getButtonBorder()->color.g, theme->getButtonBorder()->color.b, theme->getButtonBorder()->color.a );
	}

	glBegin( GL_LINE_LOOP );
	glVertex2i( 0, 0 );
	glVertex2i( 0, y2 - y );
	glVertex2i( x2 - x, y2 - y );
	glVertex2i( x2 - x, 0 );
	glEnd();

	glPopMatrix();
}

void Widget::breakText( char *text, int lineWidth, vector<string> *lines ) {
	char *p =  new char[ lineWidth + 3 ];
	int len = strlen( text );
	int start = 0;
	//cerr << "len=" << len << endl;
	//cerr << "text=" << text << endl;
	while ( start < len ) {
		int end = start + lineWidth;
		if ( end > len ) end = len;
		else {
			// find a space (if any)
			int n = end;
			while ( n > start && *( text + n ) != ' ' ) n--;
			if ( n > start ) end = n + 1;
		}

		// search for hard breaks
		int n = start;
		while ( n < end && *( text + n ) != '|' ) n++;
		bool hardbreak = false;
		if ( n < end ) {
			end = n + 1;
			hardbreak = true;
		}

		//cerr << "\tstart=" << start << endl;
		//cerr << "\tend=" << end << endl;
		//cerr << "\tend-start=" << (end-start) << endl;

		strncpy( p, text + start, end - start );
		*( p + end - ( hardbreak ? 1 : 0 ) - start ) = 0;
		//cerr << "p=" << p << endl;

		string s = p;
		lines->push_back( s );
		start = end;
	}
	delete [] p;
}

void Widget::drawTooltip( Window* parent ) {
	int xpos = parent->getScourgeGui()->getMouseX() - parent->getX();
	int ypos = parent->getScourgeGui()->getMouseY() - parent->getY() - parent->getGutter();
	bool b = isInside( xpos, ypos );

	if ( !( tooltipShowing && b ) ) {
		if ( !b ) {
			tooltipShowing = b;
			tooltipTicks = 0;
		} else {
			if ( !tooltipTicks ) tooltipTicks = SDL_GetTicks();
			else if ( SDL_GetTicks() - tooltipTicks > TOOLTIP_DELAY ) {
				tooltipShowing = b;
			}
		}
	}
	if ( !tooltipShowing || !strlen( tooltip ) ) return;
	parent->getScourgeGui()->drawTooltip( xpos, ypos, 450, 0, 0, tooltip );
}

/**
 * Draw a rectangle with a texture, such that only the middle of the texture
 * is stretched.
 *
 * texture the texture to use
 * x,y,w,h the dimensions of the quad
 * left,right the size of the border area
 * textureWidth the size of the texture used
 *
 * 3 quads will be drawn like this:
 * +--+-----+--+
 * |  |     |  |
 * |A |  B  |C |
 * |  |     |  |
 * +--+-----+--+
 *
 * The texture is only stretched on quad B. This assumes that
 * the height changes of the quad is not as important as stretching
 * it horizontally. This is generally true for buttons, progress bars, etc.
 */
void Widget::drawBorderedTexture( Texture texture, int x, int y, int width, int height, int left, int right, int textureWidth, bool inverse ) {

	glsEnable( GLS_TEXTURE_2D | GLS_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	texture.glBind();

	glPushMatrix();

	glTranslatef( x, y, 0.0f );

	glBegin( GL_QUADS );
	if ( inverse ) {

		// quad A
		glTexCoord2f( static_cast<float>( left ) / static_cast<float>( textureWidth ), 1.0f );
		glVertex2i( 0, 0 );
		glTexCoord2f( static_cast<float>( left ) / static_cast<float>( textureWidth ), 0.0f );
		glVertex2i( 0, height );
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2i( left, height );
		glTexCoord2f( 0.0f, 1.0f );
		glVertex2i( left, 0 );

		// quad B
		glTexCoord2f( 1.0f - ( static_cast<float>( right ) / static_cast<float>( textureWidth ) ), 1.0f );
		glVertex2i( left, 0 );
		glTexCoord2f( 1.0f - ( static_cast<float>( right ) / static_cast<float>( textureWidth ) ), 0.0f );
		glVertex2i( left, height );
		glTexCoord2f( static_cast<float>( left ) / static_cast<float>( textureWidth ), 0.0f );
		glVertex2i( width - right, height );
		glTexCoord2f( static_cast<float>( left ) / static_cast<float>( textureWidth ), 1.0f );
		glVertex2i( width - right, 0 );

		// quad C
		glTexCoord2f( 1.0f, 1.0f );
		glVertex2i( width - right, 0 );
		glTexCoord2f( 1.0f, 0.0f );
		glVertex2i( width - right, height );
		glTexCoord2f( 1.0f - ( static_cast<float>( right ) / static_cast<float>( textureWidth ) ), 0.0f );
		glVertex2i( width, height );
		glTexCoord2f( 1.0f - ( static_cast<float>( right ) / static_cast<float>( textureWidth ) ), 1.0f );
		glVertex2i( width, 0 );

	} else {

		// quad A
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2i( 0, 0 );
		glTexCoord2f( 0.0f, 1.0f );
		glVertex2i( 0, height );
		glTexCoord2f( static_cast<float>( left ) / static_cast<float>( textureWidth ), 1.0f );
		glVertex2i( left, height );
		glTexCoord2f( static_cast<float>( left ) / static_cast<float>( textureWidth ), 0.0f );
		glVertex2i( left, 0 );

		// quad B
		glTexCoord2f( static_cast<float>( left ) / static_cast<float>( textureWidth ), 0.0f );
		glVertex2i( left, 0 );
		glTexCoord2f( static_cast<float>( left ) / static_cast<float>( textureWidth ), 1.0f );
		glVertex2i( left, height );
		glTexCoord2f( 1.0f - ( static_cast<float>( right ) / static_cast<float>( textureWidth ) ), 1.0f );
		glVertex2i( width - right, height );
		glTexCoord2f( 1.0f - ( static_cast<float>( right ) / static_cast<float>( textureWidth ) ), 0.0f );
		glVertex2i( width - right, 0 );

		// quad C
		glTexCoord2f( 1.0f - ( static_cast<float>( right ) / static_cast<float>( textureWidth ) ), 0.0f );
		glVertex2i( width - right, 0 );
		glTexCoord2f( 1.0f - ( static_cast<float>( right ) / static_cast<float>( textureWidth ) ), 1.0f );
		glVertex2i( width - right, height );
		glTexCoord2f( 1.0f, 1.0f );
		glVertex2i( width, height );
		glTexCoord2f( 1.0f, 0.0f );
		glVertex2i( width, 0 );

	}

	glEnd();

	glPopMatrix();

	glsDisable( GLS_BLEND );
}

int Widget::getTextWidth( Window* parent, const char* s ) {
	string str = s;
	int n;
	if ( textWidthCache.find( str ) == textWidthCache.end() ) {
		n = parent->getScourgeGui()->textWidth( s );
		textWidthCache[ str ] = n;
	} else {
		n = textWidthCache[ str ];
	}
	return n;
}

