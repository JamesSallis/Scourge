/***************************************************************************
                          sqcreature.h  -  description
                             -------------------
    begin                : Sat Oct 8 2005
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

#ifndef SQCREATURE_H
#define SQCREATURE_H

#include "sqbinding.h"
#include "sqobject.h"

using namespace std;

/**
 * The topmost squirrel object in the Scourge object hierarchy.
 */
class SqCreature : public SqObject {
private:
  static const char *className;
  static SquirrelClassDecl classDecl;
  static ScriptClassMemberDecl members[];

public:
  SqCreature();
  ~SqCreature();

  inline const char *getInstanceName() { return "Creature"; }
  inline const char *getClassName() { return SqCreature::className; }
  inline SquirrelClassDecl *getClassDeclaration() { return &SqCreature::classDecl; }

  // ===========================================================================
  // Static callback methods to Creature squirrel object member functions.
  static int _squirrel_typeof( HSQUIRRELVM vm );
  static int _constructor( HSQUIRRELVM vm );

  // general
  static int _getName( HSQUIRRELVM vm );
};

#endif

