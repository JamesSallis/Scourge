/***************************************************************************
         animatedshape.h  -  Additional functions for animated shapes
                             -------------------
    begin                : Thu Aug 31 2006
    copyright            : (C) 2006 by Gabor Torok
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

#ifndef ANIMATED_SHAPE_H
#define ANIMATED_SHAPE_H
#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

# include <string>
# include <vector>
# include <map>

#include "render.h"
#include "glshape.h"

class ModelWrapper;

/// An animated shape (character model).
class AnimatedShape : public GLShape  {

// uncomment to show debug shapes
//#define DEBUG_MD2 1

private:
	bool attackEffect;
	int g_ViewMode;
	int dir;
	float angle;
	bool debug;
	float creatureSpeed;
	GLShape *debugShape;
	bool pauseAnimation;

protected:
	int currentAnim;
	int animationWaiting; // memorizes one animation waiting to be played
	// To ensure that animations with few frames are played at least once entirely
	bool playedOnce;

	AnimatedShape( int width, int depth, int height,
	               char const* name, int descriptionGroup,
	               Uint32 color, Uint8 shapePalIndex = 0 );

public:
	virtual ~AnimatedShape();

	virtual inline void cleanup() {}
	virtual void draw() = 0;
	virtual void outline( float r, float g, float b ) = 0;

	void setDir( int dir );
	// if true, the next two functions are called
	bool isBlended();
	void setupBlending();
	void endBlending();

	inline GLShape *getDebugShape() {
		return debugShape;
	}
	inline void setCreatureSpeed( float n ) {
		creatureSpeed = n;
	}
	inline float getCreatureSpeed() {
		return creatureSpeed;
	}
	inline void setDebug( bool b ) {
		debug = b;
	}
	inline bool getDebug() {
		return debug;
	}
	inline bool getAttackEffect() {
		return attackEffect;
	}
	inline void setAttackEffect( bool b ) {
		attackEffect = b;
	}
	void setCurrentAnimation( int numAnim, bool force = false );
	inline int getCurrentAnimation() {
		return currentAnim;
	}
	inline void setPauseAnimation( bool pause ) {
		pauseAnimation = pause;
	}
	inline bool isAnimationPaused() {
		return pauseAnimation;
	}

	void animationFinished();
	virtual void setModelAnimation() = 0;

	inline void setAngle( float angle ) {
		float a = angle + 90.0f;
		if ( a >= 360.0f ) a -= 360.0f;
		this->angle = a;
	}
	inline float getAngle() {
		return angle;
	}

	/// check if position is cowered by shape 
	/// AnimatedShape has circular base by default
	virtual bool isInside( int x, int y ) {
		assert( x >= 0 && x < width && y >= 0 && y < depth );
		return true;
//		assert( x >= 0 && x < width && y >= 0 && y < depth );
//		int cut = width / 3;
//		return ( // may not cut (0,0) corner: x + y >= cut &&   
//		        width - x + y > cut && 
//		        x + depth - y > cut && 
//		        width - x + depth - y - 2 >= cut );
	}


	DECLARE_NOISY_OPENGL_SUPPORT();
};

#endif
