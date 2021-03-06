/**
 * This code was originally written by Mark Kilgard (mjk@nvidia.com) (c) 1997
 * see: http://www.opengl.org/developers/code/mjktips/TexFont/TexFont.html
 */

/***************************************************************************
                  text.cpp  -  Font manager for .txf fonts
                             -------------------
    begin                : Mon Jul 7 2003
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
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "text.h"

using namespace std;

TexturedText::TexturedText() {
	useLuminanceAlpha = 0;
	filename = rootDir + "/fonts/default.txf";
	txf = txfLoadFont( filename );
	if ( txf == NULL ) {
		fprintf( stderr, "Problem loading %s\n", filename.c_str() );
		exit( 1 );
	}
}

TexturedText::~TexturedText() {
}






TexFont *TexturedText::txfLoadFont( string& filename ) {
	TexFont *txf = NULL;
	FILE *file;
	GLfloat w, h, xstep, ystep;
	char fileid[4], tmp;
	unsigned char *texbitmap;
	int min_glyph, max_glyph;
	int endianness, swap, format, stride, width, height;
	int i, j, got;

	file = fopen( filename.c_str(), "rb" );
	if ( file == NULL ) {
		lastError = "file open failed.";
		goto error;
	}
	txf = new TexFont;
	/* For easy cleanup in error case. */
	txf->tgi = NULL;
	txf->tgvi = NULL;
	txf->lut = NULL;
	txf->teximage = NULL;

	got = fread( fileid, 1, 4, file );
	if ( got != 4 || strncmp( fileid, "\377txf", 4 ) ) {
		lastError = "not a texture font file.";
		goto error;
	}
	assert( sizeof( int ) == 4 );  /* Ensure external file format size. */
	got = fread( &endianness, sizeof( int ), 1, file );
	if ( got == 1 && endianness == 0x12345678 ) {
		swap = 0;
	} else if ( got == 1 && endianness == 0x78563412 ) {
		swap = 1;
	} else {
		lastError = "not a texture font file.";
		goto error;
	}
#define EXPECT(n) if (got != n) { lastError = "premature end of file."; goto error; }
	got = fread( &format, sizeof( int ), 1, file );
	EXPECT( 1 );
	got = fread( &txf->tex_width, sizeof( int ), 1, file );
	EXPECT( 1 );
	got = fread( &txf->tex_height, sizeof( int ), 1, file );
	EXPECT( 1 );
	got = fread( &txf->max_ascent, sizeof( int ), 1, file );
	EXPECT( 1 );
	got = fread( &txf->max_descent, sizeof( int ), 1, file );
	EXPECT( 1 );
	got = fread( &txf->num_glyphs, sizeof( int ), 1, file );
	EXPECT( 1 );

	if ( swap ) {
		SWAPL( &format, tmp );
		SWAPL( &txf->tex_width, tmp );
		SWAPL( &txf->tex_height, tmp );
		SWAPL( &txf->max_ascent, tmp );
		SWAPL( &txf->max_descent, tmp );
		SWAPL( &txf->num_glyphs, tmp );
	}
	txf->tgi = new TexGlyphInfo[txf->num_glyphs];
	assert( sizeof( TexGlyphInfo ) == 12 );  /* Ensure external file format size. */
	got = fread( txf->tgi, sizeof( TexGlyphInfo ), txf->num_glyphs, file );
	EXPECT( txf->num_glyphs );

	if ( swap ) {
		for ( i = 0; i < txf->num_glyphs; i++ ) {
			SWAPS( &txf->tgi[i].c, tmp );
			SWAPS( &txf->tgi[i].x, tmp );
			SWAPS( &txf->tgi[i].y, tmp );
		}
	}
	txf->tgvi = new TexGlyphVertexInfo[txf->num_glyphs];
	w = txf->tex_width;
	h = txf->tex_height;
	xstep = 0.5 / w;
	ystep = 0.5 / h;
	for ( i = 0; i < txf->num_glyphs; i++ ) {
		TexGlyphInfo *tgi;

		tgi = &txf->tgi[i];
		txf->tgvi[i].t0[0] = tgi->x / w + xstep;
		txf->tgvi[i].t0[1] = tgi->y / h + ystep;
		txf->tgvi[i].v0[0] = tgi->xoffset;
		txf->tgvi[i].v0[1] = tgi->yoffset;
		txf->tgvi[i].t1[0] = ( tgi->x + tgi->width ) / w + xstep;
		txf->tgvi[i].t1[1] = tgi->y / h + ystep;
		txf->tgvi[i].v1[0] = tgi->xoffset + tgi->width;
		txf->tgvi[i].v1[1] = tgi->yoffset;
		txf->tgvi[i].t2[0] = ( tgi->x + tgi->width ) / w + xstep;
		txf->tgvi[i].t2[1] = ( tgi->y + tgi->height ) / h + ystep;
		txf->tgvi[i].v2[0] = tgi->xoffset + tgi->width;
		txf->tgvi[i].v2[1] = tgi->yoffset + tgi->height;
		txf->tgvi[i].t3[0] = tgi->x / w + xstep;
		txf->tgvi[i].t3[1] = ( tgi->y + tgi->height ) / h + ystep;
		txf->tgvi[i].v3[0] = tgi->xoffset;
		txf->tgvi[i].v3[1] = tgi->yoffset + tgi->height;
		txf->tgvi[i].advance = tgi->advance;
	}

	min_glyph = txf->tgi[0].c;
	max_glyph = txf->tgi[0].c;
	for ( i = 1; i < txf->num_glyphs; i++ ) {
		if ( txf->tgi[i].c < min_glyph ) {
			min_glyph = txf->tgi[i].c;
		}
		if ( txf->tgi[i].c > max_glyph ) {
			max_glyph = txf->tgi[i].c;
		}
	}
	txf->min_glyph = min_glyph;
	txf->range = max_glyph - min_glyph + 1;

	txf->lut = new TexGlyphVertexInfo*[txf->range];
	memset(txf->lut, 0, txf->range * sizeof(TexGlyphVertexInfo*));
	for ( i = 0; i < txf->num_glyphs; i++ ) {
		txf->lut[txf->tgi[i].c - txf->min_glyph] = &txf->tgvi[i];
	}

	switch ( format ) {
	case TXF_FORMAT_BYTE:
		if ( useLuminanceAlpha ) {
			unsigned char *orig = new unsigned char[txf->tex_width * txf->tex_height];
			got = fread( orig, 1, txf->tex_width * txf->tex_height, file );
			EXPECT( txf->tex_width * txf->tex_height );
			txf->teximage = new unsigned char[2 * txf->tex_width * txf->tex_height];
			for ( i = 0; i < txf->tex_width * txf->tex_height; i++ ) {
				txf->teximage[i * 2] = orig[i];
				txf->teximage[i * 2 + 1] = orig[i];
			}
			delete [] orig;
		} else {
			txf->teximage = new unsigned char[txf->tex_width * txf->tex_height];
			got = fread( txf->teximage, 1, txf->tex_width * txf->tex_height, file );
			EXPECT( txf->tex_width * txf->tex_height );
		}
		break;
	case TXF_FORMAT_BITMAP:
		width = txf->tex_width;
		height = txf->tex_height;
		stride = ( width + 7 ) >> 3;
		texbitmap = new unsigned char[stride * height];
		got = fread( texbitmap, 1, stride * height, file );
		EXPECT( stride * height );
		if ( useLuminanceAlpha ) {
			txf->teximage = new unsigned char[width * height * 2];
			memset( txf->teximage, 0, width * height * 2 * sizeof(char) );
			for ( i = 0; i < height; i++ ) {
				for ( j = 0; j < width; j++ ) {
					if ( texbitmap[i * stride + ( j >> 3 )] & ( 1 << ( j & 7 ) ) ) {
						txf->teximage[( i * width + j ) * 2] = 255;
						txf->teximage[( i * width + j ) * 2 + 1] = 255;
					}
				}
			}
		} else {
			txf->teximage = new unsigned char[width * height];
			memset( txf->teximage, 0, width * height * sizeof(char) );
			for ( i = 0; i < height; i++ ) {
				for ( j = 0; j < width; j++ ) {
					if ( texbitmap[i * stride + ( j >> 3 )] & ( 1 << ( j & 7 ) ) ) {
						txf->teximage[i * width + j] = 255;
					}
				}
			}
		}
		delete [] texbitmap;
		break;
	}

	fclose( file );
	return txf;

error:

	if ( txf ) {
		if ( txf->tgi )
			delete txf->tgi;
		if ( txf->tgvi )
			delete txf->tgvi;
		if ( txf->lut )
			delete txf->lut;
		if ( txf->teximage )
			delete txf->teximage;
		delete txf;
	}
	if ( file )
		fclose( file );
	return NULL;
}

GLuint TexturedText::txfEstablishTexture( GLuint texobj,
    GLboolean setupMipmaps ) {
	if ( txf->texobj == 0 ) {
		if ( texobj == 0 ) {
#if !defined(USE_DISPLAY_LISTS)
			glGenTextures( 1, &txf->texobj );
#else
			txf->texobj = glGenLists( 1 );
#endif
		} else {
			txf->texobj = texobj;
		}
	}
#if !defined(USE_DISPLAY_LISTS)
	glBindTexture( GL_TEXTURE_2D, txf->texobj );
#else
	glNewList( txf->texobj, GL_COMPILE );
#endif

#if 1
	/* XXX Indigo2 IMPACT in IRIX 5.3 and 6.2 does not support the GL_INTENSITY
	   internal texture format. Sigh. Win32 non-GLX users should disable this
	   code. */
	if ( useLuminanceAlpha == 0 ) {
		char *vendor, *renderer, *version;

		renderer = ( char * ) glGetString( GL_RENDERER );
		vendor = ( char * ) glGetString( GL_VENDOR );
		if ( !strcmp( vendor, "SGI" ) && !strncmp( renderer, "IMPACT", 6 ) ) {
			version = ( char * ) glGetString( GL_VERSION );
			if ( !strcmp( version, "1.0 Irix 6.2" ) ||
			        !strcmp( version, "1.0 Irix 5.3" ) ) {
				unsigned char *latex;
				int width = txf->tex_width;
				int height = txf->tex_height;
				int i;

				useLuminanceAlpha = 1;
				latex = new unsigned char[width * height * 2];
				memset(latex, 0, width * height * 2 * sizeof(char));
				/* XXX unprotected alloc. */
				for ( i = 0; i < height * width; i++ ) {
					latex[i * 2] = txf->teximage[i];
					latex[i * 2 + 1] = txf->teximage[i];
				}
				delete txf->teximage;
				txf->teximage = latex;
			}
		}
	}
#endif

	if ( useLuminanceAlpha ) {
		if ( setupMipmaps ) {
			gluBuild2DMipmaps( GL_TEXTURE_2D, GL_LUMINANCE_ALPHA,
			                   txf->tex_width, txf->tex_height,
			                   GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, txf->teximage );
		} else {
			glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA,
			              txf->tex_width, txf->tex_height, 0,
			              GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, txf->teximage );
		}
	} else {
#if defined(GL_VERSION_1_1) || defined(GL_EXT_texture)
		/* Use GL_INTENSITY4 as internal texture format since we want to use as
		   little texture memory as possible. */
		if ( setupMipmaps ) {
			gluBuild2DMipmaps( GL_TEXTURE_2D, GL_INTENSITY4,
			                   txf->tex_width, txf->tex_height,
			                   GL_LUMINANCE, GL_UNSIGNED_BYTE, txf->teximage );
		} else {
			glTexImage2D( GL_TEXTURE_2D, 0, GL_INTENSITY4,
			              txf->tex_width, txf->tex_height, 0,
			              GL_LUMINANCE, GL_UNSIGNED_BYTE, txf->teximage );
		}
#else
		abort();            /* Should not get here without EXT_texture or OpenGL
1.1. */
#endif
	}

#if defined(USE_DISPLAY_LISTS)
	glEndList();
	glCallList( txf->texobj );
#endif
	return txf->texobj;
}

void TexturedText::txfBindFontTexture() {
#if !defined(USE_DISPLAY_LISTS)
	glBindTexture( GL_TEXTURE_2D, txf->texobj );
#else
	glCallList( txf->texobj );
#endif
}

void TexturedText::txfUnloadFont() {
	if ( txf->teximage ) {
		delete txf->teximage;
	}
	delete [] txf->tgi;
	delete [] txf->tgvi;
	delete [] txf->lut;
	delete txf;
	txf = NULL;
}

void TexturedText::txfGetStringMetrics( char *string,
    int len,
    int *width,
    int *max_ascent,
    int *max_descent ) {
	TexGlyphVertexInfo *tgvi;
	int w, i;

	w = 0;
	for ( i = 0; i < len; i++ ) {
		if ( string[i] == 27 ) {
			switch ( string[i + 1] ) {
			case 'M':
				i += 4;
				break;
			case 'T':
				i += 7;
				break;
			case 'L':
				i += 7;
				break;
			case 'F':
				i += 13;
				break;
			}
		} else {
			tgvi = getTCVI( string[i] );
			w += static_cast<int>( tgvi->advance );
		}
	}
	*width = w;
	*max_ascent = txf->max_ascent;
	*max_descent = txf->max_descent;
}

void TexturedText::txfRenderGlyph( int c ) {
	TexGlyphVertexInfo *tgvi;

	glRotatef( 180.0f, 1.0f, 0.0f, 0.0f );
	tgvi = getTCVI( c );
	glBegin( GL_QUADS );
	glTexCoord2fv( tgvi->t0 );
	glVertex2sv( tgvi->v0 );
	glTexCoord2fv( tgvi->t1 );
	glVertex2sv( tgvi->v1 );
	glTexCoord2fv( tgvi->t2 );
	glVertex2sv( tgvi->v2 );
	glTexCoord2fv( tgvi->t3 );
	glVertex2sv( tgvi->v3 );
	glEnd();
	glRotatef( -180.0f, 1.0f, 0.0f, 0.0f );
	glTranslatef( tgvi->advance, 0.0f, 0.0f );
}

void TexturedText::txfRenderString( char *string,
    int len ) {
	int i;

	for ( i = 0; i < len; i++ ) {
		txfRenderGlyph( string[i] );
	}
}

void TexturedText::txfRenderFancyString( char *string,
    int len ) {
	TexGlyphVertexInfo *tgvi;
	GLubyte c[4][3];
	int mode = MONO;
	int i;

	for ( i = 0; i < len; i++ ) {
		if ( string[i] == 27 ) {
			switch ( string[i + 1] ) {
			case 'M':
				mode = MONO;
				glColor3ubv( ( GLubyte * ) & string[i + 2] );
				i += 4;
				break;
			case 'T':
				mode = TOP_BOTTOM;
				memcpy( c, &string[i + 2], 6 );
				i += 7;
				break;
			case 'L':
				mode = LEFT_RIGHT;
				memcpy( c, &string[i + 2], 6 );
				i += 7;
				break;
			case 'F':
				mode = FOUR;
				memcpy( c, &string[i + 2], 12 );
				i += 13;
				break;
			}
		} else {
			switch ( mode ) {
			case MONO:
				txfRenderGlyph( string[i] );
				break;
			case TOP_BOTTOM:
				tgvi = getTCVI( string[i] );
				glBegin( GL_QUADS );
				glColor3ubv( c[0] );
				glTexCoord2fv( tgvi->t0 );
				glVertex2sv( tgvi->v0 );
				glTexCoord2fv( tgvi->t1 );
				glVertex2sv( tgvi->v1 );
				glColor3ubv( c[1] );
				glTexCoord2fv( tgvi->t2 );
				glVertex2sv( tgvi->v2 );
				glTexCoord2fv( tgvi->t3 );
				glVertex2sv( tgvi->v3 );
				glEnd();
				glTranslatef( tgvi->advance, 0.0f, 0.0f );
				break;
			case LEFT_RIGHT:
				tgvi = getTCVI( string[i] );
				glBegin( GL_QUADS );
				glColor3ubv( c[0] );
				glTexCoord2fv( tgvi->t0 );
				glVertex2sv( tgvi->v0 );
				glColor3ubv( c[1] );
				glTexCoord2fv( tgvi->t1 );
				glVertex2sv( tgvi->v1 );
				glColor3ubv( c[1] );
				glTexCoord2fv( tgvi->t2 );
				glVertex2sv( tgvi->v2 );
				glColor3ubv( c[0] );
				glTexCoord2fv( tgvi->t3 );
				glVertex2sv( tgvi->v3 );
				glEnd();
				glTranslatef( tgvi->advance, 0.0f, 0.0f );
				break;
			case FOUR:
				tgvi = getTCVI( string[i] );
				glBegin( GL_QUADS );
				glColor3ubv( c[0] );
				glTexCoord2fv( tgvi->t0 );
				glVertex2sv( tgvi->v0 );
				glColor3ubv( c[1] );
				glTexCoord2fv( tgvi->t1 );
				glVertex2sv( tgvi->v1 );
				glColor3ubv( c[2] );
				glTexCoord2fv( tgvi->t2 );
				glVertex2sv( tgvi->v2 );
				glColor3ubv( c[3] );
				glTexCoord2fv( tgvi->t3 );
				glVertex2sv( tgvi->v3 );
				glEnd();
				glTranslatef( tgvi->advance, 0.0f, 0.0f );
				break;
			}
		}
	}
}

int TexturedText::txfInFont( int c ) {
	//TexGlyphVertexInfo *tgvi;

	/* NOTE: No uppercase/lowercase substituion. */
	if ( ( c >= txf->min_glyph ) && ( c < txf->min_glyph + txf->range ) ) {
		if ( txf->lut[c - txf->min_glyph] ) {
			return 1;
		}
	}
	return 0;
}

TexGlyphVertexInfo *TexturedText::getTCVI( int c ) {
	TexGlyphVertexInfo *tgvi;

	if ( !( ( c >= txf->min_glyph ) && ( c < txf->min_glyph + txf->range ) ) ) {
		c = 32; // space
	}

	/* Automatically substitute uppercase letters with lowercase if not
	   uppercase available (and vice versa). */
	if ( ( c >= txf->min_glyph ) && ( c < txf->min_glyph + txf->range ) ) {
		tgvi = txf->lut[c - txf->min_glyph];
		if ( tgvi ) {
			return tgvi;
		}
		if ( islower( c ) ) {
			c = toupper( c );
			if ( ( c >= txf->min_glyph ) && ( c < txf->min_glyph + txf->range ) ) {
				return txf->lut[c - txf->min_glyph];
			}
		}
		if ( isupper( c ) ) {
			c = tolower( c );
			if ( ( c >= txf->min_glyph ) && ( c < txf->min_glyph + txf->range ) ) {
				return txf->lut[c - txf->min_glyph];
			}
		}
		// last ditch effort
		c = 32; // space
		tgvi = txf->lut[c - txf->min_glyph];
		if ( tgvi ) {
			return tgvi;
		}
	}
	fprintf( stderr, "texfont: tried to access unavailable font character \"%c\" (%d)\n",
	         isprint( c ) ? c : ' ', c );
	abort();
	/* NOTREACHED */
	return NULL; // to silence warning
}

