/***************************************************************************
                          sqbinding.cpp  -  description
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
#include "sqbinding.h"
#include "../session.h"
#include "sqgame.h"
#include "../squirrel/squirrel.h"
#include "../squirrel/sqstdio.h"
#include "../squirrel/sqstdaux.h"

#ifdef SQUNICODE
#define scvprintf vwprintf
#else
#define scvprintf vprintf
#endif

/**
 * A simple print function. Later replace this by printing in the console.
 */
void printfunc(HSQUIRRELVM v, const SQChar *s, ...) {
  va_list arglist;
  va_start(arglist, s);
  scvprintf(s, arglist);
  va_end(arglist);
}

SqBinding::SqBinding( Session *session ) {
  this->session = session;
  cerr << "Initializing squirrel vm" << endl;
  vm = sq_open(1024); //creates a VM with initial stack size 1024

  sqstd_seterrorhandlers( vm );

  //sets the print function
  sq_setprintfunc( vm, printfunc );

  // push the root table(were the globals of the script will be stored)
  sq_pushroottable( vm );

  // Define some squirrel classes
  cerr << "Creating ScourgeGame class" << endl;
  game = new SqGame();
  createClass( game->getClassDeclaration() );
}

SqBinding::~SqBinding() {
  sq_pop( vm, 1 ); //pops the root table
  sq_close( vm );
}




void SqBinding::startGame() {
  // Create a game instance (root squirrel object)
  instantiateClass( _SC( game->getClassName() ), &refGame );
}

void SqBinding::endGame() {
  // release ref.
  sq_release( vm, &refGame );
}






                        
bool SqBinding::compile( const char *filename ) {
  // compile a module
  cerr << "Compiling file:" << filename << endl;
  if( SQ_SUCCEEDED( sqstd_dofile( vm, _SC( filename ), 0, 1 ) ) ) {
    cerr << "\tSuccess." << endl;
    /*
    callSayHello( v, _SC( "Gabor" ) );
    int value = 3;
    cerr << "someOtherFunction(" << value << ")=" << callSomeOtherFunction( v, value ) << endl;
    
    // callback
    SomeClass *sc = new SomeClass();
    register_global_func( v, print_args, "print_args" );
    register_global_func( v, SomeClass::square, "square" );
    callCallbackToC( v );
    
    // create a class accessible by squirrel
    CreateClass(v, &(__Scourge_decl));
    
    // create a userpointer
    HSQOBJECT scourgeObj;
    CreateNativeClassInstance(v,_SC( "Scourge" ), &scourgeObj );
    
    callCallbackToObj( v, scourgeObj );
    */
    return true;
  } else {
    cerr << "Failed to compile file." << endl;
    return false;
  }
}   

bool SqBinding::createClass( SquirrelClassDecl *cd ) {
  int n = 0;
  int oldtop = sq_gettop( vm );
  sq_pushroottable( vm );
  sq_pushstring( vm, cd->name, -1 );
  if( cd->base ) {
    sq_pushstring( vm, cd->base, -1 );
    if( SQ_FAILED( sq_get( vm, -3 ) ) ) {
      sq_settop( vm, oldtop );
      return false;
    }
  }
  if( SQ_FAILED( sq_newclass( vm, cd->base ? 1 : 0 ) ) ) {
    sq_settop( vm, oldtop );
    return false;
  }
  //sq_settypetag(v,-1,(unsigned int)cd);
  sq_settypetag( vm, -1, (SQUserPointer)cd );
  const ScriptClassMemberDecl *members = cd->members;
  const ScriptClassMemberDecl *m = NULL;
  while( members[ n ].name ) {
    m = &members[ n ];
    sq_pushstring( vm, m->name, -1 );
    sq_newclosure( vm, m->func, 0 );
    sq_setparamscheck( vm, m->params, m->typemask );
    sq_setnativeclosurename( vm, -1, m->name );
    sq_createslot( vm, -3 );
    n++;
  }
  sq_createslot( vm, -3 );
  sq_pop( vm, 1 );
  return true;
}

bool SqBinding::instantiateClass( const SQChar *classname, 
                                  HSQOBJECT *obj 
                                  //,SQRELEASEHOOK hook 
                                  ) {
  SQUserPointer ud;
  int oldtop = sq_gettop( vm );
  sq_pushroottable( vm );
  sq_pushstring( vm, classname, -1 );
  if( SQ_FAILED( sq_rawget( vm, -2 ) ) ) {
    sq_settop( vm, oldtop );
    return false;
  }
  //sq_pushroottable(v);
  if( SQ_FAILED( sq_createinstance( vm, -1 ) ) ) {
    sq_settop( vm, oldtop );
    return false;
  }
  sq_remove( vm, -3 ); //removes the root table
  sq_remove( vm, -2 ); //removes the the class
  if( SQ_FAILED( sq_setinstanceup( vm, -1, ud ) ) ) {
    sq_settop( vm, oldtop );
    return false;
  }
  //      sq_setreleasehook(v,-1,hook);
  //sq_settop(v,oldtop);
  
  // create a squirrel object
  sq_getstackobj( vm, -1, obj );
  // make sure it's not gc-ed
  sq_addref( vm, obj );

  // be sure to release it later!!!
  //sq_release(SquirrelVM::_VM,&_o);
  //_o = t;
  
  //              sq_pop( v, 1 );
  sq_settop( vm, oldtop );
  
  return true;
}   

