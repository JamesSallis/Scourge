/***************************************************************************
                          rpglib.cpp  -  description
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
#include "../common/constants.h"
#include "rpg.h"
#include "rpgitem.h"

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

using namespace std;

map<string, Skill*> Skill::skillsByName;
vector<Skill*> Skill::skills;

SkillGroup *SkillGroup::stats;
map<string, SkillGroup *> SkillGroup::groupsByName;
vector<SkillGroup*> SkillGroup::groups;

vector<std::string> Rpg::firstSyl;
vector<std::string> Rpg::midSyl;
vector<std::string> Rpg::endSyl;

map<string, StateMod*> StateMod::stateModsByName;
vector<StateMod*> StateMod::stateMods;
vector<StateMod*> StateMod::goodStateMods;
vector<StateMod*> StateMod::badStateMods;

void Rpg::initSkills( ConfigLang *config ) {
	Skill *lastSkill = NULL;
	SkillGroup *lastGroup = NULL;
	char line[255];
	char skillName[80], skillDisplayName[255], skillSymbol[80], skillDescription[255], skillAlign[32];
	char groupName[80], groupDisplayName[255], groupDescription[255];
	float alignment;

	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "group" );

	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		config->setUpdate( _( "Loading Skills" ), i, v->size() );

		strcpy( groupName, node->getValueAsString( "name" ) );
		strcpy( groupDisplayName, node->getValueAsString( "display_name" ) );
		strcpy( groupDescription, node->getValueAsString( "description" ) );

		lastGroup = new SkillGroup( groupName, groupDisplayName, groupDescription );

		vector<ConfigNode*> *vv = node->getChildrenByName( "skill" );
		for ( unsigned int i = 0; vv && i < vv->size(); i++ ) {
			ConfigNode *skillNode = ( *vv )[i];

			strcpy( skillName, skillNode->getValueAsString( "name" ) );
			strcpy( skillDisplayName, skillNode->getValueAsString( "display_name" ) );
			strcpy( skillSymbol, skillNode->getValueAsString( "symbol" ) );
			strcpy( skillDescription, skillNode->getValueAsString( "description" ) );
			strcpy( skillAlign, node->getValueAsString( "alignment" ) );
			if ( strcmp( skillAlign, "chaotic" ) == 0 ) {
				alignment = ALIGNMENT_CHAOTIC;
			} else if ( strcmp( skillAlign, "lawful" ) == 0 ) {
				alignment = ALIGNMENT_LAWFUL;
			} else {
				alignment = ALIGNMENT_NEUTRAL;
			}
			lastSkill = new Skill( skillName, skillDisplayName, skillDescription, skillSymbol, alignment, lastGroup );

			lastSkill->setPreReqMultiplier( skillNode->getValueAsInt( "prereq_multiplier" ) );
			strcpy( line, skillNode->getValueAsString( "prereq_skills" ) );
			char *p = strtok( line, "," );
			while ( p ) {
				Skill *stat = Skill::getSkillByName( p );
				if ( !stat ) {
					cerr << "*** Error: Can't find stat named: " << p << endl;
					exit( 1 );
				}
				lastSkill->addPreReqStat( stat );
				p = strtok( NULL, "," );
			}
		}
	}
}

void Rpg::initNames( ConfigLang *config ) {
	char line[4000];
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "names" );
	if ( v ) {
		ConfigNode *node = ( *v )[0];
		strcpy( line, node ->getValueAsString( "first" ) );
		char *p = strtok( line, "," );
		while ( p != NULL ) {
			firstSyl.push_back( p );
			//cerr << "first: " << firstSyl[ firstSyl.size() - 1 ] << endl;
			p = strtok( NULL, "," );
		}
		strcpy( line, node ->getValueAsString( "middle" ) );
		p = strtok( line, "," );
		while ( p != NULL ) {
			midSyl.push_back( p );
			//cerr << "mid: " << midSyl[ midSyl.size() - 1 ] << endl;
			p = strtok( NULL, "," );
		}
		strcpy( line, node ->getValueAsString( "last" ) );
		p = strtok( line, "," );
		while ( p != NULL ) {
			endSyl.push_back( p );
			//cerr << "last: " << endSyl[ endSyl.size() - 1 ] << endl;
			p = strtok( NULL, "," );
		}
	}
	//cerr << "first: " << firstSyl.size() << " mid: " << midSyl.size() << " end: " << endSyl.size() << endl;
}

void Rpg::initStateMods( ConfigLang *config ) {
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "state-mod" );

	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		config->setUpdate( _( "Loading StateMods" ), i, v->size() );

		string name = node->getValueAsString( "name" );
		int type = ( !strcmp( node->getValueAsString( "type" ), "bad" ) ?
		             StateMod::BAD :
		             ( !strcmp( node->getValueAsString( "type" ), "good" ) ? StateMod::GOOD : StateMod::NEITHER ) );
		StateMod *stateMod = new StateMod( name.c_str(),
		    node->getValueAsString( "display_name" ),
		    node->getValueAsString( "symbol" ),
		    node->getValueAsString( "setstate" ),
		    node->getValueAsString( "unsetstate" ),
		    type,
		    static_cast<int>( StateMod::stateMods.size() ) );
		StateMod::stateMods.push_back( stateMod );
		StateMod::stateModsByName[ name ] = stateMod;
		if ( type == StateMod::GOOD ) StateMod::goodStateMods.push_back( stateMod );
		if ( type == StateMod::BAD ) StateMod::badStateMods.push_back( stateMod );
	}
	//cerr << "** Read " << StateMod::stateMods.size() << " state mods." << endl;
}

void Rpg::initRpg() {
	ConfigLang *config = ConfigLang::load( "config/rpg.cfg" );
	initSkills( config );
	initStateMods( config );
	initNames( config );
	delete config;
}

void Rpg::unInitRpg() {
	for ( size_t i = 0; i < StateMod::stateMods.size(); ++i ) {
		delete StateMod::stateMods[i];
	}
	StateMod::stateMods.clear();
	StateMod::stateModsByName.clear();
	for ( size_t i = 0; i < SkillGroup::groups.size(); ++i ) {
		delete SkillGroup::groups[i];
	}
	SkillGroup::groups.clear();

}

// Create a random, cheeseball, fantasy name
std::string Rpg::createName() {
	string ret = firstSyl[ Util::dice( firstSyl.size() ) ];
	int sylCount = Util::dice( 3 );
	for ( int i = 0; i < sylCount; i++ ) {
		ret += midSyl[ Util::dice( midSyl.size() ) ];
	}
	if ( 0 == Util::dice( 2 ) ) {
		ret += endSyl[ Util::dice( endSyl.size() ) ];
	}
	return ret;
}


Skill::Skill( char *name, char *displayName, char *description, char *symbol, float alignment, SkillGroup *group ) {
	strcpy( this->name, name );
	strcpy( this->displayName, displayName );
	strcpy( this->description, description );
	strcpy( this->symbol, symbol );
	this->alignment = alignment;
	this->group = group;

	// store the skill
	this->index = static_cast<int>( skills.size() );
	skills.push_back( this );
	string s = name;
	skillsByName[ s ] = this;
	group->addSkill( this );
}

Skill::~Skill() {
}

SkillGroup::SkillGroup( char *name, char *displayName, char *description ) {
	strcpy( this->name, name );
	strcpy( this->displayName, displayName );
	strcpy( this->description, description );
	// hack: first group is the stats
	this->isStatSkill = groups.empty();
	if ( isStatSkill ) stats = this;

	// store the group
	this->index = static_cast<int>( groups.size() );
	groups.push_back( this );
	string s = name;
	groupsByName[ s ] = this;
}

SkillGroup::~SkillGroup() {
	for ( size_t i = 0; i < skills.size(); ++i ) {
		delete skills[i];
	}
	skills.clear();
}

StateMod::StateMod( char const* name, char const* displayName
                    , char const* symbol, char const* setState
                    , char const* unsetState, int type, int index ) {
	strcpy( this->name, name );
	strcpy( this->displayName, displayName );
	strcpy( this->symbol, symbol );
	strcpy( this->setState, setState );
	strcpy( this->unsetState, unsetState );
	this->type = type;
	this->index = index;
}

StateMod::~StateMod() {
}

StateMod *StateMod::getRandomGood() {
	return goodStateMods[ Util::dice( goodStateMods.size() ) ];
}

StateMod *StateMod::getRandomBad() {
	return badStateMods[ Util::dice( badStateMods.size() ) ];
}

bool StateMod::isStateModTransitionWanted( bool setting ) {
	bool effectFound = false;
	for ( int i = 0; i < static_cast<int>( goodStateMods.size() ); i++ ) {
		if ( goodStateMods[i] == this ) {
			effectFound = true;
			break;
		}
	}
	if ( effectFound && setting ) return true;

	effectFound = false;
	for ( int i = 0; i < static_cast<int>( badStateMods.size() ); i++ ) {
		if ( badStateMods[i] == this ) {
			effectFound = true;
			break;
		}
	}
	if ( ( effectFound || this == stateMods[StateMod::dead] ) && !setting ) return true;
	return false;
}

