/***************************************************************************
                    file.h  -  Platform agnostic file I/O
                             -------------------
    begin                : Sat Jul 29, 2005
    copyright            : (C) 2005 by Gabor Torok
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

#ifndef FILE_H
#define FILE_H
#pragma once

#include <SDL_endian.h>
#include <SDL_rwops.h>

// How Scourge save/loads data
#define SCOURGE_BYTE_ORDER SDL_BIG_ENDIAN
//#define SCOURGE_BYTE_ORDER SDL_LIL_ENDIAN

#define MAX_BUFF_SIZE 1024

/// A custom file object.
class File {
private:
	FILE *fp;
	SDL_RWops *rwops;
	Uint32 tmp32[MAX_BUFF_SIZE];
	Uint16 tmp16[MAX_BUFF_SIZE];

public:
	File( FILE *fp );
	virtual ~File();

	virtual int write( Uint32 *n, int count = 1 );
	virtual int write( Uint16 *n, int count = 1 );
	virtual int write( Uint8 *n, int count = 1 );

	virtual int read( Uint32 *n, int count = 1 );
	virtual int read( Uint16 *n, int count = 1 );
	virtual int read( Uint8 *n, int count = 1 );

	virtual void close();

protected:
	virtual int write( void *buff, size_t size, int count );
	virtual int read( void *buff, size_t size, int count );

};

#endif

