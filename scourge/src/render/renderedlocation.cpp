/***************************************************************************
                  shape.cpp  -  Class for static 3D shapes
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

#include "renderedlocation.h"
#include "../common/constants.h"
#include <map>
#include <vector>
#include "maprender.h"
#include "renderedprojectile.h"
#include "renderedcreature.h"
#include "maprenderhelper.h"
#include "rendereditem.h"
#include "glshape.h"
#include "frustum.h"
#include "location.h"
#include "projectilerenderer.h"
#include "map.h"
#include "mapsettings.h"
#include "mapadapter.h"
#include "effect.h"
#include "location.h"
#include "shape.h"
#include "shapes.h"
#include "virtualshape.h"
#include "md2shape.h"
#include "../quickhull.h"
#include "../debug.h"

using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif
  
const float shadowTransformMatrix[16] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
// 0.5f, -0.5f, 0, 0,
	0.75f, -0.25f, 0, 0,
	0, 0, 0, 1
};

bool RenderedLocation::useShadow = false;
bool RenderedLocation::colorAlreadySet = false;
Texture RenderedLocation::lightTex;

RenderedLocation::RenderedLocation() {
	reset();
}

RenderedLocation::~RenderedLocation() {	
}
	
void RenderedLocation::reset() {
	map = NULL;
	xpos = ypos = zpos = 0.0f;
  effect = NULL;
  pos = NULL;
  name = 0;
  x = y = 0;
  inFront = light = effectMode = false;
}

void RenderedLocation::set( Map *map, 
                            float xpos2, float ypos2, float zpos2, 
                            EffectLocation *effect,
                            Location *pos,
                            GLuint name,
                            int posX, int posY,
                            bool inFront, bool light, bool effectMode ) {
	this->map = map;
	if( !lightTex.isSpecified() ) {
		lightTex = map->getShapes()->findTextureByName( "light.png", true );
	}
	this->xpos = xpos2;
	this->ypos = ypos2;
	this->zpos = zpos2;
  this->effect = effect;
  this->pos = pos;
  this->name = name;
  this->x = posX;
  this->y = posY;
  this->inFront = inFront;
  this->light = light;
  this->effectMode = effectMode;
}

void RenderedLocation::drawEffect() {
	// fog test for creatures
	if ( map->getHelper() && pos && pos->creature && 
			!map->getAdapter()->isInMovieMode() && 
			!map->getHelper()->isVisible( pos->x, pos->y, pos->creature->getShape() ) ) {
		return;
	}


	glPushMatrix();
	
	float xdiff = 0;
	float ydiff = 0;
	float heightPos = 0.0f;
	if ( pos ) {
		GLShape *s = ( GLShape* )pos->shape;
		if ( s->isVirtual() ) {
			s = ( ( VirtualShape* )s )->getRef();
		}

		if ( !s->getIgnoreHeightMap() ) {
			heightPos = pos->heightPos * MUL;
		}
		
		if ( pos->creature ) {
			xdiff = ( pos->creature->getX() - static_cast<float>( toint( pos->creature->getX() ) ) );
			ydiff = ( pos->creature->getY() - static_cast<float>( toint( pos->creature->getY() ) ) );
		}		
	} else if ( effect ) {
		heightPos = effect->heightPos;
	}

	glTranslatef( xpos + xdiff * MUL, ypos + ydiff * MUL, zpos + heightPos );

	if ( pos ) {
		glTranslatef( pos->moveX, pos->moveY, pos->moveZ );
		glRotatef( pos->angleX, 1.0f, 0.0f, 0.0f );
		glRotatef( pos->angleY, 0.0f, 1.0f, 0.0f );
		glRotatef( pos->angleZ, 0.0f, 0.0f, 1.0f );
	}

	if ( pos && pos->creature ) {
		glTranslatef( pos->creature->getOffsetX(), pos->creature->getOffsetY(), pos->creature->getOffsetZ() );
	}

	if ( colorAlreadySet ) {
		colorAlreadySet = false;
	} else {
		map->getRender()->setupShapeColor();
	}

	glDisable( GL_CULL_FACE );

	if( pos ) {
		( ( GLShape* )pos->shape )->setCameraRot( map->getXRot(), map->getYRot(), map->getZRot() );
		( ( GLShape* )pos->shape )->setCameraPos( map->getXPos(), map->getYPos(), map->getZPos(), xpos, ypos, zpos );
	}
	
	// ==========================================================================
	// effects
	if ( pos && pos->creature ) {
		// translate hack for md2 models... see: md2shape::draw()
		//glTranslatef( 0, -1 * MUL, 0 );
		pos->creature->getEffect()->draw( pos->creature->getEffectType(),
		                                    pos->creature->getDamageEffect() );
		//glTranslatef( 0, 1 * MUL, 0 );
	} else if ( effect ) {
		effect->getEffect()->draw( effect->getEffectType(),
		                                  effect->getDamageEffect() );
	}
	
	glPopMatrix();
}

void RenderedLocation::draw() {
	if( effectMode ) {
		drawEffect();
		return;
	}	
	
	// fog test for creatures
	if ( map->getHelper() && pos->creature && 
			!map->getAdapter()->isInMovieMode() && 
			!map->getHelper()->isVisible( pos->x, pos->y, pos->creature->getShape() ) ) {
		return;
	}

	( ( GLShape* )pos->shape )->useShadow = useShadow;

	// slow on mac os X:
	// glPushAttrib(GL_ENABLE_BIT);

	glPushMatrix();

	float heightPos = 0.0f;
	GLShape *s = ( GLShape* )pos->shape;
	if ( s->isVirtual() ) {
		s = ( ( VirtualShape* )s )->getRef();
	}

	if ( !s->getIgnoreHeightMap() ) {
		heightPos = pos->heightPos * MUL;
	}

	float xdiff = 0;
	float ydiff = 0;
	if ( pos->creature ) {
		xdiff = ( pos->creature->getX() - static_cast<float>( toint( pos->creature->getX() ) ) );
		ydiff = ( pos->creature->getY() - static_cast<float>( toint( pos->creature->getY() ) ) );
	}

	if ( useShadow ) {
		// put shadow above the floor a little
		glTranslatef( xpos + xdiff * MUL, ypos + ydiff * MUL, ( 0.26f * MUL + heightPos ) );
		glMultMatrixf( shadowTransformMatrix );
		// purple shadows
		map->getRender()->setupShadowColor();
	} else {
		glTranslatef( xpos + xdiff * MUL, ypos + ydiff * MUL, zpos + heightPos );

		glTranslatef( pos->moveX, pos->moveY, pos->moveZ );
		glRotatef( pos->angleX, 1.0f, 0.0f, 0.0f );
		glRotatef( pos->angleY, 0.0f, 1.0f, 0.0f );
		glRotatef( pos->angleZ, 0.0f, 0.0f, 1.0f );

		if ( pos->creature ) {
			glTranslatef( pos->creature->getOffsetX(), pos->creature->getOffsetY(), pos->creature->getOffsetZ() );
		}

		// show detected secret doors
		if ( pos ) {
			if ( map->isSecretDoor( pos ) && ( map->isSecretDoorDetected( pos ) || map->getSettings()->isGridShowing() ) ) {
				map->getRender()->setupSecretDoorColor();
				colorAlreadySet = true;
			}
		}

		if ( colorAlreadySet ) {
			colorAlreadySet = false;
		} else {
			if ( pos && map->isLocked( pos->x, pos->y, pos->z ) ) {
				map->getRender()->setupLockedDoorColor();
			} else {
				map->getRender()->setupShapeColor();
			}
		}
	}

	glDisable( GL_CULL_FACE );

	( ( GLShape* )pos->shape )->setCameraRot( map->getXRot(), map->getYRot(), map->getZRot() );
	( ( GLShape* )pos->shape )->setCameraPos( map->getXPos(), map->getYPos(), map->getZPos(), xpos, ypos, zpos );
	( ( GLShape* )pos->shape )->setLocked( map->isLocked( pos->x, pos->y, 0 ) );		
	
	// ==========================================================================
	// lights
	if( light ) {

		glPushMatrix();
		glRotatef( -map->getZRot(), 0.0f, 0.0f, 1.0f );
		glRotatef( -map->getYRot(), 1.0f, 0.0f, 0.0f );
		lightTex.glBind();
		float r;
		if( pos->shape->getLightEmitter() ) {
			r = pos->shape->getLightEmitter()->getRadius() * MUL;
			Color color = pos->shape->getLightEmitter()->getColor();
			glColor4f( color.r, color.g, color.b, color.a );
		} else {
			r = 8 * MUL;
			map->getRender()->setupPlayerLightColor();
		}		
		
		glBegin( GL_QUADS );
		glTexCoord2f( 1.0f, 1.0f );
		glVertex3f( -r, r, 0 );
		glTexCoord2f( 1.0f, 0.0f );
		glVertex3f( -r, -r, 0 );
		glTexCoord2f( 0.0f, 0.0f );
		glVertex3f( r, -r, 0 );
		glTexCoord2f( 0.0f, 1.0f );
		glVertex3f( r, r, 0 );		
		glEnd();
		glPopMatrix();
		
		// ==========================================================================
		// effects
	} else if ( pos->creature && !useShadow ) {
		// outline mission creatures
		if ( map->getAdapter()->isMissionCreature( pos->creature ) ) {
			pos->shape->outline( 0.15f, 0.15f, 0.4f );
		} else if ( pos->creature->isBoss() ) {
			pos->shape->outline( 0.4f, 0.15f, 0.4f );
		} else if ( pos->outlineColor ) {
			pos->shape->outline( pos->outlineColor );
		}
		
		if ( pos->creature->getStateMod( StateMod::invisible ) ) {
			glColor4f( 0.3f, 0.8f, 1.0f, 0.5f );
			glEnable( GL_BLEND );
			//glDepthMask( GL_FALSE );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		} else if ( pos->creature->getStateMod( StateMod::possessed ) ) {
			glColor4f( 1.0, 0.3f, 0.8f, 1.0f );
		}
		
		if( !pos->lightFacingSurfaces.empty() ) {
			pos->shape->setLightFacingSurfaces( &pos->lightFacingSurfaces );
		}
		pos->shape->draw();

		if ( pos->creature->getStateMod( StateMod::invisible ) ) {
			glDisable( GL_BLEND );
			//glDepthMask( GL_TRUE );
		}

		// ==========================================================================
		// items
	} else if ( pos->item && !useShadow ) {

		if ( pos->item->isSpecial() ) {
			pos->shape->outline( Constants::SPECIAL_ITEM_COLOR );
		} else if ( pos->item->isMagicItem() ) {
			pos->shape->outline( Constants::MAGIC_ITEM_COLOR[ pos->item->getMagicLevel() ] );
		} else if ( pos->item->getContainsMagicItem() ) {
			pos->shape->outline( 0.8f, 0.8f, 0.3f );
		}

		if ( pos->outlineColor ) {
			pos->shape->outline( pos->outlineColor );
		}
		
		if( !pos->lightFacingSurfaces.empty() ) {
			pos->shape->setLightFacingSurfaces( &pos->lightFacingSurfaces );
		}
		pos->shape->draw();


		// ==========================================================================
		// shapes
	} else {
		bool sides[6];
		findOccludedSides( sides );
		pos->shape->setOccludedSides( sides );
		if ( pos->outlineColor && !useShadow ) {
			pos->shape->outline( pos->outlineColor );
		}
		if ( pos->shape->getTextureCount() > 3 ) {
			// select which alternate texture to use
			pos->shape->setTextureIndex( pos->texIndex );
		}
		
		if( !pos->lightFacingSurfaces.empty() ) {
			pos->shape->setLightFacingSurfaces( &pos->lightFacingSurfaces );
		}		
		pos->shape->draw();
	}
	// ==========================================================================

#if DEBUG_MOUSE_POS == 1
	if ( shape && !useShadow && ( ( later && item ) || ( later && creature ) || shape->isInteractive() ) ) {
		glDisable( GL_DEPTH_TEST );
		glDepthMask( GL_FALSE );
		glDisable( GL_CULL_FACE );
		glDisable( GL_TEXTURE_2D );
		glColor4f( 1, 1, 1, 1 );
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0, 0, 0 );
		glVertex3f( 0, shape->getDepth() * MUL, 0 );
		glVertex3f( shape->getWidth() * MUL, shape->getDepth() * MUL, 0 );
		glVertex3f( shape->getWidth() * MUL, 0, 0 );
		glEnd();
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0, 0, shape->getHeight() * MUL );
		glVertex3f( 0, shape->getDepth() * MUL, shape->getHeight() * MUL );
		glVertex3f( shape->getWidth() * MUL, shape->getDepth() * MUL, shape->getHeight() * MUL );
		glVertex3f( shape->getWidth() * MUL, 0, shape->getHeight() * MUL );
		glEnd();
		glBegin( GL_LINES );
		glVertex3f( 0, 0, 0 );
		glVertex3f( 0, 0, shape->getHeight() * MUL );
		glEnd();
		glBegin( GL_LINES );
		glVertex3f( 0, shape->getDepth() * MUL, 0 );
		glVertex3f( 0, shape->getDepth() * MUL, shape->getHeight() * MUL );
		glEnd();
		glBegin( GL_LINES );
		glVertex3f( shape->getWidth() * MUL, shape->getDepth() * MUL, 0 );
		glVertex3f( shape->getWidth() * MUL, shape->getDepth() * MUL, shape->getHeight() * MUL );
		glEnd();
		glBegin( GL_LINES );
		glVertex3f( shape->getWidth() * MUL, 0, 0 );
		glVertex3f( shape->getWidth() * MUL, 0, shape->getHeight() * MUL );
		glEnd();
		glDepthMask( GL_TRUE );
		glEnable( GL_DEPTH_TEST );
	}
#endif

	// in the map editor outline virtual shapes
	if ( pos->shape->isVirtual() && map->getSettings()->isGridShowing() && map->isGridEnabled() ) {

		if ( heightPos > 1 ) {
			cerr << "heightPos=" << heightPos << " for virtual shape " << pos->shape->getName() << endl;
		}
		if ( pos && pos->z > 0 ) {
			cerr << "z=" << pos->z << " for virtual shape " << pos->shape->getName() << endl;
		}

		glColor4f( 0.75f, 0.75f, 1.0f, 1.0f );

		float z = ( pos->shape->getHeight() + 0.25f ) * MUL;
		float lowZ = 0.25f * MUL;

		float wm = pos->shape->getWidth() * MUL;
		float dm = pos->shape->getDepth() * MUL;

		glPushMatrix();
		glTranslatef( 0.0f, 20.0f, z );
		map->getAdapter()->texPrint( 0, 0, "virtual" );
		glPopMatrix();

		glDisable( GL_TEXTURE_2D );
		glBegin( GL_LINE_LOOP );
		glVertex3f( 0.0f, 0.0f, z );
		glVertex3f( 0.0f, dm, z );
		glVertex3f( wm, dm, z );
		glVertex3f( wm, 0.0f, z );

		glVertex3f( 0.0f, 0.0f, z );
		glVertex3f( 0.0f, 0.0f, lowZ );
		glVertex3f( wm, 0.0f, lowZ );
		glVertex3f( wm, 0.0f, z );

		glVertex3f( 0.0f, dm, z );
		glVertex3f( 0.0f, dm, lowZ );
		glVertex3f( wm, dm, lowZ );
		glVertex3f( wm, dm, z );

		glVertex3f( 0.0f, 0.0f, z );
		glVertex3f( 0.0f, 0.0f, lowZ );
		glVertex3f( 0.0f, dm, lowZ );
		glVertex3f( 0.0f, dm, z );

		glVertex3f( wm, 0.0f, z );
		glVertex3f( wm, 0.0f, lowZ );
		glVertex3f( wm, dm, lowZ );
		glVertex3f( wm, dm, z );

		glEnd();
		glEnable( GL_TEXTURE_2D );
	}

	glPopMatrix();

	// slow on mac os X
	// glPopAttrib();

	if ( pos->shape ) {
		pos->shape->setLightFacingSurfaces( NULL );
		( ( GLShape* )pos->shape )->useShadow = false;
	}	
}

/// Determines which sides of a shape are not visible for various reasons.

void RenderedLocation::findOccludedSides( bool *sides ) {
	if ( colorAlreadySet || !this->pos || !this->pos->shape || !this->pos->shape->isStencil() || ( this->pos->shape && map->isDoor( this->pos->shape ) ) ) {
		sides[Shape::BOTTOM_SIDE] = sides[Shape::N_SIDE] = sides[Shape::S_SIDE] =
			sides[Shape::E_SIDE] = sides[Shape::W_SIDE] = sides[Shape::TOP_SIDE] = true;
		return;
	}

	sides[Shape::BOTTOM_SIDE] = sides[Shape::N_SIDE] =
		sides[Shape::S_SIDE] = sides[Shape::E_SIDE] =
			sides[Shape::W_SIDE] = sides[Shape::TOP_SIDE] = false;

	int x, y;
	Location *pos;
	for ( x = this->pos->x; x < this->pos->x + this->pos->shape->getWidth(); x++ ) {
		y = this->y - this->pos->shape->getDepth();
		pos = map->getLocation( x, y, this->pos->z );
		if ( !pos || !pos->shape->isStencil() || ( !map->isLocationInLight( x, y, pos->shape ) && !map->isDoorType( pos->shape ) ) ) {
			sides[Shape::N_SIDE] = true;
		}

		y = this->y + 1;
		pos = map->getLocation( x, y, this->pos->z );
		if ( !pos || !pos->shape->isStencil() || ( !map->isLocationInLight( x, y, pos->shape ) && !map->isDoorType( pos->shape ) ) ) {
			sides[Shape::S_SIDE] = true;
		}

		if ( sides[Shape::N_SIDE] && sides[Shape::S_SIDE] ) {
			break;
		}
	}


	for ( y = this->pos->y - this->pos->shape->getDepth() + 1; y <= this->pos->y; y++ ) {
		x = this->x - 1;
		pos = map->getLocation( x, y, this->pos->z );
		if ( !pos || !pos->shape->isStencil() || ( !map->isLocationInLight( x, y, pos->shape ) && !map->isDoorType( pos->shape ) ) ) {
			sides[Shape::W_SIDE] = true;
		}

		x = this->x + this->pos->shape->getWidth();
		pos = map->getLocation( x, y, this->pos->z );
		if ( !pos || !pos->shape->isStencil() || ( !map->isLocationInLight( x, y, pos->shape ) && !map->isDoorType( pos->shape ) ) ) {
			sides[Shape::E_SIDE] = true;
		}

		if ( sides[Shape::W_SIDE] && sides[Shape::E_SIDE] ) {
			break;
		}
	}


	for ( x = this->pos->x; x < this->pos->x + this->pos->shape->getWidth(); x++ ) {
		for ( y = this->pos->y - this->pos->shape->getDepth() + 1; !sides[Shape::TOP_SIDE] && y <= this->pos->y; y++ ) {
			pos = map->getLocation( x, y, this->pos->z + this->pos->shape->getHeight() );
			if ( !pos || !pos->shape->isStencil() || ( !map->isLocationInLight( x, y, pos->shape ) && !map->isDoorType( pos->shape ) ) ) {
				sides[Shape::TOP_SIDE] = true;
				break;
			}
		}
	}	
}