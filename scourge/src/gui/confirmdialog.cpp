/***************************************************************************
                confirmdialog.cpp  -  Yes/No confirmation dialog
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
#include "confirmdialog.h"
#include "window.h"
#include "button.h"
#include "label.h"

#define CONFIRM_WIDTH 440
#define CONFIRM_HEIGHT 120

ConfirmDialog::ConfirmDialog( ScourgeGui *scourgeGui, char const* title ) {
	win = new Window( scourgeGui,
	                  ( scourgeGui->getScreenWidth() / 2 ) - ( CONFIRM_WIDTH / 2 ),
	                  ( scourgeGui->getScreenHeight() / 2 ) - ( CONFIRM_HEIGHT / 2 ),
	                  CONFIRM_WIDTH, CONFIRM_HEIGHT,
	                  ( char* )( title ? title : _( "Confirmation" ) ),
	                  scourgeGui->getGuiTexture(), true,
	                  Window::BASIC_WINDOW,
	                  scourgeGui->getGuiTexture2() );
	int mx = CONFIRM_WIDTH / 2;
	okButton = new Button( mx - 80, 60, mx - 10, 80, scourgeGui->getHighlightTexture(), _( "Ok" ) );
	win->addWidget( ( Widget* )okButton );
	cancelButton = new Button( mx + 10, 60, mx + 80, 80, scourgeGui->getHighlightTexture(), _( "No" ) );
	win->addWidget( ( Widget* )cancelButton );
	label = new Label( 20, 30, "" );
	win->addWidget( ( Widget* )label );
	win->setEscapeHandler( cancelButton );

	mode = 0;
	object = NULL;
}

ConfirmDialog::~ConfirmDialog() {
	delete win;
}

void ConfirmDialog::setText( char *text ) {
	label->setText( text );
	int textWidth = win->getScourgeGui()->textWidth( text );
	label->move( ( CONFIRM_WIDTH - textWidth ) / 2, 30 );
}

void ConfirmDialog::setVisible( bool b ) {
	win->setVisible( b );
}

bool ConfirmDialog::isVisible() {
	return win->isVisible();
}

