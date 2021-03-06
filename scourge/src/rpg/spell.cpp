/***************************************************************************
                spell.cpp  -  Spell and magic school classes
                             -------------------
    begin                : Sun Sep 28 2003
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
#include "spell.h"
#include "rpg.h"

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

using namespace std;

MagicSchool *MagicSchool::schools[10];
int MagicSchool::schoolCount = 0;
map<string, Spell*> Spell::spellMap;
map<string, MagicSchool*> MagicSchool::schoolMap;


// FIXME: move this to some other location if needed elsewhere (rpg/dice.cpp?)
Dice::Dice( char const* s ) {
	strcpy( this->s, s );

	char tmp[ S_SIZE ];
	strcpy( tmp, s );
	if ( strchr( tmp, 'd' ) ) {
		count = atoi( strtok( tmp, "d" ) );
		sides = atoi( strtok( NULL, "+" ) );
		char *p = strtok( NULL, "+" );
		if ( p ) mod = atoi( p );
		else mod = 0;
	} else {
		count = 0;
		sides = 0;
		mod = atoi( s );
	}

	//  cerr << "DICE: count=" << count << " sides=" << sides << " mod=" << mod << " str=" << this->s << endl;
	//  cerr << "\troll 1:" << roll() << endl;
	//  cerr << "\troll 2:" << roll() << endl;
	//  cerr << "\troll 3:" << roll() << endl;
}

Dice::Dice( int count, int sides, int mod ) {
	this->count = count;
	this->sides = sides;
	this->mod = mod;
	if ( mod ) {
		snprintf( s, S_SIZE, "%dd%d+%d", count, sides, mod );
	} else {
		snprintf( s, S_SIZE, "%dd%d", count, sides );
	}
}

Dice::~Dice() {
}




MagicSchool::MagicSchool( char const* name, char const* displayName, char const* deity, int skill, int resistSkill, float alignment, float red, float green, float blue, char const* symbol ) {
	this->name = name;
	this->displayName = displayName;
	this->shortName = this->name.substr(0, this->name.find(' '));
	this->deity = deity;
	strcpy( this->deityDescription, "" );
	this->skill = skill;
	this->resistSkill = resistSkill;
	this->red = red;
	this->green = green;
	this->blue = blue;
	this->baseAlignment = alignment;
	this->symbol = symbol;
}

MagicSchool::~MagicSchool() {
	for ( size_t i = 0; i < spells.size(); ++i ) {
		delete spells[i];
	}
	spells.clear(); 
}

#define UPDATE_MESSAGE N_("Loading Spells")

void MagicSchool::initMagic() {
	ConfigLang *config = ConfigLang::load( "config/spell.cfg" );
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "magic_school" );

	MagicSchool *current = NULL;
	Spell *currentSpell = NULL;
	char name[255], displayName[255], notes[255], dice[255];
	char line[2000], symbol[255], targetTypeStr[255], align[255] /*, prereqName[255]*/;
	float alignment;

	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		config->setUpdate( _( UPDATE_MESSAGE ), i, v->size() );

		strcpy( name, node->getValueAsString( "name" ) );
		strcpy( displayName, node->getValueAsString( "display_name" ) );
		strcpy( notes, node->getValueAsString( "deity" ) );
		int skill = Skill::getSkillIndexByName( node->getValueAsString( "skill" ) );
		int resistSkill = Skill::getSkillIndexByName( node->getValueAsString( "resist_skill" ) );
		strcpy( line, node->getValueAsString( "rgb" ) );
		strcpy( align, node->getValueAsString( "base_alignment" ) );
		if ( strcmp( align, "chaotic" ) == 0 ) {
			alignment = ALIGNMENT_CHAOTIC;
		} else if ( strcmp( align, "neutral" ) == 0 ) {
			alignment = ALIGNMENT_NEUTRAL;
		} else if ( strcmp( align, "lawful" ) == 0 ) {
			alignment = ALIGNMENT_LAWFUL;
		}
		float red = static_cast<float>( strtod( strtok( line, "," ), NULL ) );
		float green = static_cast<float>( strtod( strtok( NULL, "," ), NULL ) );
		float blue = static_cast<float>( strtod( strtok( NULL, "," ), NULL ) );
		strcpy( symbol, node->getValueAsString( "symbol" ) );

		current = new MagicSchool( name, displayName, notes, skill, resistSkill, alignment, red, green, blue, symbol );

		strcpy( line, node->getValueAsString( "deity_description" ) );
		if ( strlen( line ) ) current->addToDeityDescription( line );

		schools[schoolCount++] = current;
		string nameStr = name;
		schoolMap[nameStr] = current;

		vector<ConfigNode*> *vv = node->getChildrenByName( "low_donate" );
		for ( unsigned int i = 0; vv && i < vv->size(); i++ ) {
			ConfigNode *node2 = ( *vv )[i];
			current->lowDonate.push_back( node2->getValueAsString( "text" ) );
		}
		vv = node->getChildrenByName( "neutral_donate" );
		for ( unsigned int i = 0; vv && i < vv->size(); i++ ) {
			ConfigNode *node2 = ( *vv )[i];
			current->neutralDonate.push_back( node2->getValueAsString( "text" ) );
		}
		vv = node->getChildrenByName( "high_donate" );
		for ( unsigned int i = 0; vv && i < vv->size(); i++ ) {
			ConfigNode *node2 = ( *vv )[i];
			current->highDonate.push_back( node2->getValueAsString( "text" ) );
		}

		vv = node->getChildrenByName( "spell" );
		for ( unsigned int i = 0; vv && i < vv->size(); i++ ) {
			ConfigNode *node2 = ( *vv )[i];

			strcpy( name, node2->getValueAsString( "name" ) );
			strcpy( displayName, node2->getValueAsString( "display_name" ) );
			strcpy( symbol, node2->getValueAsString( "symbol" ) );
			int level = node2->getValueAsInt( "level" );
			int mp = node2->getValueAsInt( "mp" );
			int exp = node2->getValueAsInt( "mp" );
			int failureRate = node2->getValueAsInt( "failureRate" );
			strcpy( dice, node2->getValueAsString( "action" ) );
			
			int distance = node2->getValueAsInt( "distance" );
			if ( distance < static_cast<int>( MIN_DISTANCE ) )
				distance = static_cast<int>( MIN_DISTANCE );
			int targetType = ( !strcmp( node2->getValueAsString( "targetType" ), "single" ) ? SINGLE_TARGET : GROUP_TARGET );
			int speed = node2->getValueAsInt( "speed" );
			int effect = Constants::getEffectByName( node2->getValueAsString( "effect" ) );
			strcpy( targetTypeStr, node2->getValueAsString( "target" ) );
			bool creatureTarget = ( strchr( targetTypeStr, 'C' ) != NULL );
			bool locationTarget = ( strchr( targetTypeStr, 'L' ) != NULL );
			bool itemTarget = ( strchr( targetTypeStr, 'I' ) != NULL );
			bool partyTarget = ( strchr( targetTypeStr, 'P' ) != NULL );
			bool doorTarget = ( strchr( targetTypeStr, 'D' ) != NULL );
			strcpy( line, node2->getValueAsString( "icon" ) );
			int iconTileX = atoi( strtok( line, "," ) ) - 1;
			int iconTileY = atoi( strtok( NULL, "," ) ) - 1;

			// Friendly/Hostile marker
			bool friendly = node2->getValueAsBool( "friendly" );

			int stateModPrereq = -1;
			strcpy( line, node2->getValueAsString( "prerequisite" ) );
			if ( strlen( line ) ) {
				// is it a potion state mod?
				int n = Constants::getPotionSkillByName( line );
				if ( n == -1 ) {
					StateMod *mod = StateMod::getStateModByName( line );
					if ( !mod ) {
						cerr << "Can't find state mod: >" << line << "<" << endl;
						exit( 1 );
					}
					n = mod->getIndex();
				}
				stateModPrereq = n;
				if ( stateModPrereq == -1 ) {
					cerr << "Error: spell=" << name << endl;
					cerr << "\tCan't understand prereq for spell: " << line << endl;
				}
			}

			currentSpell = new Spell( name, displayName, symbol, level, mp, exp, failureRate,
			                          dice, distance, targetType, speed, effect,
			                          creatureTarget, locationTarget, itemTarget, partyTarget, doorTarget,
			                          current, iconTileX, iconTileY,
			                          friendly, stateModPrereq );
			current->addSpell( currentSpell );

			strcpy( line, node2->getValueAsString( "sound" ) );
			if ( strlen( line ) ) currentSpell->setSound( line );

			strcpy( line, node2->getValueAsString( "notes" ) );
			if ( strlen( notes ) ) currentSpell->addNotes( line );
		}
	}

	delete config;
}


void MagicSchool::unInitMagic() {
	for ( int i = 0; i < schoolCount; ++i ) {
		delete schools[i];
		schools[i] = NULL;
	}
	schoolCount = 0;

}

const char *MagicSchool::getRandomString( vector<string> *v ) {
	return ( *v )[ Util::dice( v->size() ) ].c_str();
}

/// Message when you donated a small sum to the school's deity.

const char *MagicSchool::getLowDonateMessage() {
	return getRandomString( &lowDonate );
}

/// Message when you donated a moderate sum to the school's deity.

const char *MagicSchool::getNeutralDonateMessage() {
	return getRandomString( &neutralDonate );
}

/// Message when you donated a high sum to the school's deity.

const char *MagicSchool::getHighDonateMessage() {
	return getRandomString( &highDonate );
}

/// Returns a random spell from a random school.

Spell *MagicSchool::getRandomSpell( int level ) {
	int n = Util::dice( getMagicSchoolCount() );
	int c = getMagicSchool( n )->getSpellCount();
	if ( c > 0 ) {
		return getMagicSchool( n )->getSpell( Util::dice( c ) );
	} else {
		return NULL;
	}
}



Spell::Spell( char const* name, char const* displayName, char const* symbol, int level, int mp, int exp, int failureRate, char const* action,
              int distance, int targetType, int speed, int effect, bool creatureTarget,
              bool locationTarget, bool itemTarget, bool partyTarget, bool doorTarget,
              MagicSchool *school,
              int iconTileX, int iconTileY, bool friendly, int stateModPrereq ) 
		: action( action )
		, sound() {
	this->name = name;
	this->displayName = displayName;
	this->symbol = symbol;
	this->level = level;
	this->mp = mp;
	this->exp = exp;
	this->failureRate = failureRate;
	this->distance = distance;
	this->targetType = targetType;
	this->speed = speed;
	this->effect = effect;
	this->creatureTarget = creatureTarget;
	this->locationTarget = locationTarget;
	this->itemTarget = itemTarget;
	this->partyTarget = partyTarget;
	this->doorTarget = doorTarget;
	this->school = school;
	this->iconTileX = iconTileX;
	this->iconTileY = iconTileY;
	this->friendly = friendly;
	this->stateModPrereq = stateModPrereq;

	strcpy( this->notes, "" );

	string s = name;
	spellMap[s] = this;
}

Spell::~Spell() {
}

/// Returns a spell by its name.

Spell *Spell::getSpellByName( char *name ) {
	string s = name;
	if ( spellMap.find( s ) == spellMap.end() ) {
		cerr << "Warning: can't find spell: " << name << endl;
		return NULL;
	}
	return spellMap[s];
}

/// Min and max damage the spell does.

void Spell::getAttack( int casterLevel, float *maxP, float *minP ) {
	*minP = 0; *maxP = 0;
	for ( int i = 0; i <= ( casterLevel / 2 ); i++ ) {
		*minP += action.getMin();
		*maxP += action.getMax();
	}
}
