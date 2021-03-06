/***************************************************************************
                pceditor.h  -  The "recruit character" window
                             -------------------
    begin                : Tue Jul 10 2006
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

#ifndef PC_EDITOR_H
#define PC_EDITOR_H
#pragma once

#include "gui/widgetview.h"
#include "gui/eventhandler.h"
#include "rpg/rpg.h"

class Window;
class Scourge;
class Creature;
class CardContainer;
class Button;
class Widget;
class TextField;
class ScrollingList;
class ScrollingLabel;
class Label;
class Canvas;
class CharacterInfoUI;
class Character;

/// The window that lets you edit a character's skills before recruiting him/her.
class PcEditor : public EventHandler {
private:
	Window *win;
	Scourge *scourge;
	Creature *creature;
	bool deleteCreature;
	CardContainer *cards;
	Button *nameButton, *profButton, *statsButton, *deityButton;
	Button *imageButton, *okButton, *cancelButton;
	TextField *nameField;
	Button *nameChangeButton;
	ScrollingList *charType;
	ScrollingLabel *charTypeDescription;
	Label *skillValue[10];
	Button *skillPlus[10], *skillMinus[10];
	Label *remainingCaptionLabel, *remainingLabel;
	CharacterInfoUI *detailsInfo;
	Canvas *detailsCanvas;
	ScrollingLabel *deityTypeDescription;
	ScrollingList *deityType;
	Canvas *portrait;
	Button *prevPortrait, *nextPortrait;
	Button *male, *female;
	Canvas *model;
	Button *prevModel, *nextModel;
	Button *reroll;
	std::string* charTypeStr;
	std::string* deityTypeStr;
	int portraitIndex;
	int modelIndex;
	int availableSkillMod;

	enum {
		NAME_TAB = 0,
		CLASS_TAB,
		STAT_TAB,
		DEITY_TAB,
		IMAGE_TAB
	};

public:
	PcEditor( Scourge *scourge );
	~PcEditor();

	inline Window *getWindow() {
		return win;
	}

	bool handleEvent( Widget *widget, SDL_Event *event );

	/// Widget::Draw handlers for portrait and model
	bool onDrawPortrait( Widget* w );
	bool onDrawModel( Widget* w );

	inline Button *getOkButton() {
		return okButton;
	}
	inline Button *getCancelButton() {
		return cancelButton;
	}

	void saveUI();
	void rollSkills();
	void rollSkillsForCreature( Creature *c );
	Creature *createPartyMember();

	void setCreature( Creature *creature = NULL, bool isEditable = true );
	inline Creature *getCreature() {
		return creature;
	}

protected:
	void deleteLoadedShapes();
	void loadUI();
	void setCharType( int charIndex );
	void setDeityType( int deityIndex );
	void createUI();
	int getSex();
	void rollApperance();
};

#endif

